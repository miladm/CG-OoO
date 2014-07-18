/*******************************************************************************
 * sysClock.cpp
 *******************************************************************************/

#include "sysClock.h"

sysClock::sysClock () 
	: unit ("sysClock"),
      _frequency(1),
      _clk_cycles (g_stats.newScalarStat ("sysClock", "clk_cycles", "Total execution cycles", START_CYCLE, PRINT_ZERO))
{
	_clk = START_CYCLE;
}

sysClock::sysClock (GHz frequency) 
	: unit ("sysClock"),
      _frequency(frequency),
      _clk_cycles (g_stats.newScalarStat ("sysClock", "clk_cycles", "Total execution cycles", START_CYCLE, PRINT_ZERO))
{
	_clk = START_CYCLE;
}

sysClock::~sysClock () {}

void sysClock::tick () {
	_clk++;
    _clk_cycles++;
#ifdef ASSERTION
	Assert(	_clk < std::numeric_limits<CYCLE>::max() &&
		   	_clk > std::numeric_limits<CYCLE>::min() &&
			"The program sysClock reached the max vlue for CYCLE" );
#endif
}

CYCLE sysClock::now () {
	return _clk;
}
