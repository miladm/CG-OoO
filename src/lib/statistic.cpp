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
        list<ScalarHistStat*>::iterator it;
        for (it = _ScalarHistStats.begin(); it != _ScalarHistStats.end(); it++) {
            delete (*it);
        }
    }
    {
        list<RatioHistStat*>::iterator it;
        for (it = _RatioHistStats.begin(); it != _RatioHistStats.end(); it++) {
            delete (*it);
        }
    }
    {
        list<ScalarStat*>::iterator it;
        for (it = _ScalarStats.begin(); it != _ScalarStats.end(); it++) {
            delete (*it);
        }
    }
    {
        list<RatioStat*>::iterator it;
        for (it = _RatioStats.begin(); it != _RatioStats.end(); it++) {
            delete (*it);
        }
    }
    _out_file.close ();
}

void statistic::setupOutFile () {
    /*-- OUT FILE --*/
    string out_path = "/scratch/milad/qsub_outputs/perf_sim_test/out3/";
    string prog_name (g_cfg->getProgName ());
    string core_type;
    ostringstream core_t, stat_sched_mode, reg_alloc_mode, mem_model;
    core_t << g_cfg->getCoreType ();
    stat_sched_mode << g_cfg->getSchMode ();
    reg_alloc_mode << g_cfg->getRegAllocMode ();
    mem_model << g_cfg->getMemModel ();
    string out_file_path = out_path + prog_name + 
                           "_c" + core_t.str () + 
                           "_s" + stat_sched_mode.str () +
                           "_r" + reg_alloc_mode.str () +
                           "_m" + mem_model.str ();
    _out_file.open (out_file_path.c_str ());
    if(!_out_file) { cerr << "OUTPUT FILE OPEN NOT SUCCESSFUL" << endl; exit (-1); } 
    g_cfg->storeSimConfig (&_out_file);
    cout << "OUT FILE: " << out_file_path << endl;
}

ScalarHistStat& statistic::newScalarHistStat (LENGTH histogram_size, string class_name, string param_name, string _description, SCALAR init_val, PRINT_ON_ZERO print_if_zero) {
    ScalarHistStat* cnt = new ScalarHistStat (histogram_size, class_name, param_name, _description, init_val, print_if_zero);
    _ScalarHistStats.push_back (cnt);
    return *cnt;
}

RatioHistStat& statistic::newRatioHistStat (ScalarStat* divisor, LENGTH histogram_size, string class_name, string param_name, string _description, SCALAR init_val, PRINT_ON_ZERO print_if_zero) {
    RatioHistStat* cnt = new RatioHistStat (divisor, histogram_size, class_name, param_name, _description, init_val, print_if_zero);
    _RatioHistStats.push_back (cnt);
    return *cnt;
}

ScalarStat& statistic::newScalarStat (string class_name, string param_name, string _description, SCALAR init_val, PRINT_ON_ZERO print_if_zero) {
    ScalarStat* cnt = new ScalarStat (class_name, param_name, _description, init_val, print_if_zero);
    _ScalarStats.push_back (cnt);
    return *cnt;
}

RatioStat& statistic::newRatioStat (ScalarStat* divisor, string class_name, string param_name, string _description, SCALAR init_val, PRINT_ON_ZERO print_if_zero) {
    RatioStat* cnt = new RatioStat (divisor, class_name, param_name, _description, init_val, print_if_zero);
    _RatioStats.push_back (cnt);
    return *cnt;
}

void statistic::dump () {
    setupOutFile ();
    {
        list<ScalarHistStat*>::iterator it;
        for (it = _ScalarHistStats.begin (); it != _ScalarHistStats.end (); it++) {
            (*it)->print (&_out_file);
        }
        _out_file << endl;
        cout << endl;
    }
    {
        list<RatioHistStat*>::iterator it;
        for (it = _RatioHistStats.begin (); it != _RatioHistStats.end (); it++) {
            (*it)->print (&_out_file);
        }
        _out_file << endl;
        cout << endl;
    }
    {
        list<ScalarStat*>::iterator it;
        for (it = _ScalarStats.begin (); it != _ScalarStats.end (); it++) {
            (*it)->print (&_out_file);
        }
        _out_file << endl;
        cout << endl;
    }
    {
        list<RatioStat*>::iterator it;
        for (it = _RatioStats.begin (); it != _RatioStats.end (); it++) {
            (*it)->print (&_out_file);
        }
        _out_file << endl;
        cout << endl;
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
    _enable_log_stat = true;
}

stat::~stat () { }

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
    if (!(_ScalarStat == 0 && _print_if_zero == NO_PRINT_ZERO)) {
        cout << "* " << _name << ": " << (DIGIT) _ScalarStat << "\t\t\t # " << _description << endl;
    }
}

