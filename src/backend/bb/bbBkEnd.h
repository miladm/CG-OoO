/*******************************************************************************
 * bbBkEnd.h
 *******************************************************************************/

#ifndef _G_BB_BACKEND
#define _G_BB_BACKEND

#include <stdint.h>
#include "sysCore.h"

void bbBkEnd_init ();
void bbBkEndRun (FRONTEND_STATUS);
void bbBkEnd_fini ();

#endif
