/*********************************************************************************
 * commit.cpp
 *********************************************************************************/

#include "commit.h"

o3_commit::o3_commit (port<dynInstruction*>& commit_to_bp_port, 
			          port<dynInstruction*>& commit_to_scheduler_port, 
                      CAMtable<dynInstruction*>* iROB,
                      CAMtable<dynInstruction*>* iQUE,
	    	          WIDTH commit_width,
                      o3_memManager* LSQ_MGR,
                      o3_rfManager* RF_MGR,
                      sysClock* clk,
	    	          string stage_name)
	: stage (commit_width, stage_name, clk),
      s_squash_ins_cnt (g_stats.newScalarStat ( _stage_name, "squash_ins_cnt", "Number of squashed instructions", 0, PRINT_ZERO)),
      s_squash_br_cnt (g_stats.newScalarStat ( _stage_name, "squash_br_cnt", "Number of squashed branch instructions", 0, PRINT_ZERO)),
      s_squash_mem_cnt (g_stats.newScalarStat ( _stage_name, "squash_mem_cnt", "Number of squashed memory instructions", 0, PRINT_ZERO)),
      s_wp_ins_cnt (g_stats.newScalarStat (stage_name, "wp_ins_cnt", "Number of wrong-path dynamic instructions in "+stage_name, 0, PRINT_ZERO)),
      s_ins_type_hist (g_stats.newScalarHistStat ((LENGTH) NUM_INS_TYPE, stage_name, "ins_type_cnt", "Committed instruction type distribution", 0, PRINT_ZERO)),
      s_mem_type_hist (g_stats.newScalarHistStat ((LENGTH) NUM_MEM_TYPE, stage_name, "mem_type_cnt", "Committed memory instruction type distribution", 0, PRINT_ZERO))
{
	_commit_to_bp_port  = &commit_to_bp_port;
	_commit_to_scheduler_port = &commit_to_scheduler_port;
    _iROB = iROB;
    _iQUE = iQUE;
    _LSQ_MGR = LSQ_MGR;
    _RF_MGR = RF_MGR;

    _prev_ins_cnt = 0;
    _prev_commit_cyc = START_CYCLE;
}

o3_commit::~o3_commit () {}

void o3_commit::doCOMMIT () {
    dbg.print (DBG_COMMIT, "** %s: (cyc: %d)\n", _stage_name.c_str (), _clk->now ());
    /*-- STAT --*/
    regStat ();
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /*-- SQUASH HANDLING --*/
    if (! (g_var.g_pipe_state == PIPE_WAIT_FLUSH || g_var.g_pipe_state == PIPE_FLUSH)) {
        pipe_stall = commitImpl ();
    }

    verifySim ();

    /*-- STAT --*/
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;
}

/*-- COMMIT COMPLETE INS AT THE HEAD OF QUEUE --*/
PIPE_ACTIVITY o3_commit::commitImpl () {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;
    for (WIDTH i = 0; i < _stage_width; i++) {
        /*-- CHECKS --*/
        if (_iROB->getTableState () == EMPTY_BUFF) break;
        if (!_iROB->hasFreeWire (READ)) break;
        dynInstruction* ins = _iROB->getFront ();
        if (g_cfg->isEnSquash () && ins->isMemOrBrViolation ()) break;
        if (ins->getPipeStage () != COMPLETE) break;
        Assert (ins->getNumRdPR () == 0 && "instruction must have been ready long ago!");

        /*-- COMMIT INS --*/
        if (ins->getInsType () == MEM) {
            ins = _iROB->getFront ();
            if (_LSQ_MGR->commit (ins)) {
                ins->setPipeStage (COMMIT);
                _RF_MGR->commitRegs (ins);
                ins = _iROB->popFront ();
                dynInstruction* ins_dual = _iQUE->popFront ();
                Assert (ins->getInsID () == ins_dual->getInsID ());
                s_ins_type_hist[ins->getInsType ()]++;
                s_mem_type_hist[ins->getMemType ()]++;
                dbg.print (DBG_COMMIT, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                           "Commit ins", ins->getInsID (), _clk->now ());
                if (ins->getMemType () == LOAD) delIns (ins);
            }
        } else {
            ins = _iROB->popFront ();
            dynInstruction* ins_dual = _iQUE->popFront ();
            Assert (ins->getInsID () == ins_dual->getInsID ());
            _RF_MGR->commitRegs (ins);
            s_ins_type_hist[ins->getInsType ()]++;
            dbg.print (DBG_COMMIT, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                       "Commit ins", ins->getInsID (), _clk->now ());
            delIns (ins);
        }

        /*-- UPDATE WIRES --*/
        _iROB->updateWireState (READ);

        /*-- STAT --*/
        s_ipc++;
        s_ins_cnt++; //TODO this stat is not accurate if store commit returns false - fix
        pipe_stall = PIPE_BUSY;
    }
    return pipe_stall;
}

void o3_commit::squash () {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "ROB Flush", _clk->now ());
    Assert (g_var.g_pipe_state == PIPE_SQUASH_ROB);
    if (_iROB->getTableSize () == 0) return;

    PIPE_SQUASH_TYPE squash_type = g_var.getSquashType ();
    if (squash_type == BP_MISPRED) bpMispredSquash ();
    else if (squash_type == MEM_MISPRED) memMispredSquash ();
    else Assert (true == false && "Invalid squash type.");
    g_var.resetSquashType ();
}

