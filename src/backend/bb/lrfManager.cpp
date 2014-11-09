/*******************************************************************************
 * lrfManager.cpp
 ******************************************************************************/

#include "lrfManager.h"

bb_lrfManager::bb_lrfManager (WIDTH lrf_id, sysClock* clk, string rf_name)
    : unit (rf_name, clk),
      _RF (LARF_LO, LARF_SIZE, 8, 4, clk, "LocalRegisterFile"),
      _lrf_id (lrf_id),
      s_unavailable_cnt (g_stats.newScalarStat (rf_name, "unavailable_cnt", "Number of unavailable wire accesses", 0, NO_PRINT_ZERO))
{ }

bb_lrfManager::~bb_lrfManager () { }

void bb_lrfManager::resetRF () {
    dbg.print (DBG_L_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
            "Reset LRF", _lrf_id, _clk->now ());
    _RF.resetRF ();
}

/* ARE ALL READ OPERANDS READY? */
bool bb_lrfManager::isReady (bbInstruction* ins) {
    List<AR>* a_rdReg_list = ins->getLARrdList ();
    for (int i = a_rdReg_list->NumElements () - 1; i >= 0; i--) {
        AR reg = a_rdReg_list->Nth (i);
        if (!_RF.isRegValidAndReady (reg)) {
            dbg.print (DBG_L_REG_FILES, "%s: %s %d %s %d (cyc: %d)\n", _c_name.c_str (), 
                    "Reg", reg, "is invlid in LRF", _lrf_id, _clk->now ());
            return false; /* operand not available */
        } else {
            a_rdReg_list->RemoveAt (i); /*optimization */
        }
    }

    if (a_rdReg_list->NumElements () == 0) {
        dbg.print (DBG_L_REG_FILES, "%s: %s %d %s (cyc: %d)\n", _c_name.c_str (), 
                "Local operand of ins", ins->getInsID (), "are ready", _clk->now ());
        return true; /* all operands available */
    }

    dbg.print (DBG_L_REG_FILES, "%s: %s %d s (cyc: %d)\n", _c_name.c_str (), 
            "Local operand of ins", ins->getInsID (), "are ready", _clk->now ());
    return false; /* not all operands available */
}

/* RESERVE REGISTER FILE ENTRIES FOR WRITE */
void bb_lrfManager::reserveRF (bbInstruction* ins) {
    dbg.print (DBG_L_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
            "Reserving regisers for ins", ins->getInsID (), _clk->now ());
    List<AR>* a_wrReg_list = ins->getLARwrList ();
    for (int i = 0; i < a_wrReg_list->NumElements (); i++) {
        AR reg = a_wrReg_list->Nth (i);
        _RF.reserveReg (reg);
    }
}

/* CHECK IS NO OTHER OBJ IS WRITING INTO WRITE REGS */
bool bb_lrfManager::canReserveRF (bbInstruction* ins) {
    List<AR>* a_wrReg_list = ins->getLARwrList ();
    for (int i = 0; i < a_wrReg_list->NumElements (); i++) {
        AR reg = a_wrReg_list->Nth (i);
        if (_RF.isRegBusy (reg)) {
            dbg.print (DBG_L_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
                    "Can NOT Reserve regisers for ins", ins->getInsID (), _clk->now ());
            return false; /* operand not available for write */
        }
    }
    dbg.print (DBG_L_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
            "Can Reserve regisers for ins", ins->getInsID (), _clk->now ());
    return true;
}

void bb_lrfManager::writeToRF (bbInstruction* ins) {
    dbg.print (DBG_L_REG_FILES, "%s: %s %d %s %d (cyc: %d)\n", _c_name.c_str (), 
            "Write to LRF ", _lrf_id, "for ins", ins->getInsID (), _clk->now ());
    List<AR>* a_wrReg_list = ins->getLARwrList ();
    for (int i = 0; i < a_wrReg_list->NumElements (); i++) {
        AR reg = a_wrReg_list->Nth (i);
        _RF.updateReg (reg);
    }
}

void bb_lrfManager::updateReg (PR reg) {
    _RF.updateReg(reg);
}

bool bb_lrfManager::hasFreeWire (AXES_TYPE axes_type, WIDTH numRegWires) {
    if (_RF.getNumFreeWires (axes_type) >= numRegWires) {
        return true;
    } else {
        s_unavailable_cnt++;
        return false;
    }
}

void bb_lrfManager::updateWireState (AXES_TYPE axes_type, WIDTH numRegWires) {
    for (WIDTH i = 0; i < numRegWires; i++) {
        _RF.updateWireState (axes_type);
    }
}
