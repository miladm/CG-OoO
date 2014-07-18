/*******************************************************************************
 * sysClock.h
 *******************************************************************************
 * the class to track the simulation cycles
 ******************************************************************************/

#ifndef _SYSClock_H
#define _SYSClock_H

#include <limits>
#include "unit.h"

class sysClock : public unit {
	public:
		sysClock ();
		sysClock (GHz _frequency);
		~sysClock ();

		void tick ();
		CYCLE now ();

	private:
		CYCLE _clk;
		const GHz _frequency;

        /* STAT VARS */
        ScalarStat& _clk_cycles;
};

#endif
