/*******************************************************************************
 * execution.h
 ******************************************************************************/

#ifndef _BB_EXECUTION_H
#define _BB_EXECUTION_H

#include "../unit/stage.h"
#include "rfManager.h"
#include "memManager.h"
#include "bbWindow.h"

//typedef enum {COMPLETE_NORMAL, COMPLETE_SQUASH} COMPLETE_STATUS;

class bb_execution : protected stage {
	public:
		bb_execution (List<port<bbInstruction*>*>* scheduler_to_execution_port, 
                      port<bbInstruction*>& execution_to_scheduler_port, 
                      List<bbWindow*>* bbWindows,
                      WIDTH num_bbWin,
                      CAMtable<dynBasicblock*>* bbROB,
			          WIDTH execution_width,
                      bb_memManager* LSQ_MGR,
                      bb_rfManager* RF_MGR,
                      sysClock* clk,
			          string stage_name);
		~bb_execution ();
		void doEXECUTION ();

    private:
        void squashCtrl ();
        void squash ();
        void regStat ();
        PIPE_ACTIVITY executionImpl ();
        COMPLETE_STATUS completeIns ();
        void resetRF ();
        void forward (bbInstruction*, CYCLE);

	private:
		List<port<bbInstruction*>*>* _scheduler_to_execution_port;
        port<bbInstruction*>* _execution_to_scheduler_port;
        CAMtable<dynBasicblock*>* _bbROB;
        bb_memManager* _LSQ_MGR;
        bb_rfManager* _RF_MGR;
        List<exeUnit*>* _aluExeUnits;
        exeUnitLat _eu_lat;

        /*-- BB WIN STRUCTURES --*/
        WIDTH _num_bbWin;
        List<bbWindow*>* _bbWindows;

        /*-- STATS --*/
        ScalarHistStat& s_pipe_state_hist;
        ScalarHistStat& s_eu_busy_state_hist;
        RatioHistStat& s_pipe_state_hist_rat;
        ScalarStat& s_br_mispred_cnt;
        ScalarStat& s_mem_mispred_cnt;
};

#endif