void o3_commit::bpMispredSquash () {
    INS_ID squashSeqNum = g_var.getSquashSN ();
    dynInstruction* ins = NULL;
    LENGTH start_indx = 0, stop_indx = _iQUE->getTableSize () - 1;

    /*-- SQUASH iROB --*/
    _iROB->ramAccess (); /* SQUASH INS HOLDS INDEX TO ITS ROB ENTRY */
    for (LENGTH i = _iROB->getTableSize () - 1; i >= 0; i--) {
        if (_iROB->getTableSize () == 0) break;
        ins = _iROB->getNth_unsafe (i);
        if (ins->getInsID () < squashSeqNum) break;
        _iROB->removeNth_unsafe (i);
        _iROB->ramAccess ();
        s_squash_ins_cnt++;
        s_squash_br_cnt++;
    }

    /*-- SQUASH iQUE --*/
    for (LENGTH i = 0; i < _iQUE->getTableSize (); i++) {
        if (_iQUE->getTableSize () == 0) break;
        ins = _iQUE->getNth_unsafe (i);
        Assert (ins->getPipeStage () != EXECUTE);
        if (ins->getInsID () == squashSeqNum) {
            start_indx = i;
//            Assert (ins->isOnWrongPath () == true);
//            cout << "BK "<< hex << ins->getInsAddr () << endl;
        } else if (ins->getInsID () > squashSeqNum) {
            if (!ins->isMemOrBrViolation ()) {
                stop_indx = i - 1;
                Assert (i > start_indx);
                break;
            }
        }
    }
    /* NOTE: WE DELETE INSTRUCTIONS IN IQUE THAT ARE NOT IN ROB */
    Assert (_iQUE->getTableSize () > stop_indx && stop_indx >= start_indx && start_indx >= 0);

    /* PUSH BACK INS'S THAT ARE NOT ON WRONG PATH */
    for (LENGTH i = _iQUE->getTableSize () - 1; i > stop_indx; i--) {
        if (_iQUE->getTableSize () == 0) break;
        ins = _iQUE->getNth_unsafe (i);
        ins->resetStates ();
        g_var.insertFrontCodeCache (ins);
        _iQUE->removeNth_unsafe (i);
        dbg.print (DBG_COMMIT, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                   "(BR_MISPRED) Squash ins", ins->getInsID (), _clk->now ());
    }

    /* DELETE INS THAT ARE ON WRONG PATH */
    for (LENGTH i = stop_indx; i >= start_indx; i--) {
        if (_iQUE->getTableSize () == 0) break;
        ins = _iQUE->getNth_unsafe (i);
        Assert (ins->isMemOrBrViolation () == true);
        Assert (ins->getInsID () >= squashSeqNum);
        _iQUE->removeNth_unsafe (i);
        s_wp_ins_cnt++;
        dbg.print (DBG_COMMIT, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                   "(BR_MISPRED) Squash ins", ins->getInsID (), _clk->now ());
        delIns (ins);
    }
}

void o3_commit::memMispredSquash () {
    INS_ID squashSeqNum = g_var.getSquashSN ();
    dynInstruction* ins = NULL;

    /*-- SQUASH iROB --*/
    _iROB->ramAccess (); /* SQUASH INS HOLDS INDEX TO ITS ROB ENTRY */
    for (LENGTH i = _iROB->getTableSize () - 1; i >= 0; i--) {
        if (_iROB->getTableSize () == 0) break;
        ins = _iROB->getNth_unsafe (i);
        if (ins->getInsID () < squashSeqNum) break;
        _iROB->removeNth_unsafe (i);
        _iROB->ramAccess ();
        s_squash_ins_cnt++;
        s_squash_mem_cnt++;
    }

    /*-- SQUASH iQUE --*/
    for (LENGTH i = _iQUE->getTableSize () - 1; i >= 0; i--) {
        if (_iQUE->getTableSize () == 0) break;
        ins = _iQUE->getNth_unsafe (i);
        if (ins->getInsID () < squashSeqNum) break;
        ins->resetStates ();
        g_var.insertFrontCodeCache (ins);
        _iQUE->removeNth_unsafe (i);
        dbg.print (DBG_COMMIT, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                   "(MEM_MISPRED) Squash ins", ins->getInsID (), _clk->now ());
    }
}

/*-- DELETE INSTRUCTION OBJ --*/
void o3_commit::delIns (dynInstruction* ins) {
    delete ins;
}

void o3_commit::regStat () {
    _iROB->regStat ();
}

void o3_commit::verifySim () {
    if (g_cfg->isWarmedUp () || !g_cfg->warmUpEn ()) {
        if ((_clk->now () - _prev_commit_cyc) >= SIM_STALL_THR) {
            cout << "current cycle: " << _clk->now () << endl;
            cout << "last commit cycle: " << _prev_commit_cyc << endl;
            Assert (false && "No commit for too long");
        }
        if (s_ins_cnt.getValue () > _prev_ins_cnt) {
            _prev_ins_cnt = s_ins_cnt.getValue ();
            _prev_commit_cyc = _clk->now ();
        }
    }
}
