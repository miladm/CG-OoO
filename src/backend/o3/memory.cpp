/*********************************************************************************
 * memory.cpp
 *********************************************************************************/

#include "memory.h"

o3_memory::o3_memory (port<dynInstruction*>& execution_to_memory_port, 
                      port<dynInstruction*>& memory_to_scheduler_port, 
                      CAMtable<dynInstruction*>* iROB,
	    	          WIDTH memory_width,
                      o3_memManager* LSQ_MGR,
                      o3_rfManager* RF_MGR,
                      sysClock* clk,
	    	          string stage_name) 
	: stage (memory_width, stage_name, clk),
      _mshr(20, 4, 4, clk, "MSHR")
{
    _execution_to_memory_port = &execution_to_memory_port;
    _memory_to_scheduler_port = &memory_to_scheduler_port;
    _iROB = iROB;
    _LSQ_MGR = LSQ_MGR;
    _RF_MGR = RF_MGR;
}

o3_memory::~o3_memory () {}

void o3_memory::doMEMORY () {
    /*-- STAT + DEBUG --*/
    dbg.print (DBG_MEMORY, "** %s: (cyc: %d)\n", _stage_name.c_str (), _clk->now ());
    regStat ();
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /*-- WRITEBACK RESULT --*/
    completeIns ();

    /*-- SQUASH HANDLING --*/
    if (g_var.g_pipe_state == PIPE_FLUSH) { squash (); }

    pipe_stall = memoryImpl ();

    /*-- STAT --*/
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;
}

/*-- WRITE COMPLETE INS - WRITEBACK --*/
void o3_memory::completeIns () {
    /*-- COMPLETE LD INS --*/
    for (WIDTH i = 0; i < _stage_width; i++) {
        /*-- CHECKS --*/
        if (_LSQ_MGR->getTableState (LD_QU) == EMPTY_BUFF) break;
        if (!_LSQ_MGR->hasFreeWire (LD_QU, READ)) break;
        pair<bool, dynInstruction*> p = _LSQ_MGR->hasFinishedIns (LD_QU);
        if (!p.first) break;
        if (!_RF_MGR->hasFreeWire (WRITE)) break;

        /*-- COMPLETE INS --*/
        dynInstruction* finished_ld_ins = p.second;
        Assert (finished_ld_ins != NULL);
        finished_ld_ins->setPipeStage(COMPLETE);
        _LSQ_MGR->completeLd (finished_ld_ins);
        _RF_MGR->completeRegs (finished_ld_ins);
        dbg.print (DBG_MEMORY, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                   "Write Complete mem LD ins", finished_ld_ins->getInsID (), _clk->now ());

        /*-- UPDATE WIRES --*/
        _LSQ_MGR->updateWireState (LD_QU, READ);
        _RF_MGR->updateWireState (WRITE);
    }
}

PIPE_ACTIVITY o3_memory::memoryImpl () {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /*-- ACCESS MEMORY HIERARCHY --*/
    for (WIDTH i = 0; i < _stage_width; i++) {
        /*-- CHECKS --*/
        if (_LSQ_MGR->getTableState (LD_QU) == EMPTY_BUFF) break;
        if (!_LSQ_MGR->hasFreeWire (LD_QU, READ)) break;
        //if (_dcache.getNumAvialablePorts () == 0) break; (TODO)
        if (_mshr.getTableState () == FULL_BUFF || !_mshr.hasFreeWire (WRITE)) break; //TODO only on miss? here or in _LSQ_MGR? = should it be WROTE?

        /*-- MEM ACCESS --*/
        if (!_LSQ_MGR->issueToMem (LD_QU)) break;

        /*-- UPDATE WIRES --*/
        _LSQ_MGR->updateWireState (LD_QU, READ);
        _mshr.updateWireState (WRITE);

        /*-- STAT --*/
        s_ins_cnt++;
        pipe_stall = PIPE_BUSY;
    }

    manageSTbuffer ();

    return pipe_stall;
}

/*-- HANDLE STORE BUFF to CACHE --*/
void o3_memory::manageSTbuffer () {
    for (WIDTH i = 0; i < _stage_width; i++) {
        /*-- CHECKS --*/
        if (_LSQ_MGR->getTableState (ST_QU) == EMPTY_BUFF) break;
        if (!_LSQ_MGR->hasFreeWire (ST_QU, READ)) break;
        if (!_LSQ_MGR->hasCommitSt ()) break;
        //if (_dcache.getNumAvialablePorts () == 0) break; //TODO put this back soon

        /*-- MEM ACCESS FOR STORE --*/
        if (!_LSQ_MGR->issueToMem (ST_QU)) break;

        /*-- UPDATE WIRES --*/
        _LSQ_MGR->updateWireState (ST_QU, READ);
    }

    for (WIDTH i = 0; i < _stage_width; i++) {
        /*-- CHECKS --*/
        if (_LSQ_MGR->getTableState (ST_QU) == EMPTY_BUFF) break;
        if (!_LSQ_MGR->hasFreeWire (ST_QU, READ)) break;

        /*-- DEALLOCATE SQ ENTRY --*/
        _LSQ_MGR->delAfinishedSt ();

        /*-- UPDATE WIRES --*/
        _LSQ_MGR->updateWireState (ST_QU, READ);
    }
}

void o3_memory::squash () {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Memory Ports Flush", _clk->now ());
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
    INS_ID squashSeqNum = g_var.getSquashSN ();
    _memory_to_scheduler_port->flushPort (squashSeqNum);
    _LSQ_MGR->squash (squashSeqNum);
}

void o3_memory::regStat () {
    _execution_to_memory_port->regStat ();
    _LSQ_MGR->regStat ();
}

void o3_memory::manageMSHR () {
    /*-- TODO --*/
}
