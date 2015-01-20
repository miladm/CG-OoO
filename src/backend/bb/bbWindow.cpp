/*******************************************************************************
 * bbWindow.cpp
 ******************************************************************************/

#include "bbWindow.h"

bbWindow::bbWindow (string bbWin_id, sysClock* clk)
    : unit ("bbWindow_" + bbWin_id, clk),
      _win (clk, g_cfg->_root["cpu"]["backend"]["table"]["bbWindow"], "bbWindow_" + bbWin_id), //TODO configure and more real numbers
      _id (atoi (bbWin_id.c_str ()))
{ 
    _issue_cycle = START_CYCLE;
    _bypass_cycle = START_CYCLE;
    _num_issues = 0;
    _st_bypassed = false;
    g_cfg->_root["cpu"]["backend"]["table"]["bbWindow"]["rd_wire_cnt"] >> _rd_wire_cnt;
}

void bbWindow::regStat () {
    _win.regStat ();
}

void bbWindow::issueInc () {
    _num_issues++;
    Assert (_num_issues >= 0 && _num_issues <= _rd_wire_cnt);
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
        _bypass_cycle = now;
        _st_bypassed = false;
    }
}

/* deploy the code for rfManager
 * remove the bbWindow creation from sysCore back to scheduler
 * add an inherited class for bbInstruction for BB extra functionalities and update the frontend
 * integrate the functionality of RF_MGR in the core
 * TODO: delte this text
 */
