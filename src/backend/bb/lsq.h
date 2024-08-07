/*******************************************************************************
 * lsq.h
 * -----
 * LOAD-STORE QUEUEE UNIT DATA STRUCTURES
 ******************************************************************************/

#ifndef _BB_LSQ_H
#define _BB_LSQ_H 

#include "../unit/table.h"
#include "../../global/g_variable.h"

#define BB_NO_WAW_ST_INS 0

class bb_lsqCAM : public CAMtable<bbInstruction*> {
	public:
		bb_lsqCAM ( sysClock*, const YAML::Node&, string);
		~bb_lsqCAM ();

        bbInstruction* findPendingMemIns (LSQ_ID);
        void setTimer (bbInstruction*, CYCLE);
        void squash (INS_ID);
        void delFinishedMemAxes ();
        bool hasCommit ();
        bool hasMemAddr (ADDRS, INS_ID);
        INS_ID hasAnyCompleteStFromAddr (ADDRS, INS_ID);
        pair<bool, bbInstruction*> hasAnyCompleteLdFromAddr (ADDRS, INS_ID, INS_ID, BB_ID);
        pair<bool, bbInstruction*> hasFinishedIns (LSQ_ID);
        void dump ();

    private:
        /*-- STAT --*/
        ScalarStat& s_inter_bb_mem_mis_pred_cnt;
        ScalarStat& s_intra_bb_mem_mis_pred_cnt;
};

#endif
