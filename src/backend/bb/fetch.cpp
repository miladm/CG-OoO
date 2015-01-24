/*********************************************************************************
 * fetch.cpp
 ********************************************************************************/

#include "fetch.h"

bb_fetch::bb_fetch (port<bbInstruction*>& bp_to_fetch_port, 
	          port<bbInstruction*>& fetch_to_decode_port,
			  port<bbInstruction*>& fetch_to_bp_port,
              CAMtable<dynBasicblock*>* bbROB,
              CAMtable<dynBasicblock*>* bbQUE,
			  WIDTH fetch_width,
              sysClock* clk,
			  string stage_name
			 )
	: stage (fetch_width, stage_name, clk),
      s_bb_cnt (g_stats.newScalarStat (stage_name, "bb_cnt", "Number of basicblocks in " + stage_name, 0, PRINT_ZERO)),
      s_bb_size_avg (g_stats.newRatioStat (&s_bb_cnt, stage_name, "bb_size_avg", "average basicblock size in " + stage_name, 0, PRINT_ZERO))
{
    _bp_to_fetch_port = &bp_to_fetch_port;
    _fetch_to_bp_port = &fetch_to_bp_port;
    _fetch_to_decode_port = &fetch_to_decode_port;
    _bbROB = bbROB;
    _bbQUE = bbQUE;
    _insListIndx = 0;
    _switch_to_frontend = false;
    _fetch_state = FETCH_COMPLETE;
    _current_bb = NULL;
}

bb_fetch::~bb_fetch () {}

SIM_MODE bb_fetch::doFETCH (FRONTEND_STATUS frontend_status) {
    /*-- STAT + DEBUG --*/
    dbg.print (DBG_FETCH, "** %s: (cyc: %d)\n", _stage_name.c_str (), _clk->now ());
    regStat ();
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /*-- SQUASH HANDLING --*/
    if (g_var.g_pipe_state == PIPE_FLUSH) { squash (); }
    if (g_var.g_pipe_state == PIPE_NORMAL) {
        pipe_stall = fetchImpl (frontend_status);
    }

    /*-- STAT --*/
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;

    if (_switch_to_frontend) {
        _switch_to_frontend = false;
        return FRONT_END;
    }
	return BACK_END;
}

/*-- READ FW INSTRUCTIONS FORM THE PIN INPUT QUE --*/
PIPE_ACTIVITY bb_fetch::fetchImpl (FRONTEND_STATUS frontend_status) {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Fetch", _clk->now ());
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;
    if (!fetchBB (frontend_status)) return pipe_stall;
    Assert (_current_bb != NULL);

    for (WIDTH i = 0; i < _stage_width; i++) {
        /*-- CHECKS --*/
        if (_fetch_to_decode_port->getBuffState () == FULL_BUFF) break;
        if (_current_bb->getBBstate () == EMPTY_BUFF) {
//            break; /* FOR NO BACK-TO-BACK BB FETCH IN 1 CYCLE */
            if (!fetchBB (frontend_status)) break; /* COULD NOT FETCH A ANOTHER NEW BB */
            Assert (_current_bb != NULL);
        }

        /*-- FETCH INS --*/
        bbInstruction* ins = _current_bb->popFront ();
        ins->setPipeStage (FETCH);
        _fetch_to_decode_port->pushBack (ins);
        updateBBfetchState ();

        dbg.print (DBG_FETCH, "%s: %s %llu %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                "Fetch ins", ins->getInsID (), "from BB", ins->getBB()->getBBID (), _clk->now ());

        /*-- STAT --*/
        s_ipc++;
        s_ins_cnt++;
        pipe_stall = PIPE_BUSY;
    }
    updateBBfetchState ();

    return pipe_stall;
}

bool bb_fetch::fetchBB (FRONTEND_STATUS frontend_status) {
    if (_fetch_state == FETCH_COMPLETE) {
        if (isGoToFrontend (frontend_status)) { 
            _switch_to_frontend = true; 
            return false; 
        } else {
            if (getNewBB () == EMPTY_BUFF) return false;
            dbg.print (DBG_FETCH, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), 
                    "NEW BB:", _current_bb->getBBID (), _clk->now ());
            _bbQUE->pushBack (_current_bb);
            Assert (_bbQUE->getTableState () != FULL_BUFF && "bbQUE must never become full");
            s_bb_size_avg += _current_bb->getBBsize ();
            s_bb_cnt++;
        }
    }
    return true;
}

bool bb_fetch::isGoToFrontend (FRONTEND_STATUS frontend_status) {
    if (frontend_status == FRONTEND_RUNNING && 
            g_var.isBBcacheNearEmpty () == true) { 
        return true; 
    } else if (frontend_status == FRONTEND_DONE && 
            g_var.isBBcacheEmpty () == true && 
            _bbQUE->getTableState () == EMPTY_BUFF && 
            _bbROB->getTableState () == EMPTY_BUFF) { 
        return true; 
    }
    return false;
}

BUFF_STATE bb_fetch::getNewBB () {
    //Assert (_current_bb->getBBstate () == EMPTY_BUFF); //TODO put it back - detect very first BB
    BUFF_STATE buff_state = AVAILABLE_BUFF;
    if (_current_bb != NULL) _current_bb->setDoneFetch ();
    if (g_var.isBBcacheEmpty ()) return EMPTY_BUFF;
    _current_bb = g_var.popBBcache ();
    if (_current_bb->getBBsize () == 0) {
        dbg.print (DBG_FETCH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), 
                "Dumping an empty BB", _clk->now ());
        delete _current_bb;
        buff_state = getNewBB ();
    }
    return buff_state;
}

void bb_fetch::updateBBfetchState () {
    if (_current_bb->getBBstate () == EMPTY_BUFF) _fetch_state = FETCH_COMPLETE;
    else if (_current_bb->getBBstate () == FULL_BUFF) _fetch_state = FETCH_IN_PROGRESS;
    else if (_current_bb->getBBstate () == AVAILABLE_BUFF) _fetch_state = FETCH_IN_PROGRESS;
    dbg.print (DBG_FETCH, "%s: %s %d (cyc: %d)\n", _stage_name.c_str (), 
            "FETCH STATE:", _fetch_state, _clk->now ());
}

void bb_fetch::squash () {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Fetch Port Flush", _clk->now ());
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
    INS_ID squashSeqNum = g_var.getSquashSN ();
    _fetch_to_decode_port->flushPort (squashSeqNum);
    _fetch_to_bp_port->flushPort (squashSeqNum);
}

void bb_fetch::squashCurrentBB () {
    _current_bb = NULL;
    _fetch_state = FETCH_COMPLETE;
}

void bb_fetch::regStat () {
    _bp_to_fetch_port->regStat ();
}
