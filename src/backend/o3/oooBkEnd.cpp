/*******************************************************************************
 * oooBkEnd.cpp
 *******************************************************************************/

#include "oooBkEnd.h"

o3_sysCore* _ooo_core;
sysClock* g_ooo_clk;

void oooBkEndRun(FRONTEND_STATUS frontend_status) {
    _ooo_core->runCore(frontend_status);
}

void oooBkEnd_init (int argc, char const * argv[]) {
    dbg.print ((DBG_LEVEL)0x1, "Initializing Backend"); //TODO fix this
	g_ooo_clk = new sysClock (1);
    _ooo_core = new o3_sysCore (g_ooo_clk, 
                                16, 16, 16, 4, g_cfg->getNumEu (), 4, 4,
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

void oooBkEnd_fini () {
    delete _ooo_core;
    delete g_ooo_clk;
}
