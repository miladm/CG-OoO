/*******************************************************************************
 * stage.cpp
 ******************************************************************************/

#include "stage.h"

stage::stage(WIDTH stage_width, string stage_name) 
	: _stage_name(stage_name),
	_stage_width(stage_width),
    s_ins_cnt (g_stats.newScalarStat (stage_name, "ins_cnt", "Number of dynamic instructions in "+stage_name, 0, PRINT_ZERO)),
    s_stall_cycles (g_stats.newScalarStat (stage_name, "stall_cycles", "Number of stall cycles in "+stage_name, 0, PRINT_ZERO)),
    s_squash_cycles (g_stats.newScalarStat (stage_name, "squash_cycles", "Number of squash cycles in "+stage_name, 0, NO_PRINT_ZERO))
{
	Assert(!_stage_name.empty() && _stage_width > 0);
}

stage::~stage() {}
