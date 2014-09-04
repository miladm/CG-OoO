/*******************************************************************************
 * memManager.cpp
 ******************************************************************************/

#include "memManager.h"

o3_memManager::o3_memManager (port<dynInstruction*>& memory_to_scheduler_port,
                              sysClock* clk, 
                              string lsq_name = "o3_memManager") 
    : unit (lsq_name, clk),
      _L1 (1, 64, 32768),
      _L2 (1, 64, 2097152),
      _L3 (1, 64, 8388608),
      _LQ (LQ_SIZE, 8, 4, clk, "LQtable"),
      _SQ (SQ_SIZE, 8, 4, clk, "SQtable"),
      s_cache_hit_cnt  (g_stats.newScalarStat (lsq_name, "cache_hit_cnt", "Number of cache hits", 0, PRINT_ZERO)),
      s_cache_miss_cnt (g_stats.newScalarStat (lsq_name, "cache_miss_cnt", "Number of cache misses", 0, PRINT_ZERO)),
      s_ld_hit_cnt  (g_stats.newScalarStat (lsq_name, "ld_hit_cnt", "Number of load hits", 0, PRINT_ZERO)),
      s_ld_miss_cnt (g_stats.newScalarStat (lsq_name, "ld_miss_cnt", "Number of load misses", 0, PRINT_ZERO)),
      s_st_miss_cnt (g_stats.newScalarStat (lsq_name, "st_miss_cnt", "Number of store misses", 0, PRINT_ZERO)),
      s_st_hit_cnt  (g_stats.newScalarStat (lsq_name, "st_hit_cnt", "Number of store hits", 0, PRINT_ZERO))
{ 
    _memory_to_scheduler_port = &memory_to_scheduler_port;
}

o3_memManager::~o3_memManager () {}

/* ***************** *
 *      LSQ FUNC     *
 * ***************** */
BUFF_STATE o3_memManager::getTableState (LSQ_ID lsq_id) {
    if (lsq_id == LD_QU) {
        return _LQ.getTableState ();
    } else {
        return _SQ.getTableState ();
    }
}

bool o3_memManager::hasFreeWire (LSQ_ID lsq_id, AXES_TYPE axes_type) {
    if (lsq_id == LD_QU) {
        return _LQ.hasFreeWire (axes_type);
    } else {
        return _SQ.hasFreeWire (axes_type);
    }
}

void o3_memManager::updateWireState (LSQ_ID lsq_id, AXES_TYPE axes_type) {
    if (lsq_id == LD_QU) {
        _LQ.updateWireState (axes_type);
    } else {
        _SQ.updateWireState (axes_type);
    }
}

void o3_memManager::pushBack (dynInstruction *ins) {
    Assert (ins->getInsType () == MEM);
    if (ins->getMemType () == LOAD) {
        Assert (_LQ.getTableState () != FULL_BUFF);
        _LQ.pushBack (ins);
        ins->setLQstate (LQ_ADDR_WAIT);
    } else {
        Assert (_SQ.getTableState () != FULL_BUFF);
        _SQ.pushBack (ins);
        ins->setSQstate (SQ_ADDR_WAIT);
    }
}

void o3_memManager::memAddrReady (dynInstruction* ins) {
    if (ins->getMemType () == LOAD) {
        ins->setLQstate (LQ_PENDING_CACHE_DISPATCH);
    } else {
        ins->setSQstate (SQ_COMPLETE);
    }
}

bool o3_memManager::issueToMem (LSQ_ID lsq_id) {
    //TODO take all this block to lsqManager.cpp
    CYCLE axes_lat;
    dynInstruction* mem_ins;
    if (lsq_id == LD_QU) {
        mem_ins = _LQ.findPendingMemIns (LD_QU);
        if (mem_ins == NULL) return false; /* NOTHING ISSUED */
        axes_lat = getAxesLatency (mem_ins);
        _LQ.setTimer (mem_ins, axes_lat);
        forward (mem_ins, axes_lat);
        (axes_lat > L1_LATENCY) ? s_ld_miss_cnt++ : s_ld_hit_cnt++;
    } else {
        mem_ins = _SQ.findPendingMemIns (ST_QU);
        if (ENABLE_SQUASH)
            Assert (mem_ins->isMemOrBrViolation() == false);
        if (mem_ins == NULL) return false; /* NOTHING ISSUED */
        mem_ins->setSQstate (SQ_CACHE_DISPATCH);
        axes_lat = (CYCLE) cacheCtrl (WRITE,  //stIns->getMemType (), TODO fix this line
                mem_ins->getMemAddr (),
                mem_ins->getMemAxesSize(),
                &_L1, &_L2, &_L3);
        _SQ.setTimer (mem_ins, axes_lat);
        (axes_lat > L1_LATENCY) ? s_st_miss_cnt++ : s_st_hit_cnt++;
    }
    (axes_lat > L1_LATENCY) ? s_cache_miss_cnt++ : s_cache_hit_cnt++;
#ifdef ASSERTION
    Assert(axes_lat > 0);
#endif
    return true;
}

