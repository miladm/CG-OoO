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

typedef enum {NO_PRINT_ZERO, PRINT_ZERO} PRINT_ON_ZERO;
typedef long int DIGIT;
typedef long double FRACTION;

/* **************************** *
 * STAT
 * **************************** */
class stat {
    public:
        stat ();
        stat (string, string, string, SCALAR init_val = 0, PRINT_ON_ZERO print_if_zero = PRINT_ZERO);
        void init (string, string, string, SCALAR init_val = 0, PRINT_ON_ZERO print_if_zero = PRINT_ZERO);
        ~stat () {}

        /*-- GET FUNCS --*/
        SCALAR getValue ();
        string getName ();

        /*-- OPERATORS --*/
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
 * SCALAR STAT
 * **************************** */
class ScalarStat : public stat {
    public:
        ScalarStat (string, string, string, SCALAR init_val = 0, PRINT_ON_ZERO print_if_zero = PRINT_ZERO);
        ~ScalarStat () {}
        void print ();
};

/* **************************** *
 * SCALAR HIST STAT
 * **************************** */
class ScalarHistStat : public stat {
    public:
        ScalarHistStat (LENGTH, string, string, string, SCALAR init_val = 0, PRINT_ON_ZERO print_if_zero = PRINT_ZERO);
        ~ScalarHistStat ();
        stat& operator[] (LENGTH);
        void print ();

    private:
        stat* _scalar_arr_stat;
        LENGTH _histogram_size;
};

/* **************************** *
 * RATIO STAT
 * **************************** */
class RatioStat : public ScalarStat {
    public:
        RatioStat (ScalarStat* divisor, string, string, string, SCALAR init_val = 0, PRINT_ON_ZERO print_if_zero = PRINT_ZERO);
        ~RatioStat () {}
        void print ();

    private:
        ScalarStat* _divisor;
};

/* **************************** *
 * STATISTIC
 * **************************** */
class statistic {
	public:
		statistic ();
		~statistic ();
        void dump ();
        ScalarHistStat& newScalarHistStat (LENGTH, string, string, string, SCALAR, PRINT_ON_ZERO);
        ScalarStat& newScalarStat (string, string, string, SCALAR, PRINT_ON_ZERO);
        RatioStat& newRatioStat (ScalarStat*, string, string, string, SCALAR, PRINT_ON_ZERO);

    private:
        set<ScalarHistStat*> _ScalarHistStats;
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
