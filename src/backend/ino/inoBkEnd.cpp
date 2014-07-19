/*******************************************************************************
 * inoBkEnd.cpp
 *******************************************************************************/

#include "inoBkEnd.h"

sysCore* _core;
sysClock* _clk;

void inoBkEndRun() {
    _core->runCore();
}

void inoBkEnd_init (int argc, char const * argv[]) {
    dbg.print ((DBG_LEVEL)0x1, "Initializing Backend"); //TODO fix this
	_clk = new sysClock (1);
    _core = new sysCore (_clk,
                         4, 4, 4, 4, 4, 4, 4,
                         2, 10, 
                         1, 10, 
                         1, 10, 
                         1, 10, 
                         1, 10, 
                         1, 10, 
                         1, 10, 
                         1, 10, 
                         1, 10, 
                         1, 10);
}

void inoBkEnd_fini () {
    delete _core;
    delete _clk;
}
