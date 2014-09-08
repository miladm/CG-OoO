/*******************************************************************************
 * sysClock.h
 *******************************************************************************
 * the class to track the simulation cycles
 ******************************************************************************/

#ifndef _SYSCLOCK_H
#define _SYSCLOCK_H

#include <limits>
#include "../../global/global.h"
#include "../../lib/statistic.h"

#define RUNTIME_REPORT_INTERVAL 200000

class statistic;
class stat;
class RatioStat;
class ScalarStat;

class sysClock {
	public:
		sysClock ();
		sysClock (GHz _frequency);
		~sysClock ();

		void tick ();
		CYCLE now ();
        CYCLE getValue ();

	private:
		CYCLE _clk;
		const GHz _frequency;
        const string _c_name;

        /* STAT VARS */
        ScalarStat& _clk_cycles;
};

#endif
