/*******************************************************************************
 * sysClock.cpp
 *******************************************************************************/

#include "sysClock.h"

sysClock::sysClock () 
    : _frequency (1),
      _c_name ("sysClock"),
      _clk_cycles (g_stats.newScalarStat ("sysClock", "clk_cycles", "Total execution cycles", START_CYCLE, PRINT_ZERO))
{
	_clk = START_CYCLE;
}

sysClock::sysClock (GHz frequency) 
    : _frequency(frequency),
      _c_name ("sysClock"),
      _clk_cycles (g_stats.newScalarStat ("sysClock", "clk_cycles", "Total execution cycles", START_CYCLE, PRINT_ZERO))
{
	_clk = START_CYCLE;
}

sysClock::~sysClock () {}

void sysClock::tick () {
	_clk++;
    _clk_cycles++;
    if (_clk % RUNTIME_REPORT_INTERVAL == 0) _clk_cycles.print ();
#ifdef ASSERTION
	Assert(	_clk < std::numeric_limits<CYCLE>::max() &&
		   	_clk > std::numeric_limits<CYCLE>::min() &&
			"The program sysClock reached the max vlue for CYCLE" );
#endif
}

CYCLE sysClock::now () {
	return _clk;
}

/*-- ONLY USED FOR STAT --*/
CYCLE sysClock::getValue () {
	return now ();
}
