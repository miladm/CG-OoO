/*******************************************************************************
 * TIMER.h
 *******************************************************************************
 * This is a timer object used to model the delay in wires
 ******************************************************************************/
#ifndef _TIMER_H
#define _TIMER_H

#include "../global/global.h"

/* Timer struct that holds a time interval */
struct timer
{
    public:
        timer () {
        	_start = START_CYCLE;
            _latency = START_CYCLE;
        	_stop = _start + _latency;
            _valid = false;
        }
        timer (CYCLE start, CYCLE latency) 
        	: _latency (latency)
        {
        	Assert(latency >= 0 && start >= START_CYCLE);
        	_start = start;
        	_stop = _start + _latency;
            _valid = true;
        }
        void setNewTime (CYCLE start) {
        	Assert(start >= START_CYCLE);
        	_start = start;
        	_stop = _start + _latency;
            _valid = true;
        }
        void setNewTime (CYCLE start, CYCLE latency) {
        	Assert(start >= START_CYCLE);
        	_start = start;
            _latency = latency;
        	_stop = _start + _latency;
            _valid = true;
        }
        CYCLE getStopTime () {
        	Assert (_stop >= _start); 
            Assert (_valid == true);
        	return _stop;
        }
        CYCLE getLatency () {
            Assert (_valid == true);
        	return _latency;
        }
        bool isValidStopTime () { return _valid; }

	private:
		CYCLE _start;
		CYCLE _stop;
		CYCLE _latency;
        bool _valid;
};

#endif
