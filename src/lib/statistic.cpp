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
    string out_path = g_cfg->getOutPath ();
    string prog_name (g_cfg->getProgName ());
    string core_type;
    ostringstream core_t, stat_sched_mode, reg_alloc_mode, mem_model;
    core_t << g_cfg->getCoreType ();
    stat_sched_mode << g_cfg->getSchMode ();
    reg_alloc_mode << g_cfg->getRegAllocMode ();
    mem_model << g_cfg->getMemModel ();
    string out_file_path = out_path + "/" + prog_name + 
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
    updateStatMap (class_name, param_name, (stat*)cnt);
    return *cnt;
}

RatioHistStat& statistic::newRatioHistStat (ScalarStat* divisor, LENGTH histogram_size, string class_name, string param_name, string _description, SCALAR init_val, PRINT_ON_ZERO print_if_zero) {
    RatioHistStat* cnt = new RatioHistStat (divisor, histogram_size, class_name, param_name, _description, init_val, print_if_zero);
    _RatioHistStats.push_back (cnt);
    updateStatMap (class_name, param_name, (stat*)cnt);
    return *cnt;
}

ScalarStat& statistic::newScalarStat (string class_name, string param_name, string _description, SCALAR init_val, PRINT_ON_ZERO print_if_zero) {
    ScalarStat* cnt = new ScalarStat (class_name, param_name, _description, init_val, print_if_zero);
    _ScalarStats.push_back (cnt);
    updateStatMap (class_name, param_name, (stat*)cnt);
    return *cnt;
}

RatioStat& statistic::newRatioStat (ScalarStat* divisor, string class_name, string param_name, string _description, SCALAR init_val, PRINT_ON_ZERO print_if_zero) {
    RatioStat* cnt = new RatioStat (divisor, class_name, param_name, _description, init_val, print_if_zero);
    _RatioStats.push_back (cnt);
    updateStatMap (class_name, param_name, (stat*)cnt);
    return *cnt;
}

EnergyStat& statistic::newEnergyStat (string class_name, string param_name, string _description, SCALAR init_val, PRINT_ON_ZERO print_if_zero) {
    EnergyStat* cnt = new EnergyStat (class_name, param_name, _description, init_val, print_if_zero);
    _EnergyStats.push_back (cnt);
    updateStatMap (class_name, param_name, (stat*)cnt);
    return *cnt;
}

LeakageEnergyStat& statistic::newLeakageEnergyStat (string class_name, string param_name, string _description, SCALAR init_val, PRINT_ON_ZERO print_if_zero) {
    LeakageEnergyStat* cnt = new LeakageEnergyStat (class_name, param_name, _description, init_val, print_if_zero);
    _LeakageEnergyStats.push_back (cnt);
    updateStatMap (class_name, param_name, (stat*)cnt);
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
    PJ total_energy = 0;
    {
        list<EnergyStat*>::iterator it;
        for (it = _EnergyStats.begin (); it != _EnergyStats.end (); it++) {
            (*it)->print (&_out_file);
            total_energy += (*it)->getEnergyValue ();
        }
        _out_file << endl;
        cout << endl;
    }
    {
        list<LeakageEnergyStat*>::iterator it;
        for (it = _LeakageEnergyStats.begin (); it != _LeakageEnergyStats.end (); it++) {
            (*it)->print (&_out_file, _statMap);
            total_energy += (*it)->getEnergyValue (_statMap);
        }

        /*-- REPORT TOTAL ENERGY --*/
        cout << "* TOTAL Energy: " << total_energy << "\t\t\t # Toal processor energy (PJ)" << endl;
        _out_file << "* TOTAL Energy: " << total_energy << "\t\t\t # Toal processor energy (PJ)" << endl;

        _out_file << endl;
        cout << endl;
    }
}

void statistic::dumpSummary () {
//    _enable_log_stat = false;
    {
        list<RatioStat*>::iterator it;
        for (it = _RatioStats.begin (); it != _RatioStats.end (); it++) {
            (*it)->print (&_out_file);
        }
        _out_file << endl;
        cout << endl;
    }
//    _enable_log_stat = true;
}

void statistic::updateStatMap (string class_name, string param_name, stat* statObj) {
    string name = (class_name + ".") + param_name;
    _statMap.insert (pair<string, stat*>(name, statObj));
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
    if (g_cfg->isWarmedUp () || !g_cfg->warmUpEn ()) _ScalarStat++;
    return *this;
}

/* POSTFIX ++ OPERATOR */
stat stat::operator++ (int) {
    if (g_cfg->isWarmedUp () || !g_cfg->warmUpEn ()) _ScalarStat++;
    return *this;
}

/* PREFIX -- OPERATOR */
stat& stat::operator-- () {
    if (g_cfg->isWarmedUp () || !g_cfg->warmUpEn ()) _ScalarStat--;
    return *this;
}

/* POSTFIX -- OPERATOR */
stat stat::operator-- (int) {
    if (g_cfg->isWarmedUp () || !g_cfg->warmUpEn ()) _ScalarStat--;
    return *this;
}

/* += OPERATOR */
stat stat::operator+= (SCALAR val) {
    if (g_cfg->isWarmedUp () || !g_cfg->warmUpEn ()) _ScalarStat += val;
    return *this;
}

/* -= OPERATOR */
stat stat::operator-= (SCALAR val) {
    if (g_cfg->isWarmedUp () || !g_cfg->warmUpEn ()) _ScalarStat -= val;
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

/* **************************** *
 * ENERGY STAT
 * **************************** */
EnergyStat::EnergyStat (string class_name, string param_name, string description, SCALAR init_val, PRINT_ON_ZERO print_if_zero)
    : ScalarStat (class_name, param_name, description, init_val, print_if_zero)
{ _energy_per_access = 0; }

void EnergyStat::setEnergyPerAccess (PJ energy_per_access) {
    _energy_per_access = energy_per_access; 
}

void EnergyStat::print (ofstream* _out_file) {
    if (!(_ScalarStat == 0 && _print_if_zero == NO_PRINT_ZERO)) {
        cout << "* " << _name << ": " << getEnergyValue () << "\t\t\t # " << _description << endl;
        if (_enable_log_stat) (*_out_file) << "* " << _name << ": " << getEnergyValue () << "\t\t\t # " << _description << endl;
    }
}

PJ EnergyStat::getEnergyValue () {
    return _ScalarStat * _energy_per_access;
}


LeakageEnergyStat::LeakageEnergyStat (string class_name, string param_name, string description, SCALAR init_val, PRINT_ON_ZERO print_if_zero)
    : EnergyStat (class_name, param_name, description, init_val, print_if_zero)
{}

void LeakageEnergyStat::print (ofstream* _out_file, map<string, stat*> &statMap) {
    if (!(_ScalarStat == 0 && _print_if_zero == NO_PRINT_ZERO)) {
        cout << "* " << _name << ": " << getEnergyValue (statMap) << "\t\t\t # " << _description << endl;
        if (_enable_log_stat) (*_out_file) << "* " << _name << ": " << getEnergyValue (statMap) << "\t\t\t # " << _description << endl;
    }
}

PJ LeakageEnergyStat::getEnergyValue (map<string, stat*> &statMap) {
    return statMap["sysClock.clk_cycles"]->getValue () * _energy_per_access;
}

statistic g_stats;
