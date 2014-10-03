/*********************************************************************************
 * execution.cpp
 *********************************************************************************/

#include "execution.h"

o3_execution::o3_execution (port<dynInstruction*>& scheduler_to_execution_port, 
                            port<dynInstruction*>& execution_to_scheduler_port, 
                            CAMtable<dynInstruction*>* iROB,
	    	                WIDTH execution_width,
                            o3_memManager* LSQ_MGR,
                            o3_rfManager* RF_MGR,
                            sysClock* clk,
	    	                string stage_name) 
	: stage (execution_width, stage_name, clk),
      s_pipe_state_hist (g_stats.newScalarHistStat (NUM_PIPE_STATE, stage_name, "pipe_state_cnt", "Number of cycles in each squash stage", 0, PRINT_ZERO)),
      s_eu_busy_state_hist (g_stats.newScalarHistStat ((LENGTH) execution_width, stage_name, "eu_busy_state_hist", "Number of cycles execution unit is busy", 0, PRINT_ZERO)),
      s_br_mispred_cnt (g_stats.newScalarStat (stage_name, "br_mispred_cnt", "Number of branch mis-predict events", 0, PRINT_ZERO)),
      s_mem_mispred_cnt (g_stats.newScalarStat (stage_name, "mem_mispred_cnt", "Number of memory mis-predict events", 0, PRINT_ZERO))
{
    _scheduler_to_execution_port = &scheduler_to_execution_port;
    _execution_to_scheduler_port = &execution_to_scheduler_port;
    _iROB = iROB;
    _LSQ_MGR = LSQ_MGR;
    _RF_MGR = RF_MGR;
    _aluExeUnits = new List<exeUnit*>;
    for (WIDTH i = 0; i < _stage_width; i++) {
        exeUnit* newEU = new exeUnit (1,  _eu_lat._alu_lat, ALU_EU); //TODO make this config better with more EU types
        _aluExeUnits->Append (newEU);
    }
}

o3_execution::~o3_execution () {}

void o3_execution::doEXECUTION () {
    /*-- STAT + DEBUG --*/
    dbg.print (DBG_FETCH, "** %s: (cyc: %d)\n", _stage_name.c_str (), _clk->now ());
    regStat ();
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /*-- WRITEBACK RESULT --*/
    COMPLETE_STATUS cmpl_status;
    cmpl_status = completeIns ();

    /*-- EXECUTE INS --*/
    if (cmpl_status == COMPLETE_NORMAL) {
        pipe_stall = executionImpl ();
    }

    /*-- SQUASH CONTROL --*/
    if (g_cfg->isEnSquash ()) squashCtrl ();
    dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
               "PIPELINE STATE:", g_var.g_pipe_state, _clk->now ());

    /*-- STAT --*/
    s_pipe_state_hist[g_var.g_pipe_state]++;
    if (g_var.g_pipe_state != PIPE_NORMAL) s_squash_cycles++;
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;
    for (WIDTH i = 0; i < _stage_width; i++) {
        exeUnit* EU = _aluExeUnits->Nth (i);
        if (EU->getEUstate (_clk->now (), false) != AVAILABLE_EU) 
            s_eu_busy_state_hist[i]++;
    }
}

/*-- WRITE COMPLETE INS - WRITEBACK --*/
COMPLETE_STATUS o3_execution::completeIns () {
    g_var.setOldSquashSN ();
    for (WIDTH i = 0; i < _stage_width; i++) {
        exeUnit* EU = _aluExeUnits->Nth (i);
        dynInstruction* ins = EU->getEUins ();
        dynInstruction* violating_ld_ins = NULL;

        /*-- CHECKS --*/
        if (g_var.g_pipe_state == PIPE_FLUSH) break;
        if (ins == NULL) continue;
        if (!(ins->getInsType () == MEM && 
             ins->getMemType () == LOAD) &&
            !_RF_MGR->hasFreeWire (WRITE, ins->getNumWrPR ())) continue;
        if (EU->getEUstate (_clk->now (), true) != COMPLETE_EU) continue;

        /*-- COMPLETE INS --*/
        if (ins->getInsType () == MEM && ins->getMemType () == LOAD) {
            ins->setPipeStage (MEM_ACCESS);
            _LSQ_MGR->memAddrReady (ins);
            dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                      "Complete load addr calc - ins addr", ins->getInsID (), _clk->now ());
        } else if (ins->getInsType () == MEM && ins->getMemType () == STORE) {
            ins->setPipeStage (COMPLETE);
            _LSQ_MGR->memAddrReady (ins);
            _RF_MGR->completeRegs (ins); //TODO this sould not normally exist. problem with no support for u-ops (create support for both cases)
            _RF_MGR->updateWireState (WRITE, ins->getNumWrPR ());
            pair<bool, dynInstruction*> p = _LSQ_MGR->isLQviolation (ins);
            bool is_violation = p.first;
            violating_ld_ins = p.second;
            /*-- SQUASH DETECTION --*/
            dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                       "Complete store addr calc - ins addr", ins->getInsID (), _clk->now ());
            if (is_violation) { violating_ld_ins->setMemViolation (); }
        } else {
            ins->setPipeStage (COMPLETE);
            _RF_MGR->completeRegs (ins);
            _RF_MGR->updateWireState (WRITE, ins->getNumWrPR ());
            dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                       "Complete ins", ins->getInsID (), _clk->now ());
        }
        EU->resetEU ();

        /*-- SQUASH DETECTION --*/
        if (ins->isMemOrBrViolation ()) {
            g_var.setSquashSN (ins->getInsID ());
            g_var.setSquashType (BP_MISPRED);
            s_br_mispred_cnt++;
            dbg.print (DBG_EXECUTION, "%s: %s (cyc: %d)\n", 
                    _stage_name.c_str (), "BP_MISPRED", _clk->now ());
        } else if (violating_ld_ins != NULL && violating_ld_ins->isMemOrBrViolation ()) {
            g_var.setSquashSN (violating_ld_ins->getInsID ());
            g_var.setSquashType (MEM_MISPRED);
            s_mem_mispred_cnt++;
            dbg.print (DBG_EXECUTION, "%s: %s (cyc: %d)\n", 
                    _stage_name.c_str (), "MEM_MISPRED", _clk->now ());
        }
    }

    if (g_var.isSpeculationViolation ()) {
        dbg.print (DBG_EXECUTION, "%s: %s %llu %s (cyc: %d)\n", _stage_name.c_str (), 
                  "Ins on Wrong Path (ins: ", g_var.getSquashSN (), ")", _clk->now ());
        return COMPLETE_SQUASH;
    }
    dbg.print (DBG_EXECUTION, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Ins on Right Path", _clk->now ());
    return COMPLETE_NORMAL;
}

