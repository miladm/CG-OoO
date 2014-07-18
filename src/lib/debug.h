/*******************************************************************************
 * debug.h
 *******************************************************************************/

#ifndef _DEBUG_H
#define _DEBUG_H

#include "../global/global.h"
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdarg.h>

class DEBUG {
    public:
        DEBUG(DBG_LEVEL dbg);
        ~DEBUG();
        void print(DBG_LEVEL dbg, const char* fmt, ...);
    private:
        DBG_LEVEL _dbg;
};

extern DEBUG dbg;

#endif
