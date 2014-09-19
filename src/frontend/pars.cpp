/*******************************************************************************
 * pars.cpp
 *******************************************************************************/

#define INS_CNT_THR 1000
#define BB_CNT_THR 100 + BB_NEAR_EMPTY_SIZE
#define G_I_INFO_EN 1

#include "pars.h"
#include "../global/g_info.h"
#include "tournament.hh"
#include "lib/bp_lib/types.hh"
#include "lib/bp_lib/intmath.hh"
#include "tournament.hh"
#include "utilities.h"

using namespace INSTLIB;

/* ******************************************************************* *
 * STAT GLOBAL VARIABLES
 * ******************************************************************* */
static ScalarStat& s_pin_fr_to_bk_cnt (g_stats.newScalarStat ("pars", "pin_fr_to_bk_cnt", "Number of FRONTEND->BACKEND switches", 0, PRINT_ZERO));
static ScalarStat& s_pin_wp_cnt (g_stats.newScalarStat ("pars", "pin_wp_cnt", "Number of wrong-path events", 0, NO_PRINT_ZERO));
static ScalarStat& s_pin_ins_cnt (g_stats.newScalarStat ("pars", "pin_ins_cnt", "Number of instructions instrumented in frontend", 0, NO_PRINT_ZERO));
static ScalarStat& s_pin_sig_cnt (g_stats.newScalarStat ("pars", "pin_sig_cnt", "Number of SIGNAL events in frontend", 0, NO_PRINT_ZERO));
static ScalarStat& s_pin_sig_recover_cnt (g_stats.newScalarStat ("pars", "pin_sig_recover_cnt", "Number of SIGNAL recovery events in frontend", 0, NO_PRINT_ZERO));
static ScalarStat& s_pin_flush_cnt (g_stats.newScalarStat ("pars", "pin_flush_cnt", "Number of Pintool code cache flush events", 0, NO_PRINT_ZERO));
static ScalarStat& s_pin_trace_cnt (g_stats.newScalarStat ("pars", "pin_trace_cnt", "Number of Pintool code cache traces", 0, NO_PRINT_ZERO));

/* ******************************************************************* *
 * GLOBAL VARIABLES
 * ******************************************************************* */

/*-- CONTAINS KNOBS TO FILTER OUT THINGS TO INSTRUMENT --*/
FILTER filter;

unsigned char g_store_buffer[MAX_MEM_WRITE_LEN];
#ifdef G_I_INFO_EN
map<unsigned,i_info> g_i_info;
#endif
clock_t start_pars, stop_pars;
TournamentBP *g_predictor = NULL;
PIN_SEMAPHORE semaphore0, semaphore1; // semaphore that serializes access to global vars
void * rootThreadArg = (void *)0xABBA;
PIN_THREAD_UID rootThreadUid;
config * g_cfg;
staticCodeParser * g_staticCode;

/* ****************************************************************** *
 * CLASS OBJECTS INTERFACE FUNCTIONS
 * ****************************************************************** */
memory_buffer g_log;
/* ================================================================== */

void recover ()
{
	g_var.g_wrong_path = false;
	g_var.g_total_wrong_path_count += g_var.g_wrong_path_count;
	s_pin_sig_recover_cnt++;
	if (g_var.g_debug_level & DBG_SPEC) {
		cout << " *** recovering to correct path ***\n";
		cout << " recovery count = " << dec << s_pin_sig_recover_cnt.getValue () << endl;
		cout << " wrong path ins count = " << g_var.g_wrong_path_count << " instructions (avg: " 
		          << (double) g_var.g_total_wrong_path_count/ s_pin_sig_recover_cnt.getValue () << ")\n";
	}
	g_var.g_wrong_path_count = 0;
	g_var.g_spec_syscall = false;
	g_var.g_context_call_depth=0;
	g_log.recover ();
}


EXCEPT_HANDLING_RESULT handlePinException (THREADID tid, EXCEPTION_INFO * pExceptInfo, PHYSICAL_CONTEXT * pPhysCtxt, VOID *v)
{
	g_var.g_pin_signal_count++;
    if (g_var.g_wrong_path) {
        if (g_var.g_debug_level & DBG_SPEC) cout << " caught signal " << dec << PIN_GetExceptionCode (pExceptInfo) << " on wrong path " << endl;
        if (g_var.g_debug_level & DBG_SPEC) cout << " execption Info: " << PIN_ExceptionToString (pExceptInfo) << endl;
        if (g_var.g_debug_level & DBG_SPEC) cout << " pin signal count = " << dec << g_var.g_pin_signal_count << endl;
        recover ();
        PIN_ExecuteAt (&g_var.g_context);
        if (g_var.g_debug_level & DBG_SPEC) cout << "Recovered from signal." << endl;
    } else {
        cout << "ERROR for REAL: caught signal " << dec << PIN_GetExceptionCode (pExceptInfo) << " on correct path " << endl;
        cout << " execption Info: " << PIN_ExceptionToString (pExceptInfo) << endl;
        cout << " pin signal count = " << dec << g_var.g_pin_signal_count << endl;
    }
	return EHR_UNHANDLED;
}

