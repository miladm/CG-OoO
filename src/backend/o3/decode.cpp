/*********************************************************************************
 * o3_decode.cpp
 *********************************************************************************/

#include "decode.h"

o3_decode::o3_decode (port<dynInstruction*>& fetch_to_decode_port, 
			    port<dynInstruction*>& decode_to_schedule_port, 
	    		WIDTH decode_width,
                sysClock* clk,
	    		string stage_name) 
	: stage (decode_width, stage_name, g_cfg->_root["cpu"]["backend"]["pipe"]["decode"], clk)
	  
{
    _fetch_to_decode_port = &fetch_to_decode_port;
	_decode_to_schedule_port  = &decode_to_schedule_port;
}

o3_decode::~o3_decode () {}

void o3_decode::doDECODE () {
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

PIPE_ACTIVITY o3_decode::decodeImpl () {
    PIPE_ACTIVITY pipe_stall = PIPE_STALL;
    for (WIDTH i = 0; i < _stage_width; i++) {
        /*-- CHECKS --*/
        if (_fetch_to_decode_port->getBuffState () == EMPTY_BUFF) break;
        if (!_fetch_to_decode_port->isReady ()) break;
        if (_decode_to_schedule_port->getBuffState () == FULL_BUFF) break;

        /*-- DECODE INS --*/
        dynInstruction* ins = _fetch_to_decode_port->popFront ();
        _e_stage.ffAccess ();
        ins->setPipeStage (DECODE);
        _decode_to_schedule_port->pushBack (ins);
        _e_stage.ffAccess ();
        dbg.print (DBG_FETCH, "%s: %s %llu (cyc: %d)\n", _stage_name.c_str (), "Decode ins", ins->getInsID (), _clk->now ());

        /*-- STAT --*/
        s_ipc++;
        s_ins_cnt++;
        pipe_stall = PIPE_BUSY;
    }
    return pipe_stall;
}

void o3_decode::squash () {
    dbg.print (DBG_SQUASH, "%s: %s (cyc: %d)\n", _stage_name.c_str (), "Decode Ports Flush", _clk->now ());
    Assert (g_var.g_pipe_state == PIPE_FLUSH);
    INS_ID squashSeqNum = g_var.getSquashSN ();
    _decode_to_schedule_port->flushPort (squashSeqNum);
    _e_stage.ffAccess (_stage_width);
}

void o3_decode::regStat () {
    _fetch_to_decode_port->regStat ();
}
