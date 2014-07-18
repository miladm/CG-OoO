/*******************************************************************************
 * statistic.h
 ******************************************************************************/
#ifndef _STATISTIC_H
#define _STATISTIC_H

#include <map>
#include <set>
#include <string.h>
#include <string>
#include "pin.H"
#include "pin_isa.H"
#include "list.h"
#include "../global/global.h"

typedef enum {NO_PRINT_ZERO, PRINT_ZERO} PRINT_ON_ZERO;

class ScalarStat {
    public:
        ScalarStat (string, string, string, SCALAR init_val = 0, PRINT_ON_ZERO print_if_zero = PRINT_ZERO);
        ~ScalarStat () {}
        void print ();
        ScalarStat& operator++ ();
        ScalarStat  operator++ (int);
        ScalarStat& operator-- ();
        ScalarStat  operator-- (int);

    private:
        string _name;
        string _description;
        SCALAR _ScalarStat;
        PRINT_ON_ZERO _print_if_zero;
};

class statistic {
	public:
		statistic ();
		~statistic ();
        void dump ();
        ScalarStat& newScalarStat (string, string, string, SCALAR, PRINT_ON_ZERO);

    private:
        set<ScalarStat*> _ScalarStats;

    public:
        //TODO DELETE THESE
		int matchIns;
		int noMatchIns;
		set<ADDRINT> missingInsList;
        // TODO delete above
};

extern statistic g_stats;

#endif /*_STATISTIC_H*/
