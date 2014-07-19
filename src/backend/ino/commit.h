/*******************************************************************************
 * commit.h
 ******************************************************************************/

#ifndef _COMMIT_H
#define _COMMIT_H

#include "../unit/stage.h"

class commit : protected stage {
	public:
		commit (port<dynInstruction*>& commit_to_bp_port, 
			    port<dynInstruction*>& commit_to_scheduler_port, 
                CAMtable<dynInstruction*>* iROB,
			    WIDTH commit_width,
                sysClock* clk,
			    string stage_name);
		~commit ();
		void doCOMMIT ();
        void squash ();

    private:
		PIPE_ACTIVITY commitImpl ();
        void regStat ();

	private:
		port<dynInstruction*>* _commit_to_bp_port;
		port<dynInstruction*>* _commit_to_scheduler_port;
        CAMtable<dynInstruction*>* _iROB;

        /* STAT VARS */
        ScalarStat& s_squash_ins_cnt;
};

#endif
