/*******************************************************************************
 * memManager.cpp
 ******************************************************************************/

#include "memManager.h"

bb_memManager::bb_memManager (port<bbInstruction*>& memory_to_scheduler_port,
                              sysClock* clk, 
                              string lsq_name = "bb_memManager") 
    : unit (lsq_name, clk),
      _L1 (1, 64, 32768),
      _L2 (1, 64, 2097152),
      _L3 (1, 64, 8388608),
      _LQ (BB_LQ_SIZE, 8, 4, clk, "LQtable"),
      _SQ (BB_SQ_SIZE, 8, 4, clk, "SQtable"),
      s_cache_hit_cnt  (g_stats.newScalarStat (lsq_name, "cache_hit_cnt", "Number of cache hits", 0, PRINT_ZERO)),
      s_cache_miss_cnt (g_stats.newScalarStat (lsq_name, "cache_miss_cnt", "Number of cache misses", 0, PRINT_ZERO)),
      s_ld_hit_cnt  (g_stats.newScalarStat (lsq_name, "ld_hit_cnt", "Number of load hits", 0, PRINT_ZERO)),
      s_ld_miss_cnt (g_stats.newScalarStat (lsq_name, "ld_miss_cnt", "Number of load misses", 0, PRINT_ZERO)),
      s_st_miss_cnt (g_stats.newScalarStat (lsq_name, "st_miss_cnt", "Number of store misses", 0, PRINT_ZERO)),
      s_st_hit_cnt  (g_stats.newScalarStat (lsq_name, "st_hit_cnt", "Number of store hits", 0, PRINT_ZERO)),
      s_cache_to_ld_fwd_cnt (g_stats.newScalarStat (lsq_name, "cache_to_ld_fwd_cnt", "Number of cache accesses", 0, PRINT_ZERO)),
      s_st_to_ld_fwd_cnt (g_stats.newScalarStat (lsq_name, "st_to_ld_fwd_cnt", "Number of SQ -> LQ forwarding events", 0, PRINT_ZERO))
{ 
    _memory_to_scheduler_port = &memory_to_scheduler_port;
}

bb_memManager::~bb_memManager () {}

void bb_memManager::regStat () {
    _LQ.regStat ();
    _SQ.regStat ();
}

/* ***************** *
 *      LSQ FUNC     *
 * ***************** */
BUFF_STATE bb_memManager::getTableState (LSQ_ID lsq_id) {
    if (lsq_id == LD_QU) {
        return _LQ.getTableState ();
    } else {
        return _SQ.getTableState ();
    }
}

bool bb_memManager::hasFreeWire (LSQ_ID lsq_id, AXES_TYPE axes_type) {
    if (lsq_id == LD_QU) {
        return _LQ.hasFreeWire (axes_type);
    } else {
        return _SQ.hasFreeWire (axes_type);
    }
}

void bb_memManager::updateWireState (LSQ_ID lsq_id, AXES_TYPE axes_type) {
    if (lsq_id == LD_QU) {
        _LQ.updateWireState (axes_type);
    } else {
        _SQ.updateWireState (axes_type);
    }
}

