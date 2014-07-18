/*******************************************************************************
 * table.h
 ******************************************************************************/

#ifndef _TABLE_H
#define _TABLE_H

#include "unit.h"
#include "dynInstruction.h"

template <typename tableType_T>
struct TableElement {
	TableElement (CYCLE delay, CYCLE start, tableType_T element = NULL)
		: _delay (start, delay) 
    {
		_element = element;
	}
	TableElement (tableType_T element) {
		_element = element;
	}
    public:
	    timer _delay;
	    tableType_T _element;
};

typedef enum {RAM_ARRAY, CAM_ARRAY, FIFO_ARRAY} TABLE_TYPE;

template <typename tableType_T>
class table : public unit {
    public:
 		table (LENGTH, TABLE_TYPE, WIDTH, WIDTH, string);
        ~table () {}
        /* SET ROUTINS */
 		BUFF_STATE pushBack (tableType_T);
 		tableType_T popBack ();
        tableType_T getBack();

        /* CONTROL ROUTINS */
        bool hasFreeRdPort (CYCLE);
        bool hasFreeWrPort (CYCLE);
        BUFF_STATE getTableState ();
        LENGTH getTableSize ();
        tableType_T getNth_unsafe (LENGTH indx);
        void removeNth_unsafe (LENGTH indx);
        void regStat ();

    public:
		List<TableElement<tableType_T>* > _table;
        CYCLE _cycle;
        WIDTH _num_free_wr_port;
        WIDTH _num_free_rd_port;

        /* STAT OBJS */
//      ScalarStat& s_table_empty_cyc;
//      ScalarStat& s_table_full_cyc;

    public:
		const LENGTH _table_size;
        const TABLE_TYPE _table_type;
        const WIDTH _wr_port_cnt;
        const WIDTH _rd_port_cnt;
};

template <typename tableType_T>
class CAMtable : public table<tableType_T> {
    public:
        CAMtable (LENGTH, WIDTH, WIDTH, string);
        ~CAMtable () {}

        //LENGTH getNextReadyIndx ();
        tableType_T pullNextReady (LENGTH);
        tableType_T getNth (LENGTH indx);
        tableType_T pullNth (LENGTH indx);
        tableType_T getFront ();
        tableType_T popFront ();
};

template <typename tableType_T>
class RAMtable : public table<tableType_T> {
    public:
        RAMtable (LENGTH, WIDTH, WIDTH, string);
        ~RAMtable () {}

        //LENGTH getNextReadyIndx ();
        //tableType_T pullNextReady (LENGTH);
};

template <typename tableType_T>
class FIFOtable : public table<tableType_T> {
    public:
 		FIFOtable (LENGTH, WIDTH, WIDTH, string);
        ~FIFOtable () {}

        //bool isFrontReady ();
		tableType_T getFront ();
		tableType_T popFront ();
};

#endif
