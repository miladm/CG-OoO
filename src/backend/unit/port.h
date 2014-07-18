/*******************************************************************************
 * PORT.h
 ******************************************************************************/
#ifndef _PORT_H
#define _PORT_H

#include "unit.h"
#include "dynInstruction.h"

template <typename queType_T>
struct BuffElement {
	BuffElement (CYCLE delay, CYCLE start, queType_T dynIns = NULL) 
		: _delay (start, delay) 
	{
		_dynIns = dynIns;
	}
	timer _delay;
	queType_T _dynIns;
};

template <typename queType_T>
class port : public unit
{
 	public:
 		port (LENGTH, CYCLE, string port_name = "port");
 		~port ();
 		BUFF_STATE pushBack (queType_T ins, CYCLE now);
 		BUFF_STATE pushBack (queType_T ins, CYCLE now, CYCLE lat);
		queType_T popFront (CYCLE now);
		queType_T popNextReady (CYCLE now);
		queType_T popNextReadyNow (CYCLE now);
        void delOldReady (CYCLE now);
		queType_T getFront ();
		queType_T getBack ();
		queType_T popBack ();
		LENGTH getBuffSize ();
		BUFF_STATE getBuffState (CYCLE now);
        bool isReady (CYCLE now);
        bool isReadyNow (CYCLE now);
        bool hasReady (CYCLE now);
        bool hasReadyNow (CYCLE now);
        void flushPort (INS_ID, CYCLE);
        void regStat (CYCLE);

	private: //functions
 		queType_T popFront ();

 	private: //variables
		list<BuffElement<queType_T> > _buff;
		const LENGTH _buff_len;
		const CYCLE _wire_delay;

        /* STAT OBJS */
        ScalarStat& s_port_empty_cyc;
        ScalarStat& s_port_full_cyc;
};

#endif
