/*******************************************************************************
 *  annotateTrace.h
 ******************************************************************************/

#ifndef _ANNOTATE_TRACE_H
#define _ANNOTATE_TRACE_H

#include <string>
#include <list>
#include "basicblock.h"
#include "instruction.h"

//Write basicbloc/phraseblock instructions in file
void annotateTrace_forBB (List<basicblock*>* bbList, map<ADDR,instruction*> *insAddrMap, string *program_name);
void annotateTrace_forPB (List<basicblock*>* bbList, map<ADDR,instruction*> *insAddrMap, string *program_name);

#endif