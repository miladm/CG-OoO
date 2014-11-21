/*******************************************************************************
 * inoBkEnd.cpp
 *******************************************************************************/

#include "inoBkEnd.h"

sysCore* _core;
sysClock* g_ino_clk;

void inoBkEndRun(FRONTEND_STATUS frontend_status) {
    _core->runCore(frontend_status);
}

void inoBkEnd_init (int argc, char const * argv[]) {
    dbg.print ((DBG_LEVEL)0x1, "Initializing Backend"); //TODO fix this
	g_ino_clk = new sysClock (1);
    _core = new sysCore (g_ino_clk,
                         16, 16, 16, 4, g_cfg->getNumEu (), 4, 4,
                         3, 50, 
                         1, 50, 
                         1, 50, 
                         3, 50, 
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