BOOL signal_handler (THREADID tid, INT32 sig, CONTEXT *ctxt, BOOL hasHandler, const EXCEPTION_INFO *pExceptInfo, VOID *v)
{
    if (sig != 11 && sig != 4 && sig != 7) cout << sig << ',' << g_var.g_wrong_path << endl;
    g_var.g_app_signal_count++;
    if (g_var.g_wrong_path) {
        if (g_var.g_debug_level & DBG_SPEC) cout << " caught signal " << dec << sig << " on wrong path " << endl;
        if (g_var.g_debug_level & DBG_SPEC) cout << " application signal count = " << dec << g_var.g_app_signal_count << endl;
        recover ();
        PIN_SaveContext (&g_var.g_context,ctxt);
        if (g_var.g_debug_level & DBG_SPEC) cout << "Recovered from signal." << endl;
        return FALSE;
    } else {
        cout << "ERROR for REAL: caught signal " << dec << sig << " on correct path " << endl;
        cout << " application signal count = " << dec << g_var.g_app_signal_count << endl;
        return TRUE; // pass exception 
    }
}

VOID HandleInst (UINT32 uid, BOOL __is_call, BOOL __is_ret, BOOL __is_far_ret)
{
    if (!g_var.g_enable_wp) return;
#ifdef G_I_INFO_EN
    Assert (g_i_info.find (uid)!=g_i_info.end ());
    const i_info &i = g_i_info[uid];
#endif

    if (!g_var.g_wrong_path) g_var.g_icount++;

#ifdef G_I_INFO_EN
    if (g_var.g_debug_level & DBG_EXEC) cout << "EXEC i  " << (g_var.g_wrong_path?"*":" ") << " " << dec << g_var.g_seq_num << " : " << hex << i.pc << " " << i.diss << " : ";
#endif

    if (g_var.g_wrong_path) {
        g_var.g_wrong_path_count++;
        if (__is_call) g_var.g_context_call_depth++;
        if (g_var.g_debug_level & DBG_SPEC) cout << " *** wrong path *** count = " << dec << g_var.g_wrong_path_count << "\n";
        if ( (g_var.g_wrong_path_count >= g_var.g_branch_mispredict_delay) ||
           ( (g_var.g_context_call_depth==0) && __is_ret) ||
                g_var.g_invalid_size || g_var.g_invalid_addr || g_var.g_spec_syscall || __is_far_ret || __is_call || __is_ret) {
            recover ();
            if (g_var.g_debug_level & DBG_SPEC) cout << "Recovered from signal." << endl;
            if (g_var.g_enable_wp) {
                PIN_ExecuteAt (&g_var.g_context);
            }
        }
    } else if (g_var.g_debug_level & DBG_EXEC) cout << endl;
    return;
}

VOID countTrace (TRACE trace, VOID * v)
{
    s_pin_trace_cnt++;
    g_var.g_codeCacheSize +=  TRACE_CodeCacheSize (trace);
    if (g_var.g_debug_level & DBG_CC) cout << "--Code Cache Size Limit: " << CODECACHE_CacheSizeLimit () / (1024 * 1024) << "MB.\n";
    if (g_var.g_debug_level & DBG_CC) cout << "Adding Trace #" << s_pin_trace_cnt.getValue () << " (Addr: " << TRACE_CodeCacheAddress (trace) << ") to code cache with size " << TRACE_CodeCacheSize (trace) << " Bytes.\n";
    if (g_var.g_debug_level & DBG_CC) cout << "Total code cache size: " << g_var.g_codeCacheSize / (1024 * 1024) << "MB.\n";
}

/*--
 * WHEN NOTIFIED BY PIN THAT THE CACHE IS FULL, PERFORM A FLUSH AND TELL THE
 * USER ABOUT IT.
 --*/
VOID FlushOnFull (UINT32 trace_size, UINT32 stub_size)
{
	s_pin_flush_cnt++;
    if (g_var.g_debug_level & DBG_CC) cout << "Trying to insert trace size " << trace_size << " and exit stub size " << stub_size << ".\n";
	CODECACHE_FlushCache ();
	if (g_var.g_debug_level & DBG_CC) cout << "Code Cache Flushed at size " << g_var.g_codeCacheSize / (1024 * 1024) << "MB! (Flush count: " << s_pin_flush_cnt.getValue () << ")" << endl;
	g_var.g_codeCacheSize=0;
}

