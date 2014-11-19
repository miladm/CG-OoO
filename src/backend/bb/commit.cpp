/*********************************************************************************
 * commit.cpp
 *********************************************************************************/

#include "commit.h"

bb_commit::bb_commit (port<bbInstruction*>& commit_to_bp_port, 
			          port<bbInstruction*>& commit_to_scheduler_port, 
                      List<bbWindow*>* bbWindows,
                      WIDTH num_bbWin,
                      CAMtable<dynBasicblock*>* bbROB,
                      CAMtable<dynBasicblock*>* bbQUE,
	    	          WIDTH commit_width,
                      bb_memManager* LSQ_MGR,
                      bb_rfManager* RF_MGR,
                      sysClock* clk,
	    	          string stage_name)
	: stage (commit_width, stage_name, clk),
      s_squash_ins_cnt (g_stats.newScalarStat (stage_name, "squash_ins_cnt", "Number of squashed instructions", 0, PRINT_ZERO)),
      s_squash_br_cnt (g_stats.newScalarStat (stage_name, "squash_br_cnt", "Number of squashed branch instructions", 0, PRINT_ZERO)),
      s_squash_mem_cnt (g_stats.newScalarStat (stage_name, "squash_mem_cnt", "Number of squashed memory instructions", 0, PRINT_ZERO)),
      s_br_squash_bb_cnt (g_stats.newScalarStat (stage_name, "br_squash_bb_cnt", "Number of squashed basicblocks due to br mis-pred", 0, PRINT_ZERO)),
      s_mem_squash_bb_cnt (g_stats.newScalarStat (stage_name, "mem_squash_bb_cnt", "Number of squashed basicblocks due to mem mis-pred", 0, PRINT_ZERO)),
      s_num_waste_ins (g_stats.newScalarStat (stage_name, "num_waste_ins", "Number of useful instructions squashed", 0, PRINT_ZERO)),
      s_bb_cnt (g_stats.newScalarStat (stage_name, "bb_cnt", "Number of dynamic basiblocks in "+stage_name, 0, PRINT_ZERO)),
      s_bb_size_avg (g_stats.newRatioStat (&s_bb_cnt, stage_name, "bb_size_avg", "Basicblock average size", 0, PRINT_ZERO)),
      s_wp_bb_cnt (g_stats.newScalarStat (stage_name, "wp_bb_cnt", "Number of wrong-path basiblocks in "+stage_name, 0, PRINT_ZERO)),
      s_wp_ins_cnt (g_stats.newScalarStat (stage_name, "wp_ins_cnt", "Number of wrong-path dynamic instructions in "+stage_name, 0, PRINT_ZERO)),
      s_ins_type_hist (g_stats.newScalarHistStat ((LENGTH) NUM_INS_TYPE, stage_name, "ins_type_cnt", "Committed instruction type distribution", 0, PRINT_ZERO)),
      s_mem_type_hist (g_stats.newScalarHistStat ((LENGTH) NUM_MEM_TYPE, stage_name, "mem_type_cnt", "Committed memory instruction type distribution", 0, PRINT_ZERO))
{
	_commit_to_bp_port  = &commit_to_bp_port;
	_commit_to_scheduler_port = &commit_to_scheduler_port;
    _bbROB = bbROB;
    _bbQUE = bbQUE;
    _LSQ_MGR = LSQ_MGR;
    _RF_MGR = RF_MGR;
    _num_bbWin = num_bbWin;
    _bbWindows = bbWindows;
}

bb_commit::~bb_commit () {}

void bb_commit::doCOMMIT () {
    /*-- STAT & DEBUG --*/
    dbg.print (DBG_COMMIT, "** %s: (cyc: %d)\n", _stage_name.c_str (), _clk->now ());
    dbg.print (DBG_COMMIT, "bbROB size: %d, pipe_state: %d (cyc: %d)\n", _bbROB->getTableSize (), g_var.g_pipe_state, _clk->now ());
//    for (int i = 0; i < _bbROB->getTableSize (); i++) {
//        cout << "BB ID: " << _bbROB->getNth_unsafe (i)->getBBID () << " " << _bbROB->getNth_unsafe(i)->getBBheadID () << " | ";
//        List<bbInstruction*>* insList = _bbROB->getNth_unsafe (i)->getBBinsList ();
//        for (int j = 0; j < insList->NumElements (); j++) {
//            bbInstruction* ins = insList->Nth(j);
//            cout << ins->getInsType () << " " << ins->getInsID () << " (" << ins->getPipeStage () << "), ";
//        }
//        cout << endl;
//    }
    regStat ();
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /*-- SQUASH HANDLING --*/
    if (! (g_var.g_pipe_state == PIPE_WAIT_FLUSH || g_var.g_pipe_state == PIPE_FLUSH)) {
        pipe_stall = commitImpl ();
    }

    /*-- STAT --*/
    if (g_var.g_pipe_state != PIPE_NORMAL) s_squash_cycles++;
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;
    if (pipe_stall == PIPE_STALL && g_var.g_pipe_state != PIPE_NORMAL) 
        s_squash_stall_cycles++;
}

