/*******************************************************************************
 * benchAddrRangeParser.cpp
 *******************************************************************************/

#include <iostream>
#include <sstream>
#include "benchAddrRangeParser.h"
#include "message.h"

benchAddrRangeParser::benchAddrRangeParser (string benchmark) {
    g_msg.simStep ("INITIALIZE BENCHMARK ADDRESS SPACE RANGE");
    /*-- OPEN ADDRESS RANGE FILE --*/
    string file_path = "/home/milad/esc_project/svn/PARS/benchmarks/";
    string file_name = "bench_addr_space.csv";
    string file = file_path + file_name;
    _bench_addr_space = fopen (file.c_str (), "r");
    if (!_bench_addr_space) {cerr << "ADDRESS RANGE FILE OPEN NOT SUCCESSFUL" << endl; exit (-1);}

    /*-- READ ADDRESS RANGE FILE INTO MAPS --*/
    while (true) {
        char bench_name[PARSE_LEN]; 
        ADDRS start_addr, end_addr;
        if (fscanf (_bench_addr_space, "%s %llx, %llx\n", bench_name, &start_addr, &end_addr) == EOF) break;
        string bench_name_s = string (bench_name);
        _bench_start_addr.insert (pair<string, ADDRS>(bench_name_s, start_addr));
        _bench_end_addr.insert (pair<string, ADDRS>(bench_name_s, end_addr));
        //cout << bench_name_s << " " << start_addr << " " << end_addr << endl; // DEBUG
    }

    Assert (_bench_start_addr.find (benchmark) != _bench_start_addr.end ());
    Assert (_bench_end_addr.find (benchmark) != _bench_end_addr.end ());

    _start_addr = _bench_start_addr[benchmark];
    _end_addr = _bench_end_addr[benchmark];

    /*-- REPORT --*/
    ostringstream start, end;
    start << hex << _start_addr; end << hex << _end_addr;
    g_msg.simEvent (("START ADDR: 0x" + start.str ()).c_str ());
    g_msg.simEvent (("END ADDR: 0x" + end.str ()).c_str ());
}

ADDRS benchAddrRangeParser::getStartAddr () { return _start_addr; }

ADDRS benchAddrRangeParser::getEndAddr () { return _end_addr; }
