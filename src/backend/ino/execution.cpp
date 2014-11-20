/*******************************************************************************
 * execution.cpp
 *******************************************************************************/

#include "execution.h"

execution::execution (port<dynInstruction*>& scheduler_to_execution_port, 
                      port<dynInstruction*>& execution_to_scheduler_port, 
                      port<dynInstruction*>& execution_to_memory_port, 
                      CAMtable<dynInstruction*>* iROB,
	    	          WIDTH execution_width,
                      sysClock* clk,
	    	          string stage_name) 
	: stage (execution_width, stage_name, clk),
      s_pipe_state_hist (g_stats.newScalarHistStat (NUM_PIPE_STATE, stage_name, "pipe_state_cnt", "Number of cycles in each squash stage", 0, PRINT_ZERO)),
      s_eu_busy_state_hist (g_stats.newScalarHistStat ((LENGTH) execution_width, stage_name, "eu_busy_state_hist", "Number of cycles execution unit is busy", 0, PRINT_ZERO)),
      s_pipe_state_hist_rat (g_stats.newRatioHistStat (clk->getStatObj (), (LENGTH) NUM_PIPE_STATE, stage_name, "pipe_state_hist_rat", "Ratio of cycles in each squash stage / total cycles", 0, PRINT_ZERO)),
      s_br_mispred_cnt (g_stats.newScalarStat (stage_name, "br_mispred_cnt", "Number of branch mis-predict events", 0, PRINT_ZERO))
{
    /*-- CONFIG OBJS --*/
    const YAML::Node& root = g_cfg->_root["cpu"]["backend"];

    _scheduler_to_execution_port = &scheduler_to_execution_port;
    _execution_to_scheduler_port = &execution_to_scheduler_port;
    _execution_to_memory_port = &execution_to_memory_port;
    _iROB = iROB;
    _aluExeUnits = new List<exeUnit*>;
    for (WIDTH i = 0; i < _stage_width; i++) {
        exeUnit* newEU = new exeUnit (1, _eu_lat._alu_lat, ALU_EU, root["eu"]["alu"]); //TODO make this config better with more EU types
        _aluExeUnits->Append (newEU);
    }
}

execution::~execution () {}

void execution::doEXECUTION () {
    dbg.print (DBG_EXECUTION, "%s: (cyc: %d)\n", _stage_name.c_str (), _clk->now ());
    /* STAT */
    regStat ();
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /* WRITEBACK RESULT */
    COMPLETE_STATUS cmpl_status;
    cmpl_status = completeIns ();

    /* EXECUTE INS */
    if (cmpl_status == COMPLETE_NORMAL) {
        pipe_stall = executionImpl ();
    }

    /* SQUASH CONTROL */
    if (g_cfg->isEnSquash ()) squashCtrl ();
    dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "PIPELINE STATE:", g_var.g_pipe_state, _clk->now ());

    /* STAT */
    s_pipe_state_hist[g_var.g_pipe_state]++;
    s_pipe_state_hist_rat[g_var.g_pipe_state]++;
    if (g_var.g_pipe_state != PIPE_NORMAL) s_squash_cycles++;
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;
    for (WIDTH i = 0; i < _stage_width; i++) {
        exeUnit* EU = _aluExeUnits->Nth (i);
        if (EU->getEUstate (_clk->now (), false) != AVAILABLE_EU) 
            s_eu_busy_state_hist[i]++;
    }
}

