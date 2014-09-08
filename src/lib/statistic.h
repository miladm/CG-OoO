/*******************************************************************************
 * statistic.h
 ******************************************************************************/
#ifndef _STATISTIC_H
#define _STATISTIC_H

#include <map>
#include <set>
#include <string.h>
#include <string>
#include <sstream>
#include "pin.H"
#include "pin_isa.H"
#include "list.h"
#include "../global/global.h"
#include "../backend/unit/sysClock.h"

typedef enum {NO_PRINT_ZERO, PRINT_ZERO} PRINT_ON_ZERO;
typedef long int DIGIT;
typedef double FRACTION;

class sysClock;

/* **************************** *
 * STAT
 * **************************** */
class stat {
    public:
        stat ();
        stat (string, string, string, SCALAR init_val = 0, PRINT_ON_ZERO print_if_zero = PRINT_ZERO);
        void init (string, string, string, SCALAR init_val = 0, PRINT_ON_ZERO print_if_zero = PRINT_ZERO);
        ~stat () {}
        SCALAR getValue ();
        stat& operator++ ();
        stat  operator++ (int);
        stat& operator-- ();
        stat  operator-- (int);
        stat operator+= (SCALAR);

    protected:
        string _name;
        string _description;
        SCALAR _ScalarStat;
        PRINT_ON_ZERO _print_if_zero;
};

/* **************************** *
 * SCALARSTAT
 * **************************** */
class ScalarStat : public stat {
    public:
        ScalarStat (string, string, string, SCALAR init_val = 0, PRINT_ON_ZERO print_if_zero = PRINT_ZERO);
        ~ScalarStat () {}
        void print ();
};

/* **************************** *
 * SCALAR ARRAY STAT
 * **************************** */
class ScalarArryStat : public stat {
    public:
        ScalarArryStat (LENGTH, string, string, string, SCALAR init_val = 0, PRINT_ON_ZERO print_if_zero = PRINT_ZERO);
        ~ScalarArryStat ();
        stat& operator[] (LENGTH);
        void print ();

    private:
        stat* _scalar_arr_stat;
        LENGTH _array_size;
};

/* **************************** *
 * RATIOSTAT
 * **************************** */
class RatioStat : public ScalarStat {
    public:
        RatioStat (sysClock* divisor, string, string, string, SCALAR init_val = 0, PRINT_ON_ZERO print_if_zero = PRINT_ZERO);
        ~RatioStat () {}
        void print ();

    private:
        sysClock* _divisor;
};

/* **************************** *
 * STATISTIC
 * **************************** */
class statistic {
	public:
		statistic ();
		~statistic ();
        void dump ();
        ScalarArryStat& newScalarArryStat (LENGTH, string, string, string, SCALAR, PRINT_ON_ZERO);
        ScalarStat& newScalarStat (string, string, string, SCALAR, PRINT_ON_ZERO);
        RatioStat& newRatioStat (sysClock*, string, string, string, SCALAR, PRINT_ON_ZERO);

    private:
        set<ScalarArryStat*> _ScalarArryStats;
        set<ScalarStat*> _ScalarStats;
        set<RatioStat*> _RatioStats;

    public:
        //TODO DELETE THESE
		int matchIns;
		int noMatchIns;
		set<ADDRINT> missingInsList;
        // TODO delete above
};

extern statistic g_stats;

#endif /*_STATISTIC_H*/
