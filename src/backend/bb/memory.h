/*******************************************************************************
 * memory.h
 ******************************************************************************/

#ifndef _BB_MEMORY_H
#define _BB_MEMORY_H

#include "../unit/stage.h"
#include "rfManager.h"
#include "memManager.h"
#include "bbWindow.h"

class bb_memory : protected stage {
	public:
		bb_memory (port<bbInstruction*>& execution_to_memory_port, 
                   port<bbInstruction*>& memory_to_scheduler_port, 
                   List<bbWindow*>* bbWindows,
                   WIDTH num_bbWin,
                   CAMtable<dynBasicblock*>* bbROB,
			       WIDTH memory_width,
                   bb_memManager* LSQ_MGR,
                   bb_rfManager* RF_MGR,
                   sysClock* clk,
			       string stage_name);
		~bb_memory ();
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
		port<bbInstruction*>* _execution_to_memory_port;
        port<bbInstruction*>* _memory_to_scheduler_port;
        CAMtable<dynBasicblock*>* _bbROB;
        bb_memManager* _LSQ_MGR;
        bb_rfManager* _RF_MGR;
        RAMtable<bbInstruction*> _mshr;

        cache _L1;
        cache _L2;
        cache _L3;

        // BB WIN STRUCTURES
        WIDTH _num_bbWin;
        List<bbWindow*>* _bbWindows;
};

#endif
