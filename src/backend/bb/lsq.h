/*******************************************************************************
 * lsq.h
 * -----
 * LOAD-STORE QUEUEE UNIT DATA STRUCTURES
 ******************************************************************************/

#ifndef _BB_LSQ_H
#define _BB_LSQ_H 

#include "../unit/table.h"
#include "../../global/g_variable.h"

#define BB_SQ_SIZE 136
#define BB_LQ_SIZE 164
#define BB_NO_WAW_ST_INS 0

class bb_lsqCAM : public CAMtable<bbInstruction*> {
	public:
		bb_lsqCAM (LENGTH len, 
                   WIDTH rd_port_cnt, 
                   WIDTH wr_port_cnt,
                   sysClock* clk,
                   string table_name);
		~bb_lsqCAM ();

        bbInstruction* findPendingMemIns (LSQ_ID);
        void setTimer (bbInstruction*, CYCLE);
        void squash (INS_ID);
        void delFinishedMemAxes ();
        bool hasCommit ();
        bool hasMemAddr (ADDRS, INS_ID);
        INS_ID hasAnyCompleteStFromAddr (ADDRS, INS_ID);
        pair<bool, bbInstruction*> hasAnyCompleteLdFromAddr (ADDRS, INS_ID, INS_ID);
        pair<bool, bbInstruction*> hasFinishedIns (LSQ_ID);
        void dump ();
};

#endif
