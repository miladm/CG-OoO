/*******************************************************************************
 *  make_instruction.h
 ******************************************************************************/

#ifndef _MAKE_INSTRUCTION_H
#define _MAKE_INSTRUCTION_H

#include <stdio.h>
#include <stdlib.h>
#include <set>
#include <map>
#include <string.h>
#include "instruction.h"
#include "config.h"

void parse_instruction(List<instruction*> *insList,
 					   map<ADDR,instruction*> *insAddrMap,
                       std::set<ADDR> *brDstSet,
					   std::map<ADDR, double> *brBiasMap,
					   std::map<ADDR, double> *bpAccuracyMap,
					   std::map<ADDR, double> *upldMap,
					   std::map<ADDR,set<ADDR> > &memRdAddrMap,
					   std::map<ADDR,set<ADDR> > &memWrAddrMap,
					   std::string *program_name);

#endif
