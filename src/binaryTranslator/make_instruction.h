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
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include "instruction.h"
#include "config.h"

void parse_instruction(List<instruction*>*,
 					   map<ADDR,instruction*>*,
                       std::set<ADDR>*,
					   std::map<ADDR, double>*,
					   std::map<ADDR, double>*,
					   std::map<ADDR, double>*,
					   std::map<ADDR,set<ADDR> >&,
					   std::map<ADDR,set<ADDR> >&,
					   std::string*,
                       CLUSTER_MODE);

#endif