/*--
 * THIS IS WHERE THE CODE BACKEND WILL BE CALLED A SHARED BUFFER BETWEEN THE
 * INSTRUCTION ANALYSIS AND THIS WEILL BE PRESENT.  THE BYFFER WILL BE ACCESSED
 * USING LOCKS ON BOTH THE ANALYSIS ROUTIN AND THIS ROUTIN.  WE USE PIN LOCKS
 * AS SHOWN BELOW.
 --*/
static VOID backEnd (void *ptr) {
	while (!g_var.g_appEnd) { //TODO fix this while loop
		PIN_SemaphoreWait (&semaphore0);
		PIN_SemaphoreClear (&semaphore0);
		ADDRINT __pc = g_var.g_pc;
		BOOL taken = g_var.g_taken;
		ADDRINT tgt = g_var.g_tgt;
		ADDRINT fthru = g_var.g_fthru;
		if (g_var.g_enable_wp) g_var.g_pred_eip = PredictAndUpdate (__pc, taken, tgt, fthru);
		if (g_var.g_core_type == BASICBLOCK) {
			if (g_var.g_bbCache->NumElements () >= BB_CNT_THR) {
//				cout << "FRONTEND->BACKEND " << endl;
				g_var.stat.noMatchIns = 0;
				g_var.stat.matchIns = 0;
                bbBkEndRun ();
                s_pin_fr_to_bk_cnt++;
//				cout << "BACKEND->FRONTEND" << endl;
			}
		} else { /* INO & O3 */
			if (g_var.g_codeCache->NumElements () >= INS_CNT_THR) {
//				cout << "FRONTEND->BACKEND " << endl;
				g_var.stat.noMatchIns = 0;
				g_var.stat.matchIns = 0;
                if (g_var.g_core_type == OUT_OF_ORDER) oooBkEndRun ();
                else if (g_var.g_core_type == IN_ORDER) inoBkEndRun ();
                s_pin_fr_to_bk_cnt++;
//				cout << "BACKEND->FRONTEND" << endl;
			}
		}
		PIN_SemaphoreSet (&semaphore1);
	}
}

VOID pin__runPARS (char* cfgFile)
{
    pin__init (cfgFile);

	// Register a routine that gets called every time the trace is inserted
    CODECACHE_AddTraceInsertedFunction (countTrace, 0);
	// Register a routine that gets called every time the cache is full
	CODECACHE_AddFullCacheFunction (FlushOnFull, 0); 

	//Handle pin-generated exceptions
	PIN_AddInternalExceptionHandler (handlePinException,NULL);
	//Handle application-generated exceptions
	PIN_InterceptSignal (SIGTRAP,signal_handler,NULL);
	PIN_UnblockSignal (SIGTRAP,TRUE);
	PIN_InterceptSignal (SIGILL,signal_handler,NULL);
	PIN_UnblockSignal (SIGILL,TRUE);
	PIN_InterceptSignal (SIGSEGV,signal_handler,NULL);
	PIN_UnblockSignal (SIGSEGV,TRUE);
	PIN_InterceptSignal (SIGFPE,signal_handler,NULL);
	PIN_UnblockSignal (SIGFPE,TRUE);
	PIN_InterceptSignal (SIGBUS,signal_handler,NULL);
	PIN_UnblockSignal (SIGBUS,TRUE);
	PIN_InterceptSignal (SIGABRT,signal_handler,NULL);
	PIN_UnblockSignal (SIGABRT,TRUE);

    TRACE_AddInstrumentFunction (pin__instruction, 0);
    PIN_AddFiniFunction (pin__fini, 0);

    PIN_SpawnInternalThread (backEnd, rootThreadArg, 0, &rootThreadUid);

	filter.Activate ();

	start_pars = clock ();
}

/* ****************************************************************** *
 * INITALIZATION & CLEAN UP
 * ****************************************************************** */
VOID pin__parseConfig (char* cfgFile) {
	g_cfg = new config (cfgFile, &g_var);
}

