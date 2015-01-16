/*******************************************************************************
 * sysCore.h
 *******************************************************************************
 * this class sets up the processor elements (stages, connections, etc) and runs 
 * the backend of the simulator
 ******************************************************************************/

#ifndef _BB_SYSCORE_H
#define _BB_SYSCORE_H

#include "branchPred.h"
#include "fetch.h"
#include "decode.h"
#include "schedulers.h"
#include "execution.h"
#include "memory.h"
#include "commit.h"

#include "../unit/sysClock.h"
#include "../unit/unit.h"
#include "../unit/table.h"
#include "../unit/port.h"
#include "memManager.h"
#include "rfManager.h"
#include "bbWindow.h"

class bb_sysCore : public unit {
	public:
		bb_sysCore (sysClock* clk,
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
             );
        ~bb_sysCore();
		void runCore (FRONTEND_STATUS);

    private:
        void debugDump ();

	private:
		// PORT CONFIGS
		const CYCLE  _fetch_to_decode_delay;
		const LENGTH _fetch_to_decode_buff_len;
		const CYCLE  _fetch_to_bp_delay;
		const LENGTH _fetch_to_bp_buff_len;
		const CYCLE  _bp_to_fetch_delay;
		const LENGTH _bp_to_fetch_buff_len;
		const CYCLE  _decode_to_scheduler_delay;
		const LENGTH _decode_to_scheduler_buff_len;
		const CYCLE  _scheduler_to_execution_delay;
		const LENGTH _scheduler_to_execution_buff_len;
		const CYCLE  _execution_to_scheduler_delay;
		const LENGTH _execution_to_scheduler_buff_len;
		const CYCLE  _execution_to_memory_delay;
		const LENGTH _execution_to_memory_buff_len;
		const CYCLE  _memory_to_scheduler_delay;
		const LENGTH _memory_to_scheduler_buff_len;
		const CYCLE  _commit_to_bp_delay;
		const LENGTH _commit_to_bp_buff_len;
		const CYCLE  _commit_to_scheduler_delay;
		const LENGTH _commit_to_scheduler_buff_len;

		// PORTS OBJS
		port<bbInstruction*> _bp_to_fetch_port;
		port<bbInstruction*> _fetch_to_bp_port;
		port<bbInstruction*> _fetch_to_decode_port;
		port<bbInstruction*> _decode_to_scheduler_port;
        port<bbInstruction*> _execution_to_scheduler_port;
		port<bbInstruction*> _execution_to_memory_port;
        port<bbInstruction*> _memory_to_scheduler_port;
        port<bbInstruction*> _commit_to_bp_port;
        port<bbInstruction*> _commit_to_scheduler_port;
		List<port<bbInstruction*>*>* _scheduler_to_execution_port;

		// STAGES
		bb_branchPred* _bp;
		bb_fetch* _fetch;
		bb_decode* _decode;
		bb_scheduler* _scheduler;
		bb_execution* _execution;
		bb_memory* _memory;
		bb_commit* _commit;

        // RF & LSQ
        bb_memManager* _LSQ_MGR;
        bb_rfManager* _RF_MGR;

        // BB Windows
        List<bbWindow*> _bbWindows;

        // MISC
        CAMtable<dynBasicblock*>* _bbROB;
        CAMtable<dynBasicblock*>* _bbQUE;

        WIDTH _num_block_ports;
        WIDTH _alu_cnt;
        WIDTH _alu_cnt_per_blk;
};

#endif
