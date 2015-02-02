/*********************************************************************************
 * rfManager.cpp
 ********************************************************************************/

#include "rfManager.h"

o3_rfManager::o3_rfManager (sysClock* clk, const YAML::Node& root, string rf_name)
    : unit (rf_name, clk),
      _GRF (clk, root["rf"], "registerRename"),
      _e_rf (rf_name + ".rf", root["rf"]),
      _e_rat (rf_name + ".rat", root["rat"]),
      _e_apr (rf_name + ".apr", root["apr"]),
      _e_arst (rf_name + ".arst" , root["arst"]),
      s_rf_not_ready_cnt (g_stats.newScalarStat (rf_name, "rf_not_ready_cnt", "Number of RF operand-not-ready events", 0, PRINT_ZERO)),
      s_cant_rename_cnt (g_stats.newScalarStat (rf_name, "cant_rename_cnt", "Number of failed reg. rename attempts", 0, NO_PRINT_ZERO)),
      s_can_rename_cnt (g_stats.newScalarStat (rf_name, "can_rename_cnt", "Number of success reg. rename attempts", 0, NO_PRINT_ZERO)),
      s_unavailable_cnt (g_stats.newScalarStat (rf_name, "unavailable_cnt", "Number of unavailable wire accesses", 0, NO_PRINT_ZERO))
{ }

o3_rfManager::~o3_rfManager () { }

/* IN ORDER TO SUUPORT FORWARDING, WE SHOULD DO A SECOND CHECK FOR THE
 * READINESS OF THE INSTRUCTIONS FORWARDED */
bool o3_rfManager::checkReadyAgain (dynInstruction* ins) {
    List<AR>* p_rdReg_list = ins->getPRrdList ();
    for (int i = p_rdReg_list->NumElements () - 1; i >= 0; i--) {
        AR reg = p_rdReg_list->Nth (i);
        if (!_GRF.isPRvalid (reg)) {
            dbg.print (DBG_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
                    "RF read ops are ready: NO", "for ins: ", ins->getInsID (), _clk->now ());
            return false; /*-- OPERAND NOT AVAILABLE --*/
        }
    }

    if (p_rdReg_list->NumElements () == 0) {
        dbg.print (DBG_REG_FILES, "%s: %s %d %s (cyc: %d)\n", _c_name.c_str (), 
                "RF operand of ins", ins->getInsID (), "are ready", _clk->now ());
        return true; /*-- ALL OPERANDS AVAILABLE --*/
    }

    dbg.print (DBG_REG_FILES, "%s: %s %d s (cyc: %d)\n", _c_name.c_str (), 
            "RF operand of ins", ins->getInsID (), "are ready", _clk->now ());
    return false; /*-- NOT ALL OPERANDS AVAILABLE --*/
}

/*-- ARE ALL READ OPERANDS READY? --*/
bool o3_rfManager::isReady (dynInstruction* ins) {
    List<AR>* p_rdReg_list = ins->getPRrdList ();
    for (int i = p_rdReg_list->NumElements () - 1; i >= 0; i--) {
        AR reg = p_rdReg_list->Nth (i);
        if (!_GRF.isPRvalid (reg)) {
            s_rf_not_ready_cnt++;
            dbg.print (DBG_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
                    "RF read ops are ready: NO", "for ins: ", ins->getInsID (), _clk->now ());
            return false; /*-- OPERAND NOT AVAILABLE --*/
        } else {
            p_rdReg_list->RemoveAt (i); /*-- OPTIMIZATION --*/
        }
    }
    if (p_rdReg_list->NumElements () == 0) {
        _e_rf.ramAccess (ins->getTotNumRdAR ());
        dbg.print (DBG_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
                "RF read ops are ready: YES", "for ins: ", ins->getInsID (), _clk->now ());
        return true; /*-- ALL OPERANDS AVAILABLE --*/
    }
    dbg.print (DBG_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
            "RF read ops are ready: NO", "for ins: ", ins->getInsID (), _clk->now ());
    s_rf_not_ready_cnt++;
    return false; /*-- NOT ALL OPERANDS AVAILABLE --*/
}