VOID pin__init (char* cfgFile) 
{
	pin__parseConfig (cfgFile);
	g_var.msg.simStep ("SIMULATOR FRONTEND INITIALIZATION");
	PIN_SemaphoreInit (&semaphore0);
	PIN_SemaphoreInit (&semaphore1);
	PIN_SemaphoreClear (&semaphore0);
	PIN_SemaphoreClear (&semaphore1);
    g_predictor  = new TournamentBP (2048, 2, 2048, 11, 8192, 13, 2, 8192, 2, 0);
	g_var.msg.simStep ("PARS COMPILED CODE");
	g_var.g_insList = new List<string*>;
	g_var.g_codeCache = new List<dynInstruction*>;
	g_var.g_bbCache = new List<dynBasicblock*>;
	g_var.g_BBlist = new List<basicblock*>;
    g_var.g_core_type = IN_ORDER;
    g_var.g_mem_model = PERFECT; //NAIVE_SPECUL
    g_var.scheduling_mode = STATIC_SCH;
	g_staticCode = new staticCodeParser (g_cfg);
    g_bbStat = new bbStat;
	pin__uOpGenInit (*g_staticCode);

	g_var.msg.simStep ("SIMULATOR BACKEND INITIALIZATION");
	char const * dummy_argv[] = {"TraceSim", 
                                 "-o", "/scratch/tracesim/specint2006/results/ooo_listSch_dynBP_manyCache/401.bzip2.txt", 
                                 "-i", "/scratch/tracesim/specint2006/bb_trace_archReg/401.bzip2.trace", 
                                 "-c", "1", "-n", "4", "-s", "1", "-y", "1", "-j", "1", "-w", "160", 
                                 "-x", "/home/milad/esc_project/svn/memTraceMilad/TraceSim/results/bzip2/wbb_skip_static_count_bzip2.csv", 
                                 "-e", "num_wbb_bypassed_in_scheduling_each_ins", 
                                 "-z", "/home/milad/esc_project/svn/memTraceMilad/TraceSim/results/bzip2/branch_exe_count_map.csv", 
                                 NULL};
	int dummy_argc = sizeof (dummy_argv) / sizeof (char*) - 1;
	bkEnd_init (dummy_argc, dummy_argv, g_var); //TODO fix this line
	bkEnd_heading (dummy_argc, dummy_argv); //TODO fix this line
    if (g_var.g_core_type == OUT_OF_ORDER) oooBkEnd_init (dummy_argc, dummy_argv);
    else if (g_var.g_core_type == IN_ORDER) inoBkEnd_init (dummy_argc, dummy_argv);
    else if (g_var.g_core_type == BASICBLOCK) bbBkEnd_init (dummy_argc, dummy_argv);
	g_var.msg.simStep ("START OF SIMULATION");
}

VOID pin__fini (INT32 code, VOID* v) 
{
	g_var.msg.simStep ("FRONTEND TERMINATED");
	stop_pars = clock ();
	g_var.g_appEnd = true;
	delete g_var.g_insList;
	delete g_var.g_codeCache;
	delete g_var.g_bbCache;
	delete g_var.g_BBlist;
	cout << "finishing" << endl;
	PIN_SemaphoreSet (&semaphore0);
	PIN_SemaphoreSet (&semaphore1);
	cout << "finished" << endl;
	double exe_time = double (stop_pars-start_pars)/CLOCKS_PER_SEC;
	double ins_per_sec = double (s_pin_ins_cnt.getValue ()) / exe_time;
	cout << "Execution Time Under Pin: " << exe_time << " sec , Num Executed Ops: " << s_pin_ins_cnt.getValue () << endl;
	cout << "Instructions Executed Per Second Under Pin: " << ins_per_sec << endl;
	cout << "Num traces generated: " << s_pin_trace_cnt.getValue () << "; Code cach used for traces: " << g_var.g_codeCacheSize / (1024 * 1024) << " MB" << endl;
	delete g_predictor;
	delete g_staticCode;
	PIN_SemaphoreFini (&semaphore0);
	PIN_SemaphoreFini (&semaphore1);
	g_var.msg.simStep ("BACKEND TERMINATED");

    /*-- DUMP STAT --*/
    g_stats.dump ();

	/*-- FINISH BACKEND --*/
    if (g_var.g_core_type == OUT_OF_ORDER) oooBkEnd_fini ();
    else if (g_var.g_core_type == IN_ORDER) inoBkEnd_fini ();
    else if (g_var.g_core_type == BASICBLOCK) bbBkEnd_fini ();
	
    delete g_bbStat;

	g_var.msg.simStep ("END OF SIMULATION");
}
/* ================================================================== */


EXCEPT_HANDLING_RESULT handle (THREADID tid, EXCEPTION_INFO *pExceptInfo, PHYSICAL_CONTEXT *pPhysCtxt, VOID *v)
{
	s_pin_sig_cnt++;
	if (g_var.g_debug_level & DBG_SPEC) cout << " pintool signal count = " << dec << s_pin_sig_cnt.getValue () << endl;
	longjmp (g_var.g_env,1);
}

