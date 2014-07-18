/*******************************************************************************
 * sysCore.h
 *******************************************************************************
 * this class sets up the processor elements (stages, connections, etc) and runs 
 * the backend of the simulator
 ******************************************************************************/

#ifndef _SYSCORE_H
#define _SYSCORE_H

#include "registerFile.h"
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

class sysCore : public unit {
	public:
		sysCore (GHz clk_frequency,
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
             );
        ~sysCore();
		void runCore ();

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
		port<dynInstruction*> _bp_to_fetch_port;
		port<dynInstruction*> _fetch_to_bp_port;
		port<dynInstruction*> _fetch_to_decode_port;
		port<dynInstruction*> _decode_to_scheduler_port;
		port<dynInstruction*> _scheduler_to_execution_port;
        port<dynInstruction*> _execution_to_scheduler_port;
		port<dynInstruction*> _execution_to_memory_port;
        port<dynInstruction*> _memory_to_scheduler_port;
        port<dynInstruction*> _commit_to_bp_port;
        port<dynInstruction*> _commit_to_scheduler_port;

		// STAGES
		branchPred* _bp;
		fetch* _fetch;
		decode* _decode;
		scheduler* _scheduler;
		execution* _execution;
		memory* _memory;
		commit* _commit;

		// PROGRAM CLOCK
		sysClock _clk;

        // MISC
        CAMtable<dynInstruction*>* _iROB;
};

#endif
