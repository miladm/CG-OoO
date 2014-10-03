/*******************************************************************************
 * scheduler.cpp
 *******************************************************************************/

#include "schedulers.h"

scheduler::scheduler (port<dynInstruction*>& decode_to_scheduler_port, 
                      port<dynInstruction*>& execution_to_scheduler_port, 
                      port<dynInstruction*>& memory_to_scheduler_port, 
			          port<dynInstruction*>& scheduler_to_execution_port, 
                      CAMtable<dynInstruction*>* iROB,
	    	          WIDTH issue_width,
                      sysClock* clk,
	    	          string stage_name) 
	: stage (issue_width, stage_name, clk),
      //_iWindow (iWin_length, CAM_ARRAY, iWin_rd_port, iWin_wr_port, "iWindow") - TODO fix this
      _iWindow (50, 4, 4, clk, "iWindow"),
      s_mem_fwd_cnt (g_stats.newScalarStat (stage_name, "mem_fwd_cnt", "Number of memory forwarding events"+stage_name, 0, PRINT_ZERO)),
      s_alu_fwd_cnt (g_stats.newScalarStat (stage_name, "alu_fwd_cnt", "Number of ALU forwarding events"+stage_name, 0, PRINT_ZERO))
{
    _decode_to_scheduler_port = &decode_to_scheduler_port;
    _execution_to_scheduler_port = &execution_to_scheduler_port;
    _memory_to_scheduler_port = &memory_to_scheduler_port;
	_scheduler_to_execution_port  = &scheduler_to_execution_port;
    _iROB = iROB;
}

scheduler::~scheduler () {}

void scheduler::doSCHEDULER () {
    dbg.print (DBG_SCHEDULER, "%s: (cyc: %d)\n", _stage_name.c_str (), _clk->now ());
    /* STAT */
    regStat ();
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /* SQUASH HANDLING */
    if (g_var.g_pipe_state == PIPE_FLUSH) { squash (); }
    if (!(g_var.g_pipe_state == PIPE_WAIT_FLUSH || g_var.g_pipe_state == PIPE_FLUSH)) {
        pipe_stall = schedulerImpl ();
    }
    if (g_cfg->isEnFwd ()) manageCDB ();

    /* STAT */
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;
}

