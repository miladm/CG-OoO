/*******************************************************************************
 * uOpGen.cpp
 ******************************************************************************/

#include "uOpGen.h"

staticCodeParser* g__staticCode;
static bool g_br_detected = false;
static set<ADDRINT> _bbHeadSet;

/* ******************************************************************* *
 * STAT GLOBAL VARIABLES
 * ******************************************************************* */
static ScalarStat& s_pin_missing_static_bb_cnt (g_stats.newScalarStat ("uOpGen", "pin_missing_static_bb_cnt", "Number of dynamic BB's with no static counterpart.", 0, NO_PRINT_ZERO));
static ScalarStat& s_pin_bb_cnt (g_stats.newScalarStat ("uOpGen", "pin_bb_cnt", "Number of basicblocks instrumented in frontend", 0, NO_PRINT_ZERO));
static ScalarStat& s_missing_ins_in_stat_code_cnt (g_stats.newScalarStat ("uOpGen", "missing_ins_in_stat_code_cnt", "Number of dynamic instructions not found in static code", 0, NO_PRINT_ZERO));
static ScalarStat& s_dyn_gr_cnt (g_stats.newScalarStat ("uOpGen", "dyn_gr_cnt", "Number of global register operands", 0, NO_PRINT_ZERO));
static ScalarStat& s_dyn_lr_cnt (g_stats.newScalarStat ("uOpGen", "dyn_lr_cnt", "Number of local register operands", 0, NO_PRINT_ZERO));
static ScalarStat& s_dyn_gr_rd_cnt (g_stats.newScalarStat ("uOpGen", "dyn_gr_rd_cnt", "Number of global register read operands", 0, NO_PRINT_ZERO));
static ScalarStat& s_dyn_gr_wr_cnt (g_stats.newScalarStat ("uOpGen", "dyn_gr_wr_cnt", "Number of global register write operands", 0, NO_PRINT_ZERO));
static ScalarStat& s_dyn_lr_rd_cnt (g_stats.newScalarStat ("uOpGen", "dyn_lr_rd_cnt", "Number of local register read operands", 0, NO_PRINT_ZERO));
static ScalarStat& s_dyn_lr_wr_cnt (g_stats.newScalarStat ("uOpGen", "dyn_lr_wr_cnt", "Number of local register write operands", 0, NO_PRINT_ZERO));
static ScalarStat& s_ins_with_lr_operand_cnt (g_stats.newScalarStat ("uOpGen", "ins_with_lr_operand_cnt", "Number of operations with local reg operands", 0, NO_PRINT_ZERO));
static ScalarStat& s_ins_without_lr_operand_cnt (g_stats.newScalarStat ("uOpGen", "ins_without_lr_operand_cnt", "Number of operations without local reg operands", 0, NO_PRINT_ZERO));
static ScalarStat& s_large_bb_cnt (g_stats.newScalarStat ("uOpGen", "large_bb_cnt", "Number of basicblocks > bbWindow size", 0, NO_PRINT_ZERO));

/* ************************************* *
 * INS INSTRUMENTATIONS
 * ************************************ */
VOID pin__getBrIns (ADDRINT ins_addr, BOOL hasFT, ADDRINT tgAddr, ADDRINT ftAddr, 
        BOOL isTaken, BOOL isCall, BOOL isRet, BOOL isJump, BOOL isDirBrOrCallOrJmp) {
    if (g__staticCode->hasIns (ins_addr)) {
//        g_var.stat.matchIns++;
        if (g_var.g_core_type == BASICBLOCK) {
            if (_bbHeadSet.find (ins_addr) != _bbHeadSet.end ()) g_br_detected = true;
            pin__detectBB (ins_addr);
            dynInstruction* insObj = pin__makeNewBBIns (ins_addr, BR);
            if (insObj != NULL) {
                insObj->setBrAtr (tgAddr, ftAddr, hasFT, isTaken, isCall, isRet, isJump, isDirBrOrCallOrJmp);
            }
            if (!(isJump && isDirBrOrCallOrJmp))
                g_br_detected = true;
        } else { /* INO & O3 */
            dynInstruction* insObj = pin__makeNewIns (ins_addr, BR);
            insObj->setBrAtr (tgAddr, ftAddr, hasFT, isTaken, isCall, isRet, isJump, isDirBrOrCallOrJmp);
        }
        if (g_var.g_debug_level & DBG_UOP) 
            std::cout << "NEW BR: " << (g_var.g_wrong_path?"*":" ") << hex << ins_addr << 
                " (" << g_var.g_seq_num << ") in BB " << g_var.g_bb_seq_num-1 << std::endl;
    } else {
        s_missing_ins_in_stat_code_cnt++;
//        g_var.stat.missingInsList.insert (ins_addr);
    }
}

