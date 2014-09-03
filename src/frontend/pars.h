/*******************************************************************************
 * pars.h
 *******************************************************************************/

#ifndef _PARS_H
#define _PARS_H

#include <map>
#include <set>
#include <string>
#include <time.h>
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <cassert>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <setjmp.h>

#include "pin.H"
#include "pin_isa.H"
#include "instlib.H"
#include "lib/bp_lib/types.hh"

#include "tournament.hh"
#include "lib/bp_lib/types.hh"
#include "lib/bp_lib/intmath.hh"
#include "tournament.hh"
#include "utilities.h"

#include "../lib/utility.h"
//#include "../lib/debug.h"
#include "../config.h"
#include "uOpGen.h"
#include "../global/global.h"
#include "memlog.h"
#define G_I_INFO_EN 1
#include "staticCodeParser.h"
#include "../lib/statistic.h"
#include "../global/g_info.h"
#include "../global/g_variable.h"
#include "../backend/bkEnd.h"
#include "../backend/basicblock.h"
#include "../backend/ino/inoBkEnd.h"
#include "../backend/o3/oooBkEnd.h"
#include "../backend/bb/bbBkEnd.h"
#include "../backend/unit/dynInstruction.h"
#include "../backend/unit/dynBasicblock.h"


/* ------------------------------------------------------------------ */
/* Function Declarations                                              */
/* ------------------------------------------------------------------ */
VOID runPARS (char*);
VOID Init (char*);
VOID Fini (INT32, VOID*);
VOID Instruction (TRACE, VOID*);
ADDRINT PredictAndUpdate (ADDRINT, INT32, ADDRINT, ADDRINT);
VOID parseConfig ();

#endif
