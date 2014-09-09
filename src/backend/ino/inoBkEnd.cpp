/*******************************************************************************
 * inoBkEnd.cpp
 *******************************************************************************/

#include "inoBkEnd.h"

sysCore* _core;
sysClock* g_ino_clk;

void inoBkEndRun() {
    _core->runCore();
}

void inoBkEnd_init (int argc, char const * argv[]) {
    dbg.print ((DBG_LEVEL)0x1, "Initializing Backend"); //TODO fix this
	g_ino_clk = new sysClock (1);
    _core = new sysCore (g_ino_clk,
                         4, 8, 8, 8, 4, 4, 4,
                         2, 50, 
                         1, 50, 
                         1, 50, 
                         1, 50, 
                         1, 50, 
                         1, 50, 
                         1, 50, 
                         1, 50, 
                         1, 50, 
                         1, 50);
}

void inoBkEnd_fini () {
    delete _core;
    delete g_ino_clk;
}
