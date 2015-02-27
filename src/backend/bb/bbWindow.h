/*******************************************************************************
 * bbWindow.h
 *******************************************************************************
 * This is the basic-block window hardware unit.
 ******************************************************************************/

#ifndef _BB_WINDOW_H
#define _BB_WINDOW_H

#include "../unit/unit.h"
#include "../unit/table.h"
#include "lrfManager.h"

class bbWindow : public unit {
    public:
        bbWindow (string bbWin_id, sysClock* clk);
        void regStat ();
        CAMtable<bbInstruction*> _win;

        void issueIndxInc ();
        LENGTH getIssueIndx ();
        void resetIssueIndxState ();

        void issueInc ();
        WIDTH getNumIssued ();
        void resetIssueState ();

        void setStoreBypassed ();
        bool isStoreBypassed ();
        void resetBypassState ();

        void recordStallRdWrReg (bbInstruction*);
        bool conflictStallRdWrReg (bbInstruction*);
        void resetStallState ();

    public:
        const WIDTH _id;

    private:
        WIDTH _rd_wire_cnt;
        bool _runahead_issue_en;

        CYCLE _issue_cycle;
        WIDTH _num_issues;

        CYCLE _bypass_cycle;
        bool _st_bypassed;

        CYCLE _issue_indx_cycle;
        LENGTH _issue_indx;

        CYCLE _stall_cycle;
        set<PR> _stallRdRegSet;
        set<PR> _stallWrRegSet;

        /* STAT */
        ScalarStat& s_stall_war_hazard_cnt;
        RatioStat& s_stall_war_hazard_rat;
        ScalarStat& s_stall_raw_hazard_cnt;
        RatioStat& s_stall_raw_hazard_rat;
};

#endif
