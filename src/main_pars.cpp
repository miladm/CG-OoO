/*******************************************************************************
 * main_pars.cpp
 *******************************************************************************/

#include "pin.H"
#include "instlib.H"
#include "xed-interface.h"
#include "frontend/pars.h"

using namespace INSTLIB;

INT32 Usage() {
    PIN_ERROR( "PhArS: Phrase Architecture Simulator\n"
               + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

int main(int argc, char * argv[]) {
    PIN_InitSymbols();
    if(PIN_Init(argc, argv)) return Usage();

	char cfgFile[] = "config/defaul.cfg";
    runPARS(cfgFile);
    
    PIN_StartProgram();
    return 0;
}
