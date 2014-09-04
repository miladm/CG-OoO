/*******************************************************************************
 * memory.h
 ******************************************************************************/

#ifndef _O3_MEMORY_H
#define _O3_MEMORY_H

#include "../unit/stage.h"
#include "rfManager.h"
#include "memManager.h"

class o3_memory : protected stage {
	public:
		o3_memory (port<dynInstruction*>& execution_to_memory_port, 
                   port<dynInstruction*>& memory_to_scheduler_port, 
                   CAMtable<dynInstruction*>* iROB,
			       WIDTH memory_width,
                   o3_memManager* LSQ_MGR,
                   o3_rfManager* RF_MGR,
                   sysClock* clk,
			       string stage_name);
		~o3_memory ();
		void doMEMORY ();

    private:
        PIPE_ACTIVITY memoryImpl ();
        void completeIns ();
        void squash ();
        void regStat ();
        void manageSTbuffer ();
        void manageMEMbuffer ();
        void manageMSHR ();

	private:
		port<dynInstruction*>* _execution_to_memory_port;
        port<dynInstruction*>* _memory_to_scheduler_port;
        CAMtable<dynInstruction*>* _iROB;
        o3_memManager* _LSQ_MGR;
        o3_rfManager* _RF_MGR;
        RAMtable<dynInstruction*> _mshr;

        cache _L1;
        cache _L2;
        cache _L3;
};

#endif
