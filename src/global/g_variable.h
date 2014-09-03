/*******************************************************************************
 * g_variable.h
 *******************************************************************************/

#ifndef _G_VARIABLE_EN
#define _G_VARIABLE_EN

#define BB_NEAR_EMPTY_SIZE 30

#include <setjmp.h>
#include <set>
#include "pin.H"
#include "pin_isa.H"

#include "global.h"
#include "../lib/list.h"
#include "../lib/message.h"
#include "../lib/statistic.h"
#include "../backend/basicblock.h"
#include "../backend/unit/dynInstruction.h"
#include "../backend/unit/dynBasicblock.h"
#include "../backend/unit/bbInstruction.h"
#include "../backend/unit/stInstruction.h"

struct g_variable {
    g_variable() {
        g_seq_num = 1;
        g_bb_seq_num = 1;
        g_seqnum = 1;
        g_icount = 0;
        insCount = 0;
        g_debug_level = 0; //DBG_SPEC|DBG_BP|DBG_EXEC|DBG_SPEC|DBG_WRITE_MEM|DBG_CC|DBG_INSBUF|DBG_UOP;
        g_verbose_level = V_FRONTEND; //V_FRONTEND, V_BACKEND
        g_branch_mispredict_delay = 20;
        g_wrong_path_count = 0;
        g_total_wrong_path_count = 0;
        g_app_signal_count = 0;
        g_pin_signal_count = 0;
        g_recovery_count = 0;
        g_pintool_signal_count = 0;
        g_wrong_path_number = 0;
        g_context_call_depth = 0;
        g_wrong_path = false;
        g_spec_syscall = false;
        g_appEnd = false;
        g_inSimpoint = false;
        g_simpInsCnt = 0;
        g_insCountRightPath = 0;
        g_traceCount = 0;
        g_codeCacheSize = 0;
        g_flushes = 0;
        g_enable_simpoint = true;
        g_enable_wp = true;
        g_enable_instrumentation = true;

        g_pipe_state = PIPE_NORMAL;
        g_squash_seq_num = 0;
        g_old_squash_seq_num = 0;
        g_squash_type = NO_MISPRED;

        g_insList_indx = 0;
        g_BBlist_indx = -1;
        numPBinList = 0;

        g_core_type = IN_ORDER;
    }

    INS_ID g_seq_num;
    INS_ID g_bb_seq_num;
    unsigned long long g_seqnum; // including wp
    unsigned long long g_icount; // correct path
    unsigned g_debug_level;
    unsigned g_verbose_level;
    unsigned g_branch_mispredict_delay;
    unsigned g_wrong_path_count;
    unsigned g_total_wrong_path_count;
    unsigned g_app_signal_count;
    unsigned g_pin_signal_count;
    unsigned long insCount;
    unsigned g_recovery_count;
    unsigned g_pintool_signal_count;
    unsigned g_wrong_path_number;
    int g_context_call_depth;
    BOOL g_wrong_path;
    bool g_spec_syscall;
    bool g_appEnd;
    bool g_inSimpoint;
    bool g_enable_simpoint;
    bool g_enable_wp;
    bool g_enable_instrumentation;
    long unsigned g_simpInsCnt;
    long int g_traceCount;
    long int g_codeCacheSize;
    unsigned long g_insCountRightPath;
    UINT32 g_flushes;

    bool g_invalid_addr;
    bool g_invalid_size;
    unsigned g_last_len;
    jmp_buf g_env;
    CONTEXT g_context;
    ADDRINT g_last_eaddr;
    ADDRINT g_pc;
    ADDRINT g_tgt;
    ADDRINT g_fthru;
    ADDRINT g_pred_eip;
    BOOL g_taken;

    //Instruction pipe between frontend and backend
    List<basicblock*>* g_BBlist;
    List<string*>* g_insList;
    List<dynInstruction*>* g_codeCache;
    List<dynBasicblock*>* g_bbCache;
    int g_insList_indx;
    int g_BBlist_indx;
    string g_ins;
    int numPBinList;
    dynInstruction g_insObj;

    //Message printing object
    message msg;

    //Statistics object
    statistic stat;

    // Pipeline state
    PIPE_STATE g_pipe_state;

    //CPU Core
    CORE_TYPE g_core_type;

    /*****************************/
    /* PIPELINE SQUASH           */
    /*****************************/
    INS_ID g_squash_seq_num;
    INS_ID g_old_squash_seq_num;
    PIPE_SQUASH_TYPE g_squash_type;
    INS_ID getSquashSN () {return g_squash_seq_num;}
    INS_ID getOldSquashSN () {return g_old_squash_seq_num;}
    void resetSquashSN () {g_squash_seq_num = 0; g_old_squash_seq_num = 0;}
    void resetSquashType () {g_squash_type = NO_MISPRED;}
    void setSquashSN (INS_ID sn) {
        Assert (sn > 0);
        if (g_squash_seq_num > 0 && sn < g_squash_seq_num) {
            g_squash_seq_num = sn;
        } else if (g_squash_seq_num == 0) {
            g_squash_seq_num = sn;
        }
    }
    void setOldSquashSN () {
            g_old_squash_seq_num = g_squash_seq_num;
    }
    bool isSpeculationViolation () {
        return (getSquashSN () != getOldSquashSN ()) ? true : false;
    }
    void setSquashType (PIPE_SQUASH_TYPE squash_type) {
        g_squash_type = squash_type;
    }
    PIPE_SQUASH_TYPE getSquashType () {
        return g_squash_type;
    }

    /*****************************/
    /* CODE CACHE MANAGER ROUTINS */
    /*****************************/
    dynBasicblock* getNewCacheBB () {
        while (g_bbCache->NumElements () > 0 &&
               g_bbCache->Last()->getBBsize () == 0) {
            delete g_bbCache->Last();
            g_bbCache->RemoveAt (g_bbCache->NumElements () - 1);
        }
        dynBasicblock* newBB = new dynBasicblock;
        g_bbCache->Append (newBB);
        return newBB;
    }
    dynBasicblock* getLastCacheBB () {
        if (g_bbCache->NumElements () == 0) { return NULL;}
        else {return g_bbCache->Last ();}
    }
    bbInstruction* getNewIns () {
        bbInstruction* newIns = new bbInstruction;
        return newIns;
    }
    dynInstruction* getNewCodeCacheIns () {
        dynInstruction* newIns = new dynInstruction;
        g_codeCache->Append (newIns);
        return newIns;
    }
    void insertFrontCodeCache (dynInstruction* ins) { g_codeCache->InsertAt (ins, 0); }
    void insertFrontBBcache (dynBasicblock* bb) { g_bbCache->InsertAt (bb, 0); }
    void remFromCodeCache () {
        Assert (g_codeCache->NumElements () > 0);
        g_codeCache->RemoveAt (0);
    }
    dynInstruction* popCodeCache () { return g_codeCache->Nth (0); } //TODO inconsistent with pop of BB - fix
    dynBasicblock* popBBcache () {
            dynBasicblock* bb = g_bbCache->Nth (0); 
            g_bbCache->RemoveAt (0); 
            return bb;
    }
    bool isCodeCacheEmpty () { return (g_codeCache->NumElements () == 0) ? true : false; }
    bool isBBcacheNearEmpty () { return (g_bbCache->NumElements () <= BB_NEAR_EMPTY_SIZE) ? true : false; }
};

extern g_variable g_var;

#endif
