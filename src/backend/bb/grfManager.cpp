/*******************************************************************************
 * grfManager.cpp
 ******************************************************************************/

#include "grfManager.h"

bb_grfManager::bb_grfManager (sysClock* clk, string rf_name)
    : unit (rf_name, clk),
      _GRF (clk, "GlobalRegisterRename"),
      _e_table (rf_name, g_cfg->_root["cpu"]["backend"]["rf"]["reg_ren"]),
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
            return false; /*-- operand not available --*/
        } else {
            p_rdReg_list->RemoveAt (i); /*--optimization --*/
        }
    }

    if (p_rdReg_list->NumElements () == 0) {
        dbg.print (DBG_G_REG_FILES, "%s: %s %d %s (cyc: %d)\n", _c_name.c_str (), 
                "Global operand of ins", ins->getInsID (), "are ready", _clk->now ());
        _e_table.ramAccess (ins->getTotNumRdAR ());
        return true; /*-- all operands available --*/
    }

    dbg.print (DBG_G_REG_FILES, "%s: %s %d s (cyc: %d)\n", _c_name.c_str (), 
            "Global operand of ins", ins->getInsID (), "are ready", _clk->now ());
    return false; /*-- not all operands available --*/
}

/*-- CHECK IS NO OTHER OBJ IS WRITING INTO WRITE REGS --*/
bool bb_grfManager::canRename (bbInstruction* ins) {
    List<AR>* ar_wr = ins->getARwrList ();
    if (_GRF.getNumAvailablePR () < ar_wr->NumElements ()) {
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
    Assert (_GRF.getNumAvailablePR () >= ar_wr->NumElements ());

    /*-- RENAME READ REFISTERS FIRST --*/
    for (int i = 0; i < ar_rd->NumElements (); i++) {
        AR a_reg = ar_rd->Nth (i);
        PR p_reg = _GRF.renameReg (a_reg);
        ins->setPR (p_reg, READ);
        _e_table.ramAccess ();
    }

    /*-- RENAME WRITE REFISTERS SECOND --*/
    for (int i = 0; i < ar_wr->NumElements (); i++) {
        Assert (_GRF.isAnyPRavailable () == true && "A physical reg must have been available.");
        AR a_reg = ar_wr->Nth (i);
        PR prev_pr = _GRF.renameReg (a_reg);
        _e_table.ramAccess ();
        PR new_pr = _GRF.getAvailablePR ();
        _GRF.update_fRAT (a_reg, new_pr);
        _e_table.ramAccess ();
        _GRF.updatePR (new_pr, prev_pr, RENAMED_INVALID);
        _e_table.ramAccess (2); /* 2 ACCESSS */
        ins->setPR (new_pr, WRITE);
    }
}

/*-- PROCESS WRITE REGISTERS @COMPLETE --*/
void bb_grfManager::completeRegs (bbInstruction* ins) {
    dbg.print (DBG_G_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
            "Write regisers for ins", ins->getInsID (), _clk->now ());
    List<PR>* _pr = ins->getPRwrList ();
    for (int i = 0; i < _pr->NumElements (); i++) {
        PR p_reg = _pr->Nth (i);
        _GRF.updatePRstate (p_reg, RENAMED_VALID);
        _e_table.ramAccess ();
    }
}

/*-- PROCESS WRITE REFISTERS @COMMIT --*/
void bb_grfManager::commitRegs (bbInstruction* ins) {
    dbg.print (DBG_G_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
            "Commit regisers for ins", ins->getInsID (), _clk->now ());
    List<PR>* _pr = ins->getPRwrList ();
    List<PR>* _ar = ins->getARwrList ();
    Assert (_ar->NumElements () == _pr->NumElements ());
    for (int i = 0; i < _pr->NumElements (); i++) {
        PR p_reg = _pr->Nth (i);
        AR a_reg = _ar->Nth (i);
        PR prev_pr = _GRF.getPrevPR (p_reg);
        _GRF.updatePRstate (p_reg, ARCH_REG);
        _e_table.ramAccess ();
        _GRF.updatePRstate (prev_pr, AVAILABLE);
        _e_table.ramAccess ();
        _GRF.update_cRAT (a_reg,p_reg);
        _e_table.ramAccess ();
        _GRF.setAsAvailablePR (prev_pr);
    }
}

void bb_grfManager::squashRenameReg () {
    dbg.print (DBG_TEST, "** %s: %s (cyc:)\n", _c_name.c_str (), "in squash for RR");
    _GRF.squashRenameReg ();
}

bool bb_grfManager::hasFreeWire (AXES_TYPE axes_type, WIDTH numRegWires) {
    if (_GRF.getNumFreeWires (axes_type) >= numRegWires) {
        return true;
    } else {
        s_unavailable_cnt++;
        return false;
    }
}

void bb_grfManager::updateWireState (AXES_TYPE axes_type, WIDTH numRegWires) {
    for (WIDTH i = 0; i < numRegWires; i++) {
        _GRF.updateWireState (axes_type);
    }
}
