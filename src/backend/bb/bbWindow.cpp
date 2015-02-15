/*******************************************************************************
 * bbWindow.cpp
 ******************************************************************************/

#include "bbWindow.h"

bbWindow::bbWindow (string bbWin_id, sysClock* clk)
    : unit ("bbWindow_" + bbWin_id, clk),
      _win (clk, g_cfg->_root["cpu"]["backend"]["table"]["bbWindow"], "bbWindow_" + bbWin_id), //TODO configure and more real numbers
      _id (atoi (bbWin_id.c_str ())),
      s_stall_war_hazard_cnt (g_stats.newScalarStat ("bbWindow_" + bbWin_id, "stall_war_hazard_cnt", "Number of times instruction is not issued due to WAR hazard in LRF", 0, PRINT_ZERO)),
      s_stall_war_hazard_rat (g_stats.newRatioStat (clk->getStatObj (), "bbWindow_" + bbWin_id, "stall_war_hazard_rat", "Ratio of instruction is not issued due to WAR hazard in LRF / cycle", 0, PRINT_ZERO))
{ 
    int temp;
    g_cfg->_root["cpu"]["backend"]["table"]["bbWindow"]["rd_wire_cnt"] >> _rd_wire_cnt;
    g_cfg->_root["cpu"]["backend"]["table"]["bbWindow"]["runahead_issue_en"] >> temp;
    _runahead_issue_en = (bool)temp;
    _issue_cycle = START_CYCLE;
    _num_issues = 0;
    _bypass_cycle = START_CYCLE;
    _st_bypassed = false;
    _issue_indx_cycle = START_CYCLE;
    _issue_indx = 0;
    _stall_cycle = START_CYCLE;
    _stallRdRegSet.clear ();
}

void bbWindow::regStat () { _win.regStat (); }

/*-- 
 * GET ISSUE INDEX OF THE BBWINDOW FOR CASES WHERE RUNAHEAD ISSUE IS PERMITTED.
 * WHEN RUNAHEAD EXECUTION IS NOT PERMITTED, THIS INDEX IS EXPECTED TO REMAIN
 * AT 0
 --*/
void bbWindow::issueIndxInc () {
#ifdef ASSERTION
    Assert (_runahead_issue_en == true);
#endif
    _issue_indx++;
#ifdef ASSERTION
    Assert (_issue_indx >= 0 && _issue_indx <= _rd_wire_cnt);
#endif
}

LENGTH bbWindow::getIssueIndx () {
    resetIssueIndxState ();
#ifdef ASSERTION
    if (!_runahead_issue_en && _issue_indx > 0) 
        Assert (false && "Invalid non-runahead mode BB index state");
#endif
    return _issue_indx;
}

void bbWindow::resetIssueIndxState () {
    CYCLE now = _clk->now ();
    if (_issue_indx_cycle < now) {
        _issue_indx = 0;
        _issue_indx_cycle = now;
    }
}

/*--
 * NUMBER OF INSTRUCTIONS ISSUED ON EVERY CYCLE. THE STATE OF THIS COUNTER IS
 * UPDATED EVERY CYCLE
 --*/
void bbWindow::issueInc () {
    _num_issues++;
#ifdef ASSERTION
    Assert (_num_issues >= 0 && _num_issues <= _rd_wire_cnt);
#endif
}

WIDTH bbWindow::getNumIssued () {
    resetIssueState ();
    return _num_issues;
}

void bbWindow::resetIssueState () {
    CYCLE now = _clk->now ();
    if (_issue_cycle < now) {
        _num_issues = 0;
        _issue_cycle = now;
    }
}

/*--
 * THESE MODULES AVOID RAW HAZARDS IN MEM UNITS - TO AVOID CONSISTENT MEMORY
 * MIS-SPECULATION EVENTS HAPPENING BECAUSE A BB CONSISTENTLY MIS-SPECULATES
 * AND SQUASHES AVOIDING THE PROGRAM TO MAKE FORWARD PROGRESS 
 --*/
void bbWindow::setStoreBypassed () { _st_bypassed = true; }

bool bbWindow::isStoreBypassed () {
    resetBypassState ();
    return _st_bypassed;
}

void bbWindow::resetBypassState () {
    CYCLE now = _clk->now ();
    if (_bypass_cycle < now) {
        _st_bypassed = false;
        _bypass_cycle = now;
    }
}

/*--
 * KEEP TRACK OF LOCAL READ REGISTERS OF STALLING OPERATIONS EVERY CYCLE TO
 * AVOID WAR HAZARDS
 --*/
void bbWindow::recordStallRdReg (bbInstruction* ins) {
    List<AR>* rd_regs = ins->getLARrdList ();
    for (int i = 0; i < ins->getNumRdLAR (); i++) { 
        AR loc_reg = rd_regs->Nth (i);
        _stallRdRegSet.insert (loc_reg);
    }
}

bool bbWindow::conflictStallRdReg (bbInstruction* ins) {
    resetStallState ();
    List<AR>* wr_regs = ins->getLARwrList ();
    for (int i = 0; i < ins->getNumWrLAR (); i++) { 
        AR loc_reg = wr_regs->Nth (i);
        if (_stallRdRegSet.find (loc_reg) != _stallRdRegSet.end ()) {
            s_stall_war_hazard_cnt++;
            s_stall_war_hazard_rat++;
            return true;
        }
    }
    return false; /* NO CONCLICTING READ REG */
}

void bbWindow::resetStallState () {
    CYCLE now = _clk->now ();
    if (_stall_cycle < now) {
        _stallRdRegSet.clear ();
        _stall_cycle = now;
    }
}

/* deploy the code for rfManager
 * remove the bbWindow creation from sysCore back to scheduler
 * add an inherited class for bbInstruction for BB extra functionalities and update the frontend
 * integrate the functionality of RF_MGR in the core
 * TODO: delte this text
 */
