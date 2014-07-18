/*******************************************************************************
 * memory.cpp
 *******************************************************************************/

#include "memory.h"

o3_memory::o3_memory (port<dynInstruction*>& execution_to_memory_port, 
                port<dynInstruction*>& memory_to_scheduler_port, 
                CAMtable<dynInstruction*>* iROB,
	    	    WIDTH memory_width,
	    	    string stage_name) 
	: stage (memory_width, stage_name),
      _mem_buff(20, 1, "mem_buff"),
      _mshr(20, 4, 4, "MSHR"),
      _st_buff(20, 4, 4, "stBuff"),
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

void o3_memory::doMEMORY (sysClock& clk) {
    dbg.print (DBG_MEMORY, "** %s: (cyc: %d)\n", _stage_name.c_str (), clk.now ());
    /* STAT */
    regStat (clk);
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /* WRITEBACK RESULT */
    completeIns (clk);

    /* SQUASH HANDLING */
    if (g_var.g_pipe_state == PIPE_FLUSH) { squash (clk); }

    pipe_stall = memoryImpl (clk);

    /* STAT */
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;
}

/* WRITE COMPLETE INS - WRITEBACK */
void o3_memory::completeIns (sysClock& clk) {

    /* COMPLETE LD INS */
    for (WIDTH i = 0; i < _stage_width; i++) {
        /* CHECKS */
        if (g_LSQ_MGR.getTableState (LD_QU) == EMPTY_BUFF) break;
        if (!g_LSQ_MGR.hasFreeRdPort (LD_QU, clk.now ())) break;
        pair<bool, dynInstruction*> p = g_LSQ_MGR.hasFinishedIns (LD_QU, clk.now ());
        if (!p.first) break;
        dynInstruction* finished_ld_ins = p.second;
        //if (_mem_buff.getBuffState (clk.now ()) == EMPTY_BUFF) break;
        //if (!_mem_buff.isReady (clk.now ())) break;
        if (!g_GRF_MGR.hasFreeWrPort (clk.now ())) break; //TODO too conservatice - only for load ins (both ino and o3)

        /* COMPLETE INS */
        //dynInstruction* finished_ld_ins = _mem_buff.popFront (clk.now ());
        Assert (finished_ld_ins != NULL);
        finished_ld_ins->setPipeStage(COMPLETE);
        g_LSQ_MGR.completeLd (finished_ld_ins);
        g_GRF_MGR.completeRegs (finished_ld_ins);
        dbg.print (DBG_MEMORY, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Write Complete mem LD ins", finished_ld_ins->getInsID (), clk.now ());
    }

//  /* COMPLETE ST INS */
//  /* LSQ MIS-SPECULATION DETECTION */
//  for (WIDTH i = 0; i < _stage_width; i++) {
//      /* CHECKS */
//      if (g_LSQ_MGR.getTableState (ST_QU) == EMPTY_BUFF) break;
//      if (!g_LSQ_MGR.hasFreeRdPort (ST_QU, clk.now ())) break;
//      pair<bool, dynInstruction*> p = g_LSQ_MGR.hasFinishedIns (ST_QU, clk.now ());
//      if (!p.first) break;
//      delete p.second;
//      dynInstruction* complete_st_ins = p.second;

//      /* COMPLETE INS */
//      //dynInstruction* complete_st_ins = _mem_buff.popFront (clk.now ());
//      //TODO ST reaches here with one cycle delay!
//      complete_st_ins->setPipeStage(COMPLETE); //TODO should it be here? if yes, then SQ must also take note of complete state of its ST entry
//      dynInstruction* violating_ld_ins = NULL;
//      if (g_LSQ_MGR.isLQviolation (complete_st_ins, violating_ld_ins)) {
//          cout << "SQUASH LSQ\n";
//          //g_LSQ_MGR.squash (violating_ld_ins);
//          //TODO flush the pipeline
//      }
//      dbg.print (DBG_MEMORY, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Write Complete mem LD ins", complete_st_ins->getInsID (), clk.now ());
//  }
}

PIPE_ACTIVITY o3_memory::memoryImpl (sysClock& clk) {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /* ACCESS MEMORY HIERARCHY */
    for (WIDTH i = 0; i < _stage_width; i++) {
        /* CHECKS */
        //if (_execution_to_memory_port->getBuffState (clk.now ()) == EMPTY_BUFF) break;
        //if (!_execution_to_memory_port->isReady (clk.now ())) break;
        if (g_LSQ_MGR.getTableState (LD_QU) == EMPTY_BUFF) break;
        if (!g_LSQ_MGR.hasFreeRdPort (LD_QU, clk.now ())) break; //TODO it is wrong taht despite the results of future if statements, this state machine workd - fix it
        //if (_dcache.getNumAvialablePorts () == 0) break; (TODO)
        //if (_mem_buff.getBuffState (clk.now ()) == FULL_BUFF) break;
        //dynInstruction* mem_ins = _execution_to_memory_port->getFront ();
        //if (mem_ins->getMemType () == STORE && 
        //   (g_var.g_pipe_state == PIPE_WAIT_FLUSH || g_var.g_pipe_state == PIPE_FLUSH)) break;
        //if (mem_ins->getMemType () == STORE && 
        //   (_st_buff.getTableState () == FULL_BUFF || !_st_buff.hasFreeWrPort (clk.now ()))) break;
        if (_mshr.getTableState () == FULL_BUFF || !_mshr.hasFreeWrPort (clk.now ())) break; //TODO only on miss? here or in g_LSQ_MGR?

        /* MEM ACCESS */
        if (!g_LSQ_MGR.issueToMem (LD_QU, clk.now ())) break;
        //dbg.print (DBG_MEMORY, "%s: %s %llu %s %u (cyc: %d)\n", _stage_name.c_str (), "Add to mem_buff ins", mem_ins->getInsID (), "with lat", axes_lat, clk.now ());

        /* STAT */
        s_ins_cnt++;
        pipe_stall = PIPE_BUSY;
    }

    manageSTbuffer (clk);

    return pipe_stall;
}

/* FORWARD DATA 
 * NOTE: this function is not sorted based on the order in which LOAD ops
 * return data. The port must be searched for finding the data that is being
 * forwarded every cycle (in the scheduler)
 */
void o3_memory::forward (dynInstruction* ins, CYCLE mem_latency, sysClock& clk) {
#ifdef ASSERTION
    Assert (ins->getInsType () == MEM && ins->getMemType() == LOAD);
#endif
    if (_memory_to_scheduler_port->getBuffState (clk.now ()) == FULL_BUFF) return;
    CYCLE cdb_ready_latency = mem_latency - 1;
    Assert (cdb_ready_latency >= 0);
    _memory_to_scheduler_port->pushBack (ins, clk.now (), cdb_ready_latency);
    dbg.print (DBG_MEMORY, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Forward wr ops of ins", ins->getInsID (), clk.now ());
}

/* HANDLE STORE BUFF to CACHE */
void o3_memory::manageSTbuffer (sysClock& clk) {
    for (WIDTH i = 0; i < _stage_width; i++) {
        /* CHECKS */
        if (g_LSQ_MGR.getTableState (ST_QU) == EMPTY_BUFF) break;
        if (!g_LSQ_MGR.hasFreeRdPort (ST_QU, clk.now ())) break;
        if (!g_LSQ_MGR.hasCommitSt ()) break;
        //if (_st_buff.getTableState () == EMPTY_BUFF || !_st_buff.hasFreeRdPort (clk.now ())) break;
        //if (_dcache.getNumAvialablePorts () == 0) break; //TODO put this back soon

        /* MEM ACCESS FOR STORE */
        if (!g_LSQ_MGR.issueToMem (ST_QU, clk.now ())) break; //TODO of not successful, don't waste port BW and break
    }

    //TODO will this loop ever get a chance to not break?
    for (WIDTH i = 0; i < _stage_width; i++) {
        /* CHECKS */
        if (g_LSQ_MGR.getTableState (ST_QU) == EMPTY_BUFF) break;
        //if (!g_LSQ_MGR.hasFreeRdPort (ST_QU, clk.now ())) break;

        /* DEALLOCATE SQ ENTRY */
        g_LSQ_MGR.delAfinishedSt (clk.now ());
    }
}

void o3_memory::squash (sysClock& clk) {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Memory Ports Flush", clk.now ());
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
    INS_ID squashSeqNum = g_var.getSquashSN ();
    _memory_to_scheduler_port->flushPort (squashSeqNum, clk.now ());
    _mem_buff.flushPort (squashSeqNum, clk.now ());
    g_LSQ_MGR.squash (squashSeqNum);
    //_st_buff.flushTable ();
}

void o3_memory::regStat (sysClock& clk) {
    _execution_to_memory_port->regStat (clk.now ());
}

void o3_memory::manageMSHR () {
    /* TODO */
}
