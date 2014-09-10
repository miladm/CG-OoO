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
        set<ScalarHistStat*>::iterator it;
        for (it = _ScalarHistStats.begin(); it != _ScalarHistStats.end(); it++) {
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

ScalarHistStat& statistic::newScalarHistStat (LENGTH histogram_size, string class_name, string param_name, string _description, SCALAR init_val, PRINT_ON_ZERO print_if_zero) {
    ScalarHistStat* cnt = new ScalarHistStat (histogram_size, class_name, param_name, _description, init_val, print_if_zero);
    _ScalarHistStats.insert (cnt);
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
        set<ScalarHistStat*>::iterator it;
        for (it = _ScalarHistStats.begin (); it != _ScalarHistStats.end (); it++) {
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

string stat::getName () {
    return _name;
}

/* **************************** *
 * SCALAR STAT
 * **************************** */
ScalarStat::ScalarStat (string class_name, string param_name, string description, SCALAR init_val, PRINT_ON_ZERO print_if_zero)
    : stat (class_name, param_name, description, init_val, print_if_zero)
{ }

void ScalarStat::print () {
    if (!(_ScalarStat == 0 && _print_if_zero == NO_PRINT_ZERO))
        cout << "* " << _name << ": " << (DIGIT) _ScalarStat << "\t\t\t # " << _description << endl;
}

/* **************************** *
 * SCALAR HIST STAT
 * **************************** */
ScalarHistStat::ScalarHistStat (LENGTH histogram_size, string class_name, string param_name, string description, SCALAR init_val, PRINT_ON_ZERO print_if_zero)
    : stat (class_name, param_name, description, init_val, print_if_zero)
{
    _histogram_size = histogram_size;
    _scalar_arr_stat = new stat[_histogram_size];
    for (LENGTH i = 0; i < _histogram_size; i++) {
        ostringstream indx;
        indx << i;
        string param = param_name + "_" + indx.str ();
        _scalar_arr_stat[i].init (class_name, param, description, init_val, print_if_zero);
    }
}

ScalarHistStat::~ScalarHistStat () {
    delete _scalar_arr_stat;
}

stat& ScalarHistStat::operator[] (LENGTH index) {
    return _scalar_arr_stat[index];
}

void ScalarHistStat::print () {
    cout << "* " << _name  << ": " << "\t\t\t # " << _description << endl;
    for (LENGTH i = 0; i < _histogram_size; i++) {
        if (!(_scalar_arr_stat[i].getValue () == 0 && _print_if_zero == NO_PRINT_ZERO))
            cout << "\t- " << _scalar_arr_stat[i].getName ()  << ": " << (DIGIT) _scalar_arr_stat[i].getValue () << endl;
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
        cout << "* " << _name << ": " << _ScalarStat / _divisor->getValue () << "\t\t\t # " << _description << endl;
}

statistic g_stats;
