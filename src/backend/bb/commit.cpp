/*********************************************************************************
 * commit.cpp
 *********************************************************************************/

#include "commit.h"

bb_commit::bb_commit (port<dynInstruction*>& commit_to_bp_port, 
			          port<dynInstruction*>& commit_to_scheduler_port, 
                      CAMtable<dynBasicblock*>* bbROB,
	    	          WIDTH commit_width,
                      bb_memManager* LSQ_MGR,
                      bb_grfManager* RF_MGR,
                      sysClock* clk,
	    	          string stage_name)
	: stage (commit_width, stage_name, clk),
      s_squash_ins_cnt (g_stats.newScalarStat ( _stage_name, "squash_ins_cnt", "Number of squashed instructions", 0, PRINT_ZERO))
{
	_commit_to_bp_port  = &commit_to_bp_port;
	_commit_to_scheduler_port = &commit_to_scheduler_port;
    _bbROB = bbROB;
    _LSQ_MGR = LSQ_MGR;
    _RF_MGR = RF_MGR;
}

bb_commit::~bb_commit () {}

void bb_commit::doCOMMIT () {
    dbg.print (DBG_COMMIT, "** %s: (cyc: %d)\n", _stage_name.c_str (), _clk->now ());
    /*-- STAT --*/
    regStat ();
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /*-- SQUASH HANDLING --*/
    if (! (g_var.g_pipe_state == PIPE_WAIT_FLUSH || g_var.g_pipe_state == PIPE_FLUSH)) {
        pipe_stall = commitImpl ();
    }

    /*-- STAT --*/
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;
}

/*-- COMMIT COMPLETE INS AT THE HEAD OF QUEUE --*/
PIPE_ACTIVITY bb_commit::commitImpl () {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;
    for (WIDTH i = 0; i < _stage_width; i++) {
        /*-- CHECKS --*/
        if (_bbROB->getTableState () == EMPTY_BUFF) break;
        if (!_bbROB->hasFreeWire (READ)) break;
        dynBasicblock* bb = _bbROB->getFront ();
        if (bb->isMemOrBrViolation ()) break;
        if (bb->getBBstate () != EMPTY_BUFF) break; //TODO what is a BB is still getting filled up?
        if (!bb->isBBcomplete ()) break;

        /*-- COMMIT BB --*/
        bb = _bbROB->popFront ();
        dbg.print (DBG_COMMIT, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                               "Commit bb", bb->getBBID (), _clk->now ());
        commitBB (bb);

        /*-- UPDATE WIRES --*/
        _bbROB->updateWireState (READ);

        /*-- STAT --*/
        s_ins_cnt++; //TODO this stat is not accurate if store commit returns false - fix
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
    else Assert (true == false && "Invalid squash type.");
    g_var.resetSquashType ();
}

void bb_commit::bpMispredSquash () {
//    INS_ID squashSeqNum = g_var.getSquashSN ();
//    dynInstruction* ins = NULL;
//    LENGTH start_indx = 0, stop_indx = _bbROB->getTableSize () - 1;
//    for (LENGTH i = 0; i < _bbROB->getTableSize (); i++) {
//        if (_bbROB->getTableSize () == 0) break;
//        ins = _bbROB->getNth_unsafe (i);
//        Assert (ins->getPipeStage () != EXECUTE);
//        if (ins->getInsID () == squashSeqNum) {
//            start_indx = i;
//            Assert (ins->isOnWrongPath () == true);
//        } else if (ins->getInsID () > squashSeqNum) {
//            if (!ins->isMemOrBrViolation ()) {
//                stop_indx = i - 1;
//                Assert (i > start_indx);
//                break;
//            }
//        }
//    }
//    Assert (_bbROB->getTableSize () > stop_indx && stop_indx >= start_indx && start_indx >= 0);
//    for (LENGTH i = _bbROB->getTableSize () - 1; i > stop_indx; i--) {
//        if (_bbROB->getTableSize () == 0) break;
//        ins = _bbROB->getNth_unsafe (i);
//        ins->resetStates ();
//        g_var.insertFrontCodeCache (ins);
//        _bbROB->removeNth_unsafe (i);
//        s_squash_ins_cnt++;
//    }
//    for (LENGTH i = stop_indx; i >= start_indx; i--) {
//        if (_bbROB->getTableSize () == 0) break;
//        ins = _bbROB->getNth_unsafe (i);
//        Assert (ins->isMemOrBrViolation () == true);
//        Assert (ins->getInsID () >= squashSeqNum);
//        _bbROB->removeNth_unsafe (i);
//        s_squash_ins_cnt++;
//        delIns (ins);
//    }
}

void bb_commit::memMispredSquash () {
//    INS_ID squashSeqNum = g_var.getSquashSN ();
//    dynInstruction* ins = NULL;
//    for (LENGTH i = _bbROB->getTableSize () - 1; i >= 0; i--) {
//        if (_bbROB->getTableSize () == 0) break;
//        ins = _bbROB->getNth_unsafe (i);
//        if (ins->getInsID () < squashSeqNum) break;
//        ins->resetStates ();
//        g_var.insertFrontCodeCache (ins);
//        _bbROB->removeNth_unsafe (i);
//        s_squash_ins_cnt++;
//    }
}

/*-- DELETE INSTRUCTION OBJ --*/
void bb_commit::delIns (dynInstruction* ins) {
    delete ins;
}

/*-- DELETE INSTRUCTION OBJ --*/
void bb_commit::commitBB (dynBasicblock* bb) {
    List<dynInstruction*>* insList = bb->getBBinsList ();
    while (insList->NumElements () > 0) {
        dynInstruction* ins = insList->Nth (0); //TODO this is consuming a port count regardless of outcome of next step - fix
        cout << ins->getPipeStage () << endl;
        cout << ins->getInsID () << endl;
        Assert (ins->getPipeStage () == COMPLETE);
        if (ins->getInsType () == MEM) {
            if (_LSQ_MGR->commit (ins)) {
                ins->setPipeStage (COMMIT);
                _RF_MGR->commitRegs (ins);
                if (ins->getMemType () == LOAD) delIns (ins);
                insList->RemoveAt (0);
            } else {
                Assert (true == false && "implement this condition"); //TODO finish this
            }
        } else {
            _RF_MGR->commitRegs (ins);
            delIns (ins);
            insList->RemoveAt (0);
        }
    }
    delBB (bb);
}

void bb_commit::delBB (dynBasicblock* bb) {
    delete bb;
}

void bb_commit::regStat () {
    _bbROB->regStat ();
}
