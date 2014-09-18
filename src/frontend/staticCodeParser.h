/*******************************************************************************
 * staticCodeParser.h
 *******************************************************************************/

#ifndef _STATIC_CODE_PARSER_H
#define _STATIC_CODE_PARSER_H

#include <map>
#include <list>
#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include "pin.H"
#include "pin_isa.H"
#include "../global/g_variable.h"
#include "../global/global.h"
#include "../config.h"
#include "../lib/statistic.h"

#define MAX_NUM_uOP_PER_INS 3

class staticCodeParser {
    public:
        staticCodeParser (config*); //TODO add: benchmark name, path, core type
        ~staticCodeParser ();

        /*-- INS FUNCTIONS --*/
        BOOL hasIns (ADDRINT);
        stInstruction* getInsObj (ADDRINT);

        /*-- BB FUNCTIONS --*/
        BOOL isNewBB (ADDRINT);
        BOOL BBhasHeader (ADDRINT);
        string getBB_top (ADDRINT); //TODO eliminate this function
        string getBBheader (ADDRINT);
        string getBB_bottom (); //TODO eliminate this function
        list<ADDRS>& getBBinsList (ADDRINT);
        bool hasStaticBB (ADDRINT);
        BOOL bbHasBr (ADDRINT);
        ADDRINT getBBbr (ADDRINT);

    private:
        void parse ();
        void getRegisters (ADDRS, string);

        /*-- INS FUNCTIONS --*/
        void makeNewIns (char, ADDRINT, ADDRINT, string, ADDRINT);

        /*-- BB FUNCTIONS --*/
        void makeNewBB (ADDRINT);
        void addToBB (ADDRINT, ADDRINT, char);
        void addBBheader (ADDRINT, ADDRINT);

    private:
        struct bbObj{
            bbObj () {
                hasBr = false;
                brAddr = 0;
            }
            list<ADDRS> bbInsList;
            ADDRINT bbAddr;
            ADDRINT bbHeader;
            BOOL bbHasHeader;
            BOOL hasBr;
            ADDRINT brAddr;
        };
        map<ADDRINT, stInstruction*> _insObjMap;
        map<ADDRINT, bbObj*> _bbMap;
        FILE* _inFile;
        config * _g_cfg;

        /*-- STAT --*/
        ScalarStat& s_missing_static_bb_cnt;
};

#endif
