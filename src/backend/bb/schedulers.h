/*******************************************************************************
 * scheduler.h
 ******************************************************************************/

#ifndef _BB_SCHEDULER_H
#define _BB_SCHEDULER_H

#include "../unit/stage.h"
#include "grfManager.h"
#include "memManager.h"
#include "bbWindow.h"

class bb_scheduler : protected stage {
	public:
		bb_scheduler (port<dynInstruction*>& decode_to_scheduler_port,
                      port<dynInstruction*>& execution_to_scheduler_port,
                      port<dynInstruction*>& memory_to_scheduler_port,
			          port<dynInstruction*>& scheduler_to_execution_port,
                      List<bbWindow*>* bbWindows,
                      WIDTH num_bbWin,
                      CAMtable<dynBasicblock*>* bbROB,
			          WIDTH scheduler_width,
                      bb_memManager* LSQ_MGR,
                      bb_grfManager* RF_MGR,
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
        void forwardFromCDB (dynInstruction* ins);
        void regStat ();
        bool hasReadyInsInBBWins (LENGTH &readyInsIndx);
        void updateBBROB (dynBasicblock*);
        void setBBWisAvail (WIDTH bbWin_id);
        bbWindow* getAnAvailBBWin ();
        bool hasAnAvailBBWin ();
        bool detectNewBB (dynInstruction*);
        void flushBBWindow (bbWindow*);

	private:
		port<dynInstruction*>* _decode_to_scheduler_port;
		port<dynInstruction*>* _execution_to_scheduler_port;
		port<dynInstruction*>* _memory_to_scheduler_port;
		port<dynInstruction*>* _scheduler_to_execution_port;
        CAMtable<dynBasicblock*>* _bbROB;

        // RF REGISTERS
        bb_memManager* _LSQ_MGR;
        bb_grfManager* _GRF_MGR;

        // BB WIN STRUCTURES
        WIDTH _num_bbWin;
        bbWindow* _bbWin_on_fetch;
        List<bbWindow*>* _bbWindows;
        List<bbWindow*> _avail_bbWin;
        map<WIDTH, bbWindow*> _busy_bbWin;
};

#endif
