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
                      bb_grfManager* RF_MGR,
                      sysClock* clk,
	    	          string stage_name) 
	: stage (issue_width, stage_name, clk)
{
    _decode_to_scheduler_port = &decode_to_scheduler_port;
    _execution_to_scheduler_port = &execution_to_scheduler_port;
    _memory_to_scheduler_port = &memory_to_scheduler_port;
    _scheduler_to_execution_port  = &scheduler_to_execution_port;
    _bbROB = bbROB;
    _num_bbWin = 4;
    _LSQ_MGR = LSQ_MGR;
    _GRF_MGR = RF_MGR;
    _bbWin_on_fetch = NULL;
    for (WIDTH i = 0; i < _num_bbWin; i++) {
        ostringstream bbWin_num;
        bbWin_num << i;
        bbWindow* bbWin = new bbWindow (bbWin_num.str (), _clk);
        _avail_bbWin.Append (bbWin);
        _bbWindows.Append (bbWin);
    }
}

bb_scheduler::~bb_scheduler () {
    for (WIDTH i = 0; i < _num_bbWin; i++) {
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
    for (WIDTH i = 0; i < _stage_width; i++) {
        /*-- CHECKS --*/
        if (_scheduler_to_execution_port->getBuffState () == FULL_BUFF) break;
        dbg.print (DBG_SCHEDULER, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Issue ", _clk->now ());
        LENGTH readyInsInBBWinIndx;
        if (!hasReadyInsInBBWins (readyInsInBBWinIndx)) break;
        dbg.print (DBG_SCHEDULER, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Issue ", _clk->now ());
        dynInstruction* ins = _busy_bbWin[readyInsInBBWinIndx]->_win.getNth_unsafe (0); //TODO fix this with hasReadInsInBBWin
        dbg.print (DBG_SCHEDULER, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Issue ", _clk->now ());
        WIDTH num_ar = ins->getTotNumRdAR ();
        if (!_GRF_MGR->hasFreeWire (READ, num_ar)) break; //TODO this is conservative when using forwarding - fix (for ino too)
        dbg.print (DBG_SCHEDULER, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Issue ", _clk->now ());

        /*-- READ INS WIN --*/
        ins = _busy_bbWin[readyInsInBBWinIndx]->_win.popFront (); //TODO implement multi reads from a bbWin
        ins->setPipeStage (ISSUE);
        _scheduler_to_execution_port->pushBack (ins);
        dbg.print (DBG_SCHEDULER, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Issue ins", ins->getInsID (), _clk->now ());

        /*-- UPDATE RESOURCES --*/
        if (_busy_bbWin[readyInsInBBWinIndx]->_win.getTableState () == EMPTY_BUFF) {
            setBBWisAvail (readyInsInBBWinIndx);
        }
        _GRF_MGR->updateWireState (READ, num_ar);
        //_busy_bbWin[readyInsInBBWinIndx]->_win.updateWireState (READ);

        /*-- STAT --*/
        s_ins_cnt++;
        pipe_stall = PIPE_BUSY;
        dbg.print (DBG_SCHEDULER, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Issue ", _clk->now ());
    }

    manageCDB ();

    return pipe_stall;
}

bool bb_scheduler::hasReadyInsInBBWins (LENGTH &readyInsInBBWinIndx) {
    map<WIDTH, bbWindow*>::iterator bbWinEntry;
    for (bbWinEntry = _busy_bbWin.begin(); bbWinEntry != _busy_bbWin.end(); bbWinEntry++) {
    //for (auto& bbWinEntry : _busy_bbWin) { //TODO add code to get second/third/fourth entries too
        WIDTH bbWin_id = bbWinEntry->first;
        bbWindow* bbWin = bbWinEntry->second;
        if (bbWin->_win.getTableState () == EMPTY_BUFF) continue;
        if (!bbWin->_win.hasFreeWire (READ)) continue;
        dynInstruction* ins = bbWin->_win.getNth_unsafe (0);
        readyInsInBBWinIndx = bbWin_id;
        forwardFromCDB (ins);
        if (!_GRF_MGR->isReady (ins)) {continue;}
        else return true;
    }
    dbg.print (DBG_SCHEDULER, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Found NO ready ins.", _clk->now ());
    return false;
}

/*-- WRITE INTO BB WINDOW --*/
void bb_scheduler::updatebbWindows () {
    for (WIDTH i = 0; i < _stage_width; i++) {
        /*-- CHECKS --*/
        if (_decode_to_scheduler_port->getBuffState () == EMPTY_BUFF) break;
        if (!_decode_to_scheduler_port->isReady ()) break;
        dynInstruction* ins = _decode_to_scheduler_port->getFront ();
        if (ins->getInsType () == MEM && ins->getMemType () == LOAD) {
            if (_LSQ_MGR->getTableState (LD_QU) == FULL_BUFF) break;
            if (!_LSQ_MGR->hasFreeWire (LD_QU, WRITE)) break;
        } else if (ins->getInsType () == MEM && ins->getMemType () == STORE) {
            if (_LSQ_MGR->getTableState (ST_QU) == FULL_BUFF) break;
            if (!_LSQ_MGR->hasFreeWire (ST_QU, WRITE)) break;
        }
        if (!_GRF_MGR->canRename (ins)) break;

        /*-- CHECK & UPDATE --*/
        cout << "new bb?" << endl;
        if (detectNewBB (ins)) {
            if (!hasAnAvailBBWin ()) {
                cout << "no avail bb" << endl;
                break;
            } else {
                if (_bbROB->getTableState () == FULL_BUFF) break;
                if (!_bbROB->hasFreeWire (WRITE)) break;
                cout << "detected new bb: " << ins->getBB()->getBBID () << endl;
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
        _GRF_MGR->renameRegs (ins);
        ins->setPipeStage (DISPATCH);
        if (ins->getInsType () == MEM) _LSQ_MGR->pushBack (ins);
        _bbWin_on_fetch->_win.pushBack (ins);
        dbg.print (DBG_SCHEDULER, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Write bbWin ins", ins->getInsID (), _clk->now ());

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

bool bb_scheduler::detectNewBB (dynInstruction* ins) {
    if (_bbROB->getTableState () == EMPTY_BUFF) return true;
    Assert (ins->getBB()->getBBID () >= _bbROB->getLast()->getBBID ());
    cout << "is new BB? " << ins->getBB()->getBBID () << _bbROB->getLast()->getBBID () << endl;
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
    for (WIDTH j = 0; j < _num_bbWin; j++) {
        for (int i = (int)_bbWindows.Nth(j)->_win.getTableSize() - 1; i >= 0; i--) {
            if (_bbWindows.Nth(j)->_win.getTableSize() == 0) break;
            dynInstruction* ins = _bbWindows.Nth(j)->_win.getNth_unsafe (i);
            if (ins->getInsID () >= squashSeqNum) {
                _bbWindows.Nth(j)->_win.removeNth_unsafe (i);
            }
        }
    }
    _bbWin_on_fetch = NULL;
    map<WIDTH, bbWindow*>::iterator bbWinEntry;
    for (bbWinEntry = _busy_bbWin.begin(); bbWinEntry != _busy_bbWin.end(); bbWinEntry++) {
        bbWindow* bbWin = bbWinEntry->second;
        _avail_bbWin.Append (bbWin);
    }
    _busy_bbWin.clear ();
}

void bb_scheduler::regStat () {
    _decode_to_scheduler_port->regStat ();
    _execution_to_scheduler_port->regStat ();
    _memory_to_scheduler_port->regStat ();
  //for (WIDTH j = 0; j < _num_bbWin; j++) { TODO put this back
  //    _bbWindows.Nth(j)->regStat ();
  //}
}
