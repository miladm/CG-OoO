/*******************************************************************************
 * benchAddrRangeParser.h
 ******************************************************************************/
#ifndef _BENCH_ADDR_RANGE_PARSER_H
#define _BENCH_ADDR_RANGE_PARSER_H

#include <map>
#include <fstream>
#include <iostream>
#include "../global/global.h"
#include "../config.h"

class benchAddrRangeParser {
    public:
        benchAddrRangeParser (string);
        ~benchAddrRangeParser ();

        /*-- GET ADDRESSES --*/
        ADDRS getStartAddr ();
        ADDRS getEndAddr ();

    private:
        map<string, ADDRS> _bench_start_addr;
        map<string, ADDRS> _bench_end_addr;
        FILE* _bench_addr_space;
        ADDRS _start_addr;
        ADDRS _end_addr;
};

#endif
