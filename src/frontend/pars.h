/*******************************************************************************
 * pars.h
 *******************************************************************************/

#ifndef _PARS_H
#define _PARS_H

#include <map>
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
#include <bp_lib/types.hh>

#include "../config.h"
#include "memlog.h"
#include "uOpGen.h"
#include "simpointTracker.h"
#include "staticCodeParser.h"
#include "../global/global.h"
#include "../global/g_variable.h"
#include "../lib/utility.h"
//#include "../lib/debug.h"
#include "../lib/message.h"
#include "../lib/statistic.h"
#include "../lib/benchAddrRangeParser.h"
#include "../energy/table_energy.h"
#include "../backend/bkEnd.h"
#include "../backend/basicblock.h"
#include "../backend/ino/inoBkEnd.h"
#include "../backend/o3/oooBkEnd.h"
#include "../backend/bb/bbBkEnd.h"
#include "../backend/unit/dynInstruction.h"
#include "../backend/unit/dynBasicblock.h"

/* ****************************************************************** *
 * FUNCTION DECLARATIONS
 * ****************************************************************** */
VOID pin__parseConfig (string, string, string);
VOID pin__init (string, string, string);
VOID pin__runPARS (string, string, string);
VOID pin__fini (INT32, VOID*);
VOID pin__instruction (TRACE, VOID*);

ADDRINT PredictAndUpdate (ADDRINT, INT32, ADDRINT, ADDRINT, UINT32);

#endif
