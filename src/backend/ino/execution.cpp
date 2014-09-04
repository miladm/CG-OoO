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
	: stage (execution_width, stage_name, clk)
{
    _scheduler_to_execution_port = &scheduler_to_execution_port;
    _execution_to_scheduler_port = &execution_to_scheduler_port;
    _execution_to_memory_port = &execution_to_memory_port;
    _iROB = iROB;
    _aluExeUnits = new List<exeUnit*>;
    for (WIDTH i = 0; i < _stage_width; i++) {
        exeUnit* newEU = new exeUnit (1,  _eu_lat._alu_lat, ALU_EU); //TODO make this config better with more EU types
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
    if (ENABLE_SQUASH) squashCtrl ();
    dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "PIPELINE STATE:", g_var.g_pipe_state, _clk->now ());

    /* STAT */
    if (g_var.g_pipe_state != PIPE_NORMAL) s_squash_cycles++;
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;
}

/* WRITE COMPLETE INS - WRITEBACK */
COMPLETE_STATUS execution::completeIns () {
    g_var.setOldSquashSN ();
    for (WIDTH i = 0; i < _stage_width; i++) {
        exeUnit* EU = _aluExeUnits->Nth (i);
        dynInstruction* ins = EU->getEUins ();

        /* CHECKS */
        if (g_var.g_pipe_state == PIPE_FLUSH) break;
        if (ins != NULL && ins->getInsType () == MEM && 
            _execution_to_memory_port->getBuffState () == FULL_BUFF) break;
        if (EU->getEUstate (_clk->now (), true) != COMPLETE_EU) continue;

        /* COMPLETE INS */
        if (ins->getInsType () == MEM) {
            _execution_to_memory_port->pushBack (ins);
            ins->setPipeStage (MEM_ACCESS);
        } else {
            //if (!g_RF_MGR->hasFreeWire (WRITE)) break; //TODO this line is buggy for the EU state
            ins->setPipeStage (COMPLETE);
            g_RF_MGR->writeToRF (ins);
            //g_RF_MGR->updateWireState (WRITE); //TODO put back
        }
        EU->resetEU ();

        /* SQUASH DETECTION */
        if (ins->isOnWrongPath ()) {
            g_var.setSquashSN (ins->getInsID ());
        }
        dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Complete ins", ins->getInsID (), _clk->now ());
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
        if (!g_RF_MGR->isReady (ins) || !g_RF_MGR->canReserveRF (ins)) break; // INO EXE
        exeUnit* EU = _aluExeUnits->Nth (i);
        if (EU->getEUstate (_clk->now (), false) != AVAILABLE_EU) continue;

        /* EXE INS */
        ins = _scheduler_to_execution_port->popFront ();
        g_RF_MGR->reserveRF (ins);
        EU->_eu_timer.setNewTime (_clk->now ());
        EU->setEUins (ins);
        EU->runEU ();
        ins->setPipeStage (EXECUTE);
        forward (ins, EU->_eu_timer.getLatency ());
        dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Execute ins", ins->getInsID (), _clk->now ());

        /* STAT */
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
    _execution_to_scheduler_port->flushPort (squashSeqNum, false);
    _execution_to_memory_port->flushPort (squashSeqNum, false);
}

void execution::regStat () {
    _scheduler_to_execution_port->regStat ();
}
