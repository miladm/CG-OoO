/*********************************************************************************
 * execution.cpp
 *********************************************************************************/

#include "execution.h"

bb_execution::bb_execution (port<dynInstruction*>& scheduler_to_execution_port, 
                            port<dynInstruction*>& execution_to_scheduler_port, 
                            CAMtable<dynBasicblock*>* bbROB,
	    	                WIDTH execution_width,
                            bb_memManager* LSQ_MGR,
                            bb_grfManager* RF_MGR,
                            sysClock* clk,
	    	                string stage_name) 
	: stage (execution_width, stage_name, clk)
{
    _scheduler_to_execution_port = &scheduler_to_execution_port;
    _execution_to_scheduler_port = &execution_to_scheduler_port;
    _bbROB = bbROB;
    _LSQ_MGR = LSQ_MGR;
    _RF_MGR = RF_MGR;
    _aluExeUnits = new List<exeUnit*>;
    for (WIDTH i = 0; i < _stage_width; i++) {
        exeUnit* newEU = new exeUnit (1,  _eu_lat._alu_lat, ALU_EU); //TODO make this config better with more EU types
        _aluExeUnits->Append (newEU);
    }
}

bb_execution::~bb_execution () {}

void bb_execution::doEXECUTION () {
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
//    squashCtrl ();
    dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
               "PIPELINE STATE:", g_var.g_pipe_state, _clk->now ());

    /*-- STAT --*/
    if (g_var.g_pipe_state != PIPE_NORMAL) s_squash_cycles++;
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;
}

/*-- WRITE COMPLETE INS - WRITEBACK --*/
COMPLETE_STATUS bb_execution::completeIns () {
    g_var.setOldSquashSN ();
    for (WIDTH i = 0; i < _stage_width; i++) {
        exeUnit* EU = _aluExeUnits->Nth (i);
        dynInstruction* ins = EU->getEUins ();
        dynInstruction* violating_ld_ins = NULL;

        /*-- CHECKS --*/
        if (g_var.g_pipe_state == PIPE_FLUSH) break;
        if (EU->getEUstate (_clk->now (), true) != COMPLETE_EU) continue;

        /*-- COMPLETE INS --*/
        if (ins->getInsType () == MEM && ins->getMemType () == LOAD) {
            ins->setPipeStage (MEM_ACCESS);
            _LSQ_MGR->memAddrReady (ins);
            dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                      "Complete load - ins addr", ins->getInsID (), _clk->now ());
        } else if (ins->getInsType () == MEM && ins->getMemType () == STORE) {
            ins->setPipeStage (COMPLETE);
            ins->getBB()->incCompletedInsCntr ();
            _LSQ_MGR->memAddrReady (ins);
            _RF_MGR->completeRegs (ins); //TODO this sould not normally exist. problem with no support for u-ops (create support for both cases)
//            pair<bool, dynInstruction*> p = _LSQ_MGR->isLQviolation (ins);
//            bool is_violation = p.first;
//            violating_ld_ins = p.second;
            /*-- SQUASH DETECTION --*/
//            if (is_violation) { violating_ld_ins->setMemViolation (); }
            dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (),
                       "Complete store - ins addr", ins->getInsID (), _clk->now ());
        } else {
            //if (!_RF_MGR->hasFreeWrPort ()) break; //TODO put this back and clean up the assert for available EU
            ins->setPipeStage (COMPLETE);
            ins->getBB()->incCompletedInsCntr ();
            _RF_MGR->completeRegs (ins);
            //_RF_MGR->updateWireState (WRITE); //TODO put back
            dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (),
                       "Complete ins", ins->getInsID (), _clk->now ());
        }
        EU->resetEU ();

        /*-- SQUASH DETECTION --*/
        if (ins->isMemOrBrViolation ()) {
            g_var.setSquashSN (ins->getInsID ());
            g_var.setSquashType (BP_MISPRED);
        } else if (violating_ld_ins != NULL && violating_ld_ins->isMemOrBrViolation ()) {
            g_var.setSquashSN (violating_ld_ins->getInsID ());
            g_var.setSquashType (MEM_MISPRED);
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
PIPE_ACTIVITY bb_execution::executionImpl () {
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
        forward (ins, EU->_eu_timer.getLatency ());
        dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Execute ins", ins->getInsID (), _clk->now ());

        /*-- STAT --*/
        s_ins_cnt++;
        pipe_stall = PIPE_BUSY;
    }
    return pipe_stall;
}

void bb_execution::forward (dynInstruction* ins, CYCLE eu_latency) {
    if (_execution_to_scheduler_port->getBuffState () == FULL_BUFF) return;
    if (ins->getInsType () != MEM) {
        CYCLE cdb_ready_latency = eu_latency - 1;
        Assert (cdb_ready_latency >= 0);
        _execution_to_scheduler_port->pushBack (ins, cdb_ready_latency);
        dbg.print (DBG_EXECUTION, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Forward wr ops of ins", ins->getInsID (), _clk->now ());
    }
}

void bb_execution::squashCtrl () {
    string state_switch;
    if ((g_var.g_pipe_state == PIPE_NORMAL || 
         g_var.g_pipe_state == PIPE_WAIT_FLUSH || 
         g_var.g_pipe_state == PIPE_FLUSH || 
         g_var.g_pipe_state == PIPE_DRAIN || 
         g_var.g_pipe_state == PIPE_SQUASH_ROB) && 
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
    } else if (g_var.g_pipe_state == PIPE_DRAIN && _bbROB->hasFreeWire (READ) && 
               _bbROB->getFront()->getBBID () >= g_var.g_squash_seq_num) { //TODO this needs a fix
        g_var.g_pipe_state = PIPE_SQUASH_ROB;
        state_switch =  "PIPE_DRAIN -> PIPE_SQUASH_ROB";
        _bbROB->updateWireState (READ);
    } else if (g_var.g_pipe_state == PIPE_SQUASH_ROB && _bbROB->getTableSize () == 0) {
        g_var.resetSquashSN ();
        g_var.g_pipe_state = PIPE_NORMAL;
        state_switch =  "PIPE_SQUASH_ROB -> PIPE_NORMAL";
        _RF_MGR->squashRenameReg ();
    } else {
        return; /*-- No state change --*/
    }
    dbg.print (DBG_EXECUTION, "%s: %s (cyc: %d)\n", _stage_name.c_str (), state_switch.c_str (), _clk->now ());
}

void bb_execution::squash () {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "bb_execution Ports Flush", _clk->now ());
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
    INS_ID squashSeqNum = g_var.getSquashSN ();
    _execution_to_scheduler_port->flushPort (squashSeqNum);
}

void bb_execution::regStat () {
    _scheduler_to_execution_port->regStat ();
}