void read_mem_orig (ADDRINT eaddr, ADDRINT len)
{
	if (g_var.g_debug_level & DBG_WRITE_MEM) cout << "  mem[" << hex << eaddr << " ] = ";
	//Assert (len <= MAX_MEM_WRITE_LEN);
	g_var.g_invalid_size = false;
	if (len > MAX_MEM_WRITE_LEN) {
		if (g_var.g_debug_level & DBG_SPEC) cout << " (invalid memory access size - read_mem_orig ()) - " << (int) len << "Bytes\n";
		g_var.g_invalid_size = true;
		return;
	}
	g_var.g_last_len = len;
	g_var.g_last_eaddr = eaddr;
	g_var.g_invalid_addr = false;

	THREADID tid=PIN_ThreadId ();
	if (tid==INVALID_THREADID) {
		cout << " could not get thread id\n";
		exit (1);
	}

	PIN_TryStart (tid,handle,0);
	int val = setjmp (g_var.g_env);
  	if (val) {
		if (g_var.g_debug_level & DBG_SPEC) cout << " (invalid memory location - read_mem_orig ())\n";
		g_var.g_invalid_addr = true;
		PIN_TryEnd (tid);
		return;
	}

	for (int i=len-1; i >= 0; i--) {
		PIN_SafeCopy (&g_store_buffer[i], (ADDRINT*) (eaddr+i), sizeof (unsigned char));
		//g_store_buffer[i] = * ( ( (unsigned char*)eaddr)+i);
		if (g_var.g_debug_level & DBG_WRITE_MEM) cout << hex << (unsigned) g_store_buffer[i] << " ";
	}

	PIN_TryEnd (tid);

	g_log.save (eaddr, len, g_store_buffer);

	if (g_var.g_debug_level & DBG_WRITE_MEM) cout << "\n";
}

/*
map<ADDRINT,unsigned char> g_specmem;

void read_mem_new ()
{
	if (g_var.g_invalid_addr) return;
	if (g_var.g_debug_level & DBG_WRITE_MEM) cout << "  mem[" << hex << g_var.g_last_eaddr << " ] = ";

	for (int i=g_var.g_last_len-1; i >= 0; i--) {
		g_specmem[g_var.g_last_eaddr+i] = * ( ( (unsigned char*)g_last_eaddr)+i);
		if (g_var.g_debug_level & DBG_WRITE_MEM) cout << hex << (unsigned) g_specmem[g_var.g_last_eaddr+i] << " ";
	}

	if (g_var.g_debug_level & DBG_WRITE_MEM) cout << "\n";
}

void restore_mem_orig ()
{
	if (g_var.g_invalid_addr) return;
	if (g_var.g_debug_level & DBG_RESTORE_MEM) cout << "  restoring mem[" << hex << g_var.g_last_eaddr << " ] = ";

	THREADID tid=PIN_ThreadId ();
	if (tid==INVALID_THREADID) {
		cout << " could not get thread id\n";
		exit (1);
	}

	PIN_TryStart (tid,handle,0);
	int val = setjmp (g_var.g_env);
  	if (val) {
		if (g_var.g_debug_level & DBG_SPEC) cout << " (invalid memory location - restore_mem_orig ())\n";
		g_var.g_invalid_addr=true;
		PIN_TryEnd (tid);
		return;
	}

	for (int i=g_var.g_last_len-1; i >= 0; i--) {
		* ( ( (unsigned char*)g_var.g_last_eaddr)+i) = g_store_buffer[i];
		if (g_var.g_debug_level & DBG_RESTORE_MEM) cout << hex << (unsigned) g_store_buffer[i] << " ";
	}

	PIN_TryEnd (tid);

	if (g_var.g_debug_level & DBG_RESTORE_MEM) cout << "\n";
}
*/

VOID GetMemWriteOrigValue (UINT32 uid, CONTEXT *c, ADDRINT eaddr, ADDRINT len)
{
    if (!g_var.g_enable_wp) return;
    if (g_var.g_enable_wp && !g_var.g_wrong_path) return;

#ifdef G_I_INFO_EN
	Assert (g_i_info.find (uid)!=g_i_info.end ());
	const i_info &i = g_i_info[uid];
	if (g_var.g_debug_level & DBG_WRITE_MEM) cout << "EXEC wb " << (g_var.g_wrong_path?"*":" ") << " " << dec << g_var.g_seq_num << " : " << hex << i.pc << " " << i.diss << " : ";
#endif
	read_mem_orig (eaddr, len);
	return;
}

VOID GetMemWriteNewValue (UINT32 uid, CONTEXT *c)
{
    if (!g_var.g_enable_wp) return;
    if (g_var.g_enable_wp && !g_var.g_wrong_path) return;

#ifdef G_I_INFO_EN
	Assert (g_i_info.find (uid)!=g_i_info.end ());
	const i_info &i = g_i_info[uid];
	if (g_var.g_debug_level & DBG_WRITE_MEM) cout << "EXEC wa " << (g_var.g_wrong_path?"*":" ") << " " << dec << g_var.g_seq_num << " : " << hex << i.pc << " " << i.diss << " : ";
#endif

	// TODO: bypass real memory?
	// read_mem_new ();
	// restore_mem_orig ();
	return;
}

