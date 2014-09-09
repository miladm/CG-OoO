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
                   CAMtable<dynInstruction*>* iQUE,
			       WIDTH commit_width,
                   o3_memManager* LSQ_MGR,
                   o3_rfManager* RF_MGR,
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
        void delIns (dynInstruction* ins);

	private:
		port<dynInstruction*>* _commit_to_bp_port;
		port<dynInstruction*>* _commit_to_scheduler_port;
        CAMtable<dynInstruction*>* _iROB;
        CAMtable<dynInstruction*>* _iQUE;
        o3_memManager* _LSQ_MGR;
        o3_rfManager* _RF_MGR;

        /* STAT VARS */
        ScalarStat& s_squash_ins_cnt;
        ScalarStat& s_squash_br_cnt;
        ScalarStat& s_squash_mem_cnt;
        ScalarStat& s_wp_ins_cnt;
        ScalarHistStat& s_ins_type_hist;
        ScalarHistStat& s_mem_type_hist;
};

#endif
