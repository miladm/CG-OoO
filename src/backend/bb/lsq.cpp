/*********************************************************************************
 * lsq.cpp
 ********************************************************************************/

#include "lsq.h"

bb_lsqCAM::bb_lsqCAM (LENGTH len, 
                WIDTH rd_port_cnt, 
                WIDTH wr_port_cnt,
                sysClock* clk,
                string table_name)
    : CAMtable<bbInstruction*> (len, rd_port_cnt, wr_port_cnt, clk, table_name),
      s_inter_bb_mem_mis_pred_cnt (g_stats.newScalarStat (table_name, "inter_bb_mem_mis_pred_cnt", "Number of mis-predictions of load-store in diff BB's", 0, NO_PRINT_ZERO)),
      s_intra_bb_mem_mis_pred_cnt (g_stats.newScalarStat (table_name, "intra_bb_mem_mis_pred_cnt", "Number of mis-predictions of load-store in the same BB", 0, NO_PRINT_ZERO))
{}

bb_lsqCAM::~bb_lsqCAM () {}

bbInstruction* bb_lsqCAM::findPendingMemIns (LSQ_ID lsq_id) {
    if (lsq_id == ST_QU) {
        LENGTH table_size = _table.NumElements ();
        for (LENGTH i = 0; i < table_size; i++) {
            bbInstruction* ins = getNth_unsafe (i);
            if (ins->getPipeStage () == COMMIT &&
                ins->getSQstate () == SQ_COMMIT) {
                dbg.print (DBG_MEMORY, "%s: %s %llu\n", _c_name.c_str (), "ST ins issued:", ins->getInsID ());
                return getNth_unsafe (i);
            }
        }
        dbg.print (DBG_MEMORY, "%s: %s\n", _c_name.c_str (), "No ST ins issued.");
        return NULL; /*-- NOTHING ISSUED --*/
    } else { /*-- LD_QU --*/
        LENGTH table_size = _table.NumElements ();
        for (LENGTH i = 0; i < table_size; i++) {
            bbInstruction* ins = getNth_unsafe (i);
            if (ins->getPipeStage () == MEM_ACCESS &&
                ins->getLQstate () == LQ_PENDING_CACHE_DISPATCH) {
                dbg.print (DBG_MEMORY, "%s: %s %llu\n", _c_name.c_str (), "LD ins issued:", ins->getInsID ());
                return getNth_unsafe (i);
            }
        }
        dbg.print (DBG_MEMORY, "%s: %s\n", _c_name.c_str (), "No LD ins issued.");
        return NULL; /*-- NOTHING ISSUED --*/
    }
}

void bb_lsqCAM::setTimer (bbInstruction* elem, CYCLE axes_lat) {
    CYCLE now = _clk->now ();
    LENGTH table_size = _table.NumElements ();
    for (LENGTH i = 0; i < table_size; i++) {
        bbInstruction* ins = getNth_unsafe (i);
        if (ins->getInsID () == elem->getInsID ()) {
            dbg.print (DBG_MEMORY, "%s: %s %llu (lat: %d)\n", _c_name.c_str (), 
                       "Set mem access lat. for ins:", ins->getInsID (), axes_lat);
            _table.Nth(i)->_delay.setNewTime (now, axes_lat);
            Assert (_table.Nth(i)->_delay.isValidStopTime () == true);
            return;
        }
    }
    Assert (true == false && "The LSQ enetry not found.");
}

void bb_lsqCAM::squash (INS_ID ins_seq_num) {
    LENGTH table_size = _table.NumElements ();
    for (LENGTH i = table_size - 1; i >= 0; i--) {
        bbInstruction* ins = getNth_unsafe (i);
        if (ins->getInsID () >= ins_seq_num) {
//            delete _table.Nth (i);
            _table.RemoveAt (i);
            dbg.print (DBG_MEMORY, "%s: %s %llu %s %llu\n", _c_name.c_str (), 
                       "Squashing mem ins:", ins->getInsID (), "due to BB", ins_seq_num);
        } else { break; }
    }
}

void bb_lsqCAM::delFinishedMemAxes () {
    CYCLE now = _clk->now ();
    LENGTH table_size = _table.NumElements ();
    for (LENGTH i = 0; i < table_size; i++) {
        if (_table.Nth(i)->_delay.isValidStopTime () &&
            _table.Nth(i)->_delay.getStopTime () <= now) {
            Assert (getNth_unsafe (i)->getSQstate () == SQ_CACHE_DISPATCH);
            bbInstruction* ins = pullNth (i);
            dbg.print (DBG_MEMORY, "%s: %s %llu\n", _c_name.c_str (), 
                       "Found a finished ST ins: ", ins->getInsID ());
            delete ins;
            break; /*-- REMOVE ONE ELEM PER INVOCATION --*/
        }
        dbg.print (DBG_MEMORY, "%s: %s %ld %llu (cyc: %ld)\n", _c_name.c_str (), 
                   "No finished ST ins.", _table.Nth(0)->_delay.isValidStopTime (), _table.Nth(i)->_element->getInsID (), now);
    }
}

bool bb_lsqCAM::hasCommit () {
    LENGTH table_size = _table.NumElements ();
    for (LENGTH i = 0; i < table_size; i++) {
        if (getNth_unsafe(i)->getSQstate () == SQ_COMMIT) {
            return true;
        }
    }
    return false;
}

