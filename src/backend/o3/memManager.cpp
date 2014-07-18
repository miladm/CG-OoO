/*******************************************************************************
 * memManager.cpp
 ******************************************************************************/

#include "memManager.h"

o3_memManager::o3_memManager () 
    : unit ("lsqManager"),
      _L1 (1, 64, 32768),
      _L2 (1, 64, 2097152),
      _L3 (1, 64, 8388608),
      _LQ (LQ_SIZE, 8, 4, "LQtable"),
      _SQ (SQ_SIZE, 8, 4, "SQtable")
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

bool o3_memManager::hasFreeWrPort (LSQ_ID lsq_id, CYCLE now) {
    if (lsq_id == LD_QU) {
        return _LQ.hasFreeWrPort (now); //TODO set state to LQ_ADDR_WAIT
    } else {
        return _SQ.hasFreeWrPort (now); //TODO set state to LQ_ADDR_WAIT
    }
}

bool o3_memManager::hasFreeRdPort (LSQ_ID lsq_id, CYCLE now) {
    if (lsq_id == LD_QU) {
        return _LQ.hasFreeRdPort (now); //TODO set state to LQ_ADDR_WAIT
    } else {
        return _SQ.hasFreeRdPort (now); //TODO set state to LQ_ADDR_WAIT
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

bool o3_memManager::issueToMem (LSQ_ID lsq_id, CYCLE now) {
    //TODO take all this block to lsqManager.cpp
    CYCLE axes_lat;
    dynInstruction* mem_ins;
    if (lsq_id == LD_QU) {
        mem_ins = _LQ.findPendingMemIns (LD_QU);
        if (mem_ins == NULL) return false; /* NOTHING ISSUED */
        mem_ins->setLQstate (LQ_CACHE_WAIT);
        axes_lat = (CYCLE) cacheCtrl (READ,  //stIns->getMemType (), TODO fix this line
                mem_ins->getMemAddr (),
                mem_ins->getMemAxesSize(),
                &_L1, &_L2, &_L3);
        _LQ.setTimer (mem_ins, axes_lat, now); //TODO good API?
    } else {
        mem_ins = _SQ.findPendingMemIns (ST_QU);
        if (mem_ins == NULL) return false; /* NOTHING ISSUED */
        mem_ins->setSQstate (SQ_CACHE_DISPATCH);
        axes_lat = (CYCLE) cacheCtrl (WRITE,  //stIns->getMemType (), TODO fix this line
                mem_ins->getMemAddr (),
                mem_ins->getMemAxesSize(),
                &_L1, &_L2, &_L3);
        _SQ.setTimer (mem_ins, axes_lat, now);
    }
#ifdef ASSERTION
    Assert(axes_lat > 0);
#endif
    //(axes_lat > L1_LATENCY) ? s_ld_miss_cnt++ : s_ld_hit_cnt++; TODO put them back
    //(axes_lat > L1_LATENCY) ? s_cache_miss_cnt++ : s_cache_hit_cnt++;
    return true;
}

bool o3_memManager::commit (dynInstruction* ins, sysClock& clk) {
    Assert (ins->getInsType () == MEM);
    if (ins->getMemType () == LOAD) {
        if (_LQ.getTableState () == EMPTY_BUFF) return false;
        if (!_LQ.hasFreeRdPort (clk.now ())) return false;
        Assert (ins->getLQstate () == LQ_COMPLETE);
        //Assert (ins->getInsID () == _LQ.getFront()->getInsID ());
        dbg.print (DBG_MEMORY, "%s: %s %llu\n", _c_name.c_str (), "Commiting LD:", ins->getInsID ());
        dynInstruction* ld_ins = _LQ.popFront ();
        Assert (ld_ins->getInsID () == ins->getInsID ());
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
void o3_memManager::delAfinishedSt (CYCLE now) {
    _SQ.delFinishedMemAxes (now);
}

pair<bool, dynInstruction*> o3_memManager::hasFinishedIns (LSQ_ID lsq_id, CYCLE now) {
    if (lsq_id == LD_QU) {
        return _LQ.hasFinishedIns (lsq_id, now);
    } else {
        return _SQ.hasFinishedIns (lsq_id, now);
    }
}

bool o3_memManager::isLQviolation (dynInstruction* st_ins, dynInstruction* ld_ins) {
    Assert (st_ins->getMemType () == STORE);
    Assert (st_ins->getSQstate () == SQ_COMPLETE);
    bool violation = _LQ.hasAnyCompleteLdFromAddr (st_ins->getMemAddr (), ld_ins);
    if (violation) Assert (ld_ins != NULL);
    else Assert (ld_ins == NULL);
    return violation;
}

/* ***************** *
 *  LOAD QUEUE FUNC  *
 * ***************** */
void o3_memManager::completeLd (dynInstruction* ins) {
    ins->setLQstate (LQ_COMPLETE);
}

o3_memManager g_LSQ_MGR;
