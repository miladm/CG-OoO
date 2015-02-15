/*******************************************************************************
 * profiler.cpp
 *******************************************************************************/

#include "profiler.h"

profiler::profiler () { }

profiler::~profiler () {}

void profiler::update_ld_profiler (ADDRS ld_addr, CYCLE ld_lat) {
    if (_ld_cnt.find (ld_addr) != _ld_cnt.end ()) {
        _ld_cnt[ld_addr]++;
    } else { _ld_cnt.insert (pair<ADDRS, CYCLE> (ld_addr, 1)); }

    if (ld_lat > L1_LATENCY && 
        _ld_miss_cnt.find (ld_addr) != _ld_miss_cnt.end ()) {
        _ld_miss_cnt[ld_addr]++;
    } else { _ld_miss_cnt.insert (pair<ADDRS, CYCLE> (ld_addr, 1)); }
}

void profiler::update_br_profiler (ADDRS br_addr, bool taken) {
    if (_br_cnt.find (br_addr) != _br_cnt.end ()) {
        _br_cnt[br_addr]++;
    } else { _br_cnt.insert (pair<ADDRS, CYCLE> (br_addr, 1)); }

    if (taken && 
        _br_taken_cnt.find (br_addr) != _br_taken_cnt.end ()) {
        _br_taken_cnt[br_addr]++;
    } else { _br_taken_cnt.insert (pair<ADDRS, CYCLE> (br_addr, 1)); }
}

void profiler::setupOutFiles () {
    /*-- OUT FILE --*/
    _profile_path = g_cfg->getProfilePath ();
    string prog_name (g_cfg->getProgName ());
    _prog_name = prog_name;

    /* UPLD FILE */
    ostringstream upld_thr;
    upld_thr << UPLD_MISS_RAT_THR;
    string out_upld_file_path = _profile_path + "/" + _prog_name + "_upld_" + upld_thr.str () + ".csv";
    _out_upld_file.open (out_upld_file_path.c_str ());
    if(!_out_upld_file) { cerr << "OUTPUT FILE OPEN NOT SUCCESSFUL" << endl; exit (-1); } 
    cout << "UPLD FILE: " << out_upld_file_path << endl;

    /* WBB FILE */
    ostringstream lo_wbb, hi_wbb;
    lo_wbb << LO_BIAS;
    hi_wbb << HI_BIAS;
    string out_wbb_file_path = _profile_path + "/" + _prog_name + "_wbb_" + lo_wbb.str () + "_" + hi_wbb.str () + ".csv";
    _out_wbb_file.open (out_wbb_file_path.c_str ());
    if(!_out_wbb_file) { cerr << "OUTPUT FILE OPEN NOT SUCCESSFUL" << endl; exit (-1); } 
    cout << "WBB FILE: " << out_wbb_file_path << endl;
}

void profiler::dump () {
    setupOutFiles ();

    /* COMPUTE UPLD RATIOS & STORE TO FILE */
    map<ADDRS, FRACTION>::iterator wbb_it, upld_it;
    for (upld_it = _ld_cnt.begin (); upld_it != _ld_cnt.end (); upld_it++) {
        ADDRS ld_addr = upld_it->first;
        FRACTION ld_cnt = upld_it->second;
        FRACTION ld_miss_cnt = 0;
        if (_ld_miss_cnt.find (ld_addr) != _ld_miss_cnt.end ()) 
            ld_miss_cnt = _ld_miss_cnt[ld_addr];
        FRACTION miss_rat = ld_miss_cnt / ld_cnt;
        if (miss_rat >= UPLD_MISS_RAT_THR) 
            _out_upld_file << hex << ld_addr << dec << ", " << miss_rat << "," << ld_miss_cnt << "," << ld_cnt << endl;
    }

    /* COMPUTE WBB RATIOS & STORE TO FILE */
    for (wbb_it = _br_cnt.begin (); wbb_it != _br_cnt.end (); wbb_it++) {
        ADDRS br_addr = wbb_it->first;
        FRACTION br_cnt = wbb_it->second;
        FRACTION taken_cnt = 0;
        if (_br_taken_cnt.find (br_addr) != _br_taken_cnt.end ()) 
            taken_cnt = _br_taken_cnt[br_addr];
        FRACTION bias = taken_cnt / br_cnt;
        if (bias >= LO_BIAS && bias <= HI_BIAS) 
            _out_wbb_file << hex << br_addr << dec << "," << bias << "," << taken_cnt << "," << br_cnt << endl;
    }
}

profiler g_prof;
