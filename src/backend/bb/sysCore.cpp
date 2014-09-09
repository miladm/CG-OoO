/*********************************************************************************
 * sysCore.cpp
 *********************************************************************************/

#include "sysCore.h"

bb_sysCore::bb_sysCore (sysClock* clk,
		          WIDTH bp_width, 
		          WIDTH fetch_width, 
		          WIDTH decode_width,
		          WIDTH scheduler_width,
		          WIDTH execution_width,
		          WIDTH memory_width,
		          WIDTH commit_width,
			      WIDTH num_bbWin,
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
      _bp_to_fetch_port (bp_to_fetch_buff_len, bp_to_fetch_delay, _clk, "bp_to_fetch_port"),
      _fetch_to_bp_port (fetch_to_bp_buff_len, fetch_to_bp_delay, _clk, "fetch_to_bp_port"),
      _fetch_to_decode_port (fetch_to_decode_buff_len, fetch_to_decode_delay, _clk, "fetch_to_decode_port"),
      _decode_to_scheduler_port (decode_to_scheduler_buff_len, decode_to_scheduler_delay, _clk, "decode_to_scheduler_port"),
      _scheduler_to_execution_port (scheduler_to_execution_buff_len, scheduler_to_execution_delay, _clk, "scheduler_to_execution_port"),
      _execution_to_scheduler_port (execution_to_scheduler_buff_len, execution_to_scheduler_delay, _clk, "execution_to_scheduler_port"),
      _execution_to_memory_port (execution_to_memory_buff_len, execution_to_memory_delay, _clk, "execution_to_memory_port"),
      _memory_to_scheduler_port (memory_to_scheduler_buff_len, memory_to_scheduler_delay, _clk, "memory_to_scheduler_port"),
      _commit_to_bp_port (commit_to_bp_buff_len, commit_to_bp_delay, _clk, "commit_to_bp_port"),
      _commit_to_scheduler_port (commit_to_scheduler_buff_len, commit_to_scheduler_delay, _clk, "commit_to_scheduler_port")
{
    /*-- INIT UNITS --*/
    _RF_MGR = new bb_rfManager (num_bbWin, _clk, "rfManager");
    _LSQ_MGR = new bb_memManager (_memory_to_scheduler_port, _clk, "lsqManager");
    _bbROB = new CAMtable<dynBasicblock*>(100, 32, 32, _clk, "bbROB");
    _bbQUE = new CAMtable<dynBasicblock*>(1000, 1000, 1000, _clk, "bbQUE");
//    _RF_MGR = new bb_grfManager (_clk, "grfManager"); //TODO remove it
    for (WIDTH i = 0; i < num_bbWin; i++) {
        ostringstream bbWin_num;
        bbWin_num << i;
        bbWindow* bbWin = new bbWindow (bbWin_num.str (), _clk);
        _bbWindows.Append (bbWin);
    } //TODO remove it - put back in scheduler.cpp

    /*-- INIT STAGES --*/
    dbg.print (DBG_CORE, "%s: Constructing CPU Stages", _c_name.c_str ());
    _bp = new bb_branchPred (_fetch_to_bp_port, _bp_to_fetch_port, bp_width, _clk, "branchPred");
    _fetch = new bb_fetch (_bp_to_fetch_port, _fetch_to_decode_port, _fetch_to_bp_port, _bbROB, _bbQUE, fetch_width, _clk, "fetch");
    _decode = new bb_decode (_fetch_to_decode_port, _decode_to_scheduler_port, decode_width, _clk, "decode");
    _scheduler = new bb_scheduler (_decode_to_scheduler_port, _execution_to_scheduler_port, _memory_to_scheduler_port, _scheduler_to_execution_port, &_bbWindows, num_bbWin, _bbROB, scheduler_width, _LSQ_MGR, _RF_MGR, _clk, "schedule");
    _execution = new bb_execution (_scheduler_to_execution_port, _execution_to_scheduler_port, &_bbWindows, num_bbWin, _bbROB, execution_width, _LSQ_MGR, _RF_MGR, _clk, "execution");
    _memory = new bb_memory (_execution_to_memory_port, _memory_to_scheduler_port, &_bbWindows, num_bbWin, _bbROB, memory_width, _LSQ_MGR, _RF_MGR, _clk, "memory");
    _commit = new bb_commit (_commit_to_bp_port, _commit_to_scheduler_port, &_bbWindows, num_bbWin, _bbROB, _bbQUE, commit_width, _LSQ_MGR, _RF_MGR, _clk, "commit");
}

bb_sysCore::~bb_sysCore () {
    delete _RF_MGR;
    delete _LSQ_MGR;
    delete _bbROB;
    delete _bbQUE;
    delete _bp;
    delete _fetch;
    delete _decode;
    delete _scheduler;
    delete _execution;
    while (_bbWindows.NumElements () > 0) {
        delete _bbWindows.Nth (0);
        _bbWindows.RemoveAt (0);
    }
}

void bb_sysCore::runCore () {
	while (true) {
		_clk->tick ();
        dbg.print (DBG_PORT, "\n** CYCLE %d **\n", _clk->now ());
	    _commit->doCOMMIT ();
	    _memory->doMEMORY ();
	    _execution->doEXECUTION ();
        if (g_var.g_pipe_state == PIPE_SQUASH_ROB) {
            _fetch->squashCurrentBB ();
            _commit->squash ();
        }
	    _scheduler->doSCHEDULER ();
	    _decode->doDECODE ();
	    if (_fetch->doFETCH () == FRONT_END && g_var.g_pipe_state == PIPE_NORMAL) {
            dbg.print (DBG_CORE, "SWITCH TO FRONTEND\n");
            break;
        }
        _bp->doBP ();
//        if (_clk->now () == 100000) {
//            for (int i = 0; i < _bbROB->getTableSize (); i++) {
//                dynBasicblock* bb = _bbROB->getNth(i);
//                List<bbInstruction*>* insList = bb->getBBinsList ();
//                for (int i = insList->NumElements () - 1; i >= 0; i--) {
//                    bbInstruction* ins = insList->Nth (i);
//                    cout << "stage: " << ins->getPipeStage () << 
//                        " id: " << ins->getInsID () <<
//                        " type: " << ins->getInsType () << endl;
//                }
//            }
//            exit (-1); /*-- for debug --*/
//        }
	}
}
