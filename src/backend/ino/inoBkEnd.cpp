/*******************************************************************************
 * inoBkEnd.cpp
 *******************************************************************************/

#include "inoBkEnd.h"

sysCore* _core;
void inoBkEndRun() {
    _core->runCore();
}

void inoBkEnd_init (int argc, char const * argv[]) {
    dbg.print ((DBG_LEVEL)0x1, "Initializing Backend"); //TODO fix this
    _core = new sysCore (1, 4, 4, 4, 4, 4, 4, 4,
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
