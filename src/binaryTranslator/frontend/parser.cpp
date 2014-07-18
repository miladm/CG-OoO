#include "pin.H"
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
using namespace std;


FILE *trace_static;
FILE *mem_trace;
FILE *insAddrs;

/*
 * Analysis routines
 */
VOID trace_info(ADDRINT ins_addr) {
    fprintf(trace_static, "%lld\n", ins_addr);  
}

VOID setDestination (INS ins) {
	if (INS_IsDirectBranchOrCall(ins)) {
		fprintf(trace_static, "%lx\n", INS_DirectBranchOrCallTargetAddress(ins));
	}
}

VOID setType (INS ins) {
	bool noType = true;
	if (INS_IsCall(ins) || INS_IsSyscall(ins)) {
		fprintf(trace_static, "c\n");
		noType = false;
	} if (INS_IsRet(ins) || INS_IsSysret(ins)) {
		fprintf(trace_static, "r\n");
		noType = false;
	} if (INS_IsBranch(ins) && INS_HasFallThrough(ins)) {
		fprintf(trace_static, "b\n");
		noType = false;
	} if (INS_IsBranch(ins) && !INS_HasFallThrough(ins)) {
		fprintf(trace_static, "j\n");
		noType = false;
	} if (INS_IsMemoryRead(ins)) {
		fprintf(trace_static, "R\n");
		fprintf(trace_static, "%d\n", INS_MemoryReadSize(ins)); //size in bytes
		noType = false;
	} if (INS_IsMemoryWrite(ins)) {
		fprintf(trace_static, "W\n");
		fprintf(trace_static, "%d\n", INS_MemoryWriteSize(ins)); //size in bytes
		noType = false;
	} if (noType == true) {
		fprintf(trace_static, "o\n");
	}
}

VOID setReg (INS ins) {
	// UINT32 operandCount = INS_MaxNumRRegs(ins)+INS_MaxNumWRegs(ins);
	if (INS_MaxNumRRegs(ins) > 0) {
		for (int i = 0; i < INS_MaxNumRRegs(ins); i++) {
			fprintf(trace_static,"%s\n%d\n", REG_StringShort(INS_RegR(ins,i)).c_str(), 1);
		}
	}
	if (INS_MaxNumWRegs(ins) > 0) {
		for (int i = 0; i < INS_MaxNumWRegs(ins); i++) {
			fprintf(trace_static,"%s\n%d\n", REG_StringShort(INS_RegW(ins,i)).c_str(), 2);
		}
	}		
}

// Print a memory read record
VOID RecordMemRead(VOID * ip, VOID * addr)
{
	//fprintf(mem_trace,"%lx: R %lx\n", ip, addr);
}

// Print a memory write record
VOID RecordMemWrite(VOID * ip, VOID * addr)
{
    //fprintf(mem_trace,"%lx: W %lx\n", ip, addr);
}

/*
 * Instrumentation routines
 */
VOID ImageLoad(IMG img, VOID *v)
{
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            // Open the RTN.
            RTN_Open( rtn );            
            // Examine each instruction in the routine.
            for( INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins) )
            {
				setType(ins);
				fprintf(trace_static, "#%s\n", INS_Mnemonic(ins).c_str());
				fprintf(trace_static, "%s\n", INS_Disassemble(ins).c_str());
				fprintf(trace_static, "%lx\n", INS_Address(ins));
				//fprintf(insAddrs, "%lx\n", INS_Address(ins)); //for debugging address space
				setDestination(ins);
				setReg(ins);
				fprintf(trace_static, "---\n");
				// Instruments memory accesses using a predicated call
			    UINT32 memOperands = INS_MemoryOperandCount(ins);
			    // Iterate over each memory operand of the instruction.
			    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
			    {
			        if (INS_MemoryOperandIsRead(ins, memOp))
			        {
			            INS_InsertPredicatedCall(
			                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
			                IARG_INST_PTR,
			                IARG_MEMORYOP_EA, memOp,
			                IARG_END);
			        }
			        if (INS_MemoryOperandIsWritten(ins, memOp))
			        {
			            INS_InsertPredicatedCall(
			                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
			                IARG_INST_PTR,
			                IARG_MEMORYOP_EA, memOp,
			                IARG_END);
			        }
			    }
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
	const string ioPath = "/home/milad/esc_project/svn/memTraceMilad/TraceSim/phraseblock_framework/frontend/"; //argv[argc-2];
	trace_static = fopen((ioPath+"/static_trace.s").c_str(), "w");  
	mem_trace    = fopen("/scratch/tracesim/specint2006/mem_trace.csv", "w");  
	insAddrs     = fopen("/scratch/tracesim/specint2006/ins_addrs.csv", "w");  

    // Initialize pin & symbol manager
    if (PIN_Init(argc, argv)) return Usage();
    PIN_InitSymbols();

    // Register ImageLoad to be called to instrument instructions
    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
/* ===================================================================== */
