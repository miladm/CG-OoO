/*********************************************************************************
 * fetch.cpp
 ********************************************************************************/

#include "fetch.h"

o3_fetch::o3_fetch (port<dynInstruction*>& bp_to_fetch_port, 
	          port<dynInstruction*>& fetch_to_decode_port,
			  port<dynInstruction*>& fetch_to_bp_port,
              CAMtable<dynInstruction*>* iQUE,
              CAMtable<dynInstruction*>* iROB,
			  WIDTH fetch_width,
              sysClock* clk,
			  string stage_name
			 )
    : stage(fetch_width, stage_name, g_cfg->_root["cpu"]["backend"]["o3_pipe"]["fetch"], clk)
{
    _bp_to_fetch_port = &bp_to_fetch_port;
    _fetch_to_bp_port = &fetch_to_bp_port;
    _fetch_to_decode_port = &fetch_to_decode_port;
    _iQUE = iQUE;
    _iROB = iROB;
    _insListIndx = 0;
    _switch_to_frontend = false;
}

o3_fetch::~o3_fetch () {}

SIM_MODE o3_fetch::doFETCH (FRONTEND_STATUS frontend_status) {
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
PIPE_ACTIVITY o3_fetch::fetchImpl (FRONTEND_STATUS frontend_status) {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;
	for (WIDTH i = 0; i < _stage_width; i++) {
		/*-- CHECKS --*/
        if (isGoToFrontend (frontend_status)) { _switch_to_frontend = true; break;}
        if (g_var.isCodeCacheEmpty ()) {break;}
		if (_fetch_to_decode_port->getBuffState () == FULL_BUFF) break;

        /*-- FETCH INS --*/
        dynInstruction* ins = g_var.popCodeCache();
        _e_stage.ffAccess ();
        ins->setPipeStage(FETCH);
        g_var.remFromCodeCache ();
        _iQUE->pushBack (ins);
		_fetch_to_decode_port->pushBack(ins);
        _e_stage.ffAccess ();
        dbg.print (DBG_FETCH, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Fetch ins", ins->getInsID (), _clk->now ());

        /*-- STAT --*/
        s_ipc++;
        s_ins_cnt++;
        pipe_stall = PIPE_BUSY;
	}
    return pipe_stall;
}

bool o3_fetch::isGoToFrontend (FRONTEND_STATUS frontend_status) {
    if (frontend_status == FRONTEND_RUNNING && 
            g_var.isCodeCacheEmpty () == true) { 
        return true; 
    } else if (frontend_status == FRONTEND_DONE && 
            g_var.isCodeCacheEmpty () == true && 
            _iQUE->getTableState () == EMPTY_BUFF && 
            _iROB->getTableState () == EMPTY_BUFF) { 
        return true; 
    }
    return false;
}

void o3_fetch::squash () {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Fetch Port Flush", _clk->now ());
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
    INS_ID squashSeqNum = g_var.getSquashSN();
    _fetch_to_decode_port->flushPort (squashSeqNum);
    _fetch_to_bp_port->flushPort (squashSeqNum);
    _e_stage.ffAccess (_stage_width);
}

void o3_fetch::regStat () {
    _bp_to_fetch_port->regStat ();
}