VOID pin__getMemIns (ADDRINT ins_addr, ADDRINT memAccessSize, ADDRINT memAddr, 
        BOOL isStackRd, BOOL isStackWr, BOOL isMemRead) {
    if (g__staticCode->hasIns (ins_addr)) {
//        g_var.stat.matchIns++;
        if (g_var.g_core_type == BASICBLOCK) {
            if (_bbHeadSet.find (ins_addr) != _bbHeadSet.end ()) g_br_detected = true;
            pin__detectBB (ins_addr);
            dynInstruction* insObj = pin__makeNewBBIns (ins_addr, MEM);
            if (insObj != NULL) {
                MEM_TYPE mType = (isMemRead == true ? LOAD : STORE);
                insObj->setMemAtr (mType, memAddr, memAccessSize, isStackRd, isStackWr);
            }
        } else { /* INO & O3 */
            dynInstruction* insObj = pin__makeNewIns (ins_addr, MEM);
            MEM_TYPE mType = (isMemRead == true ? LOAD : STORE);
            insObj->setMemAtr (mType, memAddr, memAccessSize, isStackRd, isStackWr);
        }
        if (g_var.g_debug_level & DBG_UOP) 
            std::cout << "NEW MEM: " << (g_var.g_wrong_path?"*":" ") << hex << ins_addr << 
                " (" << g_var.g_seq_num << ") in BB " << g_var.g_bb_seq_num-1 << std::endl;
    } else {
        s_missing_ins_in_stat_code_cnt++;
//        g_var.stat.missingInsList.insert (ins_addr);
    }
}

VOID pin__getIns (ADDRINT ins_addr) {
    if (g__staticCode->hasIns (ins_addr)) {
//        g_var.stat.matchIns++;
        if (g_var.g_core_type == BASICBLOCK) {
            if (_bbHeadSet.find (ins_addr) != _bbHeadSet.end ()) g_br_detected = true;
            pin__detectBB (ins_addr);
            pin__makeNewBBIns (ins_addr, ALU);
        } else { /* INO & O3 */
            pin__makeNewIns (ins_addr, ALU);
        }
        if (g_var.g_debug_level & DBG_UOP) 
            std::cout << "NEW INS: " << (g_var.g_wrong_path?"*":" ") << hex << ins_addr << 
                " (" << g_var.g_seq_num << ") in BB " << g_var.g_bb_seq_num-1 << std::endl;
    } else {
        s_missing_ins_in_stat_code_cnt++;
//        g_var.stat.missingInsList.insert (ins_addr);
    }
}

VOID pin__getNopIns (ADDRINT ins_addr) {
    if (g__staticCode->hasIns (ins_addr)) {
//        g_var.stat.matchIns++;
        if (g_var.g_core_type == BASICBLOCK) {
            if (_bbHeadSet.find (ins_addr) != _bbHeadSet.end ()) g_br_detected = true;
            pin__detectBB (ins_addr);
            pin__makeNewBBIns (ins_addr, ALU);
        } else { /* INO & O3 */
            pin__makeNewIns (ins_addr, NOP);
        }
        if (g_var.g_debug_level & DBG_UOP) 
            std::cout << "NEW NOP: " << (g_var.g_wrong_path?"*":" ") << hex << ins_addr << 
                " (" << g_var.g_seq_num << ") in BB " << g_var.g_bb_seq_num-1 << std::endl;
    } else {
        s_missing_ins_in_stat_code_cnt++;
//        g_var.stat.missingInsList.insert (ins_addr);
    }
}