void ScalarStat::print (ofstream* _out_file) {
    if (!(_ScalarStat == 0 && _print_if_zero == NO_PRINT_ZERO)) {
        cout << "* " << _name << ": " << (DIGIT) _ScalarStat << "\t\t\t # " << _description << endl;
        if (_enable_log_stat) (*_out_file) << "* " << _name << ": " << (DIGIT) _ScalarStat << "\t\t\t # " << _description << endl;
    }
}

/* **************************** *
 * SCALAR HIST STAT
 * **************************** */
ScalarHistStat::ScalarHistStat (LENGTH histogram_size, string class_name, string param_name, string description, SCALAR init_val, PRINT_ON_ZERO print_if_zero)
    : stat (class_name, param_name, description, init_val, print_if_zero)
{
    _histogram_size = histogram_size;
    _scalar_arr_stat = new stat[_histogram_size];
    _enable_log_stat = g_cfg->isEnLogStat ();
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

void ScalarHistStat::print (ofstream* _out_file) {
    cout << "* " << _name  << ": " << "\t\t\t # " << _description << endl;
    for (LENGTH i = 0; i < _histogram_size; i++) {
        if (!(_scalar_arr_stat[i].getValue () == 0 && _print_if_zero == NO_PRINT_ZERO)) {
            cout << "\t- " << _scalar_arr_stat[i].getName ()  << ": " << (DIGIT) _scalar_arr_stat[i].getValue () << endl;
            if (_enable_log_stat) (*_out_file) << "\t- " << _scalar_arr_stat[i].getName ()  << ": " << (DIGIT) _scalar_arr_stat[i].getValue () << endl;
        }
    }
}

/* **************************** *
 * RATIO STAT
 * **************************** */
RatioStat::RatioStat (ScalarStat* divisor, string class_name, string param_name, string description, SCALAR init_val, PRINT_ON_ZERO print_if_zero)
    : ScalarStat (class_name, param_name, description, init_val, print_if_zero)
{ _divisor = divisor; }

void RatioStat::print (ofstream* _out_file) {
    if (!(_ScalarStat == 0 && _print_if_zero == NO_PRINT_ZERO)) {
        cout << "* " << _name << ": " << _ScalarStat / _divisor->getValue () << "\t\t\t # " << _description << endl;
        if (_enable_log_stat) (*_out_file) << "* " << _name << ": " << _ScalarStat / _divisor->getValue () << "\t\t\t # " << _description << endl;
    }
}

/* **************************** *
 * RATIO HIST STAT
 * **************************** */
RatioHistStat::RatioHistStat (ScalarStat* divisor, LENGTH histogram_size, string class_name, string param_name, string description, SCALAR init_val, PRINT_ON_ZERO print_if_zero)
    : ScalarHistStat (histogram_size, class_name, param_name, description, init_val, print_if_zero)
{ _divisor = divisor; }

void RatioHistStat::print (ofstream* _out_file) {
    cout << "* " << _name  << ": " << "\t\t\t # " << _description << endl;
    for (LENGTH i = 0; i < _histogram_size; i++) {
        if (!(_scalar_arr_stat[i].getValue () == 0 && _print_if_zero == NO_PRINT_ZERO)) {
            cout << "\t- " << _scalar_arr_stat[i].getName ()  << ": " << _scalar_arr_stat[i].getValue () / _divisor->getValue () << endl;
            if (_enable_log_stat) (*_out_file) << "\t- " << _scalar_arr_stat[i].getName ()  << ": " << _scalar_arr_stat[i].getValue () / _divisor->getValue () << endl;
        }
    }
}

statistic g_stats;
