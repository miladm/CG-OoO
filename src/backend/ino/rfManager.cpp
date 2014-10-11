/*******************************************************************************
 * rfManager.cpp
 ******************************************************************************/

#include "rfManager.h"

rfManager::rfManager (sysClock* clk, string rf_name)
    : unit (rf_name, clk),
      _RF (1, LARF_SIZE+GARF_SIZE, 8, 4, clk, "registerFile"),
      s_rf_not_ready_cnt (g_stats.newScalarStat (rf_name, "rf_not_ready_cnt", "Number of RF operand-not-ready events", 0, PRINT_ZERO)),
      s_lrf_busy_cnt (g_stats.newScalarStat (rf_name, "rf_busy_cnt", "Number of LRF write operand-not-ready events", 0, PRINT_ZERO)),
      s_unavailable_cnt (g_stats.newScalarStat (rf_name, "unavailable_cnt", "Number of unavailable wire accesses", 0, NO_PRINT_ZERO))
{ }

rfManager::~rfManager () { }

void rfManager::resetRF () {
    _RF.resetRF ();
}

/* ARE ALL READ OPERANDS READY? */
bool rfManager::isReady (dynInstruction* ins) {
    List<AR>* a_rdReg_list = ins->getARrdList ();
    for (int i = a_rdReg_list->NumElements () - 1; i >= 0; i--) {
        AR reg = a_rdReg_list->Nth (i);
        if (!_RF.isRegValidAndReady (reg)) {
            s_rf_not_ready_cnt++;
            return false; /* operand not available */
        } else {
            a_rdReg_list->RemoveAt (i); /*optimization */
        }
    }
    if (a_rdReg_list->NumElements () == 0) {
        return true; /* all operands available */
    }
    s_rf_not_ready_cnt++;
    return false; /* not all operands available */
}

/* RESERVE REGISTER FILE ENTRIES FOR WRITE */
void rfManager::reserveRF (dynInstruction* ins) {
    List<AR>* a_wrReg_list = ins->getARwrList ();
    for (int i = 0; i < a_wrReg_list->NumElements (); i++) {
        AR reg = a_wrReg_list->Nth (i);
        _RF.reserveReg (reg);
    }
}

/* CHECK IS NO OTHER OBJ IS WRITING INTO WRITE REGS */
bool rfManager::canReserveRF (dynInstruction* ins) {
    List<AR>* a_wrReg_list = ins->getARwrList ();
    for (int i = 0; i < a_wrReg_list->NumElements (); i++) {
        AR reg = a_wrReg_list->Nth (i);
        if (_RF.isRegBusy (reg)) {
            s_lrf_busy_cnt++;
            return false; /* operand not available for write */
        }
    }
    return true;
}

void rfManager::writeToRF (dynInstruction* ins) {
    List<AR>* a_wrReg_list = ins->getARwrList ();
    for (int i = 0; i < a_wrReg_list->NumElements (); i++) {
        AR reg = a_wrReg_list->Nth (i);
        _RF.updateReg (reg);
    }
}

void rfManager::updateReg (PR reg) {
    _RF.updateReg(reg);
}

bool rfManager::hasFreeWire (AXES_TYPE axes_type, WIDTH numRegWires) {
    if (_RF.getNumFreeWires (axes_type) >= numRegWires) {
        return true;
    } else {
        s_unavailable_cnt++;
        return false;
    }
}

void rfManager::updateWireState (AXES_TYPE axes_type, WIDTH numRegWires) {
    for (WIDTH i = 0; i < numRegWires; i++) {
        _RF.updateWireState (axes_type);
    }
}

rfManager* g_RF_MGR;
