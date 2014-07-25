/*********************************************************************************
 * scheduler.cpp
 *********************************************************************************/

#include "schedulers.h"

bb_scheduler::bb_scheduler (port<dynInstruction*>& decode_to_scheduler_port, 
                      port<dynInstruction*>& execution_to_scheduler_port, 
                      port<dynInstruction*>& memory_to_scheduler_port, 
			          port<dynInstruction*>& scheduler_to_execution_port, 
                      CAMtable<dynBasicblock*>* bbROB,
	    	          WIDTH issue_width,
                      bb_memManager* LSQ_MGR,
                      bb_rfManager* RF_MGR,
                      sysClock* clk,
	    	          string stage_name) 
	: stage (issue_width, stage_name, clk)
{
    _decode_to_scheduler_port = &decode_to_scheduler_port;
    _execution_to_scheduler_port = &execution_to_scheduler_port;
    _memory_to_scheduler_port = &memory_to_scheduler_port;
    _scheduler_to_execution_port  = &scheduler_to_execution_port;
    _bbROB = bbROB;
    _num_res_stns = 4;
    _LSQ_MGR = LSQ_MGR;
    _RF_MGR = RF_MGR;
    for (WIDTH i = 0; i < _num_res_stns; i++) {
        ostringstream rs_num;
        rs_num << i;
        FIFOtable<dynInstruction*>* bbWindow = new FIFOtable<dynInstruction*>(8, 8, 8, _clk, "bbWindow_"+rs_num.str ());
        _bbWindows.Append(bbWindow);
    }
}

bb_scheduler::~bb_scheduler () {
    for (WIDTH i = 0; i < _num_res_stns; i++) {
        delete _bbWindows.Nth(i);
    }
}

void bb_scheduler::doSCHEDULER () {
    /*-- STAT + DEBUG --*/
    dbg.print (DBG_SCHEDULER, "** %s: (cyc: %d)\n", _stage_name.c_str (), _clk->now ());
    regStat ();
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /*-- SQUASH HANDLING --*/
    if (g_var.g_pipe_state == PIPE_FLUSH) { squash (); }
    if (!(g_var.g_pipe_state == PIPE_WAIT_FLUSH || g_var.g_pipe_state == PIPE_FLUSH)) {
        pipe_stall = schedulerImpl ();
    }

    /*-- STAT --*/
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;
}