/*-- BASIC COMMANDS TO MAKE AN INSTRUCTION FOR INO & O3--*/
dynInstruction* pin__makeNewIns (ADDRINT ins_addr, INS_TYPE ins_type) {
    dynInstruction* insObj = g_var.getNewCodeCacheIns ();
    stInstruction* staticIns = g__staticCode->getInsObj (ins_addr);
    staticIns->copyRegsTo (insObj);
    insObj->setInsType (ins_type);
    insObj->setInsAddr (ins_addr);
    insObj->setInsID (g_var.g_seq_num++);
    insObj->setWrongPath (g_var.g_wrong_path);
    return insObj;
}

/*-- BASIC COMMANDS TO MAKE AN INSTRUCTION FOR BB --*/
bbInstruction* pin__makeNewBBIns (ADDRINT ins_addr, INS_TYPE ins_type) {
    dynBasicblock* bbObj = g_var.getLastCacheBB ();
    if (bbObj == NULL) return NULL;
    if (g_var.scheduling_mode == STATIC_SCH && bbObj->isInInsMap (ins_addr)) return NULL;
    bbInstruction* insObj = g_var.getNewIns ();
    stInstruction* staticIns = g__staticCode->getInsObj (ins_addr);
    staticIns->copyRegsTo (insObj);
    insObj->setInsType (ins_type);
    insObj->setInsAddr (ins_addr);
    if (g_var.scheduling_mode == DYNAMIC_SCH) { insObj->setInsID (g_var.g_seq_num++); }
    insObj->setWrongPath (g_var.g_wrong_path);
    if (bbObj->insertIns (insObj)) { Assert (0 && "to be implemented"); }
    insObj->setBB (bbObj);
    return insObj;
}

/* ************************************* *
 * BB INSTRUMENTATIONS
 * ************************************ */

/*-- DETECT IF A NEW BB BEGINS --*/
void pin__detectBB (ADDRINT ins_addr) {
    LENGTH bbWin_size;
    g_cfg->_root["cpu"]["backend"]["table"]["bbWindow"]["size"] >> bbWin_size;

    if (g__staticCode->hasStaticBB (ins_addr)) {
        BOOL is_tail_br = g__staticCode->bbHasBr (ins_addr);
        ADDRINT bb_br_addr = g__staticCode->getBBbr (ins_addr);
        pin__getBBhead (ins_addr, bb_br_addr, is_tail_br);
    } else if (g_br_detected) {
        pin__getBBhead (ins_addr, 0, false); //TODO fix this - not valid
        _bbHeadSet.insert (ins_addr);
    } else if (g_var.getLastCacheBB ()->getBBsize () > bbWin_size  ||
               g_var.getLastCacheBB ()->_insList.NumElements () > bbWin_size ||
               (LENGTH)g_var.getLastCacheBB ()->_bbInsMap.size () > bbWin_size) { //TODO temp solution to break off large BB's
        pin__getBBhead (ins_addr, 0, false); //TODO fix this - not valid
        s_large_bb_cnt++;
    }
    g_br_detected = false;
}

void pin__genInsStat (dynBasicblock* bb) {
    List<bbInstruction*>* insList = bb->getBBinsList ();
    LENGTH bb_size = bb->getBBsize ();
    for (LENGTH i = 0; i < bb_size; i++) {
        /* LOCAL VS. GLOBAL REGISTER OPERAND STAT */
        bbInstruction* ins = insList->Nth (i);
        s_dyn_lr_cnt    += ins->getTotNumRdLAR () + ins->getNumWrLAR ();
        s_dyn_gr_cnt    += ins->getTotNumRdAR () + ins->getNumWrAR ();
        s_dyn_lr_rd_cnt += ins->getTotNumRdLAR ();
        s_dyn_lr_wr_cnt += ins->getNumWrLAR ();
        s_dyn_gr_rd_cnt += ins->getTotNumRdAR ();
        s_dyn_gr_wr_cnt += ins->getNumWrAR ();

        /* INSTRUCTIONS WITH AND WITHOUGHT LOCAL OPERANDS */
        if (ins->getTotNumRdLAR () == 0 && ins->getNumWrLAR () == 0)
            s_ins_without_lr_operand_cnt++;
        else
            s_ins_with_lr_operand_cnt++;
    }

}

