/*******************************************************************************
 * sysCore.cpp
 *******************************************************************************/

#include "sysCore.h"

sysCore::sysCore (sysClock* clk,
		          WIDTH bp_width, 
		          WIDTH fetch_width, 
		          WIDTH decode_width,
		          WIDTH scheduler_width,
		          WIDTH execution_width,
		          WIDTH memory_width,
		          WIDTH commit_width,
		          CYCLE fetch_to_decode_delay, LENGTH fetch_to_decode_buff_len,
		          CYCLE fetch_to_bp_delay, LENGTH fetch_to_bp_buff_len,
		          CYCLE bp_to_fetch_delay, LENGTH bp_to_fetch_buff_len,
		          CYCLE decode_to_scheduler_delay, LENGTH decode_to_scheduler_buff_len,
		          CYCLE scheduler_to_execution_delay, LENGTH scheduler_to_execution_buff_len,
		          CYCLE execution_to_scheduler_delay, LENGTH execution_to_scheduler_buff_len,
		          CYCLE execution_to_memory_delay, LENGTH execution_to_memory_buff_len,
		          CYCLE memory_to_scheduler_delay, LENGTH memory_to_scheduler_buff_len,
		          CYCLE commit_to_bp_delay, LENGTH commit_to_bp_buff_len,
		          CYCLE commit_to_scheduler_delay, LENGTH commit_to_scheduler_buff_len
		         ) 
	: unit("SysCore", clk),
      _fetch_to_decode_delay (fetch_to_decode_delay),
	  _fetch_to_decode_buff_len (fetch_to_decode_buff_len),
	  _fetch_to_bp_delay (fetch_to_bp_delay),
	  _fetch_to_bp_buff_len (fetch_to_bp_buff_len),
	  _bp_to_fetch_delay (bp_to_fetch_delay),
	  _bp_to_fetch_buff_len (bp_to_fetch_buff_len),
	  _decode_to_scheduler_delay (decode_to_scheduler_delay),
	  _decode_to_scheduler_buff_len (decode_to_scheduler_buff_len),
	  _scheduler_to_execution_delay (scheduler_to_execution_delay),
	  _scheduler_to_execution_buff_len (scheduler_to_execution_buff_len),
	  _execution_to_scheduler_delay (execution_to_scheduler_delay),
	  _execution_to_scheduler_buff_len (execution_to_scheduler_buff_len),
	  _execution_to_memory_delay (execution_to_memory_delay),
	  _execution_to_memory_buff_len (execution_to_memory_buff_len),
	  _memory_to_scheduler_delay (memory_to_scheduler_delay),
	  _memory_to_scheduler_buff_len (memory_to_scheduler_buff_len),
	  _commit_to_bp_delay (commit_to_bp_delay),
	  _commit_to_bp_buff_len (commit_to_bp_buff_len),
	  _commit_to_scheduler_delay (commit_to_scheduler_delay),
	  _commit_to_scheduler_buff_len (commit_to_scheduler_buff_len),
      // INIT PORTS
      _bp_to_fetch_port (bp_to_fetch_buff_len, bp_to_fetch_delay, clk,  "bp_to_fetch_port"),
      _fetch_to_bp_port (fetch_to_bp_buff_len, fetch_to_bp_delay,  clk, "fetch_to_bp_port"),
      _fetch_to_decode_port (fetch_to_decode_buff_len, fetch_to_decode_delay, clk, "fetch_to_decode_port"),
      _decode_to_scheduler_port (decode_to_scheduler_buff_len, decode_to_scheduler_delay, clk, "decode_to_scheduler_port"),
      _scheduler_to_execution_port (scheduler_to_execution_buff_len, scheduler_to_execution_delay, clk, "scheduler_to_execution_port"),
      _execution_to_scheduler_port (execution_to_scheduler_buff_len, execution_to_scheduler_delay, clk, "execution_to_scheduler_port"),
      _execution_to_memory_port (execution_to_memory_buff_len, execution_to_memory_delay, clk, "execution_to_memory_port"),
      _memory_to_scheduler_port (memory_to_scheduler_buff_len, memory_to_scheduler_delay, clk, "memory_to_scheduler_port"),
      _commit_to_bp_port (commit_to_bp_buff_len, commit_to_bp_delay, clk, "commit_to_bp_port"),
      _commit_to_scheduler_port (commit_to_scheduler_buff_len, commit_to_scheduler_delay, clk, "commit_to_scheduler_port")
{
    /* INIT UNITS */
    g_RF_MGR = new rfManager (clk, "rfManager");
    _iROB = new CAMtable<dynInstruction*>(50, 4, 4, _clk, "iROB");
    _iQUE = new CAMtable<dynInstruction*>(1000, 1000, 1000, _clk, "iQUE");

    /* INIT STAGES */
    dbg.print (DBG_CORE, "%s: Constructing CPU Stages", _c_name.c_str ());
    _bp = new branchPred (_fetch_to_bp_port, _bp_to_fetch_port, bp_width, _clk, "branchPred");
    _fetch = new fetch (_bp_to_fetch_port, _fetch_to_decode_port, _fetch_to_bp_port, _iQUE, _iROB, fetch_width, _clk, "fetch");
    _decode = new decode (_fetch_to_decode_port, _decode_to_scheduler_port, decode_width, _clk, "decode");
    _scheduler = new scheduler (_decode_to_scheduler_port, _execution_to_scheduler_port, _memory_to_scheduler_port, _scheduler_to_execution_port, _iROB, scheduler_width, _clk, "schedule");
    _execution = new execution (_scheduler_to_execution_port, _execution_to_scheduler_port, _execution_to_memory_port, _iROB, execution_width, _clk, "execution");
    _memory = new memory (_execution_to_memory_port, _memory_to_scheduler_port, _iROB, memory_width, _clk, "memory");
    _commit = new commit (_commit_to_bp_port, _commit_to_scheduler_port, _iROB, _iQUE, commit_width, _clk, "commit");
}

sysCore::~sysCore () {
    delete g_RF_MGR;
    delete _iROB;
    delete _iQUE;
    delete _bp;
    delete _fetch;
    delete _decode;
    delete _scheduler;
    delete _execution;
}

void sysCore::runCore (FRONTEND_STATUS frontend_status) {
	while (true) {
		_clk->tick ();
        dbg.print (DBG_CORE, "\n** CYCLE %d **\n", _clk->now ());
	    _commit->doCOMMIT ();
	    _memory->doMEMORY ();
	    _execution->doEXECUTION ();
        if (g_var.g_pipe_state == PIPE_SQUASH_ROB) _commit->squash ();
	    _scheduler->doSCHEDULER ();
	    _decode->doDECODE ();
	    if (_fetch->doFETCH (frontend_status) == FRONT_END && g_var.g_pipe_state == PIPE_NORMAL) {
            dbg.print (DBG_CORE, "SWITCH TO FRONTEND\n");
            break;
        }
        _bp->doBP ();
        //if (_clk->now () == 10030) exit (-1); /* for debug */
	}
}
