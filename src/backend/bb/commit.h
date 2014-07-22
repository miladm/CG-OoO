/*******************************************************************************
 * commit.h
 ******************************************************************************/

#ifndef _BB_COMMIT_H
#define _BB_COMMIT_H

#include "../unit/stage.h"
#include "rfManager.h"
#include "memManager.h"

class bb_commit : protected stage {
	public:
		bb_commit (port<dynInstruction*>& commit_to_bp_port, 
			       port<dynInstruction*>& commit_to_scheduler_port, 
                   CAMtable<dynInstruction*>* iROB,
			       WIDTH commit_width,
                   bb_memManager* LSQ_MGR,
                   bb_rfManager* RF_MGR,
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
        void delIns (dynInstruction* ins);

	private:
		port<dynInstruction*>* _commit_to_bp_port;
		port<dynInstruction*>* _commit_to_scheduler_port;
        CAMtable<dynInstruction*>* _iROB;
        bb_memManager* _LSQ_MGR;
        bb_rfManager* _RF_MGR;

        /* STAT VARS */
        ScalarStat& s_squash_ins_cnt;
};

#endif
