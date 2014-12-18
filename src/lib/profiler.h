/*******************************************************************************
 * profiler.h
 ******************************************************************************/
#ifndef _PROFILER_H
#define _PROFILER_H

#include <map>
#include <string.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <yaml/yaml.h>
#include "../config.h"
#include "../global/global.h"

#define LO_BIAS 0.10
#define HI_BIAS 0.90
#define UPLD_MISS_RAT_THR 0.01

class profiler
{
 	public:
 		profiler ();
 		~profiler ();

        void update_ld_profiler (ADDRS, CYCLE);
        void update_br_profiler (ADDRS, bool);
        void dump ();

    private:
        void setupOutFiles ();

 	private: //variables
        map<ADDRS, FRACTION> _ld_miss_cnt;
        map<ADDRS, FRACTION> _ld_cnt;
        map<ADDRS, FRACTION> _br_taken_cnt;
        map<ADDRS, FRACTION> _br_cnt;

        string _profile_path;
        string _prog_name;

        ofstream _out_upld_file;
        ofstream _out_wbb_file;
};

extern profiler g_prof;

#endif /* _PROFILER_H */
