/*******************************************************************************
 * memManager.cpp
 ******************************************************************************/

#include "memManager.h"

bb_memManager::bb_memManager (port<bbInstruction*>& memory_to_scheduler_port,
                              sysClock* clk, 
                              string lsq_name = "bb_memManager") 
    : unit (lsq_name, clk),
//      _L1 (1, 64, 32768),
//      _L2 (1, 64, 2097152),
//      _L3 (1, 64, 8388608),
      _cache ("/home/milad/esc_project/svn/PARS/src/config/simple.yaml"),
      _LQ (clk, g_cfg->_root["cpu"]["backend"]["table"]["LQ"], "LQ"),
      _SQ (clk, g_cfg->_root["cpu"]["backend"]["table"]["SQ"], "SQ"),
      _e_dtlb ("dtlb", g_cfg->_root["cpu"]["backend"]["mem"]["dtlb"]),
      s_ld_cnt (g_stats.newScalarStat (lsq_name, "ld_cnt", "Number of load ops", 0, PRINT_ZERO)),
      s_cache_hit_cnt  (g_stats.newScalarStat (lsq_name, "cache_hit_cnt", "Number of cache hits", 0, PRINT_ZERO)),
      s_cache_miss_cnt (g_stats.newScalarStat (lsq_name, "cache_miss_cnt", "Number of cache misses", 0, PRINT_ZERO)),
      s_cache_access_cnt (g_stats.newScalarStat (lsq_name, "cache_access", "Number of cache accesses", 0, PRINT_ZERO)),
      s_ld_hit_cnt  (g_stats.newScalarStat (lsq_name, "ld_hit_cnt", "Number of load hits", 0, PRINT_ZERO)),
      s_ld_miss_cnt (g_stats.newScalarStat (lsq_name, "ld_miss_cnt", "Number of load misses", 0, PRINT_ZERO)),
      s_st_miss_cnt (g_stats.newScalarStat (lsq_name, "st_miss_cnt", "Number of store misses", 0, PRINT_ZERO)),
      s_st_hit_cnt  (g_stats.newScalarStat (lsq_name, "st_hit_cnt", "Number of store hits", 0, PRINT_ZERO)),
      s_cache_to_ld_fwd_cnt (g_stats.newScalarStat (lsq_name, "cache_to_ld_fwd_cnt", "Number of cache accesses", 0, PRINT_ZERO)),
      s_st_to_ld_fwd_cnt (g_stats.newScalarStat (lsq_name, "st_to_ld_fwd_cnt", "Number of SQ -> LQ forwarding events", 0, PRINT_ZERO)),
      s_st_to_ld_fwd_rat (g_stats.newRatioStat (&s_ld_cnt, lsq_name, "st_to_ld_fwd_rat", "Rate of SQ -> LQ forwarding events / all load ops", 0, PRINT_ZERO)),
      s_ld_miss_rat (g_stats.newRatioStat (&s_cache_to_ld_fwd_cnt, lsq_name, "ld_miss_rat", "Load op miss rate", 0, PRINT_ZERO)),
      s_mem_miss_rat (g_stats.newRatioStat (&s_cache_access_cnt, lsq_name, "mem_miss_rat", "Memory miss rate", 0, PRINT_ZERO)),
      s_inflight_ld_rat (g_stats.newRatioStat (clk->getStatObj (), lsq_name, "inflight_ld_rat", "Number of in-flight LOAD instructions", 0, PRINT_ZERO)),
      s_inflight_cache_ld_rat (g_stats.newRatioStat (clk->getStatObj (), lsq_name, "inflight_cache_ld_rat", "Number of in-flight LOAD instructions in CACHES", 0, PRINT_ZERO))
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

void bb_memManager::updateWireState (LSQ_ID lsq_id, AXES_TYPE axes_type, list<string> wire_name, bool update_wire) {
    if (lsq_id == LD_QU) {
        _LQ.updateWireState (axes_type, wire_name, update_wire);
    } else {
        _SQ.updateWireState (axes_type, wire_name, update_wire);
    }
}