/*-- COMMIT COMPLETE INS AT THE HEAD OF QUEUE --*/
PIPE_ACTIVITY bb_commit::commitImpl () {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;
    for (WIDTH i = 0; i < _stage_width; i++) {
        /*-- CHECKS --*/
        if (_bbROB->getTableState () == EMPTY_BUFF) break;
        if (!_bbROB->hasFreeWire (READ)) break;
        dynBasicblock* bb = _bbROB->getFront ();
        if (g_cfg->isEnSquash () && bb->isMemOrBrViolation ()) {
            dbg.print (DBG_COMMIT, "%s: BB %llu is mis-speculated (cyc: %d)\n", 
                    _stage_name.c_str (), bb->getBBID (), _clk->now ());
            break;
        }
        if (bb->getBBstate () != EMPTY_BUFF) break; //TODO what is a BB is still getting filled up?
        if (!bb->isBBcomplete ()) {
            dbg.print (DBG_COMMIT, "%s: BB %llu is not complete (cyc: %d)\n", 
                    _stage_name.c_str (), bb->getBBID (), _clk->now ());
            break;
        }

        /*-- COMMIT BB --*/
        bb = _bbROB->popFront ();
        dynBasicblock* bb_dual = _bbQUE->popFront ();
        Assert (bb->isDoneFetch () == true);
        Assert (bb->getBBID () == bb_dual->getBBID ());
        dbg.print (DBG_COMMIT, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                               "Commit bb", bb->getBBID (), _clk->now ());
        s_bb_size_avg += bb->getBBorigSize ();
        commitBB (bb);

        /*-- UPDATE WIRES --*/
        _bbROB->updateWireState (READ);

        /*-- STAT --*/
        s_bb_cnt++;
        pipe_stall = PIPE_BUSY;
    }
    return pipe_stall;
}

void bb_commit::squash () {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "ROB Flush", _clk->now ());
    Assert (g_var.g_pipe_state == PIPE_SQUASH_ROB);
    if (_bbROB->getTableSize () == 0) return;

    PIPE_SQUASH_TYPE squash_type = g_var.getSquashType ();
    if (squash_type == BP_MISPRED) bpMispredSquash ();
    else if (squash_type == MEM_MISPRED) memMispredSquash ();
    else {Assert (0 && "Invalid squash type.");}
    g_var.resetSquashType ();
}

void bb_commit::bpMispredSquash () {
    INS_ID squashSeqNum = g_var.getSquashSN ();
    dynBasicblock* bb = NULL;
    LENGTH start_indx = 0, stop_indx = _bbQUE->getTableSize () - 1;

    /*-- SQUASH BBROB --*/
    _bbROB->camAccess (); /* TO FIND THE SQUASH ADDRESS */
    for (LENGTH i = _bbROB->getTableSize () - 1; i >= 0; i--) {
        if (_bbROB->getTableSize () == 0) break;
        bb = _bbROB->getNth_unsafe (i);
        if (bb->getBBheadID () < squashSeqNum) break;
        _bbROB->removeNth_unsafe (i);
        _bbROB->ramAccess ();
    }

    /*-- SQUASH BBQUE --*/
    for (LENGTH i = 0; i < _bbQUE->getTableSize (); i++) {
        if (_bbQUE->getTableSize () == 0) break;
        bb = _bbQUE->getNth_unsafe (i);
        if (bb->getBBheadID () == squashSeqNum) {
            start_indx = i;
            Assert (bb->isOnWrongPath () == true);
        } else if (bb->getBBheadID () > squashSeqNum) {
            if (!bb->isMemOrBrViolation () || 
                bb->hasCorrectPathIns ()) { // BOTH CONDITIONS SHOULD AGREE - A PRECAUTION
                stop_indx = i - 1;
                Assert (i > start_indx);
                break;
            }
        }
    }
//    if (stop_indx == _bbQUE->getTableSize () - 1) { TODO see Issue #167 for this block of code.
//        while (1) {
//            dynBasicblock* bb = g_var.getNthCacheBB (0);
//            if (bb->isMemOrBrViolation () && !bb->hasCorrectPathIns ()) {
//                cout << "SHIIIIIIIIIIIIIIIIIIIIIIT " << _clk->now () << endl;
//                g_var.popBBcache ();
//            } else {break;}
//        }
//    }
    Assert (_bbQUE->getTableSize () > stop_indx && stop_indx >= start_indx && start_indx >= 0);

    /* PUSH BACK BB'S THAT ARE NOT ON WRONG PATH */
    for (LENGTH i = _bbQUE->getTableSize () - 1; i > stop_indx; i--) {
        if (_bbQUE->getTableSize () == 0) break;
        bb = _bbQUE->getNth_unsafe (i);
        bb->resetStates ();
        g_var.insertFrontBBcache (bb);
        _bbQUE->removeNth_unsafe (i);
        s_num_waste_ins += bb->getNumWasteIns ();
        s_squash_ins_cnt += bb->getBBorigSize ();
        s_squash_br_cnt += bb->getBBorigSize ();
        s_br_squash_bb_cnt++;
        dbg.print (DBG_COMMIT, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                               "(BR_MISPRED)Squash bb", bb->getBBID (), _clk->now ());
    }

    /* DELETE BB'S THAT ARE ON WRONG PATH */
    for (LENGTH i = stop_indx; i >= start_indx; i--) {
        if (_bbQUE->getTableSize () == 0) break;
        bb = _bbQUE->getNth_unsafe (i);
        Assert (bb->isMemOrBrViolation () == true);
        Assert (bb->getBBheadID () >= squashSeqNum);
        s_num_waste_ins += bb->getNumWasteIns ();
        _bbQUE->removeNth_unsafe (i);
        s_wp_bb_cnt++;
        s_wp_ins_cnt += bb->getBBorigSize ();
        s_squash_ins_cnt += bb->getBBorigSize ();
        s_squash_br_cnt += bb->getBBorigSize ();
        s_br_squash_bb_cnt++;
        dbg.print (DBG_COMMIT, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                               "(BR_MISPRED) Squash bb", bb->getBBID (), _clk->now ());
        delBB (bb);
    }
}

