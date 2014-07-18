/*******************************************************************************
 * FETCH.cpp
 ******************************************************************************/

#include "fetch.h"

fetch::fetch (port<dynInstruction*>& bp_to_fetch_port, 
	          port<dynInstruction*>& fetch_to_decode_port,
			  port<dynInstruction*>& fetch_to_bp_port,
			  WIDTH fetch_width,
			  string stage_name
			 )
	: stage(fetch_width, stage_name)
{
    _bp_to_fetch_port = &bp_to_fetch_port;
    _fetch_to_bp_port = &fetch_to_bp_port;
    _fetch_to_decode_port = &fetch_to_decode_port;
    _insListIndx = 0;
    _switch_to_frontend = false;
}

fetch::~fetch () {}

SIM_MODE fetch::doFETCH (sysClock& clk) {
    dbg.print (DBG_FETCH, "%s: (cyc: %d)\n", _stage_name.c_str (), clk.now ());
    /* STAT */
    regStat (clk);
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /* SQUASH HANDLING */
    if (g_var.g_pipe_state == PIPE_FLUSH) { squash (clk); }
    if (g_var.g_pipe_state == PIPE_NORMAL) {
        pipe_stall = fetchImpl (clk);
    }

    /* STAT */
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;

    if (_switch_to_frontend) {
        _switch_to_frontend = false;
        return FRONT_END;
    }
	return BACK_END;
}

/* READ FW INSTRUCTIONS FORM THE PIN INPUT QUE */
PIPE_ACTIVITY fetch::fetchImpl (sysClock& clk) {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;
	for (WIDTH i = 0; i < _stage_width; i++) {
		/* CHECKS */
        if (g_var.isCodeCacheEmpty()) { _switch_to_frontend = true; break; }
		if (_fetch_to_decode_port->getBuffState (clk.now ()) == FULL_BUFF) break;

        /* FETCH INS */
        dynInstruction* ins = g_var.popCodeCache();
        ins->setPipeStage(FETCH);
        g_var.remFromCodeCache ();
		_fetch_to_decode_port->pushBack(ins, clk.now());
        dbg.print (DBG_FETCH, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Fetch ins", ins->getInsID (), clk.now ());

        /* STAT */
        s_ins_cnt++;
        pipe_stall = PIPE_BUSY;
	}
    return pipe_stall;
}

void fetch::squash (sysClock& clk) {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Fetch Port Flush", clk.now ());
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
    INS_ID squashSeqNum = g_var.getSquashSN();
    _fetch_to_decode_port->flushPort (squashSeqNum, clk.now ());
    _fetch_to_bp_port->flushPort (squashSeqNum, clk.now ());
}

void fetch::regStat (sysClock& clk) {
    _bp_to_fetch_port->regStat (clk.now ());
}
