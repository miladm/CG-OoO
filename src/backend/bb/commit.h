/*******************************************************************************
 * commit.h
 ******************************************************************************/

#ifndef _BB_COMMIT_H
#define _BB_COMMIT_H

#include "../unit/stage.h"
#include "grfManager.h"
#include "memManager.h"
#include "bbWindow.h"

class bb_commit : protected stage {
	public:
		bb_commit (port<bbInstruction*>& commit_to_bp_port, 
			       port<bbInstruction*>& commit_to_scheduler_port, 
                   List<bbWindow*>* bbWindows,
                   WIDTH num_bbWin,
                   CAMtable<dynBasicblock*>* bbROB,
			       WIDTH commit_width,
                   bb_memManager* LSQ_MGR,
                   bb_grfManager* RF_MGR,
                   sysClock* clk,
			       string stage_name);
		~bb_commit ();
		void doCOMMIT ();
        void squash ();

    private:
		PIPE_ACTIVITY commitImpl ();
        void regStat ();
        void bpMispredSquash ();
        void memMispredSquash ();
        void commitBB (dynBasicblock*);
        void delBB (dynBasicblock*);
        void delIns (bbInstruction*);

	private:
		port<bbInstruction*>* _commit_to_bp_port;
		port<bbInstruction*>* _commit_to_scheduler_port;
        CAMtable<dynBasicblock*>* _bbROB;
        bb_memManager* _LSQ_MGR;
        bb_grfManager* _RF_MGR;

        /* STAT VARS */
        ScalarStat& s_squash_bb_cnt;

        // BB WIN STRUCTURES
        WIDTH _num_bbWin;
        List<bbWindow*>* _bbWindows;
};

#endif
