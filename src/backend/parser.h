/*******************************************************************************
 *  parser.h
 *  Parses instructions from file
 ******************************************************************************/

#ifndef _PARSER_H
#define _PARSER_H
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/list.h"
#include "../global/global.h"
#include "latency.h"
#include "instruction.h"
#include "regFile.h"

//class hist;

class parser {
    public:
	parser(){}
	~parser() {}
	bool parseIns(instruction* newIns);

    private:
	int fetchIns(instruction* ins);
	void resetInput (char *c, int i);
	long int getReg(char *c);
	int getRegType(char *c);
	long int getANumber(char *c);
	long int getAddr(char *c);
};

#endif
