#include "uop_gen.h"

// Enable micro-ops (set to 1 for enable)
#define ENABLE_MICRO_OPS 0

staticCodeParser * _g_staticCode;

void uop_gen (FILE* _outFile, staticCodeParser &g_staticCode)
{
    _g_staticCode = &g_staticCode;
}

VOID getBrIns (ADDRINT insAddr, BOOL hasFT, ADDRINT tgAddr, ADDRINT ftAddr, BOOL isTaken, BOOL isCall, BOOL isRet, BOOL isJump, BOOL isDirBrOrCallOrJmp) {
    if (_g_staticCode->hasIns (insAddr)) {
        g_var.stat.matchIns++;
        if (g_var.g_debug_level & DBG_UOP) std::cout << "NEW BR: " << (g_var.g_wrong_path?"*":" ") << dec << g_var.g_seq_num << " in BB " << g_var.g_bb_seq_num-1 << std::endl;
        if (g_var.g_core_type == BASICBLOCK) {
            dynBasicblock* g_bbObj = g_var.getLastCacheBB ();
            bbInstruction* g_insObj = g_var.getNewIns ();
            stInstruction* staticIns = _g_staticCode->getInsObj (insAddr);
            staticIns->copyRegsTo (g_insObj);
            g_insObj->setBrAtr (tgAddr, ftAddr, hasFT, isTaken, isCall, isRet, isJump, isDirBrOrCallOrJmp);
            g_insObj->setInsType (BR);
            g_insObj->setInsAddr (insAddr);
            g_insObj->setInsID (g_var.g_seq_num++);
            g_insObj->setWrongPath (g_var.g_wrong_path);
            if (g_var.g_wrong_path) g_bbObj->setWrongPath ();
            if (g_bbObj->insertIns (g_insObj)) Assert (true == false && "to be implemented");
            g_insObj->setBB (g_bbObj);
        } else { /* INO & O3 */
            dynInstruction* g_insObj = g_var.getNewCodeCacheIns ();
            stInstruction* staticIns = _g_staticCode->getInsObj (insAddr);
            staticIns->copyRegsTo (g_insObj);
            g_insObj->setBrAtr (tgAddr, ftAddr, hasFT, isTaken, isCall, isRet, isJump, isDirBrOrCallOrJmp);
            g_insObj->setInsType (BR);
            g_insObj->setInsAddr (insAddr);
            g_insObj->setInsID (g_var.g_seq_num++);
            g_insObj->setWrongPath (g_var.g_wrong_path);
        }
    } else {
        g_var.stat.noMatchIns++;
        g_var.stat.missingInsList.insert (insAddr);
    }
}

VOID getMemIns (ADDRINT insAddr, ADDRINT memAccessSize, ADDRINT memAddr, BOOL isStackRd, BOOL isStackWr, BOOL isMemRead) {
    if (_g_staticCode->hasIns (insAddr)) {
        g_var.stat.matchIns++;
        if (g_var.g_debug_level & DBG_UOP) std::cout << "NEW MEM: " << (g_var.g_wrong_path?"*":" ") << dec << g_var.g_seq_num << " in BB " << g_var.g_bb_seq_num-1 << std::endl;
        if (g_var.g_core_type == BASICBLOCK) {
            dynBasicblock* g_bbObj = g_var.getLastCacheBB ();
            bbInstruction* g_insObj = g_var.getNewIns ();
            stInstruction* staticIns = _g_staticCode->getInsObj (insAddr);
            staticIns->copyRegsTo (g_insObj);
            MEM_TYPE mType = (isMemRead == true ? LOAD : STORE);
            g_insObj->setMemAtr (mType, memAddr, memAccessSize, isStackRd, isStackWr);
            g_insObj->setInsType (MEM);
            g_insObj->setInsAddr (insAddr);
            g_insObj->setInsID (g_var.g_seq_num++);
            g_insObj->setWrongPath (g_var.g_wrong_path);
            if (g_var.g_wrong_path) g_bbObj->setWrongPath ();
            if (g_bbObj->insertIns (g_insObj)) Assert (true == false && "to be implemented");
            g_insObj->setBB (g_bbObj);
        } else { /* INO & O3 */
            dynInstruction* g_insObj = g_var.getNewCodeCacheIns ();
            stInstruction* staticIns = _g_staticCode->getInsObj (insAddr);
            staticIns->copyRegsTo (g_insObj);
            MEM_TYPE mType = (isMemRead == true ? LOAD : STORE);
            g_insObj->setMemAtr (mType, memAddr, memAccessSize, isStackRd, isStackWr);
            g_insObj->setInsType (MEM);
            g_insObj->setInsAddr (insAddr);
            g_insObj->setInsID (g_var.g_seq_num++);
            g_insObj->setWrongPath (g_var.g_wrong_path);
        }
    } else {
        g_var.stat.noMatchIns++;
        g_var.stat.missingInsList.insert (insAddr);
    }
}