/*-- POST PROCESS THE LATEST BB AND MAKE A NEW BB OBJECT --*/
void pin__getBBhead (ADDRINT bb_addr, ADDRINT bb_br_addr, BOOL is_tail_br) {
    if (g_var.g_debug_level & DBG_UOP) 
        std::cout << "NEW BB: " << (g_var.g_wrong_path?"*":" ") << hex << g_var.g_bb_seq_num << std::endl;

    /* FINAL PASSES ON THE CLOSING BASICBLOCK - SCHEDULING, WRONGPATH, ETC. */
    dynBasicblock* lastBB = g_var.getLastCacheBB ();
    if (lastBB != NULL) {
        if (g_var.scheduling_mode == STATIC_SCH) {
            lastBB->rescheduleInsList (&g_var.g_seq_num);
        }
        lastBB->wrongPathCheck ();

        /* GENERATE SOME STATISTICS */
        pin__genInsStat (lastBB);
    }

    /* MAKE THE NEXT BASICBLOCK */
    dynBasicblock* bbObj = g_var.getNewCacheBB ();
    bbObj->setBBID (g_var.g_bb_seq_num++);
    bbObj->setBBbrAddr (is_tail_br, bb_br_addr);
    if (g_var.scheduling_mode == STATIC_SCH) {
        if (g__staticCode->hasStaticBB (bb_addr))
            bbObj->setBBstaticInsList (g__staticCode->getBBinsList (bb_addr));
        else
            s_pin_missing_static_bb_cnt++;
    }
    s_pin_bb_cnt++;
}

//void pin__get_bb_header (ADDRINT bb_addr, INS bb_tail_ins) {
//    Assert (g_var.g_enable_bkEnd);
//    BOOL is_br = INS_IsBranchOrCall (bb_tail_ins) || INS_IsFarRet (bb_tail_ins) || INS_IsRet (bb_tail_ins);
//    ADDRINT bb_br_addr = INS_Address (bb_tail_ins);
//    if (is_br) {
//        if (INS_HasFallThrough (bb_tail_ins)) {
//            INS_InsertCall (bb_tail_ins, IPOINT_AFTER, (AFUNPTR) pin__getBBhead,
//                    IARG_ADDRINT, bb_addr,
//                    IARG_ADDRINT, bb_br_addr,
//                    IARG_BOOL, is_br,
//                    IARG_END);
//        }
//        INS_InsertCall (bb_tail_ins, IPOINT_TAKEN_BRANCH, (AFUNPTR) pin__getBBhead,
//                IARG_ADDRINT, bb_addr,
//                IARG_ADDRINT, bb_br_addr,
//                IARG_BOOL, is_br,
//                IARG_END);
//    } else {
//        INS_InsertCall (bb_tail_ins, IPOINT_BEFORE, (AFUNPTR) pin__getBBhead,
//                IARG_ADDRINT, bb_addr,
//                IARG_ADDRINT, bb_br_addr,
//                IARG_BOOL, is_br,
//                IARG_END);
//    }
//}

/* ************************************* *
 * PIN INSTRUMENTATION
 * ************************************ */
void pin__uOpGenInit (staticCodeParser &staticCode) {
    g__staticCode = &staticCode;
}

