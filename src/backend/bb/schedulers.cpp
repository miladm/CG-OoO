/*********************************************************************************
 * scheduler.cpp
 *********************************************************************************/

#include "schedulers.h"

bb_scheduler::bb_scheduler (port<bbInstruction*>& decode_to_scheduler_port, 
                            port<bbInstruction*>& execution_to_scheduler_port, 
                            port<bbInstruction*>& memory_to_scheduler_port, 
			                port<bbInstruction*>& scheduler_to_execution_port, 
                            List<bbWindow*>* bbWindows,
                            WIDTH num_bbWin,
                            CAMtable<dynBasicblock*>* bbROB,
	    	                WIDTH scheduler_width,
                            bb_memManager* LSQ_MGR,
                            bb_rfManager* RF_MGR,
                            sysClock* clk,
	    	                string stage_name) 
	: stage (scheduler_width, stage_name, clk),
      s_mem_g_fwd_cnt (g_stats.newScalarStat (stage_name, "mem_g_fwd_cnt", "Number of global memory forwarding events", 0, PRINT_ZERO)),
      s_alu_g_fwd_cnt (g_stats.newScalarStat (stage_name, "alu_g_fwd_cnt", "Number of global ALU forwarding events", 0, PRINT_ZERO)),
      s_mem_l_fwd_cnt (g_stats.newScalarStat (stage_name, "mem_l_fwd_cnt", "Number of local memory forwarding events", 0, PRINT_ZERO)),
      s_alu_l_fwd_cnt (g_stats.newScalarStat (stage_name, "alu_l_fwd_cnt", "Number of local ALU forwarding events", 0, PRINT_ZERO)),
      s_rf_struct_hazrd_cnt (g_stats.newScalarStat (stage_name, "rf_struct_hazrd_cnt", "Number of RF structural READ hazards", 0, PRINT_ZERO)),
      s_bbWin_usage_rat (g_stats.newRatioStat (clk, stage_name, "bbWin_usage_rat", "Number of busy bbWindows / cycle ", 0, PRINT_ZERO))
{
    _decode_to_scheduler_port = &decode_to_scheduler_port;
    _execution_to_scheduler_port = &execution_to_scheduler_port;
    _memory_to_scheduler_port = &memory_to_scheduler_port;
    _scheduler_to_execution_port  = &scheduler_to_execution_port;
    _bbROB = bbROB;
    _LSQ_MGR = LSQ_MGR;
    _RF_MGR = RF_MGR;
    _bbWin_on_fetch = NULL;
    _num_bbWin = num_bbWin;
    _bbWindows = bbWindows;
    for (WIDTH i = 0; i < _num_bbWin; i++) {
        bbWindow* bbWin = _bbWindows->Nth (i);
        _avail_bbWin.Append (bbWin);
    }
}

bb_scheduler::~bb_scheduler () { }

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
    if (ENABLE_FWD) manageCDB ();


    /*-- STAT --*/
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;
    s_bbWin_usage_rat += _busy_bbWin.size ();
    map<WIDTH, bbWindow*>::iterator bbWinEntry;
}

