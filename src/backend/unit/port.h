/*******************************************************************************
 * PORT.h
 ******************************************************************************/
#ifndef _PORT_H
#define _PORT_H

#include "unit.h"
#include "dynInstruction.h"
#include "bbInstruction.h"

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
 		port (LENGTH, CYCLE, sysClock*, string port_name = "port");
 		~port ();
 		BUFF_STATE pushBack (queType_T ins);
 		BUFF_STATE pushBack (queType_T ins, CYCLE lat);
		queType_T popFront ();
		queType_T popNextReady ();
		queType_T popNextReadyNow ();
        void delOldReady ();
		queType_T getFront ();
		queType_T getBack ();
		queType_T popBack ();
		LENGTH getBuffSize ();
		BUFF_STATE getBuffState ();
        bool isReady ();
        bool isReadyNow ();
        bool hasReady ();
        bool hasReadyNow ();
        void flushPort (INS_ID, bool);
        void regStat ();

 	private: //variables
		list<BuffElement<queType_T> > _buff;
		const LENGTH _buff_len;
		const CYCLE _wire_delay;

        /* STAT OBJS */
        ScalarStat& s_port_empty_cyc;
        ScalarStat& s_port_full_cyc;
};

#endif
