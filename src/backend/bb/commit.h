/*******************************************************************************
 * commit.h
 ******************************************************************************/

#ifndef _bb_COMMIT_H
#define _bb_COMMIT_H

#include "../unit/stage.h"
#include "rfManager.h"
#include "memManager.h"

class bb_commit : protected stage {
	public:
		bb_commit (port<dynInstruction*>& commit_to_bp_port, 
			       port<dynInstruction*>& commit_to_scheduler_port, 
                   CAMtable<dynInstruction*>* iROB,
			       WIDTH commit_width,
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

        /* STAT VARS */
        ScalarStat& s_squash_ins_cnt;
};

#endif
