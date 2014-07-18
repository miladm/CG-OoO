/*******************************************************************************
 * decode.cpp
 *******************************************************************************/

#include "decode.h"

decode::decode (port<dynInstruction*>& fetch_to_decode_port, 
			    port<dynInstruction*>& decode_to_schedule_port, 
	    		WIDTH decode_width,
	    		string stage_name) 
	: stage (decode_width, stage_name)
	  
{
    _fetch_to_decode_port = &fetch_to_decode_port;
	_decode_to_schedule_port  = &decode_to_schedule_port;
}

decode::~decode () {}

void decode::doDECODE (sysClock& clk) {
    dbg.print (DBG_DECODE, "%s: (cyc: %d)\n", _stage_name.c_str (), clk.now ());
    /* STAT */
    regStat (clk);
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /* SQUASH HANDLING */
    if (g_var.g_pipe_state == PIPE_FLUSH) squash (clk);
    if (!(g_var.g_pipe_state == PIPE_WAIT_FLUSH || g_var.g_pipe_state == PIPE_FLUSH)) {
        pipe_stall = decodeImpl (clk);
    }

    /* STAT */
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;
}

PIPE_ACTIVITY decode::decodeImpl (sysClock& clk) {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;
    for (WIDTH i = 0; i < _stage_width; i++) {
        /* CHECKS */
        if (_fetch_to_decode_port->getBuffState (clk.now ()) == EMPTY_BUFF) break;
        if (!_fetch_to_decode_port->isReady (clk.now ())) break;
        if (_decode_to_schedule_port->getBuffState (clk.now ()) == FULL_BUFF) break;

        /* DECODE INS */
        dynInstruction* ins = _fetch_to_decode_port->popFront (clk.now ());
        ins->setPipeStage (DECODE);
        _decode_to_schedule_port->pushBack (ins, clk.now ());
        dbg.print (DBG_FETCH, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Decode ins", ins->getInsID (), clk.now ());

        /* STAT */
        s_ins_cnt++;
        pipe_stall = PIPE_BUSY;
    }
    return pipe_stall;
}

void decode::squash (sysClock& clk) {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Decode Ports Flush", clk.now ());
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
    INS_ID squashSeqNum = g_var.getSquashSN();
    _decode_to_schedule_port->flushPort (squashSeqNum, clk.now ());
}

void decode::regStat (sysClock& clk) {
    _fetch_to_decode_port->regStat (clk.now ());
}
