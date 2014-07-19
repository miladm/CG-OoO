/*******************************************************************************
 * decode.cpp
 *******************************************************************************/

#include "decode.h"

decode::decode (port<dynInstruction*>& fetch_to_decode_port, 
			    port<dynInstruction*>& decode_to_schedule_port, 
	    		WIDTH decode_width,
                sysClock* clk,
	    		string stage_name) 
	: stage (decode_width, stage_name, clk)
{
    _fetch_to_decode_port = &fetch_to_decode_port;
	_decode_to_schedule_port  = &decode_to_schedule_port;
}

decode::~decode () {}

void decode::doDECODE () {
    dbg.print (DBG_DECODE, "%s: (cyc: %d)\n", _stage_name.c_str (), _clk->now ());
    /* STAT */
    regStat ();
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /* SQUASH HANDLING */
    if (g_var.g_pipe_state == PIPE_FLUSH) squash ();
    if (!(g_var.g_pipe_state == PIPE_WAIT_FLUSH || g_var.g_pipe_state == PIPE_FLUSH)) {
        pipe_stall = decodeImpl ();
    }

    /* STAT */
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;
}

PIPE_ACTIVITY decode::decodeImpl () {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;
    for (WIDTH i = 0; i < _stage_width; i++) {
        /* CHECKS */
        if (_fetch_to_decode_port->getBuffState (_clk->now ()) == EMPTY_BUFF) break;
        if (!_fetch_to_decode_port->isReady (_clk->now ())) break;
        if (_decode_to_schedule_port->getBuffState (_clk->now ()) == FULL_BUFF) break;

        /* DECODE INS */
        dynInstruction* ins = _fetch_to_decode_port->popFront (_clk->now ());
        ins->setPipeStage (DECODE);
        _decode_to_schedule_port->pushBack (ins, _clk->now ());
        dbg.print (DBG_FETCH, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Decode ins", ins->getInsID (), _clk->now ());

        /* STAT */
        s_ins_cnt++;
        pipe_stall = PIPE_BUSY;
    }
    return pipe_stall;
}

void decode::squash () {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Decode Ports Flush", _clk->now ());
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
    INS_ID squashSeqNum = g_var.getSquashSN();
    _decode_to_schedule_port->flushPort (squashSeqNum, _clk->now ());
}

void decode::regStat () {
    _fetch_to_decode_port->regStat (_clk->now ());
}
