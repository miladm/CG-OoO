/*******************************************************************************
 * oooBkEnd.cpp
 *******************************************************************************/

#include "oooBkEnd.h"

o3_sysCore* _ooo_core;
void oooBkEndRun() {
    _ooo_core->runCore();
}

void oooBkEnd_init (int argc, char const * argv[]) {
    dbg.print ((DBG_LEVEL)0x1, "Initializing Backend"); //TODO fix this
    _ooo_core = new o3_sysCore (1, 4, 32, 32, 32, 4, 4, 4,
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