VOID getIns (ADDRINT insAddr) {
    if (_g_staticCode->hasIns (insAddr)) {
        g_var.stat.matchIns++;
        if (g_var.g_debug_level & DBG_UOP) std::cout << "NEW INS: " << (g_var.g_wrong_path?"*":" ") << dec << g_var.g_seq_num << " in BB " << g_var.g_bb_seq_num-1 << std::endl;
        if (g_var.g_core_type == BASICBLOCK) {
            dynBasicblock* g_bbObj = g_var.getLastCacheBB ();
            if (g_bbObj == NULL) return;
            bbInstruction* g_insObj = g_var.getNewIns ();
            stInstruction* staticIns = _g_staticCode->getInsObj (insAddr);
            staticIns->copyRegsTo (g_insObj);
            g_insObj->setInsType (ALU);
            g_insObj->setInsAddr (insAddr);
            g_insObj->setInsID (g_var.g_seq_num++);
            g_insObj->setWrongPath (g_var.g_wrong_path);
            if (g_var.g_wrong_path) g_bbObj->setWrongPath ();
            if (g_bbObj->insertIns (g_insObj)) Assert (true == false && "to be implemented");
            g_insObj->setBB (g_bbObj);
        } else { /* INO & O3 */
            dynInstruction* g_insObj = g_var.getNewCodeCacheIns ();
            stInstruction* staticIns = _g_staticCode->getInsObj (insAddr);
            staticIns->copyRegsTo (g_insObj);
            g_insObj->setInsType (ALU);
            g_insObj->setInsAddr (insAddr);
            g_insObj->setInsID (g_var.g_seq_num++);
            g_insObj->setWrongPath (g_var.g_wrong_path);
        }
    } else {
        g_var.stat.noMatchIns++;
        g_var.stat.missingInsList.insert (insAddr);
    }
}

VOID getNopIns (ADDRINT insAddr) {
    if (_g_staticCode->hasIns (insAddr)) {
        g_var.stat.matchIns++;
        if (g_var.g_debug_level & DBG_UOP) std::cout << "NEW NOP: " << (g_var.g_wrong_path?"*":" ") << dec << g_var.g_seq_num << " in BB " << g_var.g_bb_seq_num-1 << std::endl;
        if (g_var.g_core_type == BASICBLOCK) {
            dynBasicblock* g_bbObj = g_var.getLastCacheBB ();
            bbInstruction* g_insObj = g_var.getNewIns ();
            stInstruction* staticIns = _g_staticCode->getInsObj (insAddr);
            staticIns->copyRegsTo (g_insObj);
            g_insObj->setInsType (NOP);
            g_insObj->setInsAddr (insAddr);
            g_insObj->setInsID (g_var.g_seq_num++);
            g_insObj->setWrongPath (g_var.g_wrong_path);
            if (g_var.g_wrong_path) g_bbObj->setWrongPath ();
            if (g_bbObj->insertIns (g_insObj)) Assert (true == false && "to be implemented");
            g_insObj->setBB (g_bbObj);
        } else { /* INO & O3 */
            dynInstruction* g_insObj = g_var.getNewCodeCacheIns ();
            stInstruction* staticIns = _g_staticCode->getInsObj (insAddr);
            staticIns->copyRegsTo (g_insObj);
            g_insObj->setInsType (NOP);
            g_insObj->setInsAddr (insAddr);
            g_insObj->setInsID (g_var.g_seq_num++);
            g_insObj->setWrongPath (g_var.g_wrong_path);
        }
    } else {
        g_var.stat.noMatchIns++;
        g_var.stat.missingInsList.insert (insAddr);
    }
}

void getBBhead (ADDRINT bb_tail_ins_addr, BOOL is_tail_br) {
    if (g_var.g_debug_level & DBG_UOP) std::cout << "NEW BB: " << (g_var.g_wrong_path?"*":" ") << dec << g_var.g_bb_seq_num << std::endl;
    dynBasicblock* g_bbObj = g_var.getNewCacheBB ();
    g_bbObj->setBBID (g_var.g_bb_seq_num++);
    g_bbObj->setBBbrAddr (is_tail_br, bb_tail_ins_addr);
}

