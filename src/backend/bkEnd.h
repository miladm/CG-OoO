/*******************************************************************************
 * bkEnd.h
 *******************************************************************************/

#ifndef _G_BACKEND
#define _G_BACKEND

#include "../global/g_variable.h"
#include <stdint.h>

void bkEnd_init (int argc, char const * argv[], g_variable &g_var);
void bkEnd_heading (int argc, char const * argv[]);
void bkEnd_run ();
void bkEnd_finish ();

#endif

#ifndef BP
#define BP
bool PredictAndUpdate(uint64_t __pc, int __taken);
#endif
