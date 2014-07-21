/*******************************************************************************
 * memManager.cpp
 ******************************************************************************/

#include "memManager.h"

o3_memManager::o3_memManager (sysClock* clk, string rf_name = "o3_memManager") 
    : unit (rf_name, clk),
      _L1 (1, 64, 32768),
      _L2 (1, 64, 2097152),
      _L3 (1, 64, 8388608),
      _LQ (LQ_SIZE, 8, 4, clk, "LQtable"),
      _SQ (SQ_SIZE, 8, 4, clk, "SQtable")
{ }

o3_memManager::~o3_memManager () {}

/* ***************** *
 *      LSQ FUNC     *
 * ***************** */
BUFF_STATE o3_memManager::getTableState (LSQ_ID lsq_id) {
    if (lsq_id == LD_QU) {
        return _LQ.getTableState (); //TODO set state to LQ_ADDR_WAIT
    } else {
        return _SQ.getTableState (); //TODO set state to LQ_ADDR_WAIT
    }
}

bool o3_memManager::hasFreeWire (LSQ_ID lsq_id, AXES_TYPE axes_type) {
    if (lsq_id == LD_QU) {
        return _LQ.hasFreeWire (axes_type); //TODO set state to LQ_ADDR_WAIT
    } else {
        return _SQ.hasFreeWire (axes_type); //TODO set state to LQ_ADDR_WAIT
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
        _LQ.pushBack (ins); //TODO set state to LQ_ADDR_WAIT
        ins->setLQstate (LQ_ADDR_WAIT);
    } else {
        Assert (_SQ.getTableState () != FULL_BUFF);
        _SQ.pushBack (ins); //TODO set state to LQ_ADDR_WAIT
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
        _LQ.setTimer (mem_ins, axes_lat); //TODO good API?
    } else {
        mem_ins = _SQ.findPendingMemIns (ST_QU);
        Assert (mem_ins->isMemOrBrViolation() == false);
        if (mem_ins == NULL) return false; /* NOTHING ISSUED */
        mem_ins->setSQstate (SQ_CACHE_DISPATCH);
        axes_lat = (CYCLE) cacheCtrl (WRITE,  //stIns->getMemType (), TODO fix this line
                mem_ins->getMemAddr (),
                mem_ins->getMemAxesSize(),
                &_L1, &_L2, &_L3);
        _SQ.setTimer (mem_ins, axes_lat);
    }
#ifdef ASSERTION
    Assert(axes_lat > 0);
#endif
    //(axes_lat > L1_LATENCY) ? s_ld_miss_cnt++ : s_ld_hit_cnt++; TODO put them back
    //(axes_lat > L1_LATENCY) ? s_cache_miss_cnt++ : s_cache_hit_cnt++;
    return true;
}

CYCLE o3_memManager::getAxesLatency (dynInstruction* mem_ins) {
    if (hasStToAddr (mem_ins->getMemAddr (), mem_ins->getInsID ())) {
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
        //TODO apply hardware costraints here
        Assert (ins->getSQstate () == SQ_COMPLETE);
        ins->setSQstate (SQ_COMMIT);
        //Assert (ins->getInsID () == _SQ.getFront()->getInsID ());
        dbg.print (DBG_MEMORY, "%s: %s %llu\n", _c_name.c_str (), "Commiting ST:", ins->getInsID ());
        //dynInstruction* st_ins = _SQ.popFront (); //TODO should it be in order? don't think so
        //Assert (st_ins->getInsID () == ins->getInsID ());
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

/* DELETE ONE INS WITH FINISHED MEM STORE */
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
    pair<bool, dynInstruction*> violation = _LQ.hasAnyCompleteLdFromAddr (st_ins->getMemAddr ());
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

o3_memManager* g_LSQ_MGR;
