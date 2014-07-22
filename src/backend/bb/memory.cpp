/*********************************************************************************
 * memory.cpp
 *********************************************************************************/

#include "memory.h"

o3_memory::o3_memory (port<dynInstruction*>& execution_to_memory_port, 
                      port<dynInstruction*>& memory_to_scheduler_port, 
                      CAMtable<dynInstruction*>* iROB,
	    	          WIDTH memory_width,
                      sysClock* clk,
	    	          string stage_name) 
	: stage (memory_width, stage_name, clk),
      _mem_buff(20, 1, clk, "mem_buff"),
      _mshr(20, 4, 4, clk, "MSHR"),
      _st_buff(20, 4, 4, clk, "stBuff"),
      s_cache_miss_cnt (g_stats.newScalarStat (stage_name, "cache_miss_cnt", "Number of cache misses", 0, PRINT_ZERO)),
      s_cache_hit_cnt  (g_stats.newScalarStat (stage_name, "cache_hit_cnt", "Number of cache hits", 0, PRINT_ZERO)),
      s_ld_miss_cnt (g_stats.newScalarStat (stage_name, "ld_miss_cnt", "Number of load misses", 0, PRINT_ZERO)),
      s_ld_hit_cnt  (g_stats.newScalarStat (stage_name, "ld_hit_cnt", "Number of load hits", 0, PRINT_ZERO)),
      s_st_miss_cnt (g_stats.newScalarStat (stage_name, "st_miss_cnt", "Number of store misses", 0, PRINT_ZERO)),
      s_st_hit_cnt  (g_stats.newScalarStat (stage_name, "st_hit_cnt", "Number of store hits", 0, PRINT_ZERO))
{
    _execution_to_memory_port = &execution_to_memory_port;
    _memory_to_scheduler_port = &memory_to_scheduler_port;
    _iROB = iROB;
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
        if (g_LSQ_MGR->getTableState (LD_QU) == EMPTY_BUFF) break;
        if (!g_LSQ_MGR->hasFreeWire (LD_QU, READ)) break;
        pair<bool, dynInstruction*> p = g_LSQ_MGR->hasFinishedIns (LD_QU);
        if (!p.first) break;
        if (!g_GRF_MGR->hasFreeWire (WRITE)) break;

        /*-- COMPLETE INS --*/
        dynInstruction* finished_ld_ins = p.second;
        Assert (finished_ld_ins != NULL);
        finished_ld_ins->setPipeStage(COMPLETE);
        g_LSQ_MGR->completeLd (finished_ld_ins);
        g_GRF_MGR->completeRegs (finished_ld_ins);
        dbg.print (DBG_MEMORY, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                   "Write Complete mem LD ins", finished_ld_ins->getInsID (), _clk->now ());

        /*-- UPDATE WIRES --*/
        g_LSQ_MGR->updateWireState (LD_QU, READ);
        g_GRF_MGR->updateWireState (WRITE);
    }
}

PIPE_ACTIVITY o3_memory::memoryImpl () {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /*-- ACCESS MEMORY HIERARCHY --*/
    for (WIDTH i = 0; i < _stage_width; i++) {
        /*-- CHECKS --*/
        if (g_LSQ_MGR->getTableState (LD_QU) == EMPTY_BUFF) break;
        if (!g_LSQ_MGR->hasFreeWire (LD_QU, READ)) break;
        //if (_dcache.getNumAvialablePorts () == 0) break; (TODO)
        if (_mshr.getTableState () == FULL_BUFF || !_mshr.hasFreeWire (WRITE)) break; //TODO only on miss? here or in g_LSQ_MGR? = should it be WROTE?

        /*-- MEM ACCESS --*/
        if (!g_LSQ_MGR->issueToMem (LD_QU)) break;

        /*-- UPDATE WIRES --*/
        g_LSQ_MGR->updateWireState (LD_QU, READ);
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
        if (g_LSQ_MGR->getTableState (ST_QU) == EMPTY_BUFF) break;
        if (!g_LSQ_MGR->hasFreeWire (ST_QU, READ)) break;
        if (!g_LSQ_MGR->hasCommitSt ()) break;
        //if (_dcache.getNumAvialablePorts () == 0) break; //TODO put this back soon

        /*-- MEM ACCESS FOR STORE --*/
        if (!g_LSQ_MGR->issueToMem (ST_QU)) break;

        /*-- UPDATE WIRES --*/
        g_LSQ_MGR->updateWireState (ST_QU, READ);
    }

    for (WIDTH i = 0; i < _stage_width; i++) {
        /*-- CHECKS --*/
        if (g_LSQ_MGR->getTableState (ST_QU) == EMPTY_BUFF) break;
        if (!g_LSQ_MGR->hasFreeWire (ST_QU, READ)) break;

        /*-- DEALLOCATE SQ ENTRY --*/
        g_LSQ_MGR->delAfinishedSt ();

        /*-- UPDATE WIRES --*/
        g_LSQ_MGR->updateWireState (ST_QU, READ);
    }
}

void o3_memory::squash () {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Memory Ports Flush", _clk->now ());
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
    INS_ID squashSeqNum = g_var.getSquashSN ();
    _memory_to_scheduler_port->flushPort (squashSeqNum);
    _mem_buff.flushPort (squashSeqNum);
    g_LSQ_MGR->squash (squashSeqNum);
}

void o3_memory::regStat () {
    _execution_to_memory_port->regStat ();
}

void o3_memory::manageMSHR () {
    /*-- TODO --*/
}