VOID GetMemReadBypass (UINT32 uid, CONTEXT *c, ADDRINT eaddr, ADDRINT len)
{
    if (!g_var.g_enable_wp) return;
    if (g_var.g_enable_wp && !g_var.g_wrong_path) return;

#ifdef G_I_INFO_EN
	Assert (g_i_info.find (uid)!=g_i_info.end ());
	const i_info &i = g_i_info[uid];
	if (g_var.g_debug_level & DBG_READ_MEM) cout << "EXEC rb " << (g_var.g_wrong_path?"*":" ") << " " << dec << g_var.g_seq_num << " : " << hex << i.pc << " " << i.diss << " : ";
#endif

	// TODO: bypass real memory?
	return;
}

VOID HandleBranch (UINT32 uid, CONTEXT *c, BOOL taken, ADDRINT tgt, ADDRINT fthru, ADDRINT __pc, BOOL __has_ft)
{
    if (!g_var.g_enable_wp) {
		//PIN_SemaphoreSet (&semaphore0);
		//PIN_SemaphoreWait (&semaphore1); 
		//PIN_SemaphoreClear (&semaphore1);
		return;
	}
    if (g_var.g_enable_wp && g_var.g_wrong_path) return;
#ifdef G_I_INFO_EN
	Assert (g_i_info.find (uid)!=g_i_info.end ());
	//const i_info &i = g_i_info[uid];
#endif
	bool was_wp = g_var.g_wrong_path;

	ADDRINT pred_eip = tgt;
	if (true) { // uasd to be if (__has_ft) { - clean up? (TODO)
		g_var.g_pc = __pc;
		g_var.g_taken = taken;
		g_var.g_tgt = tgt;
		g_var.g_fthru = fthru;
		PIN_SemaphoreSet (&semaphore0);
		PIN_SemaphoreWait (&semaphore1); 
		PIN_SemaphoreClear (&semaphore1);
		pred_eip = g_var.g_pred_eip;
    }
	if (g_var.g_wrong_path && !was_wp) {
		s_pin_wp_cnt++;
		if (g_var.g_debug_level & DBG_SPEC) cout << "  *** transitioning to wrong path ***\n";
		if (g_var.g_debug_level & DBG_SPEC) cout << "  wrong path number = " << dec << s_pin_wp_cnt.getValue () << endl;
		PIN_SaveContext (c,&g_var.g_context);
	}

	ADDRINT eip = taken?tgt:fthru;
	if (eip != pred_eip) {
		Assert (g_var.g_wrong_path);
		if (g_var.g_debug_level & DBG_SPEC) cout << "  *** forcing PIN to change control flow ***\n";
		if (g_var.g_debug_level & DBG_SPEC) cout << "      predicted EIP = " << hex << pred_eip << "\n";
		if (g_var.g_enable_wp) {
			PIN_SetContextReg (c,REG_INST_PTR, pred_eip);
			g_var.g_context_call_depth=0;
			PIN_ExecuteAt (c);
		}
	}
	return;
}

VOID HandleSyscall (UINT32 uid, CONTEXT *c)
{
    if (!g_var.g_enable_wp) return;
#ifdef G_I_INFO_EN
	Assert (g_i_info.find (uid)!=g_i_info.end ());
	const i_info &i = g_i_info[uid];
	if (g_var.g_debug_level & DBG_EXEC) cout << "EXEC hs " << (g_var.g_wrong_path?"*":" ") << " " << dec << g_var.g_seq_num << " : " << hex << i.pc << " " << i.diss << " : ";
#endif
	if (g_var.g_wrong_path) {
	    if (g_var.g_debug_level & DBG_SPEC) cout << " *** detected system call on wrong path ***\n";
		g_var.g_spec_syscall = true;	
	}
}

inline BOOL simpointMode () {
	static int simpCount=0; 
	unsigned long next_simp=g_cfg->_simpoint.begin ()->first; 
	if (g_var.g_enable_simpoint && !g_var.g_wrong_path) {
		if (g_var.g_inSimpoint) {
			g_var.g_simpInsCnt++;
			if (g_var.g_simpInsCnt >= SIMP_WINDOW_SIZE) {
				cout << "*** Simpoint end\n\n";
				g_var.g_inSimpoint = false;
				g_var.g_enable_wp = false;
				g_var.g_simpInsCnt = 0;
				PIN_RemoveInstrumentation ();
				g_var.g_enable_instrumentation = false;
			}
		} else {
			if (next_simp == g_var.g_insCountRightPath) {
				g_var.g_inSimpoint = true;
				g_var.g_enable_wp = true;
				g_var.g_simpInsCnt = 0;
				g_var.g_enable_instrumentation = true;
				g_cfg->_simpoint.erase (next_simp);
				next_simp = g_cfg->_simpoint.begin ()->first;
				cout << "\n*** Simpoint #" << ++simpCount << ": " << g_var.g_insCountRightPath << "\n";
				cout << "*** Next Simpoint is: #" << next_simp << "\n";
			}
		}
	}
	return g_var.g_inSimpoint;
}