/* WRITE COMPLETE INS - WRITEBACK */
COMPLETE_STATUS execution::completeIns () {
    g_var.setOldSquashSN ();
    dynInstruction* badIns = NULL;
    INS_ID badInsID = 0; //i.e. FIRST_INS_ID - 1

    for (WIDTH i = 0; i < _stage_width; i++) {
        exeUnit* EU = _aluExeUnits->Nth (i);
        dynInstruction* ins = EU->getEUins ();

        /* CHECKS */
        if (g_var.g_pipe_state == PIPE_FLUSH) break;
        if (ins == NULL) continue;
        if (ins->getInsType () == MEM && 
            _execution_to_memory_port->getBuffState () == FULL_BUFF) break;
        if (!(ins->getInsType () == MEM) &&
            !g_RF_MGR->hasFreeWire (WRITE, ins->getNumWrAR ())) continue;
        if (EU->getEUstate (_clk->now (), true) != COMPLETE_EU) continue;

        /* COMPLETE INS */
        if (ins->getInsType () == MEM) {
            _execution_to_memory_port->pushBack (ins);
            ins->setPipeStage (MEM_ACCESS);
            //TODO handle writeRF for STORE ins here (like O3 & BB)
        } else {
            ins->setPipeStage (COMPLETE);
            g_RF_MGR->writeToRF (ins);
            g_RF_MGR->updateWireState (WRITE, ins->getNumWrAR ());
            _iROB->ramAccess (); /* NOTIFY THE INSTRUCTION IS COMPLETE */
        }
        EU->resetEU ();

        /* SQUASH DETECTION */
        if (ins->isOnWrongPath () && 
            g_var.g_pipe_state == PIPE_NORMAL &&
            (ins->getInsID () < badInsID || badInsID == 0)) {
            badIns = ins;
            badInsID = ins->getInsID ();
        }
        dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Complete ins", ins->getInsID (), _clk->now ());
    }

    /*-- SQUASH HANDLING --*/
    if (badIns != NULL) {
        g_var.setSquashSN (badIns->getInsID ());
        s_br_mispred_cnt++;
        dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                "Squash SN: ", badIns->getInsID (), _clk->now ());
    }

    if (g_var.isSpeculationViolation ()) {
        dbg.print (DBG_EXECUTION, "%s: %s %llu %s (cyc: %d)\n", _stage_name.c_str (), "Ins on Wrong Path (ins: ", g_var.getSquashSN (), ")", _clk->now ());
        return COMPLETE_SQUASH;
    }
    dbg.print (DBG_EXECUTION, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Ins on Right Path", _clk->now ());
    return COMPLETE_NORMAL;
}

/* EXECUTE INS */
PIPE_ACTIVITY execution::executionImpl () {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    for (WIDTH i = 0; i < _stage_width; i++) {
        /* CHECKS */
        if (g_var.g_pipe_state == PIPE_WAIT_FLUSH || g_var.g_pipe_state == PIPE_FLUSH) break;
        if (_scheduler_to_execution_port->getBuffState () == EMPTY_BUFF) break;
        if (!_scheduler_to_execution_port->isReady ()) break;
        dynInstruction* ins = _scheduler_to_execution_port->getFront ();
        exeUnit* EU = _aluExeUnits->Nth (i);
        if (EU->getEUstate (_clk->now (), false) != AVAILABLE_EU) continue;

        /* EXE INS */
        ins = _scheduler_to_execution_port->popFront ();
        EU->_eu_timer.setNewTime (_clk->now ());
        EU->setEUins (ins);
        EU->runEU ();
        ins->setPipeStage (EXECUTE);
        if (g_cfg->isEnEuFwd ()) forward (ins, EU->_eu_timer.getLatency ());
        dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Execute ins", ins->getInsID (), _clk->now ());

        /* STAT */
        s_ipc++;
        s_ins_cnt++;
        pipe_stall = PIPE_BUSY;
    }
    return pipe_stall;
}

void execution::forward (dynInstruction* ins, CYCLE eu_latency) {
    if (_execution_to_scheduler_port->getBuffState () == FULL_BUFF) return;
    if (ins->getInsType () != MEM) {
        CYCLE cdb_ready_latency = eu_latency - 1;
        Assert (cdb_ready_latency >= 0);
        _execution_to_scheduler_port->pushBack (ins, cdb_ready_latency);
        dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Forward wr ops of ins", ins->getInsID (), _clk->now ());
    }
}

void execution::squashCtrl () {
    string state_switch;
    if ((g_var.g_pipe_state == PIPE_NORMAL || g_var.g_pipe_state == PIPE_SQUASH_ROB) && 
        g_var.g_squash_seq_num != g_var.g_old_squash_seq_num) {
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
               _iROB->getFront()->getInsID () >= g_var.g_squash_seq_num) {
        g_var.g_pipe_state = PIPE_SQUASH_ROB;
        _iROB->updateWireState (READ);
        state_switch =  "PIPE_DRAIN -> PIPE_SQUASH_ROB";
    } else if (g_var.g_pipe_state == PIPE_SQUASH_ROB && _iROB->getTableSize() == 0) {
        g_var.resetSquashSN ();
        g_var.g_pipe_state = PIPE_NORMAL;
        state_switch =  "PIPE_SQUASH_ROB -> PIPE_NORMAL";
        g_RF_MGR->resetRF ();
    } else {
        return; /* No state change */
    }
    dbg.print (DBG_EXECUTION, "%s: %s (cyc: %d)\n", _stage_name.c_str (), state_switch.c_str(), _clk->now ());
}

void execution::squash () {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Execution Ports Flush", _clk->now ());
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
    INS_ID squashSeqNum = g_var.getSquashSN ();
    _execution_to_scheduler_port->searchNflushPort (squashSeqNum);
    _execution_to_memory_port->flushPort (squashSeqNum);
}

void execution::regStat () {
    _scheduler_to_execution_port->regStat ();
}
