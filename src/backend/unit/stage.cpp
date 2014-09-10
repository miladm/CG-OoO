/*******************************************************************************
 * stage.cpp
 ******************************************************************************/

#include "stage.h"

stage::stage(WIDTH stage_width, string stage_name, sysClock* clk) 
	: _stage_name(stage_name),
      _stage_width(stage_width),
      _clk (clk),
      s_ins_cnt (g_stats.newScalarStat (stage_name, "ins_cnt", "Number of dynamic instructions in " + stage_name, 0, PRINT_ZERO)),
      s_stall_cycles (g_stats.newScalarStat (stage_name, "stall_cycles", "Number of stall cycles in " + stage_name, 0, PRINT_ZERO)),
      s_squash_cycles (g_stats.newScalarStat (stage_name, "squash_cycles", "Number of squash cycles in " + stage_name, 0, NO_PRINT_ZERO)),
      s_squash_stall_cycles (g_stats.newScalarStat (stage_name, "squash_stall_cycles", "Number of stall cycles in squash" + stage_name, 0, NO_PRINT_ZERO)),
      s_ipc  (g_stats.newRatioStat (clk, stage_name, "ipc", "Instructions per cycle of " +stage_name, 0, PRINT_ZERO))
{
	Assert(!_stage_name.empty() && _stage_width > 0);
}

stage::~stage() {}
