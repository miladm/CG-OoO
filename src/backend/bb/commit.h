/*******************************************************************************
 * commit.h
 ******************************************************************************/

#ifndef _BB_COMMIT_H
#define _BB_COMMIT_H

#include "../unit/stage.h"
#include "rfManager.h"
#include "memManager.h"
#include "bbWindow.h"

class bb_commit : protected stage {
	public:
		bb_commit (port<bbInstruction*>& commit_to_bp_port, 
			       port<bbInstruction*>& commit_to_scheduler_port, 
                   List<bbWindow*>* bbWindows,
                   WIDTH num_bbWin,
                   CAMtable<dynBasicblock*>* bbROB,
                   CAMtable<dynBasicblock*>* bbQUE,
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
        void commitBB (dynBasicblock*);
        void delBB (dynBasicblock*);
        void delIns (bbInstruction*);
        void verifySim ();
        void debugDump ();

	private:
		port<bbInstruction*>* _commit_to_bp_port;
		port<bbInstruction*>* _commit_to_scheduler_port;
        CAMtable<dynBasicblock*>* _bbROB;
        CAMtable<dynBasicblock*>* _bbQUE;
        bb_memManager* _LSQ_MGR;
        bb_rfManager* _RF_MGR;

        /*-- SIMULATION STALL VERIFICATION --*/
        SCALAR _prev_ins_cnt;
        CYCLE _prev_commit_cyc;

        /*-- BB WIN STRUCTURES --*/
        WIDTH _num_bbWin;
        List<bbWindow*>* _bbWindows;

        /*-- STAT VARS --*/
        ScalarStat& s_squash_ins_cnt;
        ScalarStat& s_squash_br_cnt;
        ScalarStat& s_squash_mem_cnt;
        ScalarStat& s_br_squash_bb_cnt;
        ScalarStat& s_mem_squash_bb_cnt;
        ScalarStat& s_num_waste_ins;
        ScalarStat& s_bb_cnt;
        RatioStat& s_bb_size_avg;
        ScalarStat& s_wp_bb_cnt;
        ScalarStat& s_wp_ins_cnt;
        ScalarHistStat& s_ins_type_hist;
        ScalarHistStat& s_mem_type_hist;
};

#endif
