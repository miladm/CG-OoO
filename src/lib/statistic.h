/*******************************************************************************
 * statistic.h
 ******************************************************************************/
#ifndef _STATISTIC_H
#define _STATISTIC_H

#include <map>
#include <set>
#include <list>
#include <string.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <time.h>
#include "pin.H"
#include "pin_isa.H"
#include "list.h"
#include "../global/global.h"
#include "../config.h"

typedef enum {NO_PRINT_ZERO, PRINT_ZERO} PRINT_ON_ZERO;

/* **************************** *
 * STAT
 * **************************** */
class stat {
    public:
        stat ();
        stat (string, string, string, SCALAR init_val = 0, PRINT_ON_ZERO print_if_zero = PRINT_ZERO);
        void init (string, string, string, SCALAR init_val = 0, PRINT_ON_ZERO print_if_zero = PRINT_ZERO);
        ~stat ();

        /*-- GET FUNCS --*/
        SCALAR getValue ();
        string getName ();

        /*-- OPERATORS --*/
        stat& operator++ ();
        stat  operator++ (int);
        stat& operator-- ();
        stat  operator-- (int);
        stat operator+= (SCALAR);
        stat operator-= (SCALAR);

    protected:
        string _name;
        string _description;
        SCALAR _ScalarStat;
        PRINT_ON_ZERO _print_if_zero;
        bool _enable_log_stat;
};

/* **************************** *
 * SCALAR STAT
 * **************************** */
class ScalarStat : public stat {
    public:
        ScalarStat (string, string, string, SCALAR init_val = 0, PRINT_ON_ZERO print_if_zero = PRINT_ZERO);
        ~ScalarStat () {}
        void print ();
        void print (ofstream*);
};

/* **************************** *
 * SCALAR HIST STAT
 * **************************** */
class ScalarHistStat : public stat {
    public:
        ScalarHistStat (LENGTH, string, string, string, SCALAR init_val = 0, PRINT_ON_ZERO print_if_zero = PRINT_ZERO);
        ~ScalarHistStat ();
        stat& operator[] (LENGTH);
        void print (ofstream*);

    protected:
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
        void print (ofstream*);

    private:
        ScalarStat* _divisor;
};

/* **************************** *
 * RATIO HIST STAT
 * **************************** */
class RatioHistStat : public ScalarHistStat {
    public:
        RatioHistStat (ScalarStat* divisor, LENGTH, string, string, string, SCALAR init_val = 0, PRINT_ON_ZERO print_if_zero = PRINT_ZERO);
        ~RatioHistStat () {}
        void print (ofstream*);

    private:
        ScalarStat* _divisor;
};

/* **************************** *
 * EENRGY STAT
 * **************************** */
class EnergyStat : public ScalarStat {
    public:
        EnergyStat (string, string, string, SCALAR init_val = 0, PRINT_ON_ZERO print_if_zero = PRINT_ZERO);
        ~EnergyStat () {}
        void setEnergyPerAccess (PJ);
        void print (ofstream*);
        PJ getEnergyValue ();

    protected:
        PJ _energy_per_access;
};

class LeakageEnergyStat : public EnergyStat {
    public:
        LeakageEnergyStat (string, string, string, SCALAR init_val = 0, PRINT_ON_ZERO print_if_zero = PRINT_ZERO);
        ~LeakageEnergyStat () {}
        void print (ofstream*, map<string, stat*>&);
        PJ getEnergyValue (map<string, stat*>&);
};

/* **************************** *
 * STATISTIC
 * **************************** */
class statistic {
	public:
		statistic ();
		~statistic ();
        void dump ();
        void dumpSummary ();
        ScalarHistStat& newScalarHistStat (LENGTH, string, string, string, SCALAR, PRINT_ON_ZERO);
        RatioHistStat& newRatioHistStat (ScalarStat*, LENGTH, string, string, string, SCALAR, PRINT_ON_ZERO);
        ScalarStat& newScalarStat (string, string, string, SCALAR, PRINT_ON_ZERO);
        RatioStat& newRatioStat (ScalarStat*, string, string, string, SCALAR, PRINT_ON_ZERO);
        EnergyStat& newEnergyStat (string, string, string, SCALAR, PRINT_ON_ZERO);
        LeakageEnergyStat& newLeakageEnergyStat (string, string, string, SCALAR, PRINT_ON_ZERO);
        void updateStatMap (string, string, stat*);

    private:
        void setupOutFile ();

    private:
        list<ScalarHistStat*> _ScalarHistStats;
        list<RatioHistStat*> _RatioHistStats;
        list<ScalarStat*> _ScalarStats;
        list<RatioStat*> _RatioStats;
        list<EnergyStat*> _EnergyStats;
        list<LeakageEnergyStat*> _LeakageEnergyStats;
        ofstream _out_file;
        map<string, stat*> _statMap;
};

extern statistic g_stats;

#endif /*_STATISTIC_H*/
