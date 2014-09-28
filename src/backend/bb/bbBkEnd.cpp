/*******************************************************************************
 * bbBkEnd.cpp
 *******************************************************************************/

#include "bbBkEnd.h"

bb_sysCore* _bb_core;
sysClock* g_bb_clk;

void bbBkEndRun() {
    _bb_core->runCore();
}

void bbBkEnd_init (int argc, char const * argv[]) {
    dbg.print ((DBG_LEVEL)0x1, "Initializing Backend"); //TODO fix this
	g_bb_clk = new sysClock (1);
    _bb_core = new bb_sysCore (g_bb_clk, 
                                16, 16, 16, 16, 4, 4, 4, 16,
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

void bbBkEnd_fini () {
    delete _bb_core;
    delete g_bb_clk;
}
