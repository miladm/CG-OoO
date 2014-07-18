/*******************************************************************************
 * basicblock.h
 ******************************************************************************/
#ifndef _BASICBLOCK_H
#define _BASICBLOCK_H

#include <map>
#include <string.h>
#include <string>
#include <iostream>
#include "../lib/list.h"
#include "../global/global.h"
#include "instruction.h"

class basicblock {
	public:
		basicblock();
		~basicblock();
		void addToBB(string* ins, ADDRS insAddr);
		void addToBB(instruction* ins);
		void addBBheader(string* header, ADDRS bbAddr);
		bool hasBBheader();
		int getNumBBIns_o();
		int getNumBBIns_s();
		string* getNthBBIns_s(int indx);
		instruction* getNthBBIns_o(int indx);
		bool isBBcomplete();

	private:
		List<string*> * _s_insList;
		List<instruction*> * _o_insList;
		List<ADDRS> * _insAddrList;
		string * _BBheader;
		bool _hasBBheader;
		ADDRS _bbAddr;
};

#endif /*_BASICBLOCK_H*/
