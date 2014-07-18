/*******************************************************************************
 * scheduler.h
 ******************************************************************************/

#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "../unit/stage.h"
#include "rfManager.h"

class scheduler : protected stage {
	public:
		scheduler (port<dynInstruction*>& decode_to_scheduler_port, 
                   port<dynInstruction*>& execution_to_scheduler_port, 
                   port<dynInstruction*>& memory_to_scheduler_port, 
			       port<dynInstruction*>& scheduler_to_execution_port, 
                   CAMtable<dynInstruction*>* iROB,
			       WIDTH scheduler_width,
			       string stage_name);
		~scheduler ();
		void doSCHEDULER (sysClock& clk);
        void squash (sysClock& clk);
        PIPE_ACTIVITY schedulerImpl (sysClock& clk);
        void updateInsWin (sysClock& clk);
        void manageCDB (sysClock& clk);
        void forwardFromCDB (dynInstruction* ins, sysClock& clk);
        void regStat (sysClock& clk);

	private:
		port<dynInstruction*>* _decode_to_scheduler_port;
		port<dynInstruction*>* _execution_to_scheduler_port;
		port<dynInstruction*>* _memory_to_scheduler_port;
		port<dynInstruction*>* _scheduler_to_execution_port;
        CAMtable<dynInstruction*>* _iROB;
        FIFOtable<dynInstruction*> _iWindow;
};

#endif