/*-- CHECK IS NO OTHER OBJ IS WRITING INTO WRITE REGS --*/
bool o3_rfManager::canRename (dynInstruction* ins) {
    List<AR>* ar_wr = ins->getARwrList ();
    if (_GRF.getNumAvailablePR () < ar_wr->NumElements ()) {
        s_cant_rename_cnt++;
        return false; /*-- STALL FETCH --*/
    }
    s_can_rename_cnt++;
    return true;
}

bool o3_rfManager::renameRegs (dynInstruction* ins) {
    List<AR>* ar_rd = ins->getARrdList ();
    List<AR>* ar_wr = ins->getARwrList ();
    Assert (_GRF.getNumAvailablePR () >= ar_wr->NumElements ());

    /*-- RENAME READ REFISTERS FIRST --*/
    for (int i = 0; i < ar_rd->NumElements (); i++) {
        AR a_reg = ar_rd->Nth (i);
        PR p_reg = _GRF.renameReg (a_reg);
        ins->setPR (p_reg, READ);
        _e_rat.ramAccess ();
    }

    /*-- RENAME WRITE REFISTERS SECOND --*/
    for (int i = 0; i < ar_wr->NumElements (); i++) {
        Assert (_GRF.isAnyPRavailable () == true && "A physical reg must have been available.");
        AR a_reg = ar_wr->Nth (i);
        PR prev_pr = _GRF.renameReg (a_reg);
        _e_rat.ramAccess ();
        PR new_pr = _GRF.getAvailablePR ();
        _e_apr.ramAccess ();
        _GRF.update_fRAT (a_reg, new_pr);
        _e_rat.ramAccess ();
        _GRF.updatePR (new_pr, prev_pr, RENAMED_INVALID);
        _e_arst.ramAccess ();
        ins->setPR (new_pr, WRITE);
    }
    return false; /*-- DON'T STALL FETCH --*/
}

/*-- PROCESS WRITE REGISTERS @COMPLETE --*/
void o3_rfManager::completeRegs (dynInstruction* ins) {
    List<PR>* _pr = ins->getPRwrList ();
    for (int i = 0; i < _pr->NumElements (); i++) {
        PR p_reg = _pr->Nth (i);
        _GRF.updatePRstate (p_reg, RENAMED_VALID);
        _e_rf.ramAccess ();
    }
}

/*-- PROCESS WRITE REFISTERS @COMMIT --*/
void o3_rfManager::commitRegs (dynInstruction* ins) {
    List<PR>* _pr = ins->getPRwrList ();
    List<PR>* _ar = ins->getARwrList ();
    Assert (_ar->NumElements () == _pr->NumElements ());
    for (int i = 0; i < _pr->NumElements (); i++) {
        PR p_reg = _pr->Nth (i);
        AR a_reg = _ar->Nth (i);
        PR prev_pr = _GRF.getPrevPR (p_reg);
        _e_arst.ramAccess ();
        _GRF.updatePRstate (p_reg,ARCH_REG);
        _GRF.updatePRstate (prev_pr,AVAILABLE);
        _GRF.update_cRAT (a_reg,p_reg);
        _e_rat.ramAccess ();
        _GRF.setAsAvailablePR (prev_pr);
        _e_apr.ramAccess ();
    }
}

/*-- 
 * ASSUMING:
 * NO ENERGY COST - SWAPPING POINTERS OF FRAT AND CRAT
 * NO LATENCY COST - FOR THE SAME REASON AS ABOVE
 --*/
void o3_rfManager::squashRenameReg () {
    dbg.print (DBG_TEST, "** %s: %s (cyc:)\n", _c_name.c_str (), "in squash for RR");
    _GRF.squashRenameReg ();
    /* TODO model the ARST and APR table energies for squash */
}

bool o3_rfManager::hasFreeWire (AXES_TYPE axes_type, WIDTH numRegWires) {
    if (_GRF.getNumFreeWires (axes_type) >= numRegWires) {
        return true;
    } else {
        s_unavailable_cnt++;
        return false;
    }
}

void o3_rfManager::updateWireState (AXES_TYPE axes_type, WIDTH numRegWires) {
    for (WIDTH i = 0; i < numRegWires; i++) {
        _GRF.updateWireState (axes_type);
    }
}

void o3_rfManager::regStat () {
    _GRF.getStat ();
}
o3_rfManager* g_GRF_MGR;
