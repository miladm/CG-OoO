/*******************************************************************************
 * commit.cpp
 *******************************************************************************/

#include "commit.h"

commit::commit (port<dynInstruction*>& commit_to_bp_port, 
			    port<dynInstruction*>& commit_to_scheduler_port, 
                CAMtable<dynInstruction*>* iROB,
	    	    WIDTH commit_width,
	    	    string stage_name)
	: stage (commit_width, stage_name),
      s_squash_ins_cnt (g_stats.newScalarStat ( _stage_name, "squash_ins_cnt", "Number of squashed instructions", 0, PRINT_ZERO))
{
	_commit_to_bp_port  = &commit_to_bp_port;
	_commit_to_scheduler_port = &commit_to_scheduler_port;
    _iROB = iROB;
}

commit::~commit () {}

void commit::doCOMMIT (sysClock& clk) {
    /* STAT */
    regStat (clk);
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /* SQUASH HANDLING */
    if (!(g_var.g_pipe_state == PIPE_WAIT_FLUSH || g_var.g_pipe_state == PIPE_FLUSH)) {
        pipe_stall = commitImpl (clk);
    }

    /* STAT */
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;
}

/* COMMIT COMPLETE INS AT THE HEAD OF QUEUE */
PIPE_ACTIVITY commit::commitImpl (sysClock& clk) {
    dbg.print (DBG_COMMIT, "%s: (cyc: %d)\n", _stage_name.c_str (), clk.now ());
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;
    for (WIDTH i = 0; i < _stage_width; i++) {
        /* CHECKS */
        if (_iROB->getTableState () == EMPTY_BUFF) break;
        if (!_iROB->hasFreeRdPort (clk.now ())) break;
        dynInstruction* ins = _iROB->getFront ();
        //Assert (ins->getNumRegRd () == 0 && "instruction must have been ready long ago!");
        if (ins->isOnWrongPath ()) break;
        if (ins->getPipeStage () != COMPLETE) break;

        /* COMMIT INS */
        ins = _iROB->popFront();
        dbg.print (DBG_COMMIT, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Commit ins", ins->getInsID (), clk.now ());
        delete ins;

        /* STAT */
        s_ins_cnt++;
        pipe_stall = PIPE_BUSY;
    }
    return pipe_stall;
}

void commit::squash (sysClock& clk) {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "ROB Flush", clk.now ());
    Assert (g_var.g_pipe_state == PIPE_SQUASH_ROB);
    if (_iROB->getTableSize() == 0) return;
    INS_ID squashSeqNum = g_var.getSquashSN ();
    dynInstruction* ins = NULL;
    LENGTH start_indx = 0, stop_indx = _iROB->getTableSize() - 1;
    for (LENGTH i = 0; i < _iROB->getTableSize(); i++) {
        if (_iROB->getTableSize() == 0) break;
        ins = _iROB->getNth_unsafe (i);
        Assert (ins->getPipeStage () != EXECUTE);// && ins->getPipeStage () != MEM_ACCESS);
        if (ins->getInsID () == squashSeqNum) {
            start_indx = i;
            Assert (ins->isOnWrongPath () == true);
        } else if (ins->getInsID () > squashSeqNum) {
            if (!ins->isOnWrongPath ()) {
                stop_indx = i - 1;
                Assert (i > start_indx);
                break;
            }
        }
    }
    Assert (_iROB->getTableSize() > stop_indx && stop_indx >= start_indx && start_indx >= 0);
    for (LENGTH i = _iROB->getTableSize() - 1; i > stop_indx; i--) {
        if (_iROB->getTableSize() == 0) break;
        ins = _iROB->getNth_unsafe (i);
        ins->resetStates ();
        g_var.insertFrontCodeCache(ins);
        _iROB->removeNth_unsafe (i);
        s_squash_ins_cnt++;
    }
    for (LENGTH i = stop_indx; i >= start_indx; i--) {
        if (_iROB->getTableSize() == 0) break;
        ins = _iROB->getNth_unsafe (i);
        Assert (ins->isOnWrongPath () == true);
        Assert (ins->getInsID () >= squashSeqNum);
        _iROB->removeNth_unsafe (i);
        s_squash_ins_cnt++;
        delete ins;
    }
}

void commit::regStat (sysClock& clk) {
    //_iROB->regStat (); TODO - fix this
}