void bb_memManager::pushBack (bbInstruction *ins) {
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

void bb_memManager::memAddrReady (bbInstruction* ins) {
    if (ins->getMemType () == LOAD) {
        ins->setLQstate (LQ_PENDING_CACHE_DISPATCH);
    } else {
        ins->setSQstate (SQ_COMPLETE);
    }
}

bool bb_memManager::issueToMem (LSQ_ID lsq_id) {
    CYCLE axes_lat;
    bbInstruction* mem_ins;
    if (lsq_id == LD_QU) {
        mem_ins = _LQ.findPendingMemIns (LD_QU);
        if (mem_ins == NULL) return false; /* NOTHING ISSUED */
        axes_lat = getAxesLatency (mem_ins);
        _LQ.setTimer (mem_ins, axes_lat);
        if (ENABLE_FWD) forward (mem_ins, axes_lat);
        (axes_lat > L1_LATENCY) ? s_ld_miss_cnt++ : s_ld_hit_cnt++;
    } else {
        mem_ins = _SQ.findPendingMemIns (ST_QU);
        if (ENABLE_SQUASH) Assert (mem_ins->isMemOrBrViolation() == false);
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

CYCLE bb_memManager::getAxesLatency (bbInstruction* mem_ins) {
    if (hasStToAddr (mem_ins->getMemAddr (), mem_ins->getInsID ())) {
        s_st_to_ld_fwd_cnt++;
        mem_ins->setLQstate (LQ_FWD_FROM_SQ);
        return g_eu_lat._st_buff_lat;
    //} else if () { /*TODO for MSHR */
    } else {
        s_cache_to_ld_fwd_cnt++;
        if (mem_ins->getMemType () == LOAD) mem_ins->setCacheAxes ();
        mem_ins->setLQstate (LQ_CACHE_WAIT);
        return (CYCLE) cacheCtrl (READ,  //stIns->getMemType (), TODO fix this line
                mem_ins->getMemAddr (),
                mem_ins->getMemAxesSize(),
                &_L1, &_L2, &_L3);
    }
}

bool bb_memManager::commit (bbInstruction* ins) {
    Assert (ins->getInsType () == MEM);
    if (ins->getMemType () == LOAD) {
        if (_LQ.getTableState () == EMPTY_BUFF) return false;
//        if (!_LQ.hasFreeWire (READ)) return false; TODO put this back when multi-cycle BB commit is enabled - should it?
        bbInstruction* ld_ins = _LQ.popFront ();
#ifdef ASSERTION
        Assert (ins->getLQstate () == LQ_COMPLETE);
        Assert (ld_ins->getInsID () == ins->getInsID ());
#endif
//        _LQ.updateWireState (READ); TODO put it back too if you decided to go for this. same goes for SQ?
        dbg.print (DBG_MEMORY, "%s: %s %llu\n", _c_name.c_str (), "Commiting LD:", ins->getInsID ());
    } else { /*-- STORE --*/
        Assert (ins->getSQstate () == SQ_COMPLETE);
        ins->setSQstate (SQ_COMMIT);
        dbg.print (DBG_MEMORY, "%s: %s %llu\n", _c_name.c_str (), "Commiting ST:", ins->getInsID ());
    }
    return true;
}

void bb_memManager::squash (INS_ID squash_seq_num) {
    dbg.print (DBG_MEMORY, "%s: %s\n", _c_name.c_str (), "LQ SQUASH");
    _LQ.squash (squash_seq_num);
    dbg.print (DBG_MEMORY, "%s: %s\n", _c_name.c_str (), "SQ SQUASH");
    _SQ.squash (squash_seq_num);
}

/* ***************** *
 * STORE QUEUE FUNC  *
 * ***************** */
bool bb_memManager::hasCommitSt () {
    return _SQ.hasCommit ();
}

/*-- DELETE ONE INS WITH FINISHED MEM STORE --*/
void bb_memManager::delAfinishedSt () {
    _SQ.delFinishedMemAxes ();
}

pair<bool, bbInstruction*> bb_memManager::hasFinishedIns (LSQ_ID lsq_id) {
    if (lsq_id == LD_QU) {
        return _LQ.hasFinishedIns (lsq_id);
    } else {
        return _SQ.hasFinishedIns (lsq_id);
    }
}

bool bb_memManager::hasStToAddr (ADDRS mem_addr, INS_ID ins_seq_num) {
    return _SQ.hasMemAddr (mem_addr, ins_seq_num);
}

pair<bool, bbInstruction*> bb_memManager::isLQviolation (bbInstruction* st_ins) {
#ifdef ASSERTION
    Assert (st_ins->getMemType () == STORE);
    Assert (st_ins->getSQstate () == SQ_COMPLETE);
#endif
    INS_ID WAW_st_ins_sn = _SQ.hasAnyCompleteStFromAddr (st_ins->getMemAddr (), st_ins->getInsID ());
    pair<bool, bbInstruction*> violation = _LQ.hasAnyCompleteLdFromAddr (st_ins->getMemAddr (), st_ins->getInsID (), WAW_st_ins_sn, st_ins->getBBWinID ());
#ifdef ASSERTION
    if (violation.first) Assert (violation.second != NULL);
    else Assert (violation.second == NULL);
#endif
    return violation;
}

/* ***************** *
 *  LOAD QUEUE FUNC  *
 * ***************** */
void bb_memManager::completeLd (bbInstruction* ins) {
    ins->setLQstate (LQ_COMPLETE);
}

/* FORWARD DATA 
 * NOTE: this function is not sorted based on the order in which LOAD ops
 * return data. The port must be searched for finding the data that is being
 * forwarded every cycle (in the scheduler)
 */
void bb_memManager::forward (bbInstruction* ins, CYCLE mem_latency) {
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
