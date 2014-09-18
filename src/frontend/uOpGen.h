/*******************************************************************************
 * uOpGen.h
 ******************************************************************************/

#ifndef _UOP_GEN_H
#define _UOP_GEN_H

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <map>

#include "pin.H"
#include "pin_isa.H"
#include "instlib.H"
#include "xed-interface.h"

#include "staticCodeParser.h"
#include "../global/global.h"
#include "../global/g_variable.h"
#include "../backend/unit/bbStat.h"

#define ENABLE_MICRO_OPS 0

//extern staticCodeParser* _staticCode;

/*-- PIN INSTRUMENTATION --*/
void pin__uOpGenInit (staticCodeParser &staticCode);
void pin__getOp (INS);

/*-- BB INSTRUMENTATIONS --*/
void pin__detectBB (ADDRINT);
void pin__get_bb_header (ADDRINT, INS);
void pin__getBBhead (ADDRINT, ADDRINT, BOOL);
dynInstruction* pin__makeNewIns (ADDRINT, INS_TYPE);
bbInstruction* pin__makeNewBBIns (ADDRINT, INS_TYPE);

/*-- INSTRUCTION INSTRUMENTATIONS --*/
VOID pin__getBrIns (ADDRINT, BOOL, ADDRINT, ADDRINT, BOOL, BOOL, BOOL, BOOL, BOOL);
VOID pin__getMemIns (ADDRINT, ADDRINT, ADDRINT, BOOL, BOOL, BOOL);
VOID pin__getIns (ADDRINT);
VOID pin__getNopIns (ADDRINT);

#endif
