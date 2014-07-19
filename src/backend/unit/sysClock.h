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

class sysClock {
	public:
		sysClock ();
		sysClock (GHz _frequency);
		~sysClock ();

		void tick ();
		CYCLE now ();

	private:
		CYCLE _clk;
		const GHz _frequency;
        const string _c_name;

        /* STAT VARS */
        ScalarStat& _clk_cycles;
};

#endif