PIPE_ACTIVITY bb_scheduler::schedulerImpl () {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    updatebbWindows ();

    /*-- READ FROM INS WINDOW --*/
    for (WIDTH i = 0; i < _stage_width; i++) {
        /*-- CHECKS --*/
        LENGTH readyInsInBBWinIndx;
        if (!hasReadyInsInBBWins (readyInsInBBWinIndx)) break;
        if (_scheduler_to_execution_port->getBuffState () == FULL_BUFF) break;
        bbInstruction* ins = _busy_bbWin[readyInsInBBWinIndx]->_win.getNth_unsafe (0); //TODO fix this with hasReadInsInBBWin
        if (!_RF_MGR->hasFreeWire (READ, ins)) {s_rf_struct_hazrd_cnt++; break;}

        /*-- READ INS WIN --*/
        ins = _busy_bbWin[readyInsInBBWinIndx]->_win.popFront ();
        ins->setPipeStage (ISSUE);
        _scheduler_to_execution_port->pushBack (ins);
        dbg.print (DBG_SCHEDULER, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Issue ins", ins->getInsID (), _clk->now ());

        /*-- UPDATE RESOURCES --*/
        _busy_bbWin[readyInsInBBWinIndx]->_win.updateWireState (READ);
        _RF_MGR->updateWireState (READ, ins);

        /*-- STAT --*/
        s_ins_cnt++;
        pipe_stall = PIPE_BUSY;
        dbg.print (DBG_SCHEDULER, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Issue ", _clk->now ());
    }

    return pipe_stall;
}

void bb_scheduler::manageBusyBBWin (bbWindow* bbWin) {
#ifdef ASSERTION
    Assert (bbWin->_win.getTableState () == EMPTY_BUFF);
#endif
    if (_bbWin_on_fetch == NULL ||
        bbWin->_id != _bbWin_on_fetch->_id) {
        setBBWisAvail (bbWin->_id);
    }
}

bool bb_scheduler::hasReadyInsInBBWins (LENGTH &readyInsInBBWinIndx) {
    map<WIDTH, bbWindow*>::iterator bbWinEntry;
    for (bbWinEntry = _busy_bbWin.begin (); bbWinEntry != _busy_bbWin.end (); bbWinEntry++) {
        WIDTH bbWin_id = bbWinEntry->first;
        bbWindow* bbWin = bbWinEntry->second;
        if (bbWin->_win.getTableState () == EMPTY_BUFF) { manageBusyBBWin (bbWin); continue; }
        if (!bbWin->_win.hasFreeWire (READ)) continue;
        bbInstruction* ins = bbWin->_win.getNth_unsafe (0);
        readyInsInBBWinIndx = bbWin_id;
        if (ENABLE_FWD) forwardFromCDB (ins);
        if (!_RF_MGR->isReady (ins)) {continue; }
        else {
           dbg.print (DBG_SCHEDULER, "%s: %s %d (cyc: %d)\n", _stage_name.c_str (), 
                   "Found ready ins in BBWin", bbWin_id, _clk->now ()); 
           return true;
        }
    }
    dbg.print (DBG_SCHEDULER, "%s: %s (cyc: %d)\n", _stage_name.c_str (), 
            "Found NO ready ins OR BBWindows are empty or out of ports.", _clk->now ());
    return false;
}

/*-- WRITE INTO BB WINDOW --*/
void bb_scheduler::updatebbWindows () {
    for (WIDTH i = 0; i < _stage_width; i++) {
        /*-- CHECKS --*/
        if (_decode_to_scheduler_port->getBuffState () == EMPTY_BUFF) break;
        if (!_decode_to_scheduler_port->isReady ()) break;
        bbInstruction* ins = _decode_to_scheduler_port->getFront ();
        if (ins->getInsType () == MEM && ins->getMemType () == LOAD) {
            if (_LSQ_MGR->getTableState (LD_QU) == FULL_BUFF) break;
            if (!_LSQ_MGR->hasFreeWire (LD_QU, WRITE)) break;
        } else if (ins->getInsType () == MEM && ins->getMemType () == STORE) {
            if (_LSQ_MGR->getTableState (ST_QU) == FULL_BUFF) break;
            if (!_LSQ_MGR->hasFreeWire (ST_QU, WRITE)) break;
        }
        if (!_RF_MGR->canRename (ins)) break;

        /*-- CHECK & UPDATE --*/
        if (detectNewBB (ins)) {
            if (!hasAnAvailBBWin ()) {
                break;
            } else {
                if (_bbROB->getTableState () == FULL_BUFF) break;
                if (!_bbROB->hasFreeWire (WRITE)) break;
                updateBBROB (ins->getBB ());
                _bbWin_on_fetch = getAnAvailBBWin ();
                _bbROB->updateWireState (WRITE);
            }
        }
        Assert (_bbWin_on_fetch != NULL);
        if (!_bbWin_on_fetch->_win.hasFreeWire (WRITE)) break;
        Assert (_bbWin_on_fetch->_win.getTableState () != FULL_BUFF);

        /*-- WRITE INTO BB WIN --*/
        ins = _decode_to_scheduler_port->popFront ();
        _RF_MGR->renameRegs (ins);
        ins->setPipeStage (DISPATCH);
        if (ins->getInsType () == MEM) _LSQ_MGR->pushBack (ins);
        _bbWin_on_fetch->_win.pushBack (ins);
        ins->setBBWinID (_bbWin_on_fetch->_id);
        dbg.print (DBG_SCHEDULER, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                "Write bbWin ins", ins->getInsID (), _clk->now ());

        /*-- UPDATE WIRES --*/
        _bbWin_on_fetch->_win.updateWireState (WRITE);
        if (ins->getInsType () == MEM && ins->getMemType () == LOAD) {
            _LSQ_MGR->updateWireState (LD_QU, WRITE);
        } else if (ins->getInsType () == MEM && ins->getMemType () == STORE) {
            _LSQ_MGR->updateWireState (ST_QU, WRITE);
        }
    }
}

void bb_scheduler::updateBBROB (dynBasicblock* bb) {
    if (_bbROB->getTableState () == EMPTY_BUFF) {
        dbg.print (DBG_SCHEDULER, "%s: %s (cyc: %d)\n", _stage_name.c_str (), 
                "Adding new BB to BBROB", _clk->now ());
        _bbROB->pushBack (bb);
    } else {
        dynBasicblock* rob_tail_bb = _bbROB->getLast ();
        Assert (bb->getBBID () >= rob_tail_bb->getBBID ());
        if (bb->getBBID () > rob_tail_bb->getBBID ()) {
            dbg.print (DBG_SCHEDULER, "%s: %s (cyc: %d)\n", _stage_name.c_str (), 
                    "Adding new BB to BBROB", _clk->now ());
            _bbROB->pushBack (bb);
        }
    }
}

void bb_scheduler::setBBWisAvail (WIDTH bbWin_id) {
    Assert (_busy_bbWin.size () > 0);
    bbWindow* bbWin = _busy_bbWin[bbWin_id];
    Assert (bbWin->_win.getTableState () == EMPTY_BUFF);
    _avail_bbWin.Append (bbWin);
    _busy_bbWin.erase (bbWin_id);
}

bool bb_scheduler::hasAnAvailBBWin () {
    return (_avail_bbWin.NumElements () == 0) ? false : true;
}

bbWindow* bb_scheduler::getAnAvailBBWin () {
    Assert (_avail_bbWin.NumElements () > 0);
    bbWindow* bbWin = _avail_bbWin.Nth (0);
    _avail_bbWin.RemoveAt (0);
    _busy_bbWin.insert (pair<WIDTH, bbWindow*> (bbWin->_id, bbWin));
    return bbWin;
}

bool bb_scheduler::detectNewBB (bbInstruction* ins) {
    if (_bbROB->getTableState () == EMPTY_BUFF) return true;
    Assert (ins->getBB()->getBBID () >= _bbROB->getLast()->getBBID ());
    return (ins->getBB()->getBBID () > _bbROB->getLast()->getBBID ()) ? true : false;
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
void bb_scheduler::forwardFromCDB (bbInstruction* ins) {
    { /*-- FWD FROM EXE STAGE --*/
        if (_execution_to_scheduler_port->getBuffState () == EMPTY_BUFF) goto mem_fwd;
        List<PR>* rd_reg_list = ins->getPRrdList ();
        List<PR>* rd_lreg_list = ins->getLARrdList ();
        List<bbInstruction*> fwd_list;
        for (WIDTH i = 0; i < _stage_width; i++) { //TODO _stage_width replace with exe_num_EU
            if (!_execution_to_scheduler_port->isReadyNow ()) break;
            bbInstruction* fwd_ins = _execution_to_scheduler_port->popFront ();
            fwd_list.Append (fwd_ins);
        }
        for (WIDTH i = 0; i < fwd_list.NumElements (); i++) {
            bbInstruction* fwd_ins = fwd_list.Nth (i);
            List<PR>* wr_reg_list = fwd_ins->getPRwrList ();
            for (int j = rd_reg_list->NumElements () - 1; j >= 0; j--) {
                PR rd_reg = rd_reg_list->Nth (j);
                for (int k = wr_reg_list->NumElements () - 1; k >= 0; k--) {
                    PR wr_reg = wr_reg_list->Nth (k);
                    if (rd_reg == wr_reg) {
                        rd_reg_list->RemoveAt(j);
                        s_alu_g_fwd_cnt++;
                    }
                }
            }
            if (USE_LRF && fwd_ins->getBBWinID () == ins->getBBWinID ()) {
                List<PR>* wr_lreg_list = fwd_ins->getLARwrList ();
                for (int j = rd_lreg_list->NumElements () - 1; j >= 0; j--) {
                    PR rd_reg = rd_lreg_list->Nth (j);
                    for (int k = wr_lreg_list->NumElements () - 1; k >= 0; k--) {
                        PR wr_reg = wr_lreg_list->Nth (k);
                        if (rd_reg == wr_reg) {
                            rd_lreg_list->RemoveAt(j);
                            s_alu_l_fwd_cnt++;
                        }
                    }
                }
            }
        }
    }

    mem_fwd:
    { /*-- FWD FROM MEM STAGE --*/
        if (_memory_to_scheduler_port->getBuffState () == EMPTY_BUFF) return;
        List<PR>* rd_reg_list = ins->getPRrdList ();
        List<PR>* rd_lreg_list = ins->getLARrdList ();
        List<bbInstruction*> fwd_list;
        for (WIDTH i = 0; i < _stage_width; i++) { //TODO _stage_width replace with exe_num_EU
            if (!_memory_to_scheduler_port->hasReadyNow ()) break;
            bbInstruction* fwd_ins = _memory_to_scheduler_port->popNextReadyNow ();
            fwd_list.Append (fwd_ins);
        }
        for (WIDTH i = 0; i < fwd_list.NumElements (); i++) {
            bbInstruction* fwd_ins = fwd_list.Nth (i);
            List<PR>* wr_reg_list = fwd_ins->getPRwrList ();
            for (int j = rd_reg_list->NumElements () - 1; j >= 0; j--) {
                PR rd_reg = rd_reg_list->Nth (j);
                for (int k = wr_reg_list->NumElements () - 1; k >= 0; k--) {
                    PR wr_reg = wr_reg_list->Nth (k);
                    if (rd_reg == wr_reg) {
                        rd_reg_list->RemoveAt(j);
                        s_mem_g_fwd_cnt++;
                    }
                }
            }
            if (USE_LRF && fwd_ins->getBBWinID () == ins->getBBWinID ()) {
                List<PR>* wr_lreg_list = fwd_ins->getLARwrList ();
                for (int j = rd_lreg_list->NumElements () - 1; j >= 0; j--) {
                    PR rd_reg = rd_lreg_list->Nth (j);
                    for (int k = wr_lreg_list->NumElements () - 1; k >= 0; k--) {
                        PR wr_reg = wr_lreg_list->Nth (k);
                        if (rd_reg == wr_reg) {
                            rd_lreg_list->RemoveAt(j);
                            s_mem_l_fwd_cnt++;
                        }
                    }
                }
            }
        }
    }
}

/*-- MANAGE COMMON DATA BUS (CDB) 
 * TODO: this operation is also handled throuh delOldReady () function. Consolidate.
 * --*/
void bb_scheduler::manageCDB () {
    _execution_to_scheduler_port->delOldReady (); /*-- Only FWD what is on CDB now --*/
    _memory_to_scheduler_port->delOldReady (); /*-- Only FWD what is on CDB now --*/
}

void bb_scheduler::squash () {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Scheduler Ports Flush", _clk->now ());
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
    INS_ID squashSeqNum = g_var.getSquashSN ();
    _scheduler_to_execution_port->searchNflushPort (squashSeqNum);
    if (_bbWin_on_fetch != NULL) {
        flushBBWindow (_bbWin_on_fetch);
        if (_bbWin_on_fetch->_win.getTableState () == EMPTY_BUFF) { 
            setBBWisAvail (_bbWin_on_fetch->_id); 
            _bbWin_on_fetch = NULL;
        }
    }
    map<WIDTH, bbWindow*>::iterator bbWinEntry = _busy_bbWin.begin ();
    while (bbWinEntry != _busy_bbWin.end ()) {
        bbWindow* bbWin = bbWinEntry->second;
        flushBBWindow (bbWin);
        if (bbWin->_win.getTableState () == EMPTY_BUFF) {
            _avail_bbWin.Append (bbWin);
            _busy_bbWin.erase (bbWinEntry++);
        } else { ++bbWinEntry; }
    }
}

void bb_scheduler::flushBBWindow (bbWindow* bbWin) {
    INS_ID squashSeqNum = g_var.getSquashSN ();
    while (bbWin->_win.getTableState () != EMPTY_BUFF &&
           bbWin->_win.getBack()->getInsID () >= squashSeqNum) {
        bbWin->_win.popBack ();
    }
}

void bb_scheduler::regStat () {
    _decode_to_scheduler_port->regStat ();
    _execution_to_scheduler_port->regStat ();
    _memory_to_scheduler_port->regStat ();
    for (WIDTH j = 0; j < _num_bbWin; j++) {
        _bbWindows->Nth(j)->regStat ();
    }
}