PIPE_ACTIVITY scheduler::schedulerImpl () {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    updateInsWin ();

    /* READ FROM INS WINDOW */
    for (WIDTH i = 0; i < _stage_width; i++) {
        /* CHECKS */
        if (_iWindow.getTableState () == EMPTY_BUFF) break;
        if (!_iWindow.hasFreeWire (READ)) break;
        if (_scheduler_to_execution_port->getBuffState () == FULL_BUFF) break;
        dynInstruction* ins = _iWindow.getNth_unsafe (0);
        if (g_cfg->isEnFwd ()) forwardFromCDB (ins);
        if (!g_RF_MGR->hasFreeWire (READ, ins->getNumRdAR ())) break;
        if (!g_RF_MGR->isReady (ins)) break;

        /* READ INS WIN */
        ins = _iWindow.popFront ();
        ins->setPipeStage (ISSUE);
        _scheduler_to_execution_port->pushBack (ins);
        dbg.print (DBG_SCHEDULER, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Issue ins", ins->getInsID (), _clk->now ());

        /* UPDATE WIRES */
        _iWindow.updateWireState (READ);
        g_RF_MGR->updateWireState (READ, ins->getNumRdAR ());

        /* STAT */
        s_ipc++;
        s_ins_cnt++;
        pipe_stall = PIPE_BUSY;
    }

    return pipe_stall;
}

/* WRITE INTO INS WINDOW */
void scheduler::updateInsWin () {
    for (WIDTH i = 0; i < _stage_width; i++) {
        /* CHECKS */
        if (_iROB->getTableState () == FULL_BUFF) break;
        if (!_iROB->hasFreeWire (WRITE)) break;
        if (_iWindow.getTableState () == FULL_BUFF) break;
        if (!_iWindow.hasFreeWire (WRITE)) break;
        if (_decode_to_scheduler_port->getBuffState () == EMPTY_BUFF) break;
        if (!_decode_to_scheduler_port->isReady ()) break;

        /* WRITE INS WIN */
        dynInstruction* ins = _decode_to_scheduler_port->popFront ();
        ins->setPipeStage (DISPATCH);
        _iWindow.pushBack (ins);
        _iROB->pushBack (ins);
        dbg.print (DBG_SCHEDULER, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Write iWin ins", ins->getInsID (), _clk->now ());

        /* UPDATE WIRES */
        _iWindow.updateWireState (WRITE);
        _iROB->updateWireState (WRITE);
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
void scheduler::forwardFromCDB (dynInstruction* ins) {
    { /* FWD FROM EXE STAGE */
        if (_execution_to_scheduler_port->getBuffState () == EMPTY_BUFF) return;
        List<AR>* rd_reg_list = ins->getARrdList ();
        List<dynInstruction*> fwd_list;
        for (WIDTH i = 0; i < _stage_width; i++) { //TODO _stage_width replace with exe_num_EU
            if (!_execution_to_scheduler_port->isReadyNow ()) break;
            dynInstruction* fwd_ins = _execution_to_scheduler_port->popFront ();
            fwd_list.Append (fwd_ins);
        }
        for (WIDTH i = 0; i < fwd_list.NumElements (); i++) {
            dynInstruction* fwd_ins = fwd_list.Nth (i);
            List<AR>* wr_reg_list = fwd_ins->getARwrList ();
            for (int j = rd_reg_list->NumElements () - 1; j >= 0; j--) {
                AR rd_reg = rd_reg_list->Nth (j);
                for (int k = wr_reg_list->NumElements () - 1; k >= 0; k--) {
                    AR wr_reg = wr_reg_list->Nth (k);
                    if (rd_reg == wr_reg) {
                        rd_reg_list->RemoveAt(j);
                        s_alu_fwd_cnt++;
                    }
                }
            }
        }
    }

    { /* FWD FROM MEM STAGE */
        if (_memory_to_scheduler_port->getBuffState () == EMPTY_BUFF) return;
        List<AR>* rd_reg_list = ins->getARrdList ();
        List<dynInstruction*> fwd_list;
        for (WIDTH i = 0; i < _stage_width; i++) { //TODO _stage_width replace with exe_num_EU
            if (!_memory_to_scheduler_port->hasReadyNow ()) break;
            dynInstruction* fwd_ins = _memory_to_scheduler_port->popNextReadyNow ();
            fwd_list.Append (fwd_ins);
        }
        for (WIDTH i = 0; i < fwd_list.NumElements (); i++) {
            dynInstruction* fwd_ins = fwd_list.Nth (i);
            List<AR>* wr_reg_list = fwd_ins->getARwrList ();
            for (int j = rd_reg_list->NumElements () - 1; j >= 0; j--) {
                AR rd_reg = rd_reg_list->Nth (j);
                for (int k = wr_reg_list->NumElements () - 1; k >= 0; k--) {
                    AR wr_reg = wr_reg_list->Nth (k);
                    if (rd_reg == wr_reg) {
                        rd_reg_list->RemoveAt(j);
                        s_mem_fwd_cnt++;
                    }
                }
            }
        }
    }
}

/* MANAGE COMMON DATA BUS (CDB) */
void scheduler::manageCDB () {
    _execution_to_scheduler_port->delOldReady (); /* Only FWD what is on CDB now */
    _memory_to_scheduler_port->delOldReady (); /* Only FWD what is on CDB now */
}

void scheduler::squash () {
#ifdef ASSERTION
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
#endif
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Scheduler Ports Flush", _clk->now ());
    INS_ID squashSeqNum = g_var.getSquashSN ();
    _scheduler_to_execution_port->flushPort (squashSeqNum);
    for (int i = (int)_iWindow.getTableSize() - 1; i >= 0; i--) {
        if (_iWindow.getTableSize() == 0) break;
        dynInstruction* ins = _iWindow.getNth_unsafe (i);
        if (ins->getInsID () >= squashSeqNum) {
            _iWindow.removeNth_unsafe (i);
        }
    }
}

void scheduler::regStat () {
    _decode_to_scheduler_port->regStat ();
    _execution_to_scheduler_port->regStat ();
    _memory_to_scheduler_port->regStat ();
    //_iWindow.regStat (); -TODO fix this - put it back
}
