/*******************************************************************************
 * main_pars.cpp
 *******************************************************************************/

#include "pin.H"
#include "instlib.H"
#include "xed-interface.h"
#include "frontend/pars.h"

using namespace INSTLIB;

KNOB<string> KnobBenchInputFile (KNOB_MODE_WRITEONCE, "pintool",
            "b", "bench", "specify input benchmark config file name");
KNOB<string> KnobSimCfgInputFile (KNOB_MODE_WRITEONCE, "pintool",
            "c", "default", "specify input simulator config file name");


INT32 Usage () {
    PIN_ERROR ( "PhArS: Phrase Architecture Simulator\n"
               + KNOB_BASE::StringKnobSummary () + "\n");
    return -1;
}

int main (int argc, char * argv[]) {
    PIN_InitSymbols ();
    if (PIN_Init (argc, argv)) return Usage ();

    std::string benchmark = KnobBenchInputFile.Value ().c_str ();
    std::string sim_cfg = KnobSimCfgInputFile.Value ().c_str ();
    cout << sim_cfg << " " << benchmark << endl;
    pin__runPARS (benchmark, sim_cfg);
    
    PIN_StartProgram ();
    return 0;
}