PIPE_ACTIVITY bb_scheduler::schedulerImpl () {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    updatebbWindows ();

    /*-- READ FROM INS WINDOW --*/
    for (WIDTH j = 0; j < _num_res_stns; j++) {
        for (WIDTH i = 0; i < _stage_width; i++) {
            /*-- CHECKS --*/
            if (_scheduler_to_execution_port->getBuffState () == FULL_BUFF) break;
            if (_bbWindows.Nth(j)->getTableState () == EMPTY_BUFF) continue;
            if (!_bbWindows.Nth(j)->hasFreeWire (READ)) continue;
            LENGTH readyInsIndx;
            if (!hasReadyInsInResStn (j, readyInsIndx)) break;
            dynInstruction* ins = _bbWindows.Nth(j)->getNth_unsafe (readyInsIndx);
            WIDTH num_ar = ins->getTotNumRdAR ();
            if (!_RF_MGR->hasFreeWire (READ, num_ar)) break; //TODO this is conservative when using forwarding - fix (for ino too)

            /*-- READ INS WIN --*/
            ins = _bbWindows.Nth(j)->pullNextReady (readyInsIndx);
            ins->setPipeStage (ISSUE);
            _scheduler_to_execution_port->pushBack (ins);
            dbg.print (DBG_SCHEDULER, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Issue ins", ins->getInsID (), _clk->now ());

            /*-- UPDATE WIRES --*/
            _bbWindows.Nth(j)->updateWireState (READ);
            _RF_MGR->updateWireState (READ, num_ar);

            /*-- STAT --*/
            s_ins_cnt++;
            pipe_stall = PIPE_BUSY;
        }
    }

    manageCDB ();

    return pipe_stall;
}

bool bb_scheduler::hasReadyInsInResStn (WIDTH resStnId, LENGTH &readyInsIndx) {
    for (WIDTH i = 0; i < _bbWindows.Nth(resStnId)->getTableSize(); i++) {
        dynInstruction* ins = _bbWindows.Nth(resStnId)->getNth_unsafe (i);
        readyInsIndx = i;
        forwardFromCDB (ins);
        if (!_RF_MGR->isReady (ins)) continue;
        else return true;
    }
    return false;
}

/*-- WRITE INTO INS WINDOW --*/
void bb_scheduler::updatebbWindows () {
    for (WIDTH j = 0; j < _num_res_stns; j++) {
        for (WIDTH i = 0; i < _stage_width; i++) {
            /*-- CHECKS --*/
            if (_bbROB->getTableState () == FULL_BUFF) break;
            if (!_bbROB->hasFreeWire (WRITE)) break;
            if (_bbWindows.Nth(j)->getTableState () == FULL_BUFF) continue;
            if (!_bbWindows.Nth(j)->hasFreeWire (WRITE)) continue;
            if (_decode_to_scheduler_port->getBuffState () == EMPTY_BUFF) break;
            if (!_decode_to_scheduler_port->isReady ()) break;
            dynInstruction* ins = _decode_to_scheduler_port->getFront ();
            if (!_RF_MGR->canRename (ins)) break;
            if (ins->getInsType () == MEM && ins->getMemType () == LOAD) {
                if (_LSQ_MGR->getTableState (LD_QU) == FULL_BUFF) break;
                if (!_LSQ_MGR->hasFreeWire (LD_QU, WRITE)) break;
            } else if (ins->getInsType () == MEM && ins->getMemType () == STORE) {
                if (_LSQ_MGR->getTableState (ST_QU) == FULL_BUFF) break;
                if (!_LSQ_MGR->hasFreeWire (ST_QU, WRITE)) break;
            }

            /*-- WRITE INTO RES STN --*/
            dbg.print (DBG_PORT, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "ADD INS", _clk->now ());
            ins = _decode_to_scheduler_port->popFront ();
            _RF_MGR->renameRegs (ins);
            ins->setPipeStage (DISPATCH);
            if (ins->getInsType () == MEM) _LSQ_MGR->pushBack (ins);
            _bbWindows.Nth(j)->pushBack (ins);
            updateBBROB (ins);
            dbg.print (DBG_SCHEDULER, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Write iWin ins", ins->getInsID (), _clk->now ());

            /*-- UPDATE WIRES --*/
            _bbROB->updateWireState (WRITE);
            _bbWindows.Nth(j)->updateWireState (WRITE);
            if (ins->getInsType () == MEM && ins->getMemType () == LOAD) {
                _LSQ_MGR->updateWireState (LD_QU, WRITE);
            } else if (ins->getInsType () == MEM && ins->getMemType () == STORE) {
                _LSQ_MGR->updateWireState (ST_QU, WRITE);
            }
        }
    }
}

void bb_scheduler::updateBBROB (dynInstruction* ins) {
    dynBasicblock* bb = ins->getBB ();
    if (_bbROB->getTableState () == EMPTY_BUFF) {
        _bbROB->pushBack (bb);
    } else {
        dynBasicblock* rob_tail_bb = _bbROB->getLast ();
        Assert (bb->getBBID () >= rob_tail_bb->getBBID ());
        if (bb->getBBID () > rob_tail_bb->getBBID ()) {
            _bbROB->pushBack (bb);
        }
    }
}

/*-- FORWARD OPERANDS 
 * NOTE: this function forwards even if the ins would not execute at the same
 * cycle because of waiting for another operand that is still in flight (see
 * the calling function(s)). This is okay because at a later cycle the ins will
 * have been given the cycle time needed to access the RF anyway. This means
 * the number of accesses to RF is scewed by this function even though the time
 * model is correct (i.e. maybe more RF accesses happen than what we see in
 * this design).
 --*/
void bb_scheduler::forwardFromCDB (dynInstruction* ins) {
    { /*-- FWD FROM EXE STAGE --*/
        if (_execution_to_scheduler_port->getBuffState () == EMPTY_BUFF) return;
        List<PR>* rd_reg_list = ins->getPRrdList ();
        List<dynInstruction*> fwd_list;
        for (WIDTH i = 0; i < _stage_width; i++) { //TODO _stage_width replace with exe_num_EU
            if (!_execution_to_scheduler_port->isReadyNow ()) break;
            dynInstruction* fwd_ins = _execution_to_scheduler_port->popFront ();
            fwd_list.Append (fwd_ins);
        }
        for (WIDTH i = 0; i < fwd_list.NumElements (); i++) {
            dynInstruction* fwd_ins = fwd_list.Nth (i);
            List<PR>* wr_reg_list = fwd_ins->getPRwrList ();
            for (int j = rd_reg_list->NumElements () - 1; j >= 0; j--) {
                PR rd_reg = rd_reg_list->Nth (j);
                for (int k = wr_reg_list->NumElements () - 1; k >= 0; k--) {
                    PR wr_reg = wr_reg_list->Nth (k);
                    if (rd_reg == wr_reg) {
                        rd_reg_list->RemoveAt(j);
                    }
                }
            }
        }
        _execution_to_scheduler_port->delOldReady (); /*-- Only FWD what is on CDB now --*/
    }

    { /*-- FWD FROM MEM STAGE --*/
        if (_memory_to_scheduler_port->getBuffState () == EMPTY_BUFF) return;
        List<PR>* rd_reg_list = ins->getPRrdList ();
        List<dynInstruction*> fwd_list;
        for (WIDTH i = 0; i < _stage_width; i++) { //TODO _stage_width replace with exe_num_EU
            if (!_memory_to_scheduler_port->hasReadyNow ()) break;
            dynInstruction* fwd_ins = _memory_to_scheduler_port->popNextReadyNow ();
            fwd_list.Append (fwd_ins);
        }
        for (WIDTH i = 0; i < fwd_list.NumElements (); i++) {
            dynInstruction* fwd_ins = fwd_list.Nth (i);
            List<PR>* wr_reg_list = fwd_ins->getPRwrList ();
            for (int j = rd_reg_list->NumElements () - 1; j >= 0; j--) {
                PR rd_reg = rd_reg_list->Nth (j);
                for (int k = wr_reg_list->NumElements () - 1; k >= 0; k--) {
                    PR wr_reg = wr_reg_list->Nth (k);
                    if (rd_reg == wr_reg) {
                        rd_reg_list->RemoveAt(j);
                        cout <<  "mem FWD" << endl;
                    }
                }
            }
        }
        _memory_to_scheduler_port->delOldReady (); /*-- Only FWD what is on CDB now --*/
    }
}

/*-- MANAGE COMMON DATA BUS (CDB) --*/
void bb_scheduler::manageCDB () {
    if (_execution_to_scheduler_port->getBuffState () == EMPTY_BUFF) return;
    for (WIDTH i = 0; i < _stage_width; i++) { //TODO _stage_width replace with exe_num_EU
        if (_execution_to_scheduler_port->isReady ())
            _execution_to_scheduler_port->popFront ();
    }
}

void bb_scheduler::squash () {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Scheduler Ports Flush", _clk->now ());
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
    INS_ID squashSeqNum = g_var.getSquashSN ();
    _scheduler_to_execution_port->flushPort (squashSeqNum);
    for (WIDTH j = 0; j < _num_res_stns; j++) {
        for (int i = (int)_bbWindows.Nth(j)->getTableSize() - 1; i >= 0; i--) {
            if (_bbWindows.Nth(j)->getTableSize() == 0) break;
            dynInstruction* ins = _bbWindows.Nth(j)->getNth_unsafe (i);
            if (ins->getInsID () >= squashSeqNum) {
                _bbWindows.Nth(j)->removeNth_unsafe (i);
            }
        }
    }
}

void bb_scheduler::regStat () {
    _decode_to_scheduler_port->regStat ();
    _execution_to_scheduler_port->regStat ();
    _memory_to_scheduler_port->regStat ();
    for (WIDTH j = 0; j < _num_res_stns; j++) {
        _bbWindows.Nth(j)->regStat ();
    }
}
