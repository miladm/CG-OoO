#include "pin.H"
#include "instlib.H"
#include "portability.H"
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include "stInstruction.h"
#include "../../lib/benchAddrRangeParser.h"
#include "../../lib/message.h"

using namespace std;
using namespace INSTLIB;

KNOB<string> KnobBenchName (KNOB_MODE_WRITEONCE, "pintool",
            "b", "bench", "specify input benchmark config file name");

FILE* trace_static;
FILE* mem_trace;
FILE* insAddrs;

List<stInstruction*>* g_ins_list;
benchAddrRangeParser* bench_addr_space;

VOID setTarget (INS ins) {
    if (g_ins_list->NumElements () != 2) return;
    stInstruction* cur_ins = g_ins_list->Nth (1);
    stInstruction* prv_ins = g_ins_list->Nth (0);
	if (cur_ins->_has_destination == false && INS_IsDirectBranchOrCall (ins)) {
		cur_ins->_has_destination = true;
        cur_ins->_destination = INS_DirectBranchOrCallTargetAddress(ins);
    }

    if (prv_ins->_type == 'r' ||
        (prv_ins->_type == 'j' && prv_ins->_has_destination == false) ||
        (prv_ins->_type == 'c' && prv_ins->_has_destination == false)) {
        prv_ins->_destination = INS_Address (ins);
        prv_ins->_has_destination = true;
    }

    // Not sure if I should add the following check to the above line - TODO for later
    if (prv_ins->_type == 'b' && prv_ins->_has_destination == false) {
        cout << prv_ins->_disassemble << endl;
        Assert (0 && "this feature is not supported");
    }
}

VOID setType (INS ins) {
    stInstruction* st_ins = g_ins_list->Last ();
	bool noType = true;
	if (INS_IsSyscall(ins)) {
        st_ins->_type = 's';
		noType = false;
    } else if(INS_IsCall(ins) || INS_IsProcedureCall (ins)) {
        st_ins->_type = 'c';
		noType = false;
	} else if(INS_IsRet(ins) || INS_IsSysret(ins)) {
        st_ins->_type = 'r';
		noType = false;
	} else if(INS_IsBranch(ins) && INS_HasFallThrough(ins)) {
        st_ins->_type = 'b';
		noType = false;
	} else if(INS_IsBranch(ins) && !INS_HasFallThrough(ins)) {
        st_ins->_type = 'j';
		noType = false;
	} else if(INS_IsMemoryRead(ins)) {
        st_ins->_type = 'R';
        st_ins->_mem_r_size = INS_MemoryReadSize(ins);
		noType = false;
	} else if(INS_IsMemoryWrite(ins)) {
        st_ins->_type = 'W';
        st_ins->_mem_w_size = INS_MemoryWriteSize(ins);
		noType = false;
	} else if(INS_IsNop(ins)) {
        st_ins->_type = 'n';
		noType = false;
	} else if(noType == true) {
        st_ins->_type = 'o';
	}
}

VOID setReg (INS ins) {
    stInstruction* st_ins = g_ins_list->Last ();
	// UINT32 operandCount = INS_MaxNumRRegs(ins)+INS_MaxNumWRegs(ins);
	if (INS_MaxNumRRegs(ins) > 0) {
		for (int i = 0; i < (int)INS_MaxNumRRegs(ins); i++) {
			st_ins->_regs.Append (REG_StringShort(INS_RegR(ins,i)));
			st_ins->_reg_types.Append (REG_READ);
		}
	}
	if (INS_MaxNumWRegs(ins) > 0) {
		for (int i = 0; i < (int)INS_MaxNumWRegs(ins); i++) {
			st_ins->_regs.Append (REG_StringShort(INS_RegW(ins,i)));
			st_ins->_reg_types.Append (REG_WRITE);
		}
	}
}

void dumpIns () {
    Assert (g_ins_list->NumElements () <= 2);
    if (g_ins_list->NumElements () == 2) {
        stInstruction* st_ins = g_ins_list->Nth (0);
        st_ins->dump ();
        delete g_ins_list->Nth (0);
        g_ins_list->RemoveAt (0);
    }
}

void makeNewIns () {
    stInstruction* st_ins = new stInstruction (trace_static);
    g_ins_list->Append (st_ins);
}

/*
 * Instrumentation routines
 */
//VOID ImageLoad(TRACE trace, VOID *v)
VOID ImageLoad(IMG img, VOID *v)
{
//    for (BBL bbl = TRACE_BblHead (trace); BBL_Valid (bbl); bbl = BBL_Next (bbl))
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            // Open the RTN.
            RTN_Open( rtn );            
            // Examine each instruction in the routine.
//            for (INS ins = BBL_InsHead (bbl); INS_Valid (ins); ins = INS_Next (ins))
            for( INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins) )
            {
                if (!(INS_Address (ins) >= bench_addr_space->getStartAddr () && 
                      INS_Address (ins) <= bench_addr_space->getEndAddr ())) continue;
                makeNewIns ();
                stInstruction* st_ins = g_ins_list->Last ();
                setType (ins);
                st_ins->_mnemonic = INS_Mnemonic (ins);
                st_ins->_disassemble = INS_Disassemble (ins);
                st_ins->_addr = INS_Address (ins);
                if (INS_HasFallThrough (ins)) {
                    st_ins->_has_fall_through = true; 
                    st_ins->_fall_through = INS_NextAddress (ins);
                }
                setTarget (ins);
                setReg (ins);
                dumpIns ();
            }
            // Close the RTN.
            RTN_Close( rtn );
        }
    }
}

VOID Fini(INT32 code, VOID *v)
{
    fclose(trace_static);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This is the invocation pintool" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    g_msg.simStep ("INITIALIZE STATIC CODE PARSER");
    // Initialize pin & symbol manager
    if (PIN_Init(argc, argv)) return Usage();
    PIN_InitSymbols();

	const string ioPath = "/home/milad/esc_project/svn/PARS/src/binaryTranslator/frontend/"; //argv[argc-2];
	trace_static = fopen((ioPath+"/static_trace.s").c_str(), "w");  
	mem_trace    = fopen("/scratch/tracesim/specint2006/mem_trace.csv", "w");  
	insAddrs     = fopen("/scratch/tracesim/specint2006/ins_addrs.csv", "w");  
    g_ins_list = new List<stInstruction*>;
    string benchmark = KnobBenchName.Value ();
    g_msg.simEvent (("BENCHMARK: " + benchmark).c_str ());
    bench_addr_space = new benchAddrRangeParser (benchmark);
    g_msg.simStep ("START OF CODE GENERATION");


    // Register ImageLoad to be called to instrument instructions
    IMG_AddInstrumentFunction(ImageLoad, 0);
//    TRACE_AddInstrumentFunction (ImageLoad, 0);
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
/* ===================================================================== */