void pin__getOp (INS ins) {
    Assert (g_var.g_enable_bkEnd);
    BOOL is_ret = INS_IsRet (ins) || INS_IsFarRet (ins);
    BOOL is_call = INS_IsCall (ins) || INS_IsFarCall (ins);
    BOOL is_dir_br_jmp = INS_IsDirectFarJump (ins) || INS_IsDirectBranchOrCall (ins);
    BOOL is_br_jmp = INS_IsDirectFarJump (ins) || INS_IsFarJump (ins) || (INS_IsBranch (ins) && INS_HasFallThrough (ins));

    if (INS_IsBranchOrCall (ins) || INS_IsDirectBranchOrCall (ins) ||
        INS_IsFarRet (ins) || INS_IsRet (ins) || INS_IsSysret (ins) || 
        INS_IsDirectFarJump (ins) || INS_IsFarJump (ins) ||
        INS_IsCall (ins) || INS_IsFarCall (ins) || INS_IsProcedureCall (ins) ||
        INS_IsDirectCall (ins) || INS_IsDirectBranch (ins)) {
        if (INS_HasFallThrough (ins)) {
            INS_InsertCall (ins, IPOINT_AFTER, (AFUNPTR) pin__getBrIns,
                    IARG_ADDRINT, INS_Address (ins),
                    IARG_BOOL, INS_HasFallThrough (ins),
                    IARG_BRANCH_TARGET_ADDR, 
                    IARG_FALLTHROUGH_ADDR,
                    IARG_BRANCH_TAKEN,
                    IARG_BOOL, is_call,
                    IARG_BOOL, is_ret,
                    IARG_BOOL, is_br_jmp,
                    IARG_BOOL, is_dir_br_jmp,
                    IARG_END);
        }
        INS_InsertCall (ins, IPOINT_TAKEN_BRANCH, (AFUNPTR) pin__getBrIns,
                IARG_ADDRINT, INS_Address (ins),
                IARG_BOOL, INS_HasFallThrough (ins),
                IARG_BRANCH_TARGET_ADDR, 
                IARG_FALLTHROUGH_ADDR,
                IARG_BRANCH_TAKEN,
                IARG_BOOL, is_call,
                IARG_BOOL, is_ret,
                IARG_BOOL, is_br_jmp,
                IARG_BOOL, is_dir_br_jmp,
                IARG_END);
        /*
        //capture mem u-op
        if (INS_IsMemoryWrite (ins) || INS_IsMemoryRead (ins)) {
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) pin__getMemIns,
        IARG_ADDRINT, INS_Address (ins),
        IARG_MEMORYWRITE_SIZE,
        IARG_MEMORYWRITE_EA,
        IARG_END);
        }
        */
    } else if (INS_IsSyscall (ins)) {
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) pin__getBrIns,
                IARG_ADDRINT, INS_Address (ins),
                IARG_BOOL, INS_HasFallThrough (ins),
                IARG_BRANCH_TARGET_ADDR, 
                IARG_FALLTHROUGH_ADDR,
                IARG_BRANCH_TAKEN,
                IARG_BOOL, is_call,
                IARG_BOOL, is_ret,
                IARG_BOOL, is_br_jmp,
                IARG_BOOL, is_dir_br_jmp,
                IARG_END);
    } else if (INS_IsMemoryRead (ins)) {
        bool isMemRead;
        isMemRead = true;
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) pin__getMemIns,
                IARG_ADDRINT, INS_Address (ins),        
                IARG_MEMORYREAD_SIZE,
                IARG_MEMORYREAD_EA,
                IARG_BOOL, INS_IsStackRead (ins),
                IARG_BOOL, INS_IsStackWrite (ins),
                IARG_BOOL, isMemRead,
                IARG_END);
    } else if (INS_IsMemoryWrite (ins)) {
        BOOL isMemRead;
        isMemRead = false;
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) pin__getMemIns,
                IARG_ADDRINT, INS_Address (ins),
                IARG_MEMORYWRITE_SIZE,
                IARG_MEMORYWRITE_EA,
                IARG_BOOL, INS_IsStackRead (ins),
                IARG_BOOL, INS_IsStackWrite (ins),
                IARG_BOOL, isMemRead,
                IARG_END);
    } else if (INS_IsNop (ins)) {
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) pin__getNopIns,
                IARG_ADDRINT, INS_Address (ins),
                IARG_END);
    } else {
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) pin__getIns,
                IARG_ADDRINT, INS_Address (ins),
                IARG_END);
        /*
        //capture mem u-op
        if (INS_IsMemoryWrite (ins) || INS_IsMemoryRead (ins)) {
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) pin__getMemIns,
        IARG_ADDRINT, INS_Address (ins),
        IARG_MEMORYWRITE_SIZE,
        IARG_MEMORYWRITE_EA,
        IARG_END);
        */
    }
}
