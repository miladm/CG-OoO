/*******************************************************************************
 * debug.cpp
 *******************************************************************************/

#include "debug.h"

DEBUG::DEBUG (DBG_LEVEL dbg) {
    _dbg = dbg;
}

DEBUG::~DEBUG () {}

void DEBUG::print (DBG_LEVEL dbg, const char* fmt, ...) {
#ifdef __do_debug
    if (_dbg & dbg) {
        va_list args;
        va_start (args,fmt);
        vprintf (fmt,args);
        va_end (args);
    }
#endif
}

DEBUG dbg((DBG_LEVEL)0); //TODO fix this
