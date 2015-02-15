/*******************************************************************************
 * rfManager.cpp
 ******************************************************************************/

#include "rfManager.h"

rfManager::rfManager (sysClock* clk, const YAML::Node& root, string rf_name)
    : unit (rf_name, clk),
      _RF (clk, root, "registerFile"),
      _e_table (rf_name, root),
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
            dbg.print (DBG_REG_FILES, "%s: %s %d %s (cyc: %d)\n", _c_name.c_str (), 
                    "Reg", reg, "is invlid in RF", _clk->now ());
            s_rf_not_ready_cnt++;
            return false; /* operand not available */
        } else {
            a_rdReg_list->RemoveAt (i); /*optimization */
        }
    }

    if (a_rdReg_list->NumElements () == 0) {
        _e_table.ramAccess (ins->getTotNumRdAR ());
        dbg.print (DBG_REG_FILES, "%s: %s %d %s (cyc: %d)\n", _c_name.c_str (), 
                "RF operand of ins", ins->getInsID (), "are ready", _clk->now ());
        return true; /* all operands available */
    }

    s_rf_not_ready_cnt++;
    dbg.print (DBG_L_REG_FILES, "%s: %s %d s (cyc: %d)\n", _c_name.c_str (), 
            "RF operand of ins", ins->getInsID (), "are ready", _clk->now ());
    return false; /* not all operands available */
}

bool rfManager::checkReadyAgain (dynInstruction* ins) {
    List<AR>* a_rdReg_list = ins->getARrdList ();
    for (int i = a_rdReg_list->NumElements () - 1; i >= 0; i--) {
        AR reg = a_rdReg_list->Nth (i);
        if (!_RF.isRegValidAndReady (reg)) {
            dbg.print (DBG_REG_FILES, "%s: %s %d %s (cyc: %d)\n", _c_name.c_str (), 
                    "Reg", reg, "is invlid in RF", _clk->now ());
            return false; /* OPERAND NOT AVAILABLE */
        }
    }

    if (a_rdReg_list->NumElements () == 0) {
        dbg.print (DBG_REG_FILES, "%s: %s %d %s (cyc: %d)\n", _c_name.c_str (), 
                "RF operand of ins", ins->getInsID (), "are ready", _clk->now ());
        return true; /* ALL OPERANDS AVAILABLE */
    }

    dbg.print (DBG_L_REG_FILES, "%s: %s %d s (cyc: %d)\n", _c_name.c_str (), 
            "RF operand of ins", ins->getInsID (), "are ready", _clk->now ());
    return false; /* NOT ALL OPERANDS AVAILABLE */
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
        _e_table.ramAccess ();
    }
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

void rfManager::getStat () {
}

rfManager* g_RF_MGR;