/*-- EXECUTE INS --*/
PIPE_ACTIVITY o3_execution::executionImpl () {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    for (WIDTH i = 0; i < _stage_width; i++) {
        /*-- CHECKS --*/
        if (g_var.g_pipe_state == PIPE_WAIT_FLUSH || g_var.g_pipe_state == PIPE_FLUSH) break;
        if (_scheduler_to_execution_port->getBuffState () == EMPTY_BUFF) break;
        if (!_scheduler_to_execution_port->isReady ()) break;
        exeUnit* EU = _aluExeUnits->Nth (i);
        if (EU->getEUstate (_clk->now (), false) != AVAILABLE_EU) continue;

        /*-- EXE INS --*/
        dynInstruction* ins = _scheduler_to_execution_port->popFront ();
        EU->_eu_timer.setNewTime (_clk->now ());
        EU->setEUins (ins);
        EU->runEU ();
        ins->setPipeStage (EXECUTE);
        if (g_cfg->isEnEuFwd ()) forward (ins, EU->_eu_timer.getLatency ());
        dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Execute ins", ins->getInsID (), _clk->now ());

        /*-- STAT --*/
        s_ipc++;
        s_ins_cnt++;
        pipe_stall = PIPE_BUSY;
    }
    return pipe_stall;
}

void o3_execution::forward (dynInstruction* ins, CYCLE eu_latency) {
    if (_execution_to_scheduler_port->getBuffState () == FULL_BUFF) return;
    if (ins->getInsType () != MEM) {
        CYCLE cdb_ready_latency = eu_latency - 1;
        Assert (cdb_ready_latency >= 0);
        _execution_to_scheduler_port->pushBack (ins, cdb_ready_latency);
        dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Forward wr ops of ins", ins->getInsID (), _clk->now ());
    }
}

void o3_execution::squashCtrl () {
    string state_switch;
    if ((g_var.g_pipe_state == PIPE_NORMAL || 
         g_var.g_pipe_state == PIPE_WAIT_FLUSH || 
         g_var.g_pipe_state == PIPE_FLUSH || 
         g_var.g_pipe_state == PIPE_DRAIN || 
         g_var.g_pipe_state == PIPE_SQUASH_ROB) && 
         g_var.isSpeculationViolation ()) {
        g_var.g_pipe_state = PIPE_WAIT_FLUSH;
        state_switch =  "PIPE_NORMAL -> PIPE_WAIT_FLUSH";
    } else if (g_var.g_pipe_state == PIPE_NORMAL) {
        g_var.resetSquashSN ();
        return;
    } else if (g_var.g_pipe_state == PIPE_WAIT_FLUSH) {
        for (WIDTH i = 0; i < _stage_width; i++) {
            exeUnit* EU = _aluExeUnits->Nth (i);
            if (EU->getEUstate (_clk->now (), false) != AVAILABLE_EU) return;
        }
        g_var.g_pipe_state = PIPE_FLUSH;
        state_switch =  "PIPE_WAIT_FLUSH -> PIPE_FLUSH";
        for (WIDTH i = 0; i < _stage_width; i++) {
            exeUnit* EU = _aluExeUnits->Nth (i);
            EU->resetEU ();
        }
        squash ();
    } else if (g_var.g_pipe_state == PIPE_FLUSH) {
        g_var.g_pipe_state = PIPE_DRAIN;
        state_switch =  "PIPE_FLUSH -> PIPE_DRAIN";
    } else if (g_var.g_pipe_state == PIPE_DRAIN && _iROB->hasFreeWire (READ) && 
               _iROB->getFront()->getInsID () >= g_var.getSquashSN ()) {
        g_var.g_pipe_state = PIPE_SQUASH_ROB;
        state_switch =  "PIPE_DRAIN -> PIPE_SQUASH_ROB";
        _iROB->updateWireState (READ);
    } else if (g_var.g_pipe_state == PIPE_SQUASH_ROB && _iROB->getTableSize() == 0) {
        g_var.resetSquashSN ();
        g_var.g_pipe_state = PIPE_NORMAL;
        state_switch =  "PIPE_SQUASH_ROB -> PIPE_NORMAL";
        _RF_MGR->squashRenameReg ();
    } else {
        return; /*-- NO STATE CHANGE --*/
    }
    dbg.print (DBG_EXECUTION, "%s: %s (cyc: %d)\n", _stage_name.c_str (), state_switch.c_str(), _clk->now ());
}

void o3_execution::squash () {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "o3_execution Ports Flush", _clk->now ());
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
    INS_ID squashSeqNum = g_var.getSquashSN ();
    _execution_to_scheduler_port->searchNflushPort (squashSeqNum);
}

void o3_execution::regStat () {
    _scheduler_to_execution_port->regStat ();
}
