/*******************************************************************************
 * scheduler.h
 ******************************************************************************/

#ifndef _BB_SCHEDULER_H
#define _BB_SCHEDULER_H

#include "../unit/stage.h"
#include "rfManager.h"
#include "memManager.h"
#include "bbWindow.h"

class bb_scheduler : protected stage {
	public:
		bb_scheduler (port<bbInstruction*>& decode_to_scheduler_port,
                      port<bbInstruction*>& execution_to_scheduler_port,
                      port<bbInstruction*>& memory_to_scheduler_port,
			          List<port<bbInstruction*>*>* scheduler_to_execution_port,
                      List<bbWindow*>* bbWindows,
                      WIDTH num_bbWin,
                      CAMtable<dynBasicblock*>* bbROB,
			          WIDTH scheduler_width,
                      bb_memManager* LSQ_MGR,
                      bb_rfManager* RF_MGR,
                      sysClock* clk,
			          string stage_name);
		~bb_scheduler ();
		void doSCHEDULER ();

    private:
        void squash ();
        PIPE_ACTIVITY schedulerImpl ();
        void updatebbWindows ();
        void manageCDB ();
        void manageBusyBBWin (bbWindow*);
        void forwardFromCDB (bbInstruction* ins);
        void regStat ();
        bool hasReadyInsInBBWins (LENGTH&);
        void updateBBROB (dynBasicblock*);
        void setBBWisAvail (WIDTH bbWin_id);
        bbWindow* getAnAvailBBWin ();
        bool hasAnAvailBBWin ();
        bool detectNewBB (bbInstruction*);
        void flushBBWindow (bbWindow*);
        WIDTH getIssuePortIndx (WIDTH);
        bool runaheadPermit (bbInstruction*);

	private:
		port<bbInstruction*>* _decode_to_scheduler_port;
		port<bbInstruction*>* _execution_to_scheduler_port;
		port<bbInstruction*>* _memory_to_scheduler_port;
		List<port<bbInstruction*>*>* _scheduler_to_execution_port;
        CAMtable<dynBasicblock*>* _bbROB;

        /*-- RF REGISTERS --*/
        bb_memManager* _LSQ_MGR;
        bb_rfManager* _RF_MGR;

        /*-- BB WIN STRUCTURES --*/
        WIDTH _num_bbWin;
        bbWindow* _bbWin_on_fetch;
        List<bbWindow*>* _bbWindows;
        List<bbWindow*> _avail_bbWin;
        map<WIDTH, bbWindow*> _busy_bbWin;

        /*-- RUNAHEAD MODE PARAMS --*/
        WIDTH _runahead_issue_cnt;
        bool _runahead_issue_en;

        WIDTH _blk_cluster_siz;

        /*-- STAT --*/
        ScalarStat& s_mem_g_fwd_cnt;
        ScalarStat& s_alu_g_fwd_cnt;
        ScalarStat& s_mem_l_fwd_cnt;
        ScalarStat& s_alu_l_fwd_cnt;
        ScalarStat& s_no_ld_bypass;
        RatioStat& s_bbWin_inflight_rat;
};

#endif
