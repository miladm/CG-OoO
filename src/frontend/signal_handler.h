/*******************************************************************************
 * signal_handler.h
 *******************************************************************************/

#ifndef _SIGNAL_HANDLER_H
#define _SIGNAL_HANDLER_H

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include "pin.H"
#include "pin_isa.H"
#include "global.h"
#include "g_variable.h"

void recover();

EXCEPT_HANDLING_RESULT handlePinException(THREADID tid, EXCEPTION_INFO * pExceptInfo, PHYSICAL_CONTEXT * pPhysCtxt, VOID *v);

BOOL signal_handler(THREADID tid, INT32 sig, CONTEXT *ctxt, BOOL hasHandler, const EXCEPTION_INFO *pExceptInfo, VOID *v);

#endif
