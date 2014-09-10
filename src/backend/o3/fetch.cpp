/*********************************************************************************
 * fetch.cpp
 ********************************************************************************/

#include "fetch.h"

o3_fetch::o3_fetch (port<dynInstruction*>& bp_to_fetch_port, 
	          port<dynInstruction*>& fetch_to_decode_port,
			  port<dynInstruction*>& fetch_to_bp_port,
              CAMtable<dynInstruction*>* iQUE,
			  WIDTH fetch_width,
              sysClock* clk,
			  string stage_name
			 )
	: stage(fetch_width, stage_name, clk)
{
    _bp_to_fetch_port = &bp_to_fetch_port;
    _fetch_to_bp_port = &fetch_to_bp_port;
    _fetch_to_decode_port = &fetch_to_decode_port;
    _iQUE = iQUE;
    _insListIndx = 0;
    _switch_to_frontend = false;
}

o3_fetch::~o3_fetch () {}

SIM_MODE o3_fetch::doFETCH () {
    /*-- STAT + DEBUG --*/
    dbg.print (DBG_FETCH, "** %s: (cyc: %d)\n", _stage_name.c_str (), _clk->now ());
    regStat ();
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /*-- SQUASH HANDLING --*/
    if (g_var.g_pipe_state == PIPE_FLUSH) { squash (); }
    if (g_var.g_pipe_state == PIPE_NORMAL) {
        pipe_stall = fetchImpl ();
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
PIPE_ACTIVITY o3_fetch::fetchImpl () {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;
	for (WIDTH i = 0; i < _stage_width; i++) {
		/*-- CHECKS --*/
        if (g_var.isCodeCacheEmpty()) { _switch_to_frontend = true; break; }
		if (_fetch_to_decode_port->getBuffState () == FULL_BUFF) break;

        /*-- FETCH INS --*/
        dynInstruction* ins = g_var.popCodeCache();
        ins->setPipeStage(FETCH);
        g_var.remFromCodeCache ();
        _iQUE->pushBack (ins);
		_fetch_to_decode_port->pushBack(ins);
        dbg.print (DBG_FETCH, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Fetch ins", ins->getInsID (), _clk->now ());

        /*-- STAT --*/
        s_ipc++;
        s_ins_cnt++;
        pipe_stall = PIPE_BUSY;
	}
    return pipe_stall;
}

void o3_fetch::squash () {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Fetch Port Flush", _clk->now ());
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
    INS_ID squashSeqNum = g_var.getSquashSN();
    _fetch_to_decode_port->flushPort (squashSeqNum);
    _fetch_to_bp_port->flushPort (squashSeqNum);
}

void o3_fetch::regStat () {
    _bp_to_fetch_port->regStat ();
}
