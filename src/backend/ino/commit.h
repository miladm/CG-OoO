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
                CAMtable<dynInstruction*>* iQUE,
			    WIDTH commit_width,
                sysClock* clk,
			    string stage_name);
		~commit ();
		void doCOMMIT ();
        void squash ();

    private:
		PIPE_ACTIVITY commitImpl ();
        void regStat ();
        void verifySim ();

	private:
		port<dynInstruction*>* _commit_to_bp_port;
		port<dynInstruction*>* _commit_to_scheduler_port;
        CAMtable<dynInstruction*>* _iROB;
        CAMtable<dynInstruction*>* _iQUE;

        /*-- SIMULATION STALL VERIFICATION --*/
        SCALAR _prev_ins_cnt;
        CYCLE _prev_commit_cyc;

        /* STAT VARS */
        ScalarStat& s_squash_ins_cnt;
        ScalarStat& s_wp_ins_cnt;
        ScalarHistStat& s_ins_type_hist;
        ScalarHistStat& s_mem_type_hist;
};

#endif