/*-- COUNTS THE NUMBER OF DYNAMIC INSTRUCTIONS (WRONG-PATH INSTRUCTIONS INCLUDED) --*/
VOID doCount ()
{
	s_pin_ins_cnt++; /*total ins count: wrong and right path*/
	if (!g_var.g_wrong_path) g_var.g_insCountRightPath++;
	unsigned long countRem = (unsigned long) s_pin_ins_cnt.getValue () % BILLION;
	unsigned long countQ = (unsigned long) s_pin_ins_cnt.getValue () / MILLION;
	static clock_t past = 0.0;
	static clock_t now = double (clock ())/CLOCKS_PER_SEC;
	simpointMode ();
	if (countRem == 0) {
	    now = double (clock ())/CLOCKS_PER_SEC;
		cout << countQ << " million passed at " << double (clock ())/CLOCKS_PER_SEC << " seconds. (Diff Time: " << now-past << ")\n";
		cout << "  correct path ins count: " << g_var.g_insCountRightPath << " (fraction: " << double (g_var.g_insCountRightPath)/ s_pin_ins_cnt.getValue () << ")\n";
		cout << "  wrong path ins count: " << g_var.g_total_wrong_path_count << "\n";
		cout << "  wrong path count: " << s_pin_wp_cnt.getValue () << "\n";
		cout << "  trace count: " << s_pin_trace_cnt.getValue () << "\n";
		cout << "  app signal count: " << s_pin_sig_cnt.getValue () << "\n";
		cout << "  pin signal count: " << s_pin_sig_cnt.getValue () << "\n";
		cout << "  out of mem count: " << s_pin_flush_cnt.getValue () << "\n";
		cout << "  recovery count: " << s_pin_sig_recover_cnt.getValue () << "\n";
		cout << "  code cache flush count: " << s_pin_flush_cnt.getValue () << "\n";
		cout << "  avg wrong-path length: " << double (s_pin_wp_cnt.getValue ()) / double (s_pin_wp_cnt.getValue ()) << "\n";
		cout << "  code cache size (MB): " << double (g_var.g_codeCacheSize) / (1024.0 * 1024.0) << "\n\n";
		past = now;
	}
}

long unsigned imgInsCallCount_=0;
VOID doImpCallCount_ (BOOL isCall)
{
	if (isCall)
		imgInsCallCount_++;
	unsigned long countRem = (unsigned long) s_pin_ins_cnt.getValue () % BILLION;
	if (countRem == 0) {
		cout << " (CALL_) :" << imgInsCallCount_ << "\n";
	}
}

long unsigned imgInsMemCount_=0;
VOID doImpMemCount_ (UINT32 isMem)
{
	if (isMem > 0)
		imgInsMemCount_++;
	unsigned long countRem = (unsigned long) s_pin_ins_cnt.getValue () % BILLION;
	if (countRem == 0) {
		cout << " (MEM_) :" << imgInsMemCount_ << "\n";
	}
}

VOID pin__instruction (TRACE trace, VOID * val) 
{
    if (!filter.SelectTrace (trace)) {
        printf ("NOTE: SKIPPING TRACE\n");
        return;
    }

//    bool first_bb = true;
    for (BBL bbl = TRACE_BblHead (trace); BBL_Valid (bbl); bbl = BBL_Next (bbl))
    {
//        ADDRINT bb_addr = BBL_Address (bbl);
//        if (g_var.g_core_type == BASICBLOCK) {
//            if (first_bb) {
//                first_bb = false;
//                INS bb_head = BBL_InsHead (bbl);
//                pin__get_bb_header (bb_addr, bb_head);
//            }
//            INS bb_tail = BBL_InsTail (bbl);
//            pin__get_bb_header (bb_addr, bb_tail);
//        }

        for (INS ins = BBL_InsHead (bbl); INS_Valid (ins); ins = INS_Next (ins))
        {
            ADDRINT pc = INS_Address (ins);
            string diss =  INS_Disassemble (ins);
            static unsigned long uid=0;
            ++uid;
#ifdef G_I_INFO_EN
            OPCODE opcode = INS_Opcode (ins);
            bool is_call = INS_IsCall (ins);
            bool is_ret = INS_IsRet (ins);
            bool has_ft = INS_HasFallThrough (ins);
            g_i_info[uid] = i_info (pc,opcode,diss,is_call,is_ret,has_ft);
            if (uid==0) g_i_info[0] = i_info (pc,opcode,diss,is_call,is_ret,has_ft);
#endif
            INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) doCount,
                    IARG_END);
            INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) doImpMemCount_,
                    IARG_UINT32, INS_MemoryOperandCount (ins),
                    IARG_END);
            INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) doImpCallCount_,
                    IARG_BOOL, INS_IsCall (ins),
                    IARG_END);

            if (g_var.g_enable_instrumentation) {
                pin__getOp (ins);
                if (INS_IsMemoryWrite (ins)) {
                    if (g_var.g_debug_level & DBG_INS) cout << "INS  " << hex << pc << " " << diss << " [mem write]\n";
                    INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) GetMemWriteOrigValue,
                            IARG_UINT32, uid,
                            IARG_CONTEXT,
                            IARG_MEMORYWRITE_EA,
                            IARG_MEMORYWRITE_SIZE,
                            IARG_END);
                    /*
                       if (INS_IsCall (ins) || INS_IsProcedureCall (ins)) {
                       INS_InsertCall (ins, IPOINT_TAKEN_BRANCH, (AFUNPTR) GetMemWriteNewValue,
                       IARG_UINT32, uid,
                       IARG_CONTEXT,
                       IARG_END);
                       } else {
                       INS_InsertCall (ins, IPOINT_AFTER, (AFUNPTR) GetMemWriteNewValue,
                       IARG_UINT32, uid,
                       IARG_CONTEXT,
                       IARG_END);
                       }
                       */
                }
                /*
                   if (INS_IsMemoryRead (ins)) {
                   if (g_var.g_debug_level & DBG_INS) cout << "INS  " << hex << pc << " " << diss << " [mem read]\n";
                   INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) GetMemReadBypass,
                   IARG_UINT32, uid,
                   IARG_CONTEXT,
                   IARG_MEMORYREAD_EA,
                   IARG_MEMORYREAD_SIZE,
                   IARG_END);
                   }
                   */

