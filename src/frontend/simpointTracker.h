/*******************************************************************************
 * simpointTrack.h
 *******************************************************************************/

#ifndef _SIMPOINT_TRACKER_H
#define _SIMPOINT_TRACKER_H

#include "pin.H"
#include "pin_isa.H"
#include "instlib.H"
#include <bp_lib/types.hh>

#include "../lib/utility.h"
//#include "../lib/debug.h"
#include "../config.h"
#include "uOpGen.h"
#include "../global/global.h"
#include "../lib/message.h"
#include "../lib/statistic.h"
#include "../global/g_variable.h"

BOOL doCount (ScalarStat&, ScalarStat&, 
              ScalarStat&, ScalarStat&, 
              ScalarStat&, ScalarStat&, 
              UINT32, SAMPLING_MODE);

#endif