void bb_memManager::pushBack (bbInstruction *ins) {
#ifdef ASSERTION
    Assert (ins->getInsType () == MEM);
#endif
    if (ins->getMemType () == LOAD) {
#ifdef ASSERTION
        Assert (_LQ.getTableState () != FULL_BUFF);
#endif
        _LQ.pushBack (ins);
        ins->setLQstate (LQ_ADDR_WAIT);
        dbg.print (DBG_MEMORY, "%s: %s %llu (cyc: %d)\n", _c_name.c_str (), 
                "Write LQ ins", ins->getInsID (), _clk->now ());
    } else {
#ifdef ASSERTION
        Assert (_SQ.getTableState () != FULL_BUFF);
#endif
        _SQ.pushBack (ins);
        ins->setSQstate (SQ_ADDR_WAIT);
        dbg.print (DBG_MEMORY, "%s: %s %llu (cyc: %d)\n", _c_name.c_str (), 
                "Write SQ ins", ins->getInsID (), _clk->now ());
    }
}

void bb_memManager::memAddrReady (bbInstruction* ins) {
    if (ins->getMemType () == LOAD) {
        ins->setLQstate (LQ_PENDING_CACHE_DISPATCH);
        _LQ.camAccess (); /* FIND ADDRESS ENERGY */
        _LQ.ramAccess (); /* ADDRESS WRITE ENERGY */
        _e_dtlb.camAccess ();
    } else {
        ins->setSQstate (SQ_COMPLETE);
        _SQ.camAccess (); /* FIND ADDRESS ENERGY */
        _SQ.ramAccess (); /* ADDRESS WRITE ENERGY */
        _e_dtlb.camAccess ();
    }
}

bool bb_memManager::issueToMem (LSQ_ID lsq_id) {
    CYCLE axes_lat;
    bbInstruction* mem_ins;

    if (lsq_id == LD_QU) {
        mem_ins = _LQ.findPendingMemIns (LD_QU);
        _LQ.camAccess ();
        if (mem_ins == NULL) return false; /* NOTHING ISSUED */
        axes_lat = getAxesLatency (mem_ins);
        _LQ.setTimer (mem_ins, axes_lat);
        if (g_cfg->isEnMemFwd ()) forward (mem_ins, axes_lat);

        if (axes_lat > L1_LATENCY) { 
            s_ld_miss_cnt++; 
            s_ld_miss_rat++;
            s_mem_miss_rat++;
        } else { s_ld_hit_cnt++; }
        s_inflight_ld_rat += axes_lat;
        s_ld_cnt++;
    } else { /* ST_QU */
        mem_ins = _SQ.findPendingMemIns (ST_QU);
        _SQ.ramAccess (); /* GET THE OUTSTANDING COMMITED ST OP */
#ifdef ASSERTION
        if (g_cfg->isEnSquash ()) Assert (mem_ins->isMemOrBrViolation() == false);
#endif
        if (mem_ins == NULL) return false; /* NOTHING ISSUED */
        mem_ins->setSQstate (SQ_CACHE_DISPATCH);
        axes_lat = _cache.request ((uint64_t)mem_ins->getMemAddr (), false, REQUEST_WRITE);
//        axes_lat = (CYCLE) cacheCtrl (WRITE,  //stIns->getMemType (), TODO fix this line
//                mem_ins->getMemAddr (),
//                4,
//                mem_ins->getMemAxesSize(),
//                &_L1, &_L2, &_L3);
        _SQ.setTimer (mem_ins, axes_lat);

        if (axes_lat > L1_LATENCY) { 
            s_st_miss_cnt++;
            s_mem_miss_rat++;
        } else { s_st_hit_cnt++; }
        s_cache_access_cnt++;
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
        s_st_to_ld_fwd_rat++;
        mem_ins->setLQstate (LQ_FWD_FROM_SQ);
        CYCLE lat = g_eu_lat._st_buff_lat;
        if (g_cfg->isEnProfiling ()) { g_prof.update_ld_profiler (mem_ins->getInsAddr (), lat); }
        return lat;
    //} else if () { /*TODO for MSHR */
    } else {
        if (mem_ins->getMemType () == LOAD) mem_ins->setCacheAxes ();
        mem_ins->setLQstate (LQ_CACHE_WAIT);
        CYCLE lat = _cache.request ((uint64_t)mem_ins->getMemAddr (), false, REQUEST_READ);
        if (g_cfg->isEnProfiling ()) { g_prof.update_ld_profiler (mem_ins->getInsAddr (), lat); }
//        CYCLE lat = (CYCLE) cacheCtrl (READ,  //stIns->getMemType (), TODO fix this line
//                    mem_ins->getMemAddr (),
//                    4,
////                    mem_ins->getMemAxesSize(),
//                    &_L1, &_L2, &_L3);
        s_cache_to_ld_fwd_cnt++;
        s_inflight_cache_ld_rat += lat;
        s_cache_access_cnt++;
        return lat;
    }
}

