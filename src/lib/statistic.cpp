/*******************************************************************************
 * statistic.cpp
 *******************************************************************************/

#include <iostream>
#include "statistic.h"

statistic::statistic() {
}

statistic::~statistic() {
    set<ScalarStat*>::iterator it;
    for (it = _ScalarStats.begin(); it != _ScalarStats.end(); it++) {
        delete (*it);
    }
}

ScalarStat& statistic::newScalarStat (string class_name, string param_name, string _description, SCALAR init_val, PRINT_ON_ZERO print_if_zero) {
    ScalarStat* cnt = new ScalarStat (class_name, param_name, _description, init_val, print_if_zero);
    _ScalarStats.insert (cnt);
    return *cnt;
}

void statistic::dump () {
    set<ScalarStat*>::iterator it;
    for (it = _ScalarStats.begin(); it != _ScalarStats.end(); it++) {
        (*it)->print ();
    }
}

/********************************/
/********* SCALARSTAT ***********/
/********************************/
ScalarStat::ScalarStat (string class_name, string param_name, string description, SCALAR init_val, PRINT_ON_ZERO print_if_zero) {
    _name = (class_name + ".") + param_name;
    _ScalarStat = init_val;
    _description = description;
    _print_if_zero = print_if_zero;
}

/* PREFIX ++ OPERATOR */
ScalarStat& ScalarStat::operator++ () {
    _ScalarStat++;
    return *this;
}

/* POSTFIX ++ OPERATOR */
ScalarStat ScalarStat::operator++ (int) {
    _ScalarStat++;
    return *this;
}

/* PREFIX -- OPERATOR */
ScalarStat& ScalarStat::operator-- () {
    _ScalarStat--;
    return *this;
}

/* POSTFIX -- OPERATOR */
ScalarStat ScalarStat::operator-- (int) {
    _ScalarStat--;
    return *this;
}

void ScalarStat::print () {
    if (!(_ScalarStat == 0 && _print_if_zero == NO_PRINT_ZERO))
        cout << _name << ": " << _ScalarStat << "\t\t\t - " << _description << endl;
}

statistic g_stats;
