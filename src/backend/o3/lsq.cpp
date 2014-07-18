/*******************************************************************************
 * lsq.cpp
 ******************************************************************************/

#include "lsq.h"

o3_lsqCAM::o3_lsqCAM (LENGTH len, 
                WIDTH rd_port_cnt, 
                WIDTH wr_port_cnt,
                string table_name)
    : CAMtable<dynInstruction*> (len, rd_port_cnt, wr_port_cnt, table_name)
{}

o3_lsqCAM::~o3_lsqCAM () {}

dynInstruction* o3_lsqCAM::findPendingMemIns (LSQ_ID lsq_id) {
    if (lsq_id == ST_QU) {
        LENGTH table_size = _table.NumElements ();
        for (LENGTH i = 0; i < table_size; i++) {
            dynInstruction* ins = getNth_unsafe (i);
            if (ins->getPipeStage () == COMMIT &&
                ins->getSQstate () == SQ_COMMIT) {
                dbg.print (DBG_MEMORY, "%s: %s %llu\n", _c_name.c_str (), "ST ins issued:", ins->getInsID ());
                return getNth_unsafe (i);
            }
        }
        dbg.print (DBG_MEMORY, "%s: %s\n", _c_name.c_str (), "No ST ins issued.");
        return NULL; /* NOTHING ISSUED */
    } else {
        LENGTH table_size = _table.NumElements ();
        for (LENGTH i = 0; i < table_size; i++) {
            dynInstruction* ins = getNth_unsafe (i);
            if (ins->getPipeStage () == MEM_ACCESS &&
                ins->getLQstate () == LQ_PENDING_CACHE_DISPATCH) {
                dbg.print (DBG_MEMORY, "%s: %s %llu\n", _c_name.c_str (), "LD ins issued:", ins->getInsID ());
                return getNth_unsafe (i);
            }
        }
        dbg.print (DBG_MEMORY, "%s: %s\n", _c_name.c_str (), "No LD ins issued.");
        return NULL; /* NOTHING ISSUED */
    }
}

void o3_lsqCAM::setTimer (dynInstruction* elem, CYCLE axes_lat, CYCLE now) {
    LENGTH table_size = _table.NumElements ();
    for (LENGTH i = 0; i < table_size; i++) {
        dynInstruction* ins = getNth_unsafe (i);
        if (ins->getInsID () == elem->getInsID ()) {
            dbg.print (DBG_MEMORY, "%s: %s %llu (lat: %d)\n", _c_name.c_str (), "Set mem access lat. for ins:", ins->getInsID (), axes_lat);
            _table.Nth(i)->_delay.setNewTime (now, axes_lat);
            Assert (_table.Nth(i)->_delay.isValidStopTime () == true);
            return;
        }
    }
    Assert (true == false && "The LSQ enetry not found.");
}

void o3_lsqCAM::squash (INS_ID ins_seq_num) {
    LENGTH table_size = _table.NumElements ();
    for (LENGTH i = table_size - 1; i >= 0; i--) {
        if (getNth_unsafe (i)->getInsID () >= ins_seq_num) {
            delete _table.Nth (i);
            _table.RemoveAt (i);
        } else {
            break;
        }
    }
}

void o3_lsqCAM::delFinishedMemAxes (CYCLE now) {
    LENGTH table_size = _table.NumElements ();
    for (LENGTH i = 0; i < table_size; i++) {
        if (_table.Nth(i)->_delay.isValidStopTime () &&
            _table.Nth(i)->_delay.getStopTime () <= now) {
            Assert (getNth_unsafe (i)->getSQstate () == SQ_CACHE_DISPATCH);
            dynInstruction* ins = pullNth (i);
            dbg.print (DBG_MEMORY, "%s: %s %llu\n", _c_name.c_str (), "Found a finished ST ins: ", ins->getInsID ());
            delete ins;
            break; /* REMOVE ONE ELEM PER INVOCATION */
        }
        dbg.print (DBG_MEMORY, "%s: %s %ld %ld\n", _c_name.c_str (), "No finished ST ins.", _table.Nth(0)->_delay.isValidStopTime (), now);
    }
}

bool o3_lsqCAM::hasCommit () {
    LENGTH table_size = _table.NumElements ();
    for (LENGTH i = 0; i < table_size; i++) {
        if (getNth_unsafe (i)->getSQstate () == SQ_COMMIT) {
            return true;
        }
    }
    return false;
}

bool o3_lsqCAM::hasAnyCompleteLdFromAddr (INS_ID completed_st_mem_addr, dynInstruction* violating_ld) {
    violating_ld = NULL;
    LENGTH table_size = _table.NumElements ();
    for (LENGTH i = table_size - 1; i >= 0; i--) {
        dynInstruction* ins = getNth_unsafe (i);
        if (ins->getMemAddr () == completed_st_mem_addr) {
            if (ins->getLQstate () == LQ_COMPLETE) {
                violating_ld = ins;
                return true; //TODO double cehck that this means a register write has happened in the stage
            } else if (ins->getLQstate () == LQ_FWD_FROM_SQ ||
                       ins->getLQstate () == LQ_MSHR_WAIT ||
                       ins->getLQstate () == LQ_CACHE_WAIT) {
                //TODO implement this block - FWD from SQ - return false 
                //TODO check that at ADD_WAIT state, later on the value in SQ
                //will be picked up and so we don't have to worry about that
                //here
            }
        }
    }
    return false;
}

pair<bool, dynInstruction*> o3_lsqCAM::hasFinishedIns (LSQ_ID lsq_id, CYCLE now) {
    LENGTH table_size = _table.NumElements ();
    if (lsq_id == LD_QU) {
        for (LENGTH i = 0; i < table_size; i++) {
            dynInstruction* ins = getNth_unsafe (i);
            if ((ins->getLQstate () == LQ_FWD_FROM_SQ ||
                 ins->getLQstate () == LQ_MSHR_WAIT ||
                 ins->getLQstate () == LQ_CACHE_WAIT) &&
                 _table.Nth(i)->_delay.isValidStopTime () &&
                 _table.Nth(i)->_delay.getStopTime () <= now) {
                CYCLE stop_time = _table.Nth(i)->_delay.getStopTime ();
                dbg.print (DBG_MEMORY, "%s: %s %llu %llu %llu\n", _c_name.c_str (), "Found a finished LD ins: ", ins->getInsID (), stop_time, now);
                return pair<bool, dynInstruction*> (true, ins);
            }
        }
        dbg.print (DBG_MEMORY, "%s: %s\n", _c_name.c_str (), "No finished LD ins.");
        return pair<bool, dynInstruction*> (false, NULL);
    } else {
        for (LENGTH i = 0; i < table_size; i++) {
            dynInstruction* ins = getNth_unsafe (i);
            if (ins->getSQstate () == SQ_CACHE_DISPATCH &&
                _table.Nth(i)->_delay.isValidStopTime () &&
                _table.Nth(i)->_delay.getStopTime () <= now) {
                CYCLE stop_time = _table.Nth(i)->_delay.getStopTime ();
                dbg.print (DBG_MEMORY, "%s: %s %llu %llu %llu\n", _c_name.c_str (), "Found a finished ST ins: ", ins->getInsID (), stop_time, now);
                return pair<bool, dynInstruction*> (true, ins);
            }
        }
        dbg.print (DBG_MEMORY, "%s: %s\n", _c_name.c_str (), "No finished ST ins.");
        return pair<bool, dynInstruction*> (false, NULL);
    }
}
