/*******************************************************************************
 * scheduler.cpp
 *******************************************************************************/

#include "schedulers.h"

o3_scheduler::o3_scheduler (port<dynInstruction*>& decode_to_scheduler_port, 
                      port<dynInstruction*>& execution_to_scheduler_port, 
                      port<dynInstruction*>& memory_to_scheduler_port, 
			          port<dynInstruction*>& scheduler_to_execution_port, 
                      CAMtable<dynInstruction*>* iROB,
	    	          WIDTH issue_width,
	    	          string stage_name) 
	: stage (issue_width, stage_name)
{
    _decode_to_scheduler_port = &decode_to_scheduler_port;
    _execution_to_scheduler_port = &execution_to_scheduler_port;
    _memory_to_scheduler_port = &memory_to_scheduler_port;
    _scheduler_to_execution_port  = &scheduler_to_execution_port;
    _iROB = iROB;
    _num_res_stns = 4;
    for (WIDTH i = 0; i < _num_res_stns; i++) {
        ostringstream rs_num;
        rs_num << i;
        CAMtable<dynInstruction*>* resStn = new CAMtable<dynInstruction*>(8, 8, 8, "ResStn_"+rs_num.str ());
        _ResStns.Append(resStn);
    }
}

o3_scheduler::~o3_scheduler () {
    for (WIDTH i = 0; i < _num_res_stns; i++) {
        delete _ResStns.Nth(i);
    }
}

void o3_scheduler::doSCHEDULER (sysClock& clk) {
    dbg.print (DBG_SCHEDULER, "** %s: (cyc: %d)\n", _stage_name.c_str (), clk.now ());
    /* STAT */
    regStat (clk);
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /* SQUASH HANDLING */
    if (g_var.g_pipe_state == PIPE_FLUSH) { squash (clk); }
    if (!(g_var.g_pipe_state == PIPE_WAIT_FLUSH || g_var.g_pipe_state == PIPE_FLUSH)) {
        pipe_stall = schedulerImpl (clk);
    }

    /* STAT */
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;
}