bool bb_lsqCAM::hasMemAddr (ADDRS mem_addr, INS_ID seq_num) {
    LENGTH table_size = _table.NumElements ();
    for (LENGTH i = table_size - 1; i >= 0; i--) {
        bbInstruction* ins = getNth_unsafe (i);
        PIPE_STAGE stage = ins->getPipeStage ();
        if (seq_num <= ins->getInsID ()) continue;
        if (g_var.g_mem_model == NAIVE_SPECUL &&
         ! (stage == COMPLETE || stage == COMMIT)) continue;
        if (ins->getMemAddr () == mem_addr) {
            return true;
        }
    }
    return false;
}

/*-- PRE: INSTRUCTIONS WITH SEQUENCE NUMBER = 0 DO NOT EXIST --*/ 
INS_ID bb_lsqCAM::hasAnyCompleteStFromAddr (ADDRS completed_st_mem_addr, INS_ID st_seq_num) {
    LENGTH table_size = _table.NumElements ();
    for (LENGTH i = 0; i < table_size; i++) {
        bbInstruction* st_ins = getNth_unsafe (i);
        if (st_ins->getInsID () <= st_seq_num) continue;
        if (st_ins->getSQstate () != SQ_COMPLETE) continue;
        if (st_ins->getMemAddr () == completed_st_mem_addr) 
            return st_ins->getInsID ();
    }
    return BB_NO_WAW_ST_INS; /* NO MATCHING YOUNGER STORE WITH SAME MEM ADDR FOUND */
}


pair<bool, bbInstruction*> bb_lsqCAM::hasAnyCompleteLdFromAddr (ADDRS completed_st_mem_addr, INS_ID lo_seq_num, INS_ID hi_seq_num, BB_ID st_ins_bb_id) {
    LENGTH table_size = _table.NumElements ();
    if (table_size > 0 && hi_seq_num == BB_NO_WAW_ST_INS) 
        hi_seq_num = getNth_unsafe(table_size - 1)->getInsID ();
    for (LENGTH i = table_size - 1; i > 0; i--) {
        bbInstruction* ins = getNth_unsafe (i);
        if (ins->getInsID () > hi_seq_num) continue;
        if (ins->getInsID () <= lo_seq_num) break;
        if (ins->getMemAddr () == completed_st_mem_addr) {
            if (ins->getLQstate () == LQ_COMPLETE && ins->isCacheAxes ()) {
                if (ins->getBBWinID () == st_ins_bb_id) s_intra_bb_mem_mis_pred_cnt++;
                else s_inter_bb_mem_mis_pred_cnt++;
                return pair<bool, bbInstruction*> (true, ins); //TODO double cehck that this means a register write has happened in the stage
            } else if (ins->getLQstate () == LQ_FWD_FROM_SQ ||
                       ins->getLQstate () == LQ_MSHR_WAIT ||
                       ins->getLQstate () == LQ_CACHE_WAIT) {
                //TODO implement this block - FWD from SQ - return false 
                //TODO check that at ADDR_WAIT state, later on the value in SQ
                //will be picked up and so we don't have to worry about that
                //here
            }
        }
    }
    return pair<bool, bbInstruction*> (false, NULL);
}

pair<bool, bbInstruction*> bb_lsqCAM::hasFinishedIns (LSQ_ID lsq_id) {
    CYCLE now = _clk->now ();
    LENGTH table_size = _table.NumElements ();
    if (lsq_id == LD_QU) {
        for (LENGTH i = 0; i < table_size; i++) {
            bbInstruction* ins = getNth_unsafe (i);
            if ((ins->getLQstate () == LQ_FWD_FROM_SQ ||
                 ins->getLQstate () == LQ_MSHR_WAIT ||
                 ins->getLQstate () == LQ_CACHE_WAIT) &&
                 _table.Nth(i)->_delay.isValidStopTime () &&
                 _table.Nth(i)->_delay.getStopTime () <= now) {
                CYCLE stop_time = _table.Nth(i)->_delay.getStopTime ();
                dbg.print (DBG_MEMORY, "%s: %s %llu %llu (cyc: %llu)\n", _c_name.c_str (), 
                           "Found a finished LD ins: ", ins->getInsID (), stop_time, now);
                return pair<bool, bbInstruction*> (true, ins);
            }
        }
        dbg.print (DBG_MEMORY, "%s: %s %s %d\n", _c_name.c_str (), "No finished LD ins.", "LQ Size:", table_size);
        return pair<bool, bbInstruction*> (false, NULL);
    } else {
        for (LENGTH i = 0; i < table_size; i++) {
            bbInstruction* ins = getNth_unsafe (i);
            if (ins->getSQstate () == SQ_CACHE_DISPATCH &&
                _table.Nth(i)->_delay.isValidStopTime () &&
                _table.Nth(i)->_delay.getStopTime () <= now) {
                CYCLE stop_time = _table.Nth(i)->_delay.getStopTime ();
                dbg.print (DBG_MEMORY, "%s: %s %llu %llu (cyc: %llu)\n", _c_name.c_str (), 
                           "Found a finished ST ins: ", ins->getInsID (), stop_time, now);
                return pair<bool, bbInstruction*> (true, ins);
            }
        }
        dbg.print (DBG_MEMORY, "%s: %s %s %d\n", _c_name.c_str (), "No finished ST ins.", "SQ Size:", table_size);
        return pair<bool, bbInstruction*> (false, NULL);
    }
}

void bb_lsqCAM::dump () {
    LENGTH table_size = _table.NumElements ();
    for (LENGTH i = table_size - 1; i >= 0; i--) {
        bbInstruction* ins = getNth_unsafe (i);
        cout << "ins id: " << ins->getInsID () << endl;
    }
}
