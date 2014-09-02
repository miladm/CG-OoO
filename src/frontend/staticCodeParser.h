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
#include "../lib/sqlite/test/test.h"
#include "../lib/sqlite/sqlite3.h"
#include "pin.H"
#include "pin_isa.H"
#include "../global/g_variable.h"
#include "../global/global.h"
#include "../config.h"

#define MAX_NUM_uOP_PER_INS 3

class staticCodeParser {
public:
	staticCodeParser(g_variable *g_var, config *g_cfg); //TODO add: benchmark name, path, core type
	~staticCodeParser();
	bool isNewBB(ADDRINT insAddr);
	std::string getBrIns(ADDRINT insAddr, BOOL hasFT, ADDRINT tgAddr, ADDRINT ftAddr, BOOL isTaken);
	std::string getMemIns(ADDRINT insAddr, ADDRINT memAccessSize, ADDRINT memAddr);
	std::string getIns(ADDRINT insAddr);
	std::string getBBheader(ADDRINT bbAddr);
	BOOL BBhasHeader(ADDRINT bbAddr);
	std::string getBB_top(ADDRINT bbAddr); //TODO eliminate this function
	std::string getBB_bottom(); //TODO eliminate this function
	bool isInsIn_insMap(ADDRINT insAddr);

	stInstruction* getInsObj(ADDRINT insAddr);
    void getRegisters(ADDRS insAddr, string registers);

    // SQL interface
    void createDB ();
    void populateDB ();
    void testDB ();
    string iTos (ADDRS);
    bool exists (ADDRS ins_addr);
    void openDB ();
    void closeDB ();

private:
	void parse();
	void makeNewIns(char insType, ADDRINT insAddr, ADDRINT brDest, string registers, ADDRINT memAccessSize);
	void makeNewBB(ADDRINT bbAddr);
	void addToBB(ADDRINT insAddr, ADDRINT bbAddr);
	void addBBheader(ADDRINT insAddr, ADDRINT bbAddr);

	struct insObj{
		string ins_str;
		string registers;
		ADDRINT memAccessSize;
		ADDRINT insAddr;
		ADDRINT brDest;
		char insType[MAX_NUM_uOP_PER_INS];
	};
	struct bbObj{
		std::list<ADDRINT> bbInsSet;
		ADDRINT bbAddr;
		ADDRINT bbHeader;
		BOOL bbHasHeader;
	};
	map<ADDRINT,insObj*> _insMap;
	map<ADDRINT,stInstruction*> _insObjMap;
	map<ADDRINT,bbObj*> _bbMap;
	FILE* _inFile;
	g_variable * _g_var;
	config * _g_cfg;
};

#endif
