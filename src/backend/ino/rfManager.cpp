/*******************************************************************************
 * rfManager.cpp
 ******************************************************************************/

#include "rfManager.h"

rfManager::rfManager (sysClock* clk, string rf_name)
    : unit (rf_name, clk),
      _RF (1, LARF_SIZE+GARF_SIZE, 8, 4, clk, "registerFile")
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
        if (!_RF.isRegValid (reg)) {
            return false; /* operand not available */
        } else {
            a_rdReg_list->RemoveAt (i); /*optimization */
        }
    }
    if (a_rdReg_list->NumElements () == 0) {
        return true; /* all operands available */
    }
    return false; /* not all operands available */
}

/* RESERVE REGISTER FILE ENTRIES FOR WRITE */
void rfManager::reserveRF (dynInstruction* ins) {
    List<AR>* a_wrReg_list = ins->getARrdList ();
    for (int i = 0; i < a_wrReg_list->NumElements (); i++) {
        AR reg = a_wrReg_list->Nth (i);
        _RF.reserveReg (reg);
    }
}

/* CHECK IS NO OTHER OBJ IS WRITING INTO WRITE REGS */
bool rfManager::canReserveRF (dynInstruction* ins) {
    List<AR>* a_wrReg_list = ins->getARrdList ();
    for (int i = 0; i < a_wrReg_list->NumElements (); i++) {
        AR reg = a_wrReg_list->Nth (i);
        if (_RF.isRegBusy (reg)) {
            return false; /* operand not available for write */
        }
    }
    return true;
}

void rfManager::writeToRF (dynInstruction* ins) {
    List<AR>* a_wrReg_list = ins->getARrdList ();
    for (int i = 0; i < a_wrReg_list->NumElements (); i++) {
        AR reg = a_wrReg_list->Nth (i);
        _RF.updateReg (reg);
    }
}

void rfManager::updateReg (PR reg) {
    _RF.updateReg(reg);
}

bool rfManager::hasFreeWire (AXES_TYPE axes_type) {
    return _RF.hasFreeWire (axes_type);
}

bool rfManager::hasFreeWire (AXES_TYPE axes_type, WIDTH numRegWires) {
    if (_RF.getNumFreeWires (axes_type) >= numRegWires)
        return true;
    else
        return false;
}

void rfManager::updateWireState (AXES_TYPE axes_type) {
    _RF.updateWireState (axes_type);
}

void rfManager::updateWireState (AXES_TYPE axes_type, WIDTH numRegWires) {
    for (WIDTH i = 0; i < numRegWires; i++) {
        _RF.updateWireState (axes_type);
    }
}

rfManager* g_RF_MGR;
