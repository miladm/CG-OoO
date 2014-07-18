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
#include "../../global/global.h"
#include "../../global/g_variable.h"
#include "../../lib/debug.h"
#include "sysClock.h"
#include "port.h"
#include "dynInstruction.h"
#include "table.h"
#include "exeUnit.h"
//#include "../ino/registerFile.h"
//#include "../o3/registerFile.h"

typedef enum {PIPE_STALL, PIPE_BUSY} PIPE_ACTIVITY;
typedef enum {COMPLETE_NORMAL, COMPLETE_SQUASH} COMPLETE_STATUS;
typedef enum {FRONT_END, BACK_END} SIM_MODE;

class stage {
	public:
		stage(WIDTH stage_width, string _stage_name);
		~stage();

	protected:
		const string _stage_name;
		const WIDTH _stage_width;

        /* STAT OBJS */
        ScalarStat& s_ins_cnt;
        ScalarStat& s_stall_cycles;
        ScalarStat& s_squash_cycles;
};

#endif
