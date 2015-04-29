/*******************************************************************************
 * rfManager.h
 ******************************************************************************/

#include "rfManager.h"

bb_rfManager::bb_rfManager (WIDTH num_bbWin, sysClock* clk, const YAML::Node& root, string rfm_name)
    : unit (rfm_name, clk),
      _GRF_MGR (_clk, num_bbWin, root, "grfManager"),
      s_rf_not_ready_cnt (g_stats.newScalarStat (rfm_name, "rf_not_ready_cnt", "Number of RF operand-not-ready events", 0, PRINT_ZERO)),
      s_lrf_not_ready_cnt (g_stats.newScalarStat (rfm_name, "lrf_not_ready_cnt", "Number of LRF read operand-not-ready events", 0, PRINT_ZERO)),
      s_grf_not_ready_cnt (g_stats.newScalarStat (rfm_name, "grf_not_ready_cnt", "Number of GRF read operand-not-ready events", 0, PRINT_ZERO)),
      s_lrf_busy_cnt (g_stats.newScalarStat (rfm_name, "lrf_busy_cnt", "Number of LRF write operand-not-ready events", 0, PRINT_ZERO))
{ 
    for (int i = 0; i < num_bbWin; i++) {
        ostringstream bbWin_id;
        bbWin_id << i;
        bb_lrfManager* LRF_MGR = new bb_lrfManager (i, _clk, root["lrf"], "lrfManager_" + bbWin_id.str ());
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

bool bb_rfManager::canRename (bbInstruction* ins, BB_ID bbWin_id) {
    return _GRF_MGR.canRename (ins, bbWin_id);
}

void bb_rfManager::renameRegs (bbInstruction* ins) {
    _GRF_MGR.renameRegs (ins);
}
void bb_rfManager::commitRegs (bbInstruction* ins) {
    return _GRF_MGR.commitRegs (ins);
}

void bb_rfManager::reserveRF (bbInstruction* ins) {
    if (g_cfg->getRegAllocMode () != LOCAL_GLOBAL) return; /*-- DO NOTHING --*/
    dbg.print (DBG_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
            "Reserve LRF write regs for ins", ins->getInsID (), _clk->now ());
    WIDTH bbWin_id = ins->getBBWinID ();
    return _LRF_MGRS[bbWin_id]->reserveRF (ins);
}

bool bb_rfManager::canReserveRF (bbInstruction* ins) {
    if (g_cfg->getRegAllocMode () != LOCAL_GLOBAL) return true;
    dbg.print (DBG_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
            "Check if can reserve LRF write regs for ins", ins->getInsID (), _clk->now ());
    WIDTH bbWin_id = ins->getBBWinID ();
    bool result = _LRF_MGRS[bbWin_id]->canReserveRF (ins);
    if (!result) s_lrf_busy_cnt++;
    return result;
}

/* IN ORDER TO SUUPORT FORWARDING, WE SHOULD DO A SECOND CHECK FOR THE
 * READINESS OF THE INSTRUCTIONS FORWARDED */
bool bb_rfManager::checkReadyAgain (bbInstruction* ins) {
    bool lrf_ready = true;
    if (g_cfg->getRegAllocMode () == LOCAL_GLOBAL) {
        WIDTH bbWin_id = ins->getBBWinID ();
        lrf_ready = _LRF_MGRS[bbWin_id]->checkReadyAgain (ins);
    }
    bool grf_ready = _GRF_MGR.checkReadyAgain (ins);
    dbg.print (DBG_REG_FILES, "%s: %s %d - %s %d - %s %d (cyc: %d)\n", _c_name.c_str (), 
            "LRF read ops are ready: ", ((lrf_ready)?"YES":"NO"), 
            "GRF read ops are ready: ", ((grf_ready)?"YES":"NO"), 
            "for ins: ", ins->getInsID (), _clk->now ());

    /* ENERGY TRACKING */
    if (g_cfg->getRegAllocMode () == LOCAL_GLOBAL) {
        return (grf_ready && lrf_ready);
    } else if (g_cfg->getRegAllocMode () == GLOBAL) {
        return (grf_ready);
    }
#ifdef ASSERTION
    Assert (0 && "invalid register allocation model");
#endif
    return grf_ready; /* PLEACE HOLDER */
}

bool bb_rfManager::isReady (bbInstruction* ins) {
    bool lrf_ready = true;
    if (g_cfg->getRegAllocMode () == LOCAL_GLOBAL) {
        WIDTH bbWin_id = ins->getBBWinID ();
        lrf_ready = _LRF_MGRS[bbWin_id]->isReady (ins);
    }
    bool grf_ready = _GRF_MGR.isReady (ins);
    dbg.print (DBG_REG_FILES, "%s: %s %d - %s %d - %s %d (cyc: %d)\n", _c_name.c_str (), 
            "LRF read ops are ready: ", ((lrf_ready)?"YES":"NO"), 
            "GRF read ops are ready: ", ((grf_ready)?"YES":"NO"), 
            "for ins: ", ins->getInsID (), _clk->now ());
    if (!lrf_ready) s_lrf_not_ready_cnt++;
    if (!grf_ready) s_grf_not_ready_cnt++;
    if (!grf_ready || !lrf_ready) s_rf_not_ready_cnt++;

    /* ENERGY TRACKING */
    if (g_cfg->getRegAllocMode () == LOCAL_GLOBAL) {
        if (grf_ready && lrf_ready) {
            WIDTH bbWin_id = ins->getBBWinID ();
            _LRF_MGRS[bbWin_id]->_e_table.ramAccess (ins->getTotNumRdLAR ());
            _GRF_MGR._e_rf.ramAccess (ins->getTotNumRdAR ()); /* TODO this is convervation - not cnting FWD */
        }
        return (grf_ready && lrf_ready);
    } else if (g_cfg->getRegAllocMode () == GLOBAL) {
        if (grf_ready) {
            _GRF_MGR._e_rf.ramAccess (ins->getTotNumRdAR ());
        }
        return (grf_ready);
    }
#ifdef ASSERTION
    Assert (0 && "invalid register allocation model");
#endif
    return grf_ready; /* PLEACE HOLDER */
}

void bb_rfManager::completeRegs (bbInstruction* ins) {
    dbg.print (DBG_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
            "Complete LRF write operands for ins", ins->getInsID (), _clk->now ());
    WIDTH bbWin_id = ins->getBBWinID ();
    _GRF_MGR.completeRegs (ins);
    if (g_cfg->getRegAllocMode () == LOCAL_GLOBAL)
        _LRF_MGRS[bbWin_id]->writeToRF (ins);
    dbg.print (DBG_REG_FILES, "%s: %s (cyc: %d)\n", _c_name.c_str (), 
            "Done updating RF's", _clk->now ());
}

void bb_rfManager::squashRegs () {
    _GRF_MGR.squashRenameReg ();
    dbg.print (DBG_REG_FILES, "%s: %s (cyc: %d)\n", _c_name.c_str (), 
            "Squashed GRF", _clk->now ());

    if (g_cfg->getRegAllocMode () == LOCAL_GLOBAL) {
        for (int i = 0; i < (int)_LRF_MGRS.size (); i++) {
            _LRF_MGRS[i]->resetRF ();
            dbg.print (DBG_REG_FILES, "%s: %s %d (cyc: %d)\n", _c_name.c_str (), 
                    "Squashed LRF", i, _clk->now ());
        }
    }
}

bool bb_rfManager::hasFreeWire (AXES_TYPE axes_type, bbInstruction* ins) {
    WIDTH num_g_reg = (axes_type == READ) ? ins->getNumRdPR () : ins->getNumWrPR ();
    WIDTH num_l_reg = (axes_type == READ) ? ins->getNumRdLAR () : ins->getNumWrLAR ();
    WIDTH bbWin_id = ins->getBBWinID ();
    bool grf_has_free_wire = _GRF_MGR.hasFreeWire (axes_type, num_g_reg);
    if (g_cfg->getRegAllocMode () == LOCAL_GLOBAL) {
        bool lrf_has_free_wire = _LRF_MGRS[bbWin_id]->hasFreeWire (axes_type, num_l_reg);
        return (grf_has_free_wire && lrf_has_free_wire);
    } else if (g_cfg->getRegAllocMode () == GLOBAL) {
        return (grf_has_free_wire);
    }
#ifdef ASSERTION
    Assert (true == false && "invalid register allocation model");
#endif
    return (grf_has_free_wire); /* PLEACE HOLDER */
}

void bb_rfManager::updateWireState (AXES_TYPE axes_type, bbInstruction* ins, bool update_wire) {
    list<string> local_wires, global_wires;
    if (update_wire) {
        if (axes_type == READ) {
            local_wires.push_back ("e_w_eu2lrf");
            global_wires.push_back ("e_w_eu2grf");
        } else if (axes_type == WRITE) {
            local_wires.push_back ("e_w_eu2lrf");
            global_wires.push_back ("e_w_eu2grf");
        }
    }

    WIDTH num_g_reg = (axes_type == READ) ? ins->getNumRdPR () : ins->getNumWrPR ();
    WIDTH num_l_reg = (axes_type == READ) ? ins->getNumRdLAR () : ins->getNumWrLAR ();
    WIDTH bbWin_id = ins->getBBWinID ();
    _GRF_MGR.updateWireState (axes_type, num_g_reg, global_wires, update_wire);
    if (g_cfg->getRegAllocMode () == LOCAL_GLOBAL)
        _LRF_MGRS[bbWin_id]->updateWireState (axes_type, num_l_reg, local_wires, update_wire);
}

void bb_rfManager::getStat () {
    _GRF_MGR.getStat ();
    map<WIDTH, bb_lrfManager*>::iterator it;
    for (it = _LRF_MGRS.begin (); it != _LRF_MGRS.end (); it++) {
        bb_lrfManager* lrf_mgr = it->second;
        lrf_mgr->getStat ();
    }
}