//                if (INS_IsBranchOrCall (ins) || INS_IsFarRet (ins) || INS_IsRet (ins)) { //TODO put it back
                if (INS_IsBranchOrCall (ins)) {
                    if (g_var.g_debug_level & DBG_INS) cout << "INS  " << hex << pc << " " << diss << " [branch]\n";
//                    if (INS_HasFallThrough (ins)) { //TODO put it back
//                        INS_InsertCall (ins, IPOINT_AFTER, (AFUNPTR) HandleBranch,
//                                IARG_UINT32, uid,
//                                IARG_CONTEXT,
//                                IARG_BRANCH_TAKEN,
//                                IARG_BRANCH_TARGET_ADDR, 
//                                IARG_FALLTHROUGH_ADDR,
//                                IARG_ADDRINT, INS_Address (ins),
//                                IARG_BOOL, INS_HasFallThrough (ins),
//                                IARG_END);
//                    }
                    INS_InsertCall (ins, IPOINT_TAKEN_BRANCH, (AFUNPTR) HandleBranch,
                            IARG_UINT32, uid,
                            IARG_CONTEXT,
                            IARG_BRANCH_TAKEN,
                            IARG_BRANCH_TARGET_ADDR, 
                            IARG_FALLTHROUGH_ADDR,
                            IARG_ADDRINT, INS_Address (ins),
                            IARG_BOOL, INS_HasFallThrough (ins),
                            IARG_END);
                }

                if (INS_IsSyscall (ins)) {
                    INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) HandleSyscall,
                            IARG_UINT32, uid,
                            IARG_CONTEXT,
                            IARG_END);
                }

                INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) HandleInst,
                        IARG_UINT32, uid,
                        IARG_BOOL, INS_IsCall (ins),
                        IARG_BOOL, INS_IsRet (ins),
                        IARG_BOOL, INS_IsFarRet (ins),
                        IARG_END);
            }
        }
    }
}

/* ****************************************************************** *
 * BRANCH PREDICTOR
 * ****************************************************************** */
ADDRINT PredictAndUpdate (ADDRINT __pc, INT32 __taken, ADDRINT tgt, ADDRINT fthru)
{
    bool taken = __taken;
    ADDRINT pc = __pc;
    void *bp_hist = NULL;
    bool pred = g_predictor->lookup (pc, bp_hist);
    if (g_var.g_debug_level & DBG_BP) cout << "  prediction = " << (pred?"T":"N");
	if (!g_var.g_wrong_path) {
   		if (g_var.g_debug_level & DBG_BP) cout << ", actual = " << (taken?"T":"N") << " : "; 
		if (pred != taken) {
		    if (g_var.g_debug_level & DBG_BP) cout << "mispredicted!\n";
			g_var.g_wrong_path = true;
			//printf ("\nSTART OF WRONG PATH\n");
			//fprintf (__outFile, "\nSTART OF WRONG PATH\n");
		} else {
		    if (g_var.g_debug_level & DBG_BP) cout << "correct prediction\n";
		}
		g_predictor->update (pc, taken, bp_hist, false);
	} else {
		if (g_var.g_debug_level & DBG_BP) cout << " on wrong path\n";
	}
    return  pred ? tgt : fthru;
}