void bb_commit::memMispredSquash () {
    INS_ID squashSeqNum = g_var.getSquashSN ();
    dynBasicblock* bb = NULL;

    /*-- SQUASH BBROB --*/
    _bbROB->camAccess (); /* TO FIND THE SQUASH ADDRESS */
    for (LENGTH i = _bbROB->getTableSize () - 1; i >= 0; i--) {
        if (_bbROB->getTableSize () == 0) break;
        bb = _bbROB->getNth_unsafe (i);
        if (bb->getBBheadID () < squashSeqNum) break;
        _bbROB->removeNth_unsafe (i);
        _bbROB->ramAccess ();
    }

    /*-- SQUASH BBQUE --*/
    for (LENGTH i = _bbQUE->getTableSize () - 1; i >= 0; i--) {
        if (_bbQUE->getTableSize () == 0) break;
        bb = _bbQUE->getNth_unsafe (i);
        if (bb->getBBheadID () < squashSeqNum) break;
        bb->resetStates ();
        g_var.insertFrontBBcache (bb);
        _bbQUE->removeNth_unsafe (i);
        s_squash_ins_cnt += bb->getBBorigSize ();
        s_squash_mem_cnt += bb->getBBorigSize ();
        s_num_waste_ins += bb->getNumWasteIns ();
        s_mem_squash_bb_cnt++;
        dbg.print (DBG_COMMIT, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                               "(MEM_MISPRED) Squash bb", bb->getBBID (), _clk->now ());
    }
}

/*-- DELETE INSTRUCTION OBJ --*/
void bb_commit::commitBB (dynBasicblock* bb) {
    static int commited_bb = 0;
    List<bbInstruction*>* insList = bb->getBBinsList ();
    commited_bb++;
    while (insList->NumElements () > 0) {
        bbInstruction* ins = insList->Nth (0);
        Assert (ins->getPipeStage () == COMPLETE);
        if (ins->getInsType () == MEM) {
            if (_LSQ_MGR->commit (ins)) {
                ins->setPipeStage (COMMIT);
                _RF_MGR->commitRegs (ins);
                s_ins_type_hist[ins->getInsType ()]++;
                s_mem_type_hist[ins->getMemType ()]++;
                if (ins->getMemType () == LOAD) delIns (ins);
                insList->RemoveAt (0);
            } else {
//                return; //TODO add stat on average number of cycles to commit a BB
                Assert (0 && "implement this condition"); //TODO finish this
            }
        } else {
            _RF_MGR->commitRegs (ins);
            s_ins_type_hist[ins->getInsType ()]++;
            delIns (ins);
            insList->RemoveAt (0);
        }
        s_ins_cnt++; //TODO this stat is not accurate if store commit returns false - fix
        s_ipc++;
    }
    delete bb;
}

/*-- DELETES ALL INSTRUCTIONS INCLUSING STORE OPS --*/
void bb_commit::delBB (dynBasicblock* bb) {
    List<bbInstruction*>* insList = bb->getBBinsList ();
    while (insList->NumElements () > 0) {
        bbInstruction* ins = insList->Nth (0);
        delIns (ins);
        insList->RemoveAt (0);
    }
    delete bb;
}

/*-- DELETE INSTRUCTION OBJ --*/
void bb_commit::delIns (bbInstruction* ins) {
    delete ins;
}

void bb_commit::regStat () {
    _bbROB->regStat ();
}
