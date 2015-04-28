/*******************************************************************************
 * grfManager.cpp
 ******************************************************************************/

#include "grfManager.h"

bb_grfManager::bb_grfManager (sysClock* clk, WIDTH blk_cnt, const YAML::Node& root, string rf_name)
    : unit (rf_name, clk),
      _GRF (clk, blk_cnt, root["grf"], "GlobalRegisterRename"),
      _e_rf (rf_name + ".grf", root["grf"]),
      _e_rat (rf_name + ".grat", root["grat"]),
      _e_apr (rf_name + ".gapr", root["gapr"]),
      _e_arst (rf_name + ".garst", root["garst"]),
      _e_w_rr (rf_name + ".rr.wire", root["grat"]),
      s_cant_rename_cnt (g_stats.newScalarStat (rf_name, "cant_rename_cnt", "Number of failed reg. rename attempts", 0, NO_PRINT_ZERO)),
      s_can_rename_cnt (g_stats.newScalarStat (rf_name, "can_rename_cnt", "Number of success reg. rename attempts", 0, NO_PRINT_ZERO)),
      s_unavailable_cnt (g_stats.newScalarStat (rf_name, "unavailable_cnt", "Number of unavailable wire accesses", 0, NO_PRINT_ZERO))
{ }

bb_grfManager::~bb_grfManager () { }

/*-- ARE ALL READ OPERANDS READY? --*/
bool bb_grfManager::isReady (bbInstruction* ins) {
    List<AR>* p_rdReg_list = ins->getPRrdList ();
    for (int i = p_rdReg_list->NumElements () - 1; i >= 0; i--) {
        AR reg = p_rdReg_list->Nth (i);
        if (!_GRF.isPRvalid (reg)) {
            dbg.print (DBG_G_REG_FILES, "%s: %s %d %s (cyc: %d)\n", _c_name.c_str (), 
                    "Reg", reg, "is invlid", _clk->now ());
            return false; /*-- OPERAND NOT AVAILABLE --*/
        } else {
            p_rdReg_list->RemoveAt (i); /*-- OPTIMIZATION --*/
        }
    }

    if (p_rdReg_list->NumElements () == 0) {
        dbg.print (DBG_G_REG_FILES, "%s: %s %d %s (cyc: %d)\n", _c_name.c_str (), 
                "Global operand of ins", ins->getInsID (), "are ready", _clk->now ());
        return true; /*-- ALL OPERANDS AVAILABLE --*/
    }

    dbg.print (DBG_G_REG_FILES, "%s: %s %d s (cyc: %d)\n", _c_name.c_str (), 
            "Global operand of ins", ins->getInsID (), "are ready", _clk->now ());
    return false; /*-- NOT ALL OPERANDS AVAILABLE --*/
}

bool bb_grfManager::checkReadyAgain (bbInstruction* ins) {
    List<AR>* p_rdReg_list = ins->getPRrdList ();
    for (int i = p_rdReg_list->NumElements () - 1; i >= 0; i--) {
        AR reg = p_rdReg_list->Nth (i);
        if (!_GRF.isPRvalid (reg)) {
            dbg.print (DBG_G_REG_FILES, "%s: %s %d %s (cyc: %d)\n", _c_name.c_str (), 
                    "Reg", reg, "is invlid", _clk->now ());
            return false; /*-- OPERAND NOT AVAILABLE --*/
        }
    }

    if (p_rdReg_list->NumElements () == 0) {
        dbg.print (DBG_G_REG_FILES, "%s: %s %d %s (cyc: %d)\n", _c_name.c_str (), 
                "Global operand of ins", ins->getInsID (), "are ready", _clk->now ());
        return true; /*-- ALL OPERANDS AVAILABLE --*/
    }

    dbg.print (DBG_G_REG_FILES, "%s: %s %d s (cyc: %d)\n", _c_name.c_str (), 
            "Global operand of ins", ins->getInsID (), "are ready", _clk->now ());
    return false; /*-- NOT ALL OPERANDS AVAILABLE --*/
}

/*-- CHECK IS NO OTHER OBJ IS WRITING INTO WRITE REGS --*/
bool bb_grfManager::canRename (bbInstruction* ins, BB_ID bbWin_id) {
    List<AR>* ar_wr = ins->getARwrList ();
    if (_GRF.getNumAvailablePR (bbWin_id) < ar_wr->NumElements ()) {
        dbg.print (DBG_G_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
                "Can NOT rename regisers for ins", ins->getInsID (), _clk->now ());
        s_cant_rename_cnt++;
        return false; /*-- STALL FETCH --*/
    }
    dbg.print (DBG_G_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
            "Can rename regisers for ins", ins->getInsID (), _clk->now ());
    s_can_rename_cnt++;
    return true;
}

