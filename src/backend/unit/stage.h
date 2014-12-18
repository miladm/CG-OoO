/*******************************************************************************
 * stage.h
 ******************************************************************************/
#ifndef _STAGE_H
#define _STAGE_H

#include <string>
#include <iostream>
#include "../../lib/statistic.h"
#include "../../lib/timer.h"
#include "../../lib/utility.h"
#include "../../lib/list.h"
#include "../../lib/profiler.h"
#include "../../global/global.h"
#include "../../global/g_variable.h"
#include "../../global/g_objs.h"
#include "../../lib/debug.h"
#include <yaml/yaml.h>
#include "sysClock.h"
#include "port.h"
#include "bbInstruction.h"
#include "dynInstruction.h"
#include "dynBasicblock.h"
#include "table.h"
#include "exeUnit.h"

typedef enum {PIPE_STALL, PIPE_BUSY} PIPE_ACTIVITY;
typedef enum {COMPLETE_NORMAL, COMPLETE_SQUASH} COMPLETE_STATUS;
typedef enum {FRONT_END, BACK_END} SIM_MODE;

class stage {
	public:
		stage(WIDTH, string, sysClock*);
		~stage();

	protected:
		const string _stage_name;
		const WIDTH _stage_width;
        sysClock* _clk;

        /*-- STAT OBJS --*/
        ScalarStat& s_ins_cnt;
        ScalarStat& s_stall_cycles;
        ScalarStat& s_squash_cycles;
        ScalarStat& s_squash_stall_cycles;
        RatioStat& s_ipc;
};

#endif
