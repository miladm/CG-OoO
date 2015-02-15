/*********************************************************************************
 * scheduler.cpp
 *********************************************************************************/

#include "schedulers.h"

bb_scheduler::bb_scheduler (port<bbInstruction*>& decode_to_scheduler_port, 
                            port<bbInstruction*>& execution_to_scheduler_port, 
                            port<bbInstruction*>& memory_to_scheduler_port, 
			                List<port<bbInstruction*>*>* scheduler_to_execution_port, 
                            List<bbWindow*>* bbWindows,
                            WIDTH num_bbWin,
                            CAMtable<dynBasicblock*>* bbROB,
	    	                WIDTH scheduler_width,
                            bb_memManager* LSQ_MGR,
                            bb_rfManager* RF_MGR,
                            sysClock* clk,
	    	                string stage_name) 
	: stage (scheduler_width, stage_name, clk),
      s_mem_g_fwd_cnt (g_stats.newScalarStat (stage_name, "mem_g_fwd_cnt", "Number of global memory forwarding events", 0, NO_PRINT_ZERO)),
      s_alu_g_fwd_cnt (g_stats.newScalarStat (stage_name, "alu_g_fwd_cnt", "Number of global ALU forwarding events", 0, NO_PRINT_ZERO)),
      s_mem_l_fwd_cnt (g_stats.newScalarStat (stage_name, "mem_l_fwd_cnt", "Number of local memory forwarding events", 0, NO_PRINT_ZERO)),
      s_alu_l_fwd_cnt (g_stats.newScalarStat (stage_name, "alu_l_fwd_cnt", "Number of local ALU forwarding events", 0, NO_PRINT_ZERO)),
      s_no_ld_bypass (g_stats.newScalarStat (stage_name, "no_ld_bypass", "Number of times a LD not issued because of a non-issued ST in same BB", 0, NO_PRINT_ZERO)),
      s_bbWin_inflight_rat (g_stats.newRatioStat (clk->getStatObj (), stage_name, "bbWin_inflight_rat", "Number of in-flight bbWindows / cycle ", 0, PRINT_ZERO))
{
    _decode_to_scheduler_port = &decode_to_scheduler_port;
    _execution_to_scheduler_port = &execution_to_scheduler_port;
    _memory_to_scheduler_port = &memory_to_scheduler_port;
    _scheduler_to_execution_port = scheduler_to_execution_port;
    _bbROB = bbROB;
    _LSQ_MGR = LSQ_MGR;
    _RF_MGR = RF_MGR;
    _bbWin_on_fetch = NULL;
    _num_bbWin = num_bbWin;
    _bbWindows = bbWindows;
    WIDTH rd_wire_cnt;
    int temp;
    g_cfg->_root["cpu"]["backend"]["table"]["bbWindow"]["rd_wire_cnt"] >> rd_wire_cnt;
    g_cfg->_root["cpu"]["backend"]["table"]["bbWindow"]["runahead_issue_cnt"] >> _runahead_issue_cnt;
    g_cfg->_root["cpu"]["backend"]["table"]["bbWindow"]["runahead_issue_en"] >> temp;
    _runahead_issue_en = (bool)temp;
    if (!_runahead_issue_en) _runahead_issue_cnt = 1;
#ifdef ASSERTION
    Assert (_runahead_issue_cnt > 0 && _runahead_issue_cnt <= rd_wire_cnt && 
            "Number of read ports does not support the runahead setup");
#endif

    for (WIDTH i = 0; i < _num_bbWin; i++) {
        bbWindow* bbWin = _bbWindows->Nth (i);
        _avail_bbWin.Append (bbWin);
    }

    /*-- SETUP THE MATH TO FIND THE NUMBER OF BLOCK WINDOES SHARING THE SAME PORT --*/
    WIDTH num_ports = _scheduler_to_execution_port->NumElements ();
    _blk_cluster_siz = _num_bbWin / num_ports;
#ifdef ASSERTION
    Assert (_num_bbWin >= num_ports && 
            num_ports > 0 &&
            _num_bbWin % num_ports == 0);
#endif
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
    if (g_cfg->isEnFwd ()) manageCDB ();


    /*-- STAT --*/
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;
    map<WIDTH, bbWindow*>::iterator bbWinEntry;
}

