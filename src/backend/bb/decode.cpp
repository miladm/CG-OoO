/*********************************************************************************
 * bb_decode.cpp
 *********************************************************************************/

#include "decode.h"

bb_decode::bb_decode (port<bbInstruction*>& fetch_to_decode_port, 
			          port<bbInstruction*>& decode_to_scheduler_port, 
	    	          WIDTH decode_width,
                      sysClock* clk,
	    	          string stage_name) 
	: stage (decode_width, stage_name, clk)
{
    _fetch_to_decode_port     = &fetch_to_decode_port;
	_decode_to_scheduler_port  = &decode_to_scheduler_port;
}

bb_decode::~bb_decode () {}

void bb_decode::doDECODE () {
    /*-- STAT + DEBUG --*/
    dbg.print (DBG_FETCH, "** %s: (cyc: %d)\n", _stage_name.c_str (), _clk->now ());
    regStat ();
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;

    /*-- SQUASH HANDLING --*/
    if (g_var.g_pipe_state == PIPE_FLUSH) squash ();
    if (! (g_var.g_pipe_state == PIPE_WAIT_FLUSH || g_var.g_pipe_state == PIPE_FLUSH)) {
        pipe_stall = decodeImpl ();
    }

    /*-- STAT --*/
    if (pipe_stall == PIPE_STALL) s_stall_cycles++;
}

PIPE_ACTIVITY bb_decode::decodeImpl () {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;
    for (WIDTH i = 0; i < _stage_width; i++) {
        /*-- CHECKS --*/
        if (_fetch_to_decode_port->getBuffState () == EMPTY_BUFF) break;
        if (!_fetch_to_decode_port->isReady ()) break;
        if (_decode_to_scheduler_port->getBuffState () == FULL_BUFF) break;

        /*-- DECODE INS --*/
        bbInstruction* ins = _fetch_to_decode_port->popFront ();
        ins->setPipeStage (DECODE);
        _decode_to_scheduler_port->pushBack (ins);
        dbg.print (DBG_FETCH, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Decode ins", ins->getInsID (), _clk->now ());

        /*-- STAT --*/
        s_ins_cnt++;
        pipe_stall = PIPE_BUSY;
    }
    return pipe_stall;
}

void bb_decode::squash () {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Decode Ports Flush", _clk->now ());
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
    INS_ID squashSeqNum = g_var.getSquashSN ();
    _decode_to_scheduler_port->flushPort (squashSeqNum, false);
}

void bb_decode::regStat () {
    _fetch_to_decode_port->regStat ();
}
