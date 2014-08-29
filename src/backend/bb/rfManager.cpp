/*******************************************************************************
 * rfManager.h
 ******************************************************************************/

#include "rfManager.h"

bb_rfManager::bb_rfManager (WIDTH num_bbWin, sysClock* clk, string rfm_name)
    : unit (rfm_name, clk),
      _GRF_MGR (_clk, "grfManager")
{ 
    for (int i = 0; i < num_bbWin; i++) {
        ostringstream bbWin_id;
        bbWin_id << i;
        bb_lrfManager* LRF_MGR = new bb_lrfManager (_clk, "lrfManager_" + bbWin_id.str ());
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

bool bb_rfManager::renameRegs (bbInstruction* ins) {
    return _GRF_MGR.renameRegs (ins);
}
void bb_rfManager::commitRegs (bbInstruction* ins) {
    return _GRF_MGR.commitRegs (ins);
}

void bb_rfManager::reserveRF (bbInstruction* ins) {
    WIDTH bbWin_id = ins->getBBWinID ();
    return _LRF_MGRS[bbWin_id]->reserveRF (ins);
}

bool bb_rfManager::canReserveRF (bbInstruction* ins) {
    WIDTH bbWin_id = ins->getBBWinID ();
    return _LRF_MGRS[bbWin_id]->canReserveRF (ins);
}

bool bb_rfManager::isReady (bbInstruction* ins) {
//    WIDTH bbWin_id = ins->getBBWinID ();
    bool grf_ready = _GRF_MGR.isReady (ins);
//    bool lrf_ready = _LRF_MGRS[bbWin_id]->isReady (ins);
//    return (grf_ready && lrf_ready);
    return grf_ready;
}

void bb_rfManager::completeRegs (bbInstruction* ins) {
//    WIDTH bbWin_id = ins->getBBWinID ();
    _GRF_MGR.completeRegs (ins);
//    _LRF_MGRS[bbWin_id]->writeToRF (ins);
    dbg.print (DBG_REG_FILES, "%s: %s (cyc: %d)\n", _c_name.c_str (), 
            "Done updating RF's", _clk->now ());
}

void bb_rfManager::squashRegs () {
    _GRF_MGR.squashRenameReg ();
    dbg.print (DBG_REG_FILES, "%s: %s (cyc: %d)\n", _c_name.c_str (), 
            "Squashed GRF", _clk->now ());

//    for (int i = 0; i < (int)_LRF_MGRS.size (); i++) {
//        _LRF_MGRS[i]->resetRF ();
//        dbg.print (DBG_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
//                "Squashed LRF", i, _clk->now ());
//    }
}

bool bb_rfManager::hasFreeWire (AXES_TYPE axes_type) {
//    WIDTH bbWin_id = ins->getBBWinID ();
    bool grf_has_free_wire = _GRF_MGR.hasFreeWire (axes_type);
//    bool lrf_has_free_wire = _LRF_MGRS[bbWin_id]->hasFreeWire (axes_type);
//    return (grf_has_free_wire && lrf_has_free_wire);
    return grf_has_free_wire;
}

bool bb_rfManager::hasFreeWire (AXES_TYPE axes_type, WIDTH width) {
//    WIDTH bbWin_id = ins->getBBWinID ();
    bool grf_has_free_wire = _GRF_MGR.hasFreeWire (axes_type, width);
//    bool lrf_has_free_wire = _LRF_MGRS[bbWin_id]->hasFreeWire (axes_type, width);
//    return (grf_has_free_wire && lrf_has_free_wire);
    return grf_has_free_wire;
}

void bb_rfManager::updateWireState (AXES_TYPE axes_type) {
//    WIDTH bbWin_id = ins->getBBWinID ();
    _GRF_MGR.updateWireState (axes_type);
//    _LRF_MGRS[bbWin_id]->updateWireState (axes_type);
}

void bb_rfManager::updateWireState (AXES_TYPE axes_type, WIDTH width) {
//    WIDTH bbWin_id = ins->getBBWinID ();
    _GRF_MGR.updateWireState (axes_type, width);
//    _LRF_MGRS[bbWin_id]->updateWireState (axes_type, width);
}
