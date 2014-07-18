/*******************************************************************************
 * execution.h
 ******************************************************************************/

#ifndef _EXECUTION_H
#define _EXECUTION_H

#include "../unit/stage.h"
#include "rfManager.h"

class execution : protected stage {
	public:
		execution (port<dynInstruction*>& scheduler_to_execution_port, 
                   port<dynInstruction*>& execution_to_scheduler_port, 
                   port<dynInstruction*>& execution_to_memory_port, 
                   CAMtable<dynInstruction*>* iROB,
			       WIDTH execution_width,
			       string stage_name);
		~execution ();

		void doEXECUTION (sysClock&);
        void squashCtrl (sysClock&);
        void squash (sysClock&);
        void regStat (sysClock& clk);
        PIPE_ACTIVITY executionImpl (sysClock&);
        COMPLETE_STATUS completeIns (sysClock&);
        void forward (dynInstruction*, CYCLE, sysClock&);

	private:
		port<dynInstruction*>* _scheduler_to_execution_port;
        port<dynInstruction*>* _execution_to_scheduler_port;
        port<dynInstruction*>* _execution_to_memory_port;
        CAMtable<dynInstruction*>* _iROB;
        List<exeUnit*>* _aluExeUnits;
        exeUnitLat _eu_lat;
};

#endif
