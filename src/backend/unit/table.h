/*******************************************************************************
 * table.h
 ******************************************************************************/

#ifndef _TABLE_H
#define _TABLE_H

#include "unit.h"
#include "wires.h"
#include "dynInstruction.h"
#include "bbInstruction.h"
#include "sysClock.h"

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
        table (LENGTH, TABLE_TYPE, WIDTH, WIDTH, sysClock*, const YAML::Node&, string);
        ~table () {}

        /* SET ROUTINS */
        BUFF_STATE pushBack (tableType_T);
        tableType_T popBack ();
        tableType_T getBack ();

        /* CONTROL ROUTINS */
        BUFF_STATE getTableState ();
        LENGTH getTableSize ();
        tableType_T getNth_unsafe (LENGTH);
        void removeNth_unsafe (LENGTH);
        void regStat ();

        /* WIRES CTRL */
        void updateWireState (AXES_TYPE);
        bool hasFreeWire (AXES_TYPE);

    public:
        List<TableElement<tableType_T>* > _table;
        CYCLE _cycle;
        wires _wr_port;
        wires _rd_port;
        const LENGTH _table_size;
        const TABLE_TYPE _table_type;

        /*-- ENERGY --*/
        energy _energy;

        /* STAT OBJS */
        ScalarStat& s_table_empty_cyc;
        ScalarStat& s_table_full_cyc;
        RatioStat& s_table_size_rat;
};

template <typename tableType_T>
class CAMtable : public table<tableType_T> {
    public:
        CAMtable (LENGTH, WIDTH, WIDTH, sysClock*, const YAML::Node&, string);
        ~CAMtable () {}

        //LENGTH getNextReadyIndx ();
        tableType_T pullNextReady (LENGTH);
        tableType_T getNth (LENGTH indx);
        tableType_T pullNth (LENGTH indx);
        tableType_T getFront ();
        tableType_T popFront ();
        tableType_T getLast ();
};

template <typename tableType_T>
class RAMtable : public table<tableType_T> {
    public:
        RAMtable (LENGTH, WIDTH, WIDTH, sysClock*, const YAML::Node&, string);
        ~RAMtable () {}

        //LENGTH getNextReadyIndx ();
        //tableType_T pullNextReady (LENGTH);
};

template <typename tableType_T>
class FIFOtable : public table<tableType_T> {
    public:
 		FIFOtable (LENGTH, WIDTH, WIDTH, sysClock*, const YAML::Node&, string);
        ~FIFOtable () {}

        //bool isFrontReady ();
		tableType_T getFront ();
		tableType_T popFront ();
};

#endif
