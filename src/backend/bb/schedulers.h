/*******************************************************************************
 * scheduler.h
 ******************************************************************************/

#ifndef _BB_SCHEDULER_H
#define _BB_SCHEDULER_H

#include "../unit/stage.h"
#include "grfManager.h"
#include "memManager.h"

struct bbWindow {
    bbWindow (string bbWin_id, sysClock* clk)
        : _win (8, 8, 8, clk, "bbWindow_" + bbWin_id),
          _LRF_MGR (clk, "lrfManager_" + bbWin_id),
          _id (atoi (bbWin_id.c_str ()))
    { }
    FIFOtable<dynInstruction*> _win;
    bb_grfManager _LRF_MGR; //TODO turn it into a real LRF
    const WIDTH _id;
};

class bb_scheduler : protected stage {
	public:
		bb_scheduler (port<dynInstruction*>& decode_to_scheduler_port,
                   port<dynInstruction*>& execution_to_scheduler_port,
                   port<dynInstruction*>& memory_to_scheduler_port,
			       port<dynInstruction*>& scheduler_to_execution_port,
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
        void forwardFromCDB (dynInstruction* ins);
        void regStat ();
        bool hasReadyInsInBBWins (LENGTH &readyInsIndx);
        void updateBBROB (dynBasicblock*);
        void setBBWisAvail (WIDTH bbWin_id);
        bbWindow* getAnAvailBBWin ();
        bool hasAnAvailBBWin ();
        bool detectNewBB (dynInstruction* ins);

	private:
		port<dynInstruction*>* _decode_to_scheduler_port;
		port<dynInstruction*>* _execution_to_scheduler_port;
		port<dynInstruction*>* _memory_to_scheduler_port;
		port<dynInstruction*>* _scheduler_to_execution_port;
        CAMtable<dynBasicblock*>* _bbROB;
        bb_memManager* _LSQ_MGR;
        bb_grfManager* _GRF_MGR;
        bbWindow* _bbWin_on_fetch;
        map<WIDTH, bbWindow*> _busy_bbWin;
        List<bbWindow*> _avail_bbWin;
        List<bbWindow*> _bbWindows;
        WIDTH _num_bbWin;
};

#endif
