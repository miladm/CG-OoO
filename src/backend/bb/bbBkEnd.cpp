/*******************************************************************************
 * oooBkEnd.cpp
 *******************************************************************************/

#include "oooBkEnd.h"

o3_sysCore* _ooo_core;
sysClock* g_ooo_clk;

void oooBkEndRun() {
    _ooo_core->runCore();
}

void oooBkEnd_init (int argc, char const * argv[]) {
    dbg.print ((DBG_LEVEL)0x1, "Initializing Backend"); //TODO fix this
	g_ooo_clk = new sysClock (1);
    _ooo_core = new o3_sysCore (g_ooo_clk, 
                                4, 32, 32, 32, 4, 4, 4,
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

void oooBkEnd_fini () {
    delete _ooo_core;
    delete g_ooo_clk;
}
