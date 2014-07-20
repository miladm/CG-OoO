/*******************************************************************************
 * rfManager.cpp
 ******************************************************************************/

#include "rfManager.h"

o3_rfManager::o3_rfManager (sysClock* clk, string rf_name)
    : unit (rf_name, clk),
      _GRF (clk, "registerRename")
{ }

o3_rfManager::~o3_rfManager () { }

/* ARE ALL READ OPERANDS READY? */
bool o3_rfManager::isReady (dynInstruction* ins) {
    List<AR>* p_rdReg_list = ins->getPRrdList ();
    for (int i = p_rdReg_list->NumElements () - 1; i >= 0; i--) {
        AR reg = p_rdReg_list->Nth (i);
        if (!_GRF.isPRvalid (reg)) {
            return false; /* operand not available */
        } else {
            p_rdReg_list->RemoveAt (i); /*optimization */
        }
    }
    if (p_rdReg_list->NumElements () == 0) {
        return true; /* all operands available */
    }
    return false; /* not all operands available */
}

/* CHECK IS NO OTHER OBJ IS WRITING INTO WRITE REGS */
bool o3_rfManager::canRename (dynInstruction* ins) {
    List<AR>* ar_wr = ins->getARwrList ();
    if (_GRF.getNumAvailablePR () < ar_wr->NumElements ()) {
        return false; /* STALL FETCH */
    }
    return true;
}

bool o3_rfManager::renameRegs (dynInstruction* ins) {
    List<AR>* ar_rd = ins->getARrdList ();
    List<AR>* ar_wr = ins->getARwrList ();
    Assert (_GRF.getNumAvailablePR () >= ar_wr->NumElements ());

    /* RENAME READ REFISTERS FIRST */
    for (int i = 0; i < ar_rd->NumElements (); i++) {
        AR a_reg = ar_rd->Nth (i);
        PR p_reg = _GRF.renameReg (a_reg);
        ins->setPR (p_reg, READ);
    }

    /* RENAME WRITE REFISTERS SECOND */
    for (int i = 0; i < ar_wr->NumElements (); i++) {
        Assert (_GRF.isAnyPRavailable () == true && "A physical reg must have been available.");
        AR a_reg = ar_wr->Nth (i);
        PR prev_pr = _GRF.renameReg (a_reg);
        PR new_pr = _GRF.getAvailablePR ();
        _GRF.update_fRAT (a_reg, new_pr);
        _GRF.updatePR (new_pr, prev_pr, RENAMED_INVALID);
        ins->setPR (new_pr, WRITE);
    }
    return false; /* DON'T STALL FETCH */
}

/* PROCESS WRITE REGISTERS @COMPLETE */
void o3_rfManager::completeRegs (dynInstruction* ins) {
    List<PR>* _pr = ins->getPRwrList ();
    for (int i = 0; i < _pr->NumElements (); i++) {
        PR p_reg = _pr->Nth (i);
        _GRF.updatePRstate (p_reg, RENAMED_VALID);
    }
}

/* PROCESS WRITE REFISTERS @COMMIT */
void o3_rfManager::commitRegs (dynInstruction* ins) {
    List<PR>* _pr = ins->getPRwrList ();
    List<PR>* _ar = ins->getARwrList ();
    Assert (_ar->NumElements () == _pr->NumElements ());
    for (int i = 0; i < _pr->NumElements (); i++) {
        PR p_reg = _pr->Nth (i);
        AR a_reg = _ar->Nth (i);
        PR prev_pr = _GRF.getPrevPR (p_reg);
        _GRF.updatePRstate (p_reg,ARCH_REG);
        _GRF.updatePRstate (prev_pr,AVAILABLE);
        _GRF.update_cRAT (a_reg,p_reg);
        _GRF.setAsAvailablePR (prev_pr);
    }
}

void o3_rfManager::squashRenameReg () {
    dbg.print (DBG_TEST, "** %s: %s (cyc:)\n", _c_name.c_str (), "in squash for RR");
    _GRF.squashRenameReg ();
}

bool o3_rfManager::hasFreeWire (AXES_TYPE axes_type) {
    return _GRF.hasFreeWire (axes_type);
}

bool o3_rfManager::hasFreeWire (AXES_TYPE axes_type, WIDTH numRegWires) {
    if (_GRF.getNumFreeWires (axes_type) >= numRegWires)
        return true;
    else
        return false;
}

void o3_rfManager::updateWireState (AXES_TYPE axes_type) {
    _GRF.updateWireState (axes_type);
}

void o3_rfManager::updateWireState (AXES_TYPE axes_type, WIDTH numRegWires) {
    for (WIDTH i = 0; i < numRegWires; i++) {
        _GRF.updateWireState (axes_type);
    }
}

o3_rfManager* g_GRF_MGR;