CYCLE o3_memManager::getAxesLatency (dynInstruction* mem_ins) {
    if (hasStToAddr (mem_ins->getMemAddr (), mem_ins->getInsID ())) {
        mem_ins->setLQstate (LQ_FWD_FROM_SQ);
        return g_eu_lat._st_buff_lat;
    //} else if () { /*TODO for MSHR */
    } else {
        mem_ins->setLQstate (LQ_CACHE_WAIT);
        return (CYCLE) cacheCtrl (READ,  //stIns->getMemType (), TODO fix this line
                mem_ins->getMemAddr (),
                mem_ins->getMemAxesSize(),
                &_L1, &_L2, &_L3);
    }
}

bool o3_memManager::commit (dynInstruction* ins) {
    Assert (ins->getInsType () == MEM);
    if (ins->getMemType () == LOAD) {
        if (_LQ.getTableState () == EMPTY_BUFF) return false;
        if (!_LQ.hasFreeWire (READ)) return false;
        dynInstruction* ld_ins = _LQ.popFront ();
#ifdef ASSERTION
        Assert (ins->getLQstate () == LQ_COMPLETE);
        Assert (ld_ins->getInsID () == ins->getInsID ());
#endif
        _LQ.updateWireState (READ);
        dbg.print (DBG_MEMORY, "%s: %s %llu\n", _c_name.c_str (), "Commiting LD:", ins->getInsID ());
    } else {
        Assert (ins->getSQstate () == SQ_COMPLETE);
        ins->setSQstate (SQ_COMMIT);
        dbg.print (DBG_MEMORY, "%s: %s %llu\n", _c_name.c_str (), "Commiting ST:", ins->getInsID ());
    }
    return true;
}

void o3_memManager::squash (INS_ID squash_seq_num) {
    _LQ.squash (squash_seq_num);
    _SQ.squash (squash_seq_num);
}

/* ***************** *
 * STORE QUEUE FUNC  *
 * ***************** */
bool o3_memManager::hasCommitSt () {
    return _SQ.hasCommit ();
}

/*-- DELETE ONE INS WITH FINISHED MEM STORE --*/
void o3_memManager::delAfinishedSt () {
    _SQ.delFinishedMemAxes ();
}

pair<bool, dynInstruction*> o3_memManager::hasFinishedIns (LSQ_ID lsq_id) {
    if (lsq_id == LD_QU) {
        return _LQ.hasFinishedIns (lsq_id);
    } else {
        return _SQ.hasFinishedIns (lsq_id);
    }
}

bool o3_memManager::hasStToAddr (ADDRS mem_addr, INS_ID ins_seq_num) {
    return _SQ.hasMemAddr (mem_addr, ins_seq_num);
}

pair<bool, dynInstruction*> o3_memManager::isLQviolation (dynInstruction* st_ins) {
#ifdef ASSERTION
    Assert (st_ins->getMemType () == STORE);
    Assert (st_ins->getSQstate () == SQ_COMPLETE);
#endif
    INS_ID WAW_st_ins_sn = _SQ.hasAnyCompleteStFromAddr (st_ins->getMemAddr (), st_ins->getInsID ());
    pair<bool, dynInstruction*> violation = _LQ.hasAnyCompleteLdFromAddr (st_ins->getMemAddr (), st_ins->getInsID (), WAW_st_ins_sn);
#ifdef ASSERTION
    if (violation.first) Assert (violation.second != NULL);
    else Assert (violation.second == NULL);
#endif
    return violation;
}

/* ***************** *
 *  LOAD QUEUE FUNC  *
 * ***************** */
void o3_memManager::completeLd (dynInstruction* ins) {
    ins->setLQstate (LQ_COMPLETE);
}

/* FORWARD DATA 
 * NOTE: this function is not sorted based on the order in which LOAD ops
 * return data. The port must be searched for finding the data that is being
 * forwarded every cycle (in the scheduler)
 */
void o3_memManager::forward (dynInstruction* ins, CYCLE mem_latency) {
#ifdef ASSERTION
    Assert (ins->getInsType () == MEM && ins->getMemType() == LOAD);
#endif
    if (_memory_to_scheduler_port->getBuffState () == FULL_BUFF) return;
    CYCLE cdb_ready_latency = mem_latency - 1;
    Assert (cdb_ready_latency >= 0);
    _memory_to_scheduler_port->pushBack (ins, cdb_ready_latency);
    dbg.print (DBG_MEMORY, "%s: %s %llu (cyc: %d)\n", _c_name.c_str (), 
               "Forward wr ops of ins", ins->getInsID (), _clk->now ());
}

o3_memManager* g_LSQ_MGR;
