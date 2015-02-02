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
      _iWindow (clk, g_cfg->_root["cpu"]["backend"]["table"]["iWindow"], "iWindow"),
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
        _iWindow.ramAccess ();
        if (!g_RF_MGR->hasFreeWire (READ, ins->getNumRdAR ())) break; /*-- INO SCHEDULING --*/
        if (!isReady (ins) || !g_RF_MGR->canReserveRF (ins)) break;

        /* READ INS WIN */
        g_RF_MGR->reserveRF (ins);
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

bool scheduler::isReady (dynInstruction* ins) {
    if (g_cfg->isEnFwd ()) {
        if (g_RF_MGR->isReady (ins)) return true;
        bool done_any_fwd = forwardFromCDB (ins);
        bool is_ready = g_RF_MGR->checkReadyAgain (ins);
        Assert ((is_ready && done_any_fwd) || (!is_ready && !done_any_fwd));
        return is_ready;
    } else {
        if (g_RF_MGR->isReady (ins)) return true;
        else return false;
    }
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

/* FORWARD OPERANDS */
bool scheduler::forwardFromCDB (dynInstruction* ins) {
    List<PR>* rd_reg_list = ins->getPRrdList ();
    int num_global_match = rd_reg_list->NumElements ();
    bool en_global_fwd = false;
    bool done_any_fwd = false;
    List<dynInstruction*> alu_fwd_list, mem_fwd_list;

    { /*-- FWD FROM EXE STAGE --*/
        if (_execution_to_scheduler_port->getBuffState () == EMPTY_BUFF) goto mem_fwd;
        for (WIDTH i = 0; i < _stage_width; i++) { //TODO _stage_width replace with exe_num_EU
            if (!_execution_to_scheduler_port->isReadyNow ()) break;
            dynInstruction* fwd_ins = _execution_to_scheduler_port->popFront ();
            alu_fwd_list.Append (fwd_ins);
        }
        for (WIDTH i = 0; i < alu_fwd_list.NumElements (); i++) {
            /* CHECK IF INS WILL BECOME READY ONCE THE FORWARDING TAKES PLACE */
            dynInstruction* fwd_ins = alu_fwd_list.Nth (i);
            List<PR>* wr_reg_list = fwd_ins->getPRwrList ();
            for (int j = rd_reg_list->NumElements () - 1; j >= 0; j--) {
                PR rd_reg = rd_reg_list->Nth (j);
                for (int k = wr_reg_list->NumElements () - 1; k >= 0; k--) {
                    PR wr_reg = wr_reg_list->Nth (k);
                    if (rd_reg == wr_reg) { num_global_match--; break; }
                }
            }
            if (num_global_match == 0) {en_global_fwd = true;}
        }
    }

    mem_fwd:
    { /*-- FWD FROM MEM STAGE --*/
        if (_memory_to_scheduler_port->getBuffState () == EMPTY_BUFF) goto done_point;
        for (WIDTH i = 0; i < _stage_width; i++) { //TODO _stage_width replace with exe_num_EU
            if (!_memory_to_scheduler_port->hasReadyNow ()) break;
            dynInstruction* fwd_ins = _memory_to_scheduler_port->popNextReadyNow ();
            mem_fwd_list.Append (fwd_ins);
        }
        for (WIDTH i = 0; i < mem_fwd_list.NumElements (); i++) {
            /* CHECK IF INS WILL BECOME READY ONCE THE FORWARDING TAKES PLACE */
            dynInstruction* fwd_ins = mem_fwd_list.Nth (i);
            List<PR>* wr_reg_list = fwd_ins->getPRwrList ();
            for (int j = rd_reg_list->NumElements () - 1; j >= 0; j--) {
                PR rd_reg = rd_reg_list->Nth (j);
                for (int k = wr_reg_list->NumElements () - 1; k >= 0; k--) {
                    PR wr_reg = wr_reg_list->Nth (k);
                    if (rd_reg == wr_reg) { num_global_match--; break; }
                }
            }
            if (num_global_match == 0) {en_global_fwd = true;}
        }
    }

    /* DO FORWARDING NOW THAT CONFIDENT ABOUT IT */
    if (en_global_fwd) {
        { /*-- FWD FROM EXE STAGE --*/
            for (WIDTH i = 0; i < alu_fwd_list.NumElements (); i++) {
                /* CHECK IF INS WILL BECOME READY ONCE THE FORWARDING TAKES PLACE */
                dynInstruction* fwd_ins = alu_fwd_list.Nth (i);
                List<PR>* wr_reg_list = fwd_ins->getPRwrList ();
                for (int j = rd_reg_list->NumElements () - 1; j >= 0; j--) {
                    PR rd_reg = rd_reg_list->Nth (j);
                    for (int k = wr_reg_list->NumElements () - 1; k >= 0; k--) {
                        PR wr_reg = wr_reg_list->Nth (k);
                        if (rd_reg == wr_reg) {
                            rd_reg_list->RemoveAt (j);
                            s_alu_fwd_cnt++;
                            done_any_fwd = true;
                            break;
                        }
                    }
                }
            }
        }

        { /*-- FWD FROM MEM STAGE --*/
            for (WIDTH i = 0; i < mem_fwd_list.NumElements (); i++) {
                dynInstruction* fwd_ins = mem_fwd_list.Nth (i);
                List<PR>* wr_reg_list = fwd_ins->getPRwrList ();
                for (int j = rd_reg_list->NumElements () - 1; j >= 0; j--) {
                    PR rd_reg = rd_reg_list->Nth (j);
                    for (int k = wr_reg_list->NumElements () - 1; k >= 0; k--) {
                        PR wr_reg = wr_reg_list->Nth (k);
                        if (rd_reg == wr_reg) {
                            rd_reg_list->RemoveAt(j);
                            s_mem_fwd_cnt++;
                            done_any_fwd = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    done_point:
    Assert (num_global_match >= 0);
    return done_any_fwd;
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
        _iWindow.ramAccess (); /* SQUASH CAUSED BY ONE OF THE INSTRUCTIONS IN THE QUE HEAD */
        if (ins->getInsID () >= squashSeqNum) {
            _iWindow.removeNth_unsafe (i);
        }
    }
}

void scheduler::regStat () {
    _decode_to_scheduler_port->regStat ();
    _execution_to_scheduler_port->regStat ();
    _memory_to_scheduler_port->regStat ();
    _iWindow.regStat ();
}