void get_bb_header (INS bb_tail_ins) {
    if (INS_IsBranchOrCall (bb_tail_ins) || INS_IsFarRet (bb_tail_ins) || INS_IsRet (bb_tail_ins)) {
        if (INS_HasFallThrough (bb_tail_ins)) {
            INS_InsertCall (bb_tail_ins, IPOINT_AFTER, (AFUNPTR) getBBhead,
                    IARG_ADDRINT, INS_Address (bb_tail_ins),
                    IARG_BOOL, INS_IsBranchOrCall (bb_tail_ins) || INS_IsFarRet (bb_tail_ins) || INS_IsRet (bb_tail_ins),
                    IARG_END);
        }
        INS_InsertCall (bb_tail_ins, IPOINT_TAKEN_BRANCH, (AFUNPTR) getBBhead,
                IARG_ADDRINT, INS_Address (bb_tail_ins),
                IARG_BOOL, INS_IsBranchOrCall (bb_tail_ins) || INS_IsFarRet (bb_tail_ins) || INS_IsRet (bb_tail_ins),
                IARG_END);
    } else {
        INS_InsertCall (bb_tail_ins, IPOINT_BEFORE, (AFUNPTR) getBBhead,
                IARG_ADDRINT, INS_Address (bb_tail_ins),
                IARG_BOOL, INS_IsBranchOrCall (bb_tail_ins) || INS_IsFarRet (bb_tail_ins) || INS_IsRet (bb_tail_ins),
                IARG_END);
    }
}

void get_uop (INS ins) {
//    if (INS_IsBranchOrCall (ins) || INS_IsFarRet (ins) || INS_IsRet (ins)) { //TODO put is back
    if (INS_IsBranchOrCall (ins)) {
        if (INS_HasFallThrough (ins)) {
            INS_InsertCall (ins, IPOINT_AFTER, (AFUNPTR) getBrIns,
                    IARG_ADDRINT, INS_Address (ins),
                    IARG_BOOL, INS_HasFallThrough (ins),
                    IARG_BRANCH_TARGET_ADDR, 
                    IARG_FALLTHROUGH_ADDR,
                    IARG_BRANCH_TAKEN,
                    IARG_BOOL, INS_IsCall (ins) || INS_IsFarCall (ins),
                    IARG_BOOL, INS_IsRet (ins) || INS_IsFarRet (ins),
                    IARG_BOOL, INS_IsDirectFarJump (ins) || INS_IsFarJump (ins) || (INS_IsBranch (ins) && INS_HasFallThrough (ins)),
                    IARG_BOOL, INS_IsDirectFarJump (ins) || INS_IsDirectBranchOrCall (ins),
                    IARG_END);
        }
        INS_InsertCall (ins, IPOINT_TAKEN_BRANCH, (AFUNPTR) getBrIns,
                IARG_ADDRINT, INS_Address (ins),
                IARG_BOOL, INS_HasFallThrough (ins),
                IARG_BRANCH_TARGET_ADDR, 
                IARG_FALLTHROUGH_ADDR,
                IARG_BRANCH_TAKEN,
                IARG_BOOL, INS_IsCall (ins) || INS_IsFarCall (ins),
                IARG_BOOL, INS_IsRet (ins) || INS_IsFarRet (ins),
                IARG_BOOL, INS_IsDirectFarJump (ins) || INS_IsFarJump (ins) || (INS_IsBranch (ins) && INS_HasFallThrough (ins)),
                IARG_BOOL, INS_IsDirectFarJump (ins) || INS_IsDirectBranchOrCall (ins),
                IARG_END);
        /*
        //capture mem u-op
        if (INS_IsMemoryWrite (ins) || INS_IsMemoryRead (ins)) {
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) getMemIns,
        IARG_ADDRINT, INS_Address (ins),
        IARG_MEMORYWRITE_SIZE,
        IARG_MEMORYWRITE_EA,
        IARG_END);
        }
        */
    } else if (INS_IsMemoryWrite (ins)) {
        BOOL isMemRead;
        isMemRead = false;
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) getMemIns,
                IARG_ADDRINT, INS_Address (ins),
                IARG_MEMORYWRITE_SIZE,
                IARG_MEMORYWRITE_EA,
                IARG_BOOL, INS_IsStackRead (ins),
                IARG_BOOL, INS_IsStackWrite (ins),
                IARG_BOOL, isMemRead,
                IARG_END);
    } else if (INS_IsMemoryRead (ins)) {
        bool isMemRead;
        isMemRead = true;
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) getMemIns,
                IARG_ADDRINT, INS_Address (ins),
                IARG_MEMORYREAD_SIZE,
                IARG_MEMORYREAD_EA,
                IARG_BOOL, INS_IsStackRead (ins),
                IARG_BOOL, INS_IsStackWrite (ins),
                IARG_BOOL, isMemRead,
                IARG_END);
    } else if (INS_IsNop (ins)) {
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) getNopIns,
                IARG_ADDRINT, INS_Address (ins),
                IARG_END);
    } else {
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) getIns,
                IARG_ADDRINT, INS_Address (ins),
                IARG_END);
        /*
        //capture mem u-op
        if (INS_IsMemoryWrite (ins) || INS_IsMemoryRead (ins)) {
        INS_InsertCall (ins, IPOINT_BEFORE, (AFUNPTR) getMemIns,
        IARG_ADDRINT, INS_Address (ins),
        IARG_MEMORYWRITE_SIZE,
        IARG_MEMORYWRITE_EA,
        IARG_END);
        */
    }
}
