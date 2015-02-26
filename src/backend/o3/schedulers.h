/*******************************************************************************
 * scheduler.h
 ******************************************************************************/

#ifndef _O3_SCHEDULER_H
#define _O3_SCHEDULER_H

#include "../unit/stage.h"
#include "rfManager.h"
#include "memManager.h"

#define MAX_INS_SEQ_LEN 200

class o3_scheduler : protected stage {
	public:
		o3_scheduler (port<dynInstruction*>& decode_to_scheduler_port, 
                   port<dynInstruction*>& execution_to_scheduler_port, 
                   port<dynInstruction*>& memory_to_scheduler_port, 
			       port<dynInstruction*>& scheduler_to_execution_port, 
                   CAMtable<dynInstruction*>* iROB,
			       WIDTH scheduler_width,
                   o3_memManager* LSQ_MGR,
                   o3_rfManager* RF_MGR,
                   sysClock* clk,
			       string stage_name);
		~o3_scheduler ();
		void doSCHEDULER ();

    private:
        void squash ();
        PIPE_ACTIVITY schedulerImpl ();
        void updateResStns ();
        void manageCDB ();
        bool forwardFromCDB (dynInstruction*);
        bool isReady (dynInstruction*);
        void regStat ();
        bool hasReadyInsInResStn (WIDTH resStnId, LENGTH &readyInsIndx);

	private:
		port<dynInstruction*>* _decode_to_scheduler_port;
		port<dynInstruction*>* _execution_to_scheduler_port;
		port<dynInstruction*>* _memory_to_scheduler_port;
		port<dynInstruction*>* _scheduler_to_execution_port;
        CAMtable<dynInstruction*>* _iROB;
        o3_memManager* _LSQ_MGR;
        o3_rfManager* _RF_MGR;
        List<CAMtable<dynInstruction*>* > _ResStns;
        WIDTH _num_res_stns;

        /*-- STAT --*/
        ScalarStat& s_mem_fwd_cnt;
        ScalarStat& s_alu_fwd_cnt;
        ScalarStat& s_rf_struct_hazrd_cnt;
        ScalarHistStat& s_ins_cluster_hist;
};

#endif
