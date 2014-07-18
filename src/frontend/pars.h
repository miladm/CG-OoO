/*******************************************************************************
 * pars.h
 *******************************************************************************/
#ifndef _PARS_H
#define _PARS_H

#include "../global/g_variable.h"

/* ------------------------------------------------------------------ */
/* Function Declarations                                              */
/* ------------------------------------------------------------------ */
VOID runPARS(char*);
VOID Init(char*);
VOID Fini(INT32, VOID*);
VOID Instruction(TRACE trace, VOID * val);
ADDRINT PredictAndUpdate(ADDRINT __pc, INT32 __taken, ADDRINT tgt, ADDRINT fthru);
VOID parseConfig();

#endif
