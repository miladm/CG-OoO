/*******************************************************************************
 * uop_gen.h
 ******************************************************************************/

#ifndef _UOP_GEN_H
#define _UOP_GEN_H

#include <stdio.h>
#include "pin.H"
#include "xed-interface.h"
#include "pin_isa.H"
#include "instlib.H"
#include <iostream>
#include <fstream>
#include <map>
#include <string.h>
#include <string>
#include "../global/g_variable.h"
#include "../global/global.h"
#include "staticCodeParser.h"
#include "../backend/unit/dynInstruction.h"

void uop_gen(FILE* _outFile, staticCodeParser &g_staticCode);
void get_uop(INS ins);

#endif