WIDTH bb_scheduler::getIssuePortIndx (WIDTH BBWinIndx) {
    return BBWinIndx / _blk_cluster_siz;
}

PIPE_ACTIVITY bb_scheduler::schedulerImpl () {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    updatebbWindows ();

    /*-- READ FROM INS WINDOW --*/
    for (WIDTH i = 0; i < _stage_width; i++) {
        /*-- CHECKS --*/
        LENGTH ready_bbWin_indx;
        if (!hasReadyInsInBBWins (ready_bbWin_indx)) break;
        if (_scheduler_to_execution_port->Nth(getIssuePortIndx(ready_bbWin_indx))->getBuffState () == FULL_BUFF) break;
        LENGTH ready_ins_indx = _busy_bbWin[ready_bbWin_indx]->getIssueIndx ();
        bbInstruction* ins = _busy_bbWin[ready_bbWin_indx]->_win.getNth_unsafe (ready_ins_indx); //TODO fix this with hasReadInsInBBWin
        if (!_RF_MGR->hasFreeWire (READ, ins)) {break;}

        /*-- READ INS WIN --*/
        _busy_bbWin[ready_bbWin_indx]->issueInc ();
        _RF_MGR->reserveRF (ins);
        ins = _busy_bbWin[ready_bbWin_indx]->_win.pullNth (ready_ins_indx);
        _scheduler_to_execution_port->Nth(getIssuePortIndx(ready_bbWin_indx))->pushBack (ins);
        ins->setPipeStage (ISSUE);
        dbg.print (DBG_SCHEDULER, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Issue ins", ins->getInsID (), _clk->now ());
        //---------------------------------------
        //TODO put this code where it belongs 
        WIDTH num_glb_update = ins->getNumWrPR () * _num_bbWin; //TODO mus replace with a loop thru all BBWin's - wake up logic
        WIDTH num_loc_update = ins->getNumWrLAR ();
        _busy_bbWin[ready_bbWin_indx]->_win.ramAccess (num_glb_update);
        _busy_bbWin[ready_bbWin_indx]->_win.ramAccess (num_loc_update);
        //---------------------------------------

        /*-- UPDATE RESOURCES --*/
        _busy_bbWin[ready_bbWin_indx]->_win.updateWireState (READ);
        _RF_MGR->updateWireState (READ, ins);
        _busy_bbWin[ready_bbWin_indx]->_win.ramAccess (); //assume this step is not free for now - TODO

        /*-- STAT --*/
        s_ipc++;
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

bool bb_scheduler::hasReadyInsInBBWins (LENGTH &ready_bbWin_indx) {
    map<WIDTH, bbWindow*>::iterator it;
    map<BB_ID,WIDTH> sorted_busy_bbWin;

    /*-- SORT BASED ON BASICBLOCK AGE --*/
    for (it= _busy_bbWin.begin (); it != _busy_bbWin.end (); it++) {
        WIDTH bbWin_id = it->first;
        bbWindow* bbWin = it->second;
        if (bbWin->_win.getTableState () == EMPTY_BUFF) { manageBusyBBWin (bbWin); continue; }
        BB_ID bb_id = bbWin->_win.getNth_unsafe(0)->getBB()->getBBID ();
        sorted_busy_bbWin.insert (pair<BB_ID, WIDTH> (bb_id, bbWin_id));
    }

    /* FIND READY INS */
    map<BB_ID, WIDTH>::iterator bbWinEntry;
    for (bbWinEntry = sorted_busy_bbWin.begin (); bbWinEntry != sorted_busy_bbWin.end (); bbWinEntry++) {
        WIDTH bbWin_id = bbWinEntry->second;
        bbWindow* bbWin = _busy_bbWin[bbWin_id];
//        if (bbWin->_win.getTableState () == EMPTY_BUFF) { manageBusyBBWin (bbWin); continue; }
        if (!bbWin->_win.hasFreeWire (READ)) {continue;}
        if ((bbWin->getNumIssued () + bbWin->getIssueIndx ()) >= _runahead_issue_cnt) {continue;}
        for (int i = bbWin->getIssueIndx (); i < _runahead_issue_cnt; i++) {
            if (i >= bbWin->_win.getTableSize ()) break;
            bbInstruction* ins = bbWin->_win.getNth_unsafe (i);
            ready_bbWin_indx = bbWin_id;
            //        bbWin->_win.ramAccess (); //assume this step is free for now - TODO
            if (ins->getInsType () == MEM && ins->getMemType () == LOAD && bbWin->isStoreBypassed ()) {/* MEM RAW AVOIDANCE IN BB */
                s_no_ld_bypass++;
                if (runaheadPermit (ins)) {
                    bbWin->issueIndxInc ();
                    bbWin->recordStallRdReg (ins);
                }
                else break;
            } else if (!isReady (ins) || !_RF_MGR->canReserveRF (ins) || bbWin->conflictStallRdReg (ins)) {
                if (ins->getInsType () == MEM && ins->getMemType () == STORE) bbWin->setStoreBypassed ();
                if (runaheadPermit (ins)) {
                    bbWin->issueIndxInc ();
                    bbWin->recordStallRdReg (ins);
                }
                else break;
            } else {
                if (ins->getInsType () == MEM && ins->getMemType () == STORE) bbWin->setStoreBypassed ();
                dbg.print (DBG_SCHEDULER, "%s: %s %d (cyc: %d)\n", _stage_name.c_str (), 
                        "Found ready ins in BBWin", bbWin_id, _clk->now ()); 
                return true;
            }
        }
    }

    dbg.print (DBG_SCHEDULER, "%s: %s (cyc: %d)\n", _stage_name.c_str (), 
            "Found NO ready ins OR BBWindows are empty or out of ports.", _clk->now ());
    return false;
}

bool bb_scheduler::isReady (bbInstruction* ins) {
    if (g_cfg->isEnFwd ()) {
        if (_RF_MGR->isReady (ins)) return true;
        bool done_any_fwd = forwardFromCDB (ins);
        bool is_ready = _RF_MGR->checkReadyAgain (ins);
#ifdef ASSERTION
        Assert ((is_ready && done_any_fwd) || (!is_ready && !done_any_fwd));
#endif
        return is_ready;
    } else {
        if (_RF_MGR->isReady (ins)) return true;
        else return false;
    }
}

bool bb_scheduler::runaheadPermit (bbInstruction* ins) {
    return (_runahead_issue_en && ins->getBB()->runaheadPermit ());
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
#ifdef ASSERTION
        Assert (_bbWin_on_fetch != NULL);
#endif
        if (!_bbWin_on_fetch->_win.hasFreeWire (WRITE)) break;
        if (_bbWin_on_fetch->_win.getTableState () == FULL_BUFF) break;
//        Assert (_bbWin_on_fetch->_win.getTableState () != FULL_BUFF); TODO put back when have fixed BB size & remove check above

        if (!_RF_MGR->canRename (ins, _bbWin_on_fetch->_id)) break;

        /*-- WRITE INTO BB WIN --*/
        _bbWin_on_fetch->_win.pushBack (ins);
        ins->setBBWinID (_bbWin_on_fetch->_id);
        ins = _decode_to_scheduler_port->popFront ();
        _RF_MGR->renameRegs (ins);
        ins->setPipeStage (DISPATCH);
        if (ins->getInsType () == MEM) _LSQ_MGR->pushBack (ins);
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
#ifdef ASSERTION
        Assert (bb->getBBID () >= rob_tail_bb->getBBID ());
#endif
        if (bb->getBBID () > rob_tail_bb->getBBID ()) {
            dbg.print (DBG_SCHEDULER, "%s: %s (cyc: %d)\n", _stage_name.c_str (), 
                    "Adding new BB to BBROB", _clk->now ());
            _bbROB->pushBack (bb);
        }
    }
}

void bb_scheduler::setBBWisAvail (WIDTH bbWin_id) {
#ifdef ASSERTION
    Assert (_busy_bbWin.size () > 0);
#endif
    bbWindow* bbWin = _busy_bbWin[bbWin_id];
#ifdef ASSERTION
    Assert (bbWin->_win.getTableState () == EMPTY_BUFF);
#endif
    _avail_bbWin.Append (bbWin);
    _busy_bbWin.erase (bbWin_id);
}

bool bb_scheduler::hasAnAvailBBWin () {
    return (_avail_bbWin.NumElements () == 0) ? false : true;
}

bbWindow* bb_scheduler::getAnAvailBBWin () {
#ifdef ASSERTION
    Assert (_avail_bbWin.NumElements () > 0);
#endif
    bbWindow* bbWin = _avail_bbWin.Nth (0);
    _avail_bbWin.RemoveAt (0);
    _busy_bbWin.insert (pair<WIDTH, bbWindow*> (bbWin->_id, bbWin));
    return bbWin;
}

bool bb_scheduler::detectNewBB (bbInstruction* ins) {
    if (_bbROB->getTableState () == EMPTY_BUFF) return true;
#ifdef ASSERTION
    Assert (ins->getBB()->getBBID () >= _bbROB->getLast()->getBBID ());
#endif
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
bool bb_scheduler::forwardFromCDB (bbInstruction* ins) {
    List<PR>* rd_reg_list = ins->getPRrdList ();
    List<PR>* rd_lreg_list = ins->getLARrdList ();
    int num_global_match = rd_reg_list->NumElements ();
    int num_local_match = rd_lreg_list->NumElements ();
    bool en_global_fwd = false;
    bool en_local_fwd = false;
    bool done_any_fwd = false;
    List<bbInstruction*> alu_fwd_list, mem_fwd_list;

    { /*-- FWD FROM EXE STAGE --*/
        if (_execution_to_scheduler_port->getBuffState () == EMPTY_BUFF) goto mem_fwd;
        for (WIDTH i = 0; i < _stage_width; i++) { //TODO _stage_width replace with exe_num_EU
            if (!_execution_to_scheduler_port->isReadyNow ()) break;
            bbInstruction* fwd_ins = _execution_to_scheduler_port->popFront ();
            alu_fwd_list.Append (fwd_ins);
        }
        for (WIDTH i = 0; i < alu_fwd_list.NumElements (); i++) {
            /* CHECK IF INS WILL BECOME READY ONCE THE FORWARDING TAKES PLACE */
            bbInstruction* fwd_ins = alu_fwd_list.Nth (i);
            List<PR>* wr_reg_list = fwd_ins->getPRwrList ();
            for (int j = rd_reg_list->NumElements () - 1; j >= 0; j--) {
                PR rd_reg = rd_reg_list->Nth (j);
                for (int k = wr_reg_list->NumElements () - 1; k >= 0; k--) {
                    PR wr_reg = wr_reg_list->Nth (k);
                    if (rd_reg == wr_reg) { num_global_match--; break; }
                }
            }
            if (num_global_match == 0) {en_global_fwd = true;}
            if (USE_LRF && fwd_ins->getBBWinID () == ins->getBBWinID ()) { //todo this needs fix
                List<PR>* wr_lreg_list = fwd_ins->getLARwrList ();
                for (int j = rd_lreg_list->NumElements () - 1; j >= 0; j--) {
                    PR rd_reg = rd_lreg_list->Nth (j);
                    for (int k = wr_lreg_list->NumElements () - 1; k >= 0; k--) {
                        PR wr_reg = wr_lreg_list->Nth (k);
                        if (rd_reg == wr_reg) { num_local_match--; break; }
                    }
                }
                if (num_local_match == 0) {en_local_fwd = true;}
            }
        }
    }

    mem_fwd:
    { /*-- FWD FROM MEM STAGE --*/
        if (_memory_to_scheduler_port->getBuffState () == EMPTY_BUFF) goto done_point;
        for (WIDTH i = 0; i < _stage_width; i++) { //TODO _stage_width replace with exe_num_EU
            if (!_memory_to_scheduler_port->hasReadyNow ()) break;
            bbInstruction* fwd_ins = _memory_to_scheduler_port->popNextReadyNow ();
            mem_fwd_list.Append (fwd_ins);
        }
        for (WIDTH i = 0; i < mem_fwd_list.NumElements (); i++) {
            /* CHECK IF INS WILL BECOME READY ONCE THE FORWARDING TAKES PLACE */
            bbInstruction* fwd_ins = mem_fwd_list.Nth (i);
            List<PR>* wr_reg_list = fwd_ins->getPRwrList ();
            for (int j = rd_reg_list->NumElements () - 1; j >= 0; j--) {
                PR rd_reg = rd_reg_list->Nth (j);
                for (int k = wr_reg_list->NumElements () - 1; k >= 0; k--) {
                    PR wr_reg = wr_reg_list->Nth (k);
                    if (rd_reg == wr_reg) { num_global_match--; break; }
                }
            }
            if (num_global_match == 0) {en_global_fwd = true;}
            if (USE_LRF && fwd_ins->getBBWinID () == ins->getBBWinID ()) {
                List<PR>* wr_lreg_list = fwd_ins->getLARwrList ();
                for (int j = rd_lreg_list->NumElements () - 1; j >= 0; j--) {
                    PR rd_reg = rd_lreg_list->Nth (j);
                    for (int k = wr_lreg_list->NumElements () - 1; k >= 0; k--) {
                        PR wr_reg = wr_lreg_list->Nth (k);
                        if (rd_reg == wr_reg) { num_local_match--; break; }
                    }
                }
                if (num_local_match == 0) {en_local_fwd = true;}
            }
        }
    }

//    cout << "y: " << num_global_match << " " << num_local_match << endl;
    /* DO FORWARDING NOW THAT CONFIDENT ABOUT IT */
    if ((en_local_fwd || !USE_LRF) && en_global_fwd) {
        { /*-- FWD FROM EXE STAGE --*/
            for (WIDTH i = 0; i < alu_fwd_list.NumElements (); i++) {
                /* CHECK IF INS WILL BECOME READY ONCE THE FORWARDING TAKES PLACE */
                bbInstruction* fwd_ins = alu_fwd_list.Nth (i);
                List<PR>* wr_reg_list = fwd_ins->getPRwrList ();
                for (int j = rd_reg_list->NumElements () - 1; j >= 0; j--) {
                    PR rd_reg = rd_reg_list->Nth (j);
                    for (int k = wr_reg_list->NumElements () - 1; k >= 0; k--) {
                        PR wr_reg = wr_reg_list->Nth (k);
                        if (rd_reg == wr_reg) {
                            rd_reg_list->RemoveAt (j);
                            s_alu_g_fwd_cnt++;
                            done_any_fwd = true;
                            break;
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
                                done_any_fwd = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        { /*-- FWD FROM MEM STAGE --*/
            for (WIDTH i = 0; i < mem_fwd_list.NumElements (); i++) {
                bbInstruction* fwd_ins = mem_fwd_list.Nth (i);
                List<PR>* wr_reg_list = fwd_ins->getPRwrList ();
                for (int j = rd_reg_list->NumElements () - 1; j >= 0; j--) {
                    PR rd_reg = rd_reg_list->Nth (j);
                    for (int k = wr_reg_list->NumElements () - 1; k >= 0; k--) {
                        PR wr_reg = wr_reg_list->Nth (k);
                        if (rd_reg == wr_reg) {
                            rd_reg_list->RemoveAt(j);
                            s_mem_g_fwd_cnt++;
                            done_any_fwd = true;
                            break;
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
                                done_any_fwd = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    done_point:
#ifdef ASSERTION
    Assert (num_global_match >= 0); Assert (num_local_match >= 0);
#endif
    return done_any_fwd;
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
#ifdef ASSERTION
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
#endif
    INS_ID squashSeqNum = g_var.getSquashSN ();
    for (int i = 0; i < _scheduler_to_execution_port->NumElements (); i++) {
        _scheduler_to_execution_port->Nth(i)->searchNflushPort (squashSeqNum);
    }
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
    bbWin->_win.ramAccess (); /* BECAUSE SN IS THE HEAD OF BB, THE WHOLE BB GETS SQUASHED AT ONCE */

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
    s_bbWin_inflight_rat += _busy_bbWin.size ();
    _RF_MGR->getStat ();
}
