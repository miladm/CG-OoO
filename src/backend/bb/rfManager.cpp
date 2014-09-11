/*******************************************************************************
 * rfManager.h
 ******************************************************************************/

#include "rfManager.h"

bb_rfManager::bb_rfManager (WIDTH num_bbWin, sysClock* clk, string rfm_name)
    : unit (rfm_name, clk),
      _GRF_MGR (_clk, "grfManager"),
      s_rf_not_ready_cnt (g_stats.newScalarStat (rfm_name, "rf_not_ready_cnt", "Number of RF operand-not-ready events", 0, PRINT_ZERO)),
      s_lrf_not_ready_cnt (g_stats.newScalarStat (rfm_name, "lrf_not_ready_cnt", "Number of LRF read operand-not-ready events", 0, PRINT_ZERO)),
      s_grf_not_ready_cnt (g_stats.newScalarStat (rfm_name, "grf_not_ready_cnt", "Number of GRF read operand-not-ready events", 0, PRINT_ZERO)),
      s_lrf_busy_cnt (g_stats.newScalarStat (rfm_name, "lrf_busy_cnt", "Number of LRF write operand-not-ready events", 0, PRINT_ZERO))
{ 
    for (int i = 0; i < num_bbWin; i++) {
        ostringstream bbWin_id;
        bbWin_id << i;
        bb_lrfManager* LRF_MGR = new bb_lrfManager (i, _clk, "lrfManager_" + bbWin_id.str ());
        _LRF_MGRS.insert (pair<WIDTH, bb_lrfManager*>(i, LRF_MGR));
    }
}

bb_rfManager::~bb_rfManager () { 
    map<WIDTH, bb_lrfManager*>::iterator it;
    for (it = _LRF_MGRS.begin (); it != _LRF_MGRS.end (); it++) {
        bb_lrfManager* lrf_mgr = it->second;
        delete lrf_mgr;
    }
}

bool bb_rfManager::canRename (bbInstruction* ins) {
    return _GRF_MGR.canRename (ins);
}

void bb_rfManager::renameRegs (bbInstruction* ins) {
    _GRF_MGR.renameRegs (ins);
}
void bb_rfManager::commitRegs (bbInstruction* ins) {
    return _GRF_MGR.commitRegs (ins);
}

void bb_rfManager::reserveRF (bbInstruction* ins) {
    dbg.print (DBG_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
            "Reserve LRF write regs for ins", ins->getInsID (), _clk->now ());
    WIDTH bbWin_id = ins->getBBWinID ();
    return _LRF_MGRS[bbWin_id]->reserveRF (ins);
}

bool bb_rfManager::canReserveRF (bbInstruction* ins) {
    dbg.print (DBG_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
            "Check if can reserve LRF write regs for ins", ins->getInsID (), _clk->now ());
    WIDTH bbWin_id = ins->getBBWinID ();
    bool result = _LRF_MGRS[bbWin_id]->canReserveRF (ins);
    if (!result) s_lrf_busy_cnt++;
    return result;
}

bool bb_rfManager::isReady (bbInstruction* ins) {
    WIDTH bbWin_id = ins->getBBWinID ();
    bool grf_ready = _GRF_MGR.isReady (ins);
    bool lrf_ready = _LRF_MGRS[bbWin_id]->isReady (ins);
    dbg.print (DBG_REG_FILES, "%s: %s %d - %s %d - %s %d (cyc: %d)\n", _c_name.c_str (), 
            "LRF read ops are ready: ", lrf_ready?"YES":"NO", 
            "GRF read ops are ready: ", grf_ready?"YES":"NO", 
            "for ins: ", ins->getInsID (), _clk->now ());
    if (!lrf_ready) s_lrf_not_ready_cnt++;
    if (!grf_ready) s_grf_not_ready_cnt++;
    if (!grf_ready || !lrf_ready) s_rf_not_ready_cnt++;
    return (grf_ready && lrf_ready);
}

void bb_rfManager::completeRegs (bbInstruction* ins) {
    dbg.print (DBG_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
            "Complete LRF write operands for ins", ins->getInsID (), _clk->now ());
    WIDTH bbWin_id = ins->getBBWinID ();
    _GRF_MGR.completeRegs (ins);
    _LRF_MGRS[bbWin_id]->writeToRF (ins);
    dbg.print (DBG_REG_FILES, "%s: %s (cyc: %d)\n", _c_name.c_str (), 
            "Done updating RF's", _clk->now ());
}

void bb_rfManager::squashRegs () {
    _GRF_MGR.squashRenameReg ();
    dbg.print (DBG_REG_FILES, "%s: %s (cyc: %d)\n", _c_name.c_str (), 
            "Squashed GRF", _clk->now ());

    for (int i = 0; i < (int)_LRF_MGRS.size (); i++) {
        _LRF_MGRS[i]->resetRF ();
        dbg.print (DBG_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
                "Squashed LRF", i, _clk->now ());
    }
}

bool bb_rfManager::hasFreeWire (AXES_TYPE axes_type, bbInstruction* ins) {
    WIDTH num_g_reg = (axes_type == READ) ? ins->getNumRdPR () : ins->getNumWrPR ();
    WIDTH num_l_reg = (axes_type == READ) ? ins->getNumRdLAR () : ins->getNumWrLAR ();
    WIDTH bbWin_id = ins->getBBWinID ();
    bool grf_has_free_wire = _GRF_MGR.hasFreeWire (axes_type, num_g_reg);
    bool lrf_has_free_wire = _LRF_MGRS[bbWin_id]->hasFreeWire (axes_type, num_l_reg);
    return (grf_has_free_wire && lrf_has_free_wire);
}

void bb_rfManager::updateWireState (AXES_TYPE axes_type, bbInstruction* ins) {
    WIDTH num_g_reg = (axes_type == READ) ? ins->getNumRdPR () : ins->getNumWrPR ();
    WIDTH num_l_reg = (axes_type == READ) ? ins->getNumRdLAR () : ins->getNumWrLAR ();
    WIDTH bbWin_id = ins->getBBWinID ();
    _GRF_MGR.updateWireState (axes_type, num_g_reg);
    _LRF_MGRS[bbWin_id]->updateWireState (axes_type, num_l_reg);
}