void bb_grfManager::renameRegs (bbInstruction* ins) {
    dbg.print (DBG_G_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
            "Rename regisers for ins", ins->getInsID (), _clk->now ());
    List<AR>* ar_rd = ins->getARrdList ();
    List<AR>* ar_wr = ins->getARwrList ();
    WIDTH grf_segmnt_indx = ins->getBBWinID ();
#ifdef ASSERTION
    Assert (_GRF.getNumAvailablePR (grf_segmnt_indx) >= ar_wr->NumElements ());
#endif

    /*-- RENAME READ REFISTERS FIRST --*/
    for (int i = 0; i < ar_rd->NumElements (); i++) {
        AR a_reg = ar_rd->Nth (i);
        PR p_reg = _GRF.renameReg (a_reg);
        ins->setPR (p_reg, READ);
        _e_rat.ramAccess ();
    }

    /*-- RENAME WRITE REFISTERS SECOND --*/
    for (int i = 0; i < ar_wr->NumElements (); i++) {
        AR a_reg = ar_wr->Nth (i);
        PR prev_pr = _GRF.renameReg (a_reg);
        _e_rat.ramAccess ();
        PR new_pr = _GRF.getAvailablePR (grf_segmnt_indx);
        _e_apr.ramAccess ();
        _GRF.update_fRAT (a_reg, new_pr);
        _e_rat.ramAccess ();
        _GRF.updatePR (new_pr, prev_pr, RENAMED_INVALID);
        _e_arst.ramAccess ();
        ins->setPR (new_pr, WRITE);
    }

    /*-- WIRE ENERGY HANDLING --*/
    list<string> wires;
    WIDTH num_elements = 1; //opcode
    num_elements += (ins->getNumRdAR () + ins->getNumRdLAR () + ins->getNumWrAR () + ins->getNumWrLAR ());
    for (int i = 0; i < num_elements; i++) {
        wires.push_back ("e_w_cache2rr");
    }
    _e_w_rr.wireAccess (wires);
}

/*-- PROCESS WRITE REGISTERS @COMPLETE --*/
void bb_grfManager::completeRegs (bbInstruction* ins) {
    dbg.print (DBG_G_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
            "Write regisers for ins", ins->getInsID (), _clk->now ());
    List<PR>* _pr = ins->getPRwrList ();
    for (int i = 0; i < _pr->NumElements (); i++) {
        PR p_reg = _pr->Nth (i);
        _GRF.updatePRstate (p_reg, RENAMED_VALID);
        _e_rf.ramAccess ();
    }
}

/*-- PROCESS WRITE REFISTERS @COMMIT --*/
void bb_grfManager::commitRegs (bbInstruction* ins) {
    dbg.print (DBG_G_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
            "Commit regisers for ins", ins->getInsID (), _clk->now ());
    List<PR>* _pr = ins->getPRwrList ();
    List<PR>* _ar = ins->getARwrList ();
#ifdef ASSERTION
    Assert (_ar->NumElements () == _pr->NumElements ());
#endif
    for (int i = 0; i < _pr->NumElements (); i++) {
        PR p_reg = _pr->Nth (i);
        AR a_reg = _ar->Nth (i);
        PR prev_pr = _GRF.getPrevPR (p_reg);
        _e_arst.ramAccess ();
        _GRF.updatePRstate (p_reg, ARCH_REG);
        _GRF.updatePRstate (prev_pr, AVAILABLE);
        _GRF.update_cRAT (a_reg,p_reg);
        _e_rat.ramAccess ();
        _GRF.setAsAvailablePR (prev_pr);
        _e_apr.ramAccess ();
    }
}

void bb_grfManager::squashRenameReg () {
    dbg.print (DBG_TEST, "** %s: %s (cyc:)\n", _c_name.c_str (), "in squash for RR");
    _GRF.squashRenameReg ();
    /* TODO model the ARST and APR table energies for squash */
}

bool bb_grfManager::hasFreeWire (AXES_TYPE axes_type, WIDTH numRegWires) {
    if (_GRF.getNumFreeWires (axes_type) >= numRegWires) {
        return true;
    } else {
        s_unavailable_cnt++;
        return false;
    }
}

void bb_grfManager::updateWireState (AXES_TYPE axes_type, WIDTH numRegWires, list<string> wire_name, bool update_wire) {
    for (WIDTH i = 0; i < numRegWires; i++) {
        _GRF.updateWireState (axes_type, wire_name, update_wire);
    }
}

void bb_grfManager::getStat () {
    _GRF.getStat ();
}
