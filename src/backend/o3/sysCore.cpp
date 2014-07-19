/*******************************************************************************
 * sysCore.cpp
 *******************************************************************************/

#include "sysCore.h"

o3_sysCore::o3_sysCore (GHz clk_frequency,
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
	: unit("SysCore"),
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
      _bp_to_fetch_port (bp_to_fetch_buff_len, bp_to_fetch_delay, "bp_to_fetch_port"),
      _fetch_to_bp_port (fetch_to_bp_buff_len, fetch_to_bp_delay, "fetch_to_bp_port"),
      _fetch_to_decode_port (fetch_to_decode_buff_len, fetch_to_decode_delay, "fetch_to_decode_port"),
      _decode_to_scheduler_port (decode_to_scheduler_buff_len, decode_to_scheduler_delay, "decode_to_scheduler_port"),
      _scheduler_to_execution_port (scheduler_to_execution_buff_len, scheduler_to_execution_delay, "scheduler_to_execution_port"),
      _execution_to_scheduler_port (execution_to_scheduler_buff_len, execution_to_scheduler_delay, "execution_to_scheduler_port"),
      _execution_to_memory_port (execution_to_memory_buff_len, execution_to_memory_delay, "execution_to_memory_port"),
      _memory_to_scheduler_port (memory_to_scheduler_buff_len, memory_to_scheduler_delay, "memory_to_scheduler_port"),
      _commit_to_bp_port (commit_to_bp_buff_len, commit_to_bp_delay, "commit_to_bp_port"),
      _commit_to_scheduler_port (commit_to_scheduler_buff_len, commit_to_scheduler_delay, "commit_to_scheduler_port"),
	  _clk (clk_frequency)
{
    _iROB = new CAMtable<dynInstruction*>(100, 32, 32, "iROB");
    // INIT STAGES
    dbg.print (DBG_CORE, "%s: Constructing CPU Stages", _c_name.c_str ());
    _bp = new o3_branchPred (_fetch_to_bp_port, _bp_to_fetch_port, bp_width, "branchPred");
    _fetch = new o3_fetch (_bp_to_fetch_port, _fetch_to_decode_port, _fetch_to_bp_port, fetch_width, "fetch");
    _decode = new o3_decode (_fetch_to_decode_port, _decode_to_scheduler_port, decode_width, "decode");
    _scheduler = new o3_scheduler (_decode_to_scheduler_port, _execution_to_scheduler_port, _memory_to_scheduler_port, _scheduler_to_execution_port, _iROB, scheduler_width, "schedule");
    _execution = new o3_execution (_scheduler_to_execution_port, _execution_to_scheduler_port, _execution_to_memory_port, _iROB, execution_width, "execution");
    _memory = new o3_memory (_execution_to_memory_port, _memory_to_scheduler_port, _iROB, memory_width, "memory");
    _commit = new o3_commit (_commit_to_bp_port, _commit_to_scheduler_port, _iROB, commit_width, "commit");
}

o3_sysCore::~o3_sysCore () {
    delete _iROB;
    delete _bp;
    delete _fetch;
    delete _decode;
    delete _scheduler;
    delete _execution;
}

void o3_sysCore::runCore () {
	while (true) {
		_clk.tick ();
        dbg.print (DBG_PORT, "\n** CYCLE %d **\n", _clk.now ());
	    _commit->doCOMMIT (_clk);
	    _memory->doMEMORY (_clk);
	    _execution->doEXECUTION (_clk);
        if (g_var.g_pipe_state == PIPE_SQUASH_ROB) _commit->squash (_clk);
	    _scheduler->doSCHEDULER (_clk);
	    _decode->doDECODE (_clk);
	    if (_fetch->doFETCH (_clk) == FRONT_END && g_var.g_pipe_state == PIPE_NORMAL) {
            dbg.print (DBG_CORE, "SWITCH TO FRONTEND\n");
            break;
        }
        _bp->doBP (_clk);
        //if (_clk.now () == 1000) exit (-1);
	}
}
