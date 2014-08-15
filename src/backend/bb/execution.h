/*******************************************************************************
 * execution.h
 ******************************************************************************/

#ifndef _BB_EXECUTION_H
#define _BB_EXECUTION_H

#include "../unit/stage.h"
#include "grfManager.h"
#include "memManager.h"

//typedef enum {COMPLETE_NORMAL, COMPLETE_SQUASH} COMPLETE_STATUS;

class bb_execution : protected stage {
	public:
		bb_execution (port<dynInstruction*>& scheduler_to_execution_port, 
                      port<dynInstruction*>& execution_to_scheduler_port, 
                      CAMtable<dynBasicblock*>* bbROB,
			          WIDTH execution_width,
                      bb_memManager* LSQ_MGR,
                      bb_grfManager* RF_MGR,
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
        void forward (dynInstruction*, CYCLE);

	private:
		port<dynInstruction*>* _scheduler_to_execution_port;
        port<dynInstruction*>* _execution_to_scheduler_port;
        CAMtable<dynBasicblock*>* _bbROB;
        bb_memManager* _LSQ_MGR;
        bb_grfManager* _RF_MGR;
        List<exeUnit*>* _aluExeUnits;
        exeUnitLat _eu_lat;
};

#endif
