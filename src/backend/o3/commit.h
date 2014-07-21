/*******************************************************************************
 * commit.h
 ******************************************************************************/

#ifndef _O3_COMMIT_H
#define _O3_COMMIT_H

#include "../unit/stage.h"
#include "rfManager.h"
#include "memManager.h"

class o3_commit : protected stage {
	public:
		o3_commit (port<dynInstruction*>& commit_to_bp_port, 
			       port<dynInstruction*>& commit_to_scheduler_port, 
                   CAMtable<dynInstruction*>* iROB,
			       WIDTH commit_width,
                   sysClock* clk,
			       string stage_name);
		~o3_commit ();
		void doCOMMIT ();
        void squash ();

    private:
		PIPE_ACTIVITY commitImpl ();
        void regStat ();
        void bpMispredSquash ();
        void memMispredSquash ();

	private:
		port<dynInstruction*>* _commit_to_bp_port;
		port<dynInstruction*>* _commit_to_scheduler_port;
        CAMtable<dynInstruction*>* _iROB;

        /* STAT VARS */
        ScalarStat& s_squash_ins_cnt;
};

#endif