bool bb_memManager::commit (bbInstruction* ins) {
#ifdef ASSERTION
    Assert (ins->getInsType () == MEM);
#endif
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
#ifdef ASSERTION
        Assert (ins->getSQstate () == SQ_COMPLETE);
#endif
        ins->setSQstate (SQ_COMMIT);
        _SQ.camAccess (); /* MUST FIND THE COMMITTING ST OPs */
        dbg.print (DBG_MEMORY, "%s: %s %llu\n", _c_name.c_str (), "Commiting ST:", ins->getInsID ());
    }
    return true;
}

/* PRE: 
 * LQ INDEX POSITION OF THE FAULTY LD IS KNOWN HERE, SO NO EXTRA ENERGY TO
 * SQUASH LQ. NOT THE CAES FOR SQ.
 */
void bb_memManager::squash (INS_ID squash_seq_num) {
    dbg.print (DBG_MEMORY, "%s: %s\n", _c_name.c_str (), "LQ SQUASH");
    _LQ.squash (squash_seq_num);
    dbg.print (DBG_MEMORY, "%s: %s\n", _c_name.c_str (), "SQ SQUASH");
    _SQ.squash (squash_seq_num);
    _SQ.camAccess (); /* FIND INS ADDRESSES > SQUASH SN */
}

/* ***************** *
 * STORE QUEUE FUNC  *
 * ***************** */
bool bb_memManager::hasCommitSt () {
    /* NO CAM ENERGY CONSUMED - ASSUMING HEAD OF QUE IS ALWAYS THE RESULT IF ANY */
    return _SQ.hasCommit ();
}

/*-- DELETE ONE INS WITH FINISHED MEM STORE --*/
void bb_memManager::delAfinishedSt () {
    /* ENERGY OF THIS STEP IS ACCOUNTED FOR IN table CLASS */
    _SQ.delFinishedMemAxes ();
}

pair<bool, bbInstruction*> bb_memManager::hasFinishedIns (LSQ_ID lsq_id) {
    if (lsq_id == LD_QU) {
        _LQ.camAccess ();
        return _LQ.hasFinishedIns (lsq_id);
    } else {
        _SQ.camAccess ();
        return _SQ.hasFinishedIns (lsq_id);
    }
}

bool bb_memManager::hasStToAddr (ADDRS mem_addr, INS_ID ins_seq_num) {
    _SQ.camAccess (); /* FIND ADDRESS ENERGY */
    return _SQ.hasMemAddr (mem_addr, ins_seq_num);
}

pair<bool, bbInstruction*> bb_memManager::isLQviolation (bbInstruction* st_ins) {
#ifdef ASSERTION
    Assert (st_ins->getMemType () == STORE);
    Assert (st_ins->getSQstate () == SQ_COMPLETE);
#endif

    INS_ID WAW_st_ins_sn = _SQ.hasAnyCompleteStFromAddr (st_ins->getMemAddr (), st_ins->getInsID ());
    _SQ.camAccess ();
    pair<bool, bbInstruction*> violation = _LQ.hasAnyCompleteLdFromAddr (st_ins->getMemAddr (), st_ins->getInsID (), WAW_st_ins_sn, st_ins->getBBWinID ());
    _LQ.camAccess ();

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
    /* NO ENERGY TRACKING HERE - hasFinishedIns () WILL COVER IT */
    if (ins->getLQstate ()) s_inflight_cache_ld_rat--;
    s_inflight_ld_rat--;
    ins->setLQstate (LQ_COMPLETE);
}

/*--
 * FORWARD DATA 
 * NOTE: THIS FUNCTION IS NOT SORTED BASED ON THE ORDER IN WHICH LOAD OPS
 * RETURN DATA. THE PORT MUST BE SEARCHED FOR FINDING THE DATA THAT IS BEING
 * FORWARDED EVERY CYCLE (IN THE SCHEDULER)
 --*/
void bb_memManager::forward (bbInstruction* ins, CYCLE mem_latency) {
#ifdef ASSERTION
    Assert (ins->getInsType () == MEM && ins->getMemType() == LOAD);
#endif
    if (_memory_to_scheduler_port->getBuffState () == FULL_BUFF) return;
    CYCLE cdb_ready_latency = mem_latency - 1;
#ifdef ASSERTION
    Assert (cdb_ready_latency >= 0);
#endif
    _memory_to_scheduler_port->pushBack (ins, cdb_ready_latency);
    dbg.print (DBG_MEMORY, "%s: %s %llu (cyc: %d)\n", _c_name.c_str (), 
               "Forward wr ops of ins", ins->getInsID (), _clk->now ());
}
