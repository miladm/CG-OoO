/*******************************************************************************
 * execution.h
 ******************************************************************************/

#ifndef _O3_EXECUTION_H
#define _O3_EXECUTION_H

#include "../unit/stage.h"
#include "rfManager.h"
#include "memManager.h"

//typedef enum {COMPLETE_NORMAL, COMPLETE_SQUASH} COMPLETE_STATUS;

class o3_execution : protected stage {
	public:
		o3_execution (port<dynInstruction*>& scheduler_to_execution_port, 
                      port<dynInstruction*>& execution_to_scheduler_port, 
                      CAMtable<dynInstruction*>* iROB,
			          WIDTH execution_width,
                      o3_memManager* LSQ_MGR,
                      o3_rfManager* RF_MGR,
                      sysClock* clk,
			          string stage_name);
		~o3_execution ();
		void doEXECUTION ();

    private:
        void squashCtrl ();
        void squash ();
        void regStat ();
        PIPE_ACTIVITY executionImpl ();
        COMPLETE_STATUS completeIns ();
        void resetRF ();
        void forward (dynInstruction*, CYCLE);

	private:
		port<dynInstruction*>* _scheduler_to_execution_port;
        port<dynInstruction*>* _execution_to_scheduler_port;
        CAMtable<dynInstruction*>* _iROB;
        o3_memManager* _LSQ_MGR;
        o3_rfManager* _RF_MGR;
        List<exeUnit*>* _aluExeUnits;
        exeUnitLat _eu_lat;

        /*-- STATS --*/
        ScalarHistStat& s_squash_state_hist;
        ScalarStat& s_br_mispred_cnt;
        ScalarStat& s_mem_mispred_cnt;
};

#endif
