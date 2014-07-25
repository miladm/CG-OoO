/*******************************************************************************
 * scheduler.h
 ******************************************************************************/

#ifndef _BB_SCHEDULER_H
#define _BB_SCHEDULER_H

#include "../unit/stage.h"
#include "rfManager.h"
#include "memManager.h"

class bb_scheduler : protected stage {
	public:
		bb_scheduler (port<dynInstruction*>& decode_to_scheduler_port,
                   port<dynInstruction*>& execution_to_scheduler_port,
                   port<dynInstruction*>& memory_to_scheduler_port,
			       port<dynInstruction*>& scheduler_to_execution_port,
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
        void forwardFromCDB (dynInstruction* ins);
        void regStat ();
        bool hasReadyInsInResStn (WIDTH resStnId, LENGTH &readyInsIndx);
        void updateBBROB (dynInstruction* ins);

	private:
		port<dynInstruction*>* _decode_to_scheduler_port;
		port<dynInstruction*>* _execution_to_scheduler_port;
		port<dynInstruction*>* _memory_to_scheduler_port;
		port<dynInstruction*>* _scheduler_to_execution_port;
        CAMtable<dynBasicblock*>* _bbROB;
        bb_memManager* _LSQ_MGR;
        bb_rfManager* _RF_MGR;
        List<FIFOtable<dynInstruction*>* > _bbWindows;
        WIDTH _num_res_stns;
};

#endif
