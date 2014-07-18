/*******************************************************************************
 * memory.cpp
 *******************************************************************************/

#include "memory.h"

memory::memory (port<dynInstruction*>& execution_to_memory_port, 
                port<dynInstruction*>& memory_to_scheduler_port, 
                CAMtable<dynInstruction*>* iROB,
	    	    WIDTH memory_width,
	    	    string stage_name) 
	: stage (memory_width, stage_name),
      _mem_buff(20, 1, "mem_buff"),
      _mshr(20, 4, 4, "MSHR"),
      _st_buff(20, 4, 4, "stBuff"),
      _L1 (1, 64, 32768),
      _L2 (1, 64, 2097152),
      _L3 (1, 64, 8388608),
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

memory::~memory () {}

void memory::doMEMORY (sysClock& clk) {
    dbg.print (DBG_MEMORY, "%s: (cyc: %d)\n", _stage_name.c_str (), clk.now ());
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
void memory::completeIns (sysClock& clk) {
    for (WIDTH i = 0; i < _stage_width; i++) {
        /* CHECKS */
        if (_mem_buff.getBuffState (clk.now ()) == EMPTY_BUFF) break;
        if (!_mem_buff.isReady (clk.now ())) break;
        if (!g_RF_MGR.hasFreeWrPort (clk.now ())) break; //TODO too conservatice - only for load ins

        /* COMPLETE INS */
        dynInstruction* mem_ins = _mem_buff.popFront (clk.now ());
        mem_ins->setPipeStage(COMPLETE);
        // FORWARD DATA //_memory_to_schedule_port->pushBack(mem_ins, clk.now ());
        g_RF_MGR.writeToRF (mem_ins);
        dbg.print (DBG_MEMORY, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Write Complete mem ins", mem_ins->getInsID (), clk.now ());
    }
}

PIPE_ACTIVITY memory::memoryImpl (sysClock& clk) {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /* ACCESS MEMORY HIERARCHY */
    for (WIDTH i = 0; i < _stage_width; i++) {
        /* CHECKS */
        if (_execution_to_memory_port->getBuffState (clk.now ()) == EMPTY_BUFF) break;
        if (!_execution_to_memory_port->isReady (clk.now ())) break;
        //if (_dcache.getNumAvialablePorts () == 0) break; (TODO)
        if (_mem_buff.getBuffState (clk.now ()) == FULL_BUFF) break;
        dynInstruction* mem_ins = _execution_to_memory_port->getFront ();
        if (mem_ins->getMemType () == STORE && 
           (g_var.g_pipe_state == PIPE_WAIT_FLUSH || g_var.g_pipe_state == PIPE_FLUSH)) break;
        if (mem_ins->getMemType () == STORE && 
           (_st_buff.getTableState () == FULL_BUFF || !_st_buff.hasFreeWrPort (clk.now ()))) break;
        if (mem_ins->getMemType () == LOAD &&
           (_mshr.getTableState () == FULL_BUFF || !_mshr.hasFreeWrPort (clk.now ()))) break; //TODO only on miss?

        /* MEM ACCESS */
        mem_ins = _execution_to_memory_port->popFront (clk.now ());
        CYCLE axes_lat;
        if (mem_ins->getMemType () == STORE) {
            Assert (mem_ins->isOnWrongPath () == false);
            _st_buff.pushBack (mem_ins);
            axes_lat = g_eu_lat._st_buff_lat;
            dbg.print (DBG_MEMORY, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Add to st_buff ins", mem_ins->getInsID (), clk.now ());
        } else {
            axes_lat = (CYCLE) cacheCtrl (READ,  //stIns->getMemType (), TODO fix this line
                                           mem_ins->getMemAddr (),
                                           mem_ins->getMemAxesSize(),
                                           &_L1, &_L2, &_L3);
#ifdef ASSERTION
            Assert(axes_lat > 0);
#endif
            forward (mem_ins, axes_lat, clk);
            (axes_lat > L1_LATENCY) ? s_ld_miss_cnt++ : s_ld_hit_cnt++;
            (axes_lat > L1_LATENCY) ? s_cache_miss_cnt++ : s_cache_hit_cnt++;
        }
        _mem_buff.pushBack(mem_ins, clk.now (), axes_lat); //CAM array - check for size limits (TODO)
        //mem_ins->setPipeStage(MEM_ACCESS); happens in execute.cpp - remove from here? (TODO)
        dbg.print (DBG_MEMORY, "%s: %s %llu %s %u (cyc: %d)\n", _stage_name.c_str (), "Add to mem_buff ins", mem_ins->getInsID (), "with lat", axes_lat, clk.now ());

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
void memory::forward (dynInstruction* ins, CYCLE mem_latency, sysClock& clk) {
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
void memory::manageSTbuffer (sysClock& clk) {
    while (true) {
        /* CHECKS */
        if (_st_buff.getTableState () == EMPTY_BUFF || !_st_buff.hasFreeRdPort (clk.now ())) break;
        //if (_dcache.getNumAvialablePorts () == 0) break;

        /* MEM ACCESS */
        dynInstruction* st_ins = _st_buff.popFront ();
        Assert (st_ins->getNumRdAR () == 0 && "instruction must have been ready long ago!");
        CYCLE lat = (CYCLE) cacheCtrl (WRITE, //st_ins->getMemType (),
                                       st_ins->getMemAddr (),
                                       st_ins->getMemAxesSize(), 
                                       &_L1, &_L2, &_L3);
        (lat > L1_LATENCY) ? s_st_miss_cnt++ : s_st_hit_cnt++;
        (lat > L1_LATENCY) ? s_cache_miss_cnt++ : s_cache_hit_cnt++;
        dbg.print (DBG_MEMORY, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Store to cach ins", st_ins->getInsID (), clk.now ());
    }
}

void memory::squash (sysClock& clk) {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Memory Ports Flush", clk.now ());
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
    INS_ID squashSeqNum = g_var.getSquashSN ();
    _memory_to_scheduler_port->flushPort (squashSeqNum, clk.now ());
    _mem_buff.flushPort (squashSeqNum, clk.now ());
    //_st_buff.flushTable ();
}

void memory::regStat (sysClock& clk) {
    _execution_to_memory_port->regStat (clk.now ());
}

void memory::manageMSHR () {
    /* TODO */
}
