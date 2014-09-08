/*******************************************************************************
 * statistic.cpp
 *******************************************************************************/

#include <iostream>
#include "statistic.h"


/* **************************** *
 * STATISTIC
 * **************************** */
statistic::statistic() { }

statistic::~statistic() {
    {
        set<ScalarArryStat*>::iterator it;
        for (it = _ScalarArryStats.begin(); it != _ScalarArryStats.end(); it++) {
            delete (*it);
        }
    }
    {
        set<ScalarStat*>::iterator it;
        for (it = _ScalarStats.begin(); it != _ScalarStats.end(); it++) {
            delete (*it);
        }
    }
    {
        set<RatioStat*>::iterator it;
        for (it = _RatioStats.begin(); it != _RatioStats.end(); it++) {
            delete (*it);
        }
    }
}

ScalarArryStat& statistic::newScalarArryStat (LENGTH array_size, string class_name, string param_name, string _description, SCALAR init_val, PRINT_ON_ZERO print_if_zero) {
    ScalarArryStat* cnt = new ScalarArryStat (array_size, class_name, param_name, _description, init_val, print_if_zero);
    _ScalarArryStats.insert (cnt);
    return *cnt;
}

ScalarStat& statistic::newScalarStat (string class_name, string param_name, string _description, SCALAR init_val, PRINT_ON_ZERO print_if_zero) {
    ScalarStat* cnt = new ScalarStat (class_name, param_name, _description, init_val, print_if_zero);
    _ScalarStats.insert (cnt);
    return *cnt;
}

RatioStat& statistic::newRatioStat (sysClock* clk, string class_name, string param_name, string _description, SCALAR init_val, PRINT_ON_ZERO print_if_zero) {
    RatioStat* cnt = new RatioStat (clk, class_name, param_name, _description, init_val, print_if_zero);
    _RatioStats.insert (cnt);
    return *cnt;
}

void statistic::dump () {
    {
        set<ScalarArryStat*>::iterator it;
        for (it = _ScalarArryStats.begin (); it != _ScalarArryStats.end (); it++) {
            (*it)->print ();
        }
    }
    {
        set<ScalarStat*>::iterator it;
        for (it = _ScalarStats.begin (); it != _ScalarStats.end (); it++) {
            (*it)->print ();
        }
    }
    {
        set<RatioStat*>::iterator it;
        for (it = _RatioStats.begin (); it != _RatioStats.end (); it++) {
            (*it)->print ();
        }
    }
}

/* **************************** *
 * STAT
 * **************************** */
stat::stat () { }

stat::stat (string class_name, string param_name, string description, SCALAR init_val, PRINT_ON_ZERO print_if_zero)
{
    _name = (class_name + ".") + param_name;
    _description = description;
    _ScalarStat = init_val;
    _print_if_zero = print_if_zero;
}

void stat::init (string class_name, string param_name, string description, SCALAR init_val, PRINT_ON_ZERO print_if_zero)
{
    _name = (class_name + ".") + param_name;
    _description = description;
    _ScalarStat = init_val;
    _print_if_zero = print_if_zero;
}

/* PREFIX ++ OPERATOR */
stat& stat::operator++ () {
    _ScalarStat++;
    return *this;
}

/* POSTFIX ++ OPERATOR */
stat stat::operator++ (int) {
    _ScalarStat++;
    return *this;
}

/* PREFIX -- OPERATOR */
stat& stat::operator-- () {
    _ScalarStat--;
    return *this;
}

/* POSTFIX -- OPERATOR */
stat stat::operator-- (int) {
    _ScalarStat--;
    return *this;
}

/* += OPERATOR */
stat stat::operator+= (SCALAR val) {
    _ScalarStat += val;
    return *this;
}

SCALAR stat::getValue () {
    return _ScalarStat;
}

/* **************************** *
 * SCALAR STAT
 * **************************** */
ScalarStat::ScalarStat (string class_name, string param_name, string description, SCALAR init_val, PRINT_ON_ZERO print_if_zero)
    : stat (class_name, param_name, description, init_val, print_if_zero)
{ }

void ScalarStat::print () {
    if (!(_ScalarStat == 0 && _print_if_zero == NO_PRINT_ZERO))
        cout << _name << ": " << (DIGIT) _ScalarStat << "\t\t\t - " << _description << endl;
}

/* **************************** *
 * SCALAR ARRAY STAT
 * **************************** */
ScalarArryStat::ScalarArryStat (LENGTH array_size, string class_name, string param_name, string description, SCALAR init_val, PRINT_ON_ZERO print_if_zero)
    : stat (class_name, param_name, description, init_val, print_if_zero)
{
    _array_size = array_size;
    _scalar_arr_stat = new stat[_array_size] ();
    for (int i = 0; i < _array_size; i++) {
        ostringstream indx;
        indx << i;
        _scalar_arr_stat[i].init (class_name, param_name + "_" + indx.str (), description, init_val, print_if_zero);
    }
}

ScalarArryStat::~ScalarArryStat () {
    delete _scalar_arr_stat;
}

stat& ScalarArryStat::operator[] (LENGTH index) {
    return _scalar_arr_stat[index];
}

void ScalarArryStat::print () {
    for (LENGTH i = 0; i < _array_size; i++) {
        if (!(_scalar_arr_stat[i].getValue () == 0 && _print_if_zero == NO_PRINT_ZERO))
            cout << _name << ": " << (DIGIT) _scalar_arr_stat[i].getValue () << "\t\t\t - " << _description << endl;
    }
}

/* **************************** *
 * RATIO STAT
 * **************************** */
RatioStat::RatioStat (sysClock* divisor, string class_name, string param_name, string description, SCALAR init_val, PRINT_ON_ZERO print_if_zero)
    : ScalarStat (class_name, param_name, description, init_val, print_if_zero)
{ _divisor = divisor; }

void RatioStat::print () {
    if (!(_ScalarStat == 0 && _print_if_zero == NO_PRINT_ZERO))
        cout << _name << ": " << (FRACTION) _ScalarStat / _divisor->getValue () << "\t\t\t - " << _description << endl;
}

statistic g_stats;