PIPE_ACTIVITY o3_scheduler::schedulerImpl (sysClock& clk) {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    updateResStns (clk);

    /* READ FROM INS WINDOW */
    for (WIDTH j = 0; j < _num_res_stns; j++) {
        for (WIDTH i = 0; i < _stage_width; i++) {
            /* CHECKS */
            if (_scheduler_to_execution_port->getBuffState (clk.now ()) == FULL_BUFF) break;
            if (_ResStns.Nth(j)->getTableState () == EMPTY_BUFF) continue;
            if (!_ResStns.Nth(j)->hasFreeRdPort (clk.now ())) continue;
            LENGTH readyInsIndx;
            if (!hasReadyInsInResStn (j, readyInsIndx)) break;
            dynInstruction* ins = _ResStns.Nth(j)->getNth_unsafe (readyInsIndx);
            if (!g_GRF_MGR.hasFreeRdPort (clk.now (), ins->getNumRdPR ())) break;
            //forwardFromCDB (ins, clk); TODO - made execution worse - WHY?!

            /* READ INS WIN */
            ins = _ResStns.Nth(j)->pullNextReady (readyInsIndx);
            ins->setPipeStage (ISSUE);
            _scheduler_to_execution_port->pushBack (ins, clk.now ());
            dbg.print (DBG_SCHEDULER, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Issue ins", ins->getInsID (), clk.now ());

            /* STAT */
            s_ins_cnt++;
            pipe_stall = PIPE_BUSY;
        }
    }

    manageCDB (clk);

    return pipe_stall;
}

bool o3_scheduler::hasReadyInsInResStn (WIDTH resStnId, LENGTH &readyInsIndx) {
    for (WIDTH i = 0; i < _ResStns.Nth(resStnId)->getTableSize(); i++) {
        dynInstruction* ins = _ResStns.Nth(resStnId)->getNth_unsafe (i);
        readyInsIndx = i;
        if (!g_GRF_MGR.isReady (ins)) continue;
        else return true;
    }
    return false;
}

/* WRITE INTO INS WINDOW */
void o3_scheduler::updateResStns (sysClock& clk) {
    for (WIDTH j = 0; j < _num_res_stns; j++) {
        for (WIDTH i = 0; i < _stage_width; i++) {
            /* CHECKS */
            if (_iROB->getTableState () == FULL_BUFF) break;
            if (!_iROB->hasFreeWrPort (clk.now ())) break;
            if (_ResStns.Nth(j)->getTableState () == FULL_BUFF) continue;
            if (!_ResStns.Nth(j)->hasFreeWrPort (clk.now ())) continue;
            if (_decode_to_scheduler_port->getBuffState (clk.now ()) == EMPTY_BUFF) break;
            if (!_decode_to_scheduler_port->isReady (clk.now ())) break;
            dynInstruction* ins = _decode_to_scheduler_port->getFront ();
            if (!g_GRF_MGR.canRename (ins)) break;
            if (ins->getInsType () == MEM && ins->getMemType () == LOAD) {
                if (g_LSQ_MGR.getTableState (LD_QU) == FULL_BUFF) break;
                if (!g_LSQ_MGR.hasFreeWrPort (LD_QU, clk.now ())) break;
            } else if (ins->getInsType () == MEM && ins->getMemType () == STORE) {
                if (g_LSQ_MGR.getTableState (ST_QU) == FULL_BUFF) break;
                if (!g_LSQ_MGR.hasFreeWrPort (ST_QU, clk.now ())) break;
            }

            /* WRITE INTO RES STN */
            dbg.print (DBG_PORT, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "ADD INS", clk.now ());
            ins = _decode_to_scheduler_port->popFront (clk.now ());
            g_GRF_MGR.renameRegs (ins);
            ins->setPipeStage (DISPATCH);
            if (ins->getInsType () == MEM) g_LSQ_MGR.pushBack (ins);
            _ResStns.Nth(j)->pushBack (ins);
            _iROB->pushBack (ins);
            dbg.print (DBG_SCHEDULER, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Write iWin ins", ins->getInsID (), clk.now ());
        }
    }
}

/* FORWARD OPERANDS 
 * NOTE: this function forwards even if the ins would not execute at the same
 * cycle because of waiting for another operand that is still in flight (see
 * the calling function(s)). This is okay because at a later cycle the ins will
 * have been given the cycle time needed to access the RF anyway. This means
 * the number of accesses to RF is scewed by this function even though the time
 * model is correct (i.e. maybe more RF accesses happen than what we see in
 * this design).
 */
void o3_scheduler::forwardFromCDB (dynInstruction* ins, sysClock& clk) {
    { /* FWD FROM EXE STAGE */
        if (_execution_to_scheduler_port->getBuffState (clk.now ()) == EMPTY_BUFF) return;
        List<PR>* rd_reg_list = ins->getPRrdList ();
        List<dynInstruction*> fwd_list;
        for (WIDTH i = 0; i < _stage_width; i++) { //TODO _stage_width replace with exe_num_EU
            if (!_execution_to_scheduler_port->isReadyNow (clk.now ())) break;
            dynInstruction* fwd_ins = _execution_to_scheduler_port->popFront (clk.now ());
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
        _execution_to_scheduler_port->delOldReady (clk.now ()); /* Only FWD what is on CDB now */
    }

    { /* FWD FROM MEM STAGE */
        if (_memory_to_scheduler_port->getBuffState (clk.now ()) == EMPTY_BUFF) return;
        List<PR>* rd_reg_list = ins->getPRrdList ();
        List<dynInstruction*> fwd_list;
        for (WIDTH i = 0; i < _stage_width; i++) { //TODO _stage_width replace with exe_num_EU
            if (!_memory_to_scheduler_port->hasReadyNow (clk.now ())) break;
            dynInstruction* fwd_ins = _memory_to_scheduler_port->popNextReadyNow (clk.now ());
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
        _memory_to_scheduler_port->delOldReady (clk.now ()); /* Only FWD what is on CDB now */
    }
}

/* MANAGE COMMON DATA BUS (CDB) */
void o3_scheduler::manageCDB (sysClock& clk) {
    if (_execution_to_scheduler_port->getBuffState (clk.now ()) == EMPTY_BUFF) return;
    for (WIDTH i = 0; i < _stage_width; i++) { //TODO _stage_width replace with exe_num_EU
        if (_execution_to_scheduler_port->isReady (clk.now ()))
            _execution_to_scheduler_port->popFront (clk.now ());
    }
}

void o3_scheduler::squash (sysClock& clk) {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Scheduler Ports Flush", clk.now ());
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
    INS_ID squashSeqNum = g_var.getSquashSN ();
    _scheduler_to_execution_port->flushPort (squashSeqNum, clk.now ());
    for (WIDTH j = 0; j < _num_res_stns; j++) {
        for (int i = (int)_ResStns.Nth(j)->getTableSize() - 1; i >= 0; i--) {
            if (_ResStns.Nth(j)->getTableSize() == 0) break;
            dynInstruction* ins = _ResStns.Nth(j)->getNth_unsafe (i);
            if (ins->getInsID () >= squashSeqNum) {
                _ResStns.Nth(j)->removeNth_unsafe (i);
            }
        }
    }
}

void o3_scheduler::regStat (sysClock& clk) {
    _decode_to_scheduler_port->regStat (clk.now ());
    _execution_to_scheduler_port->regStat (clk.now ());
    _memory_to_scheduler_port->regStat (clk.now ());
    for (WIDTH j = 0; j < _num_res_stns; j++) {
        _ResStns.Nth(j)->regStat ();
    }
}
