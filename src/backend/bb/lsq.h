/*******************************************************************************
 * lsq.h
 * -----
 * LOAD-STORE QUEUEE UNIT DATA STRUCTURES
 ******************************************************************************/

#ifndef _BB_LSQ_H
#define _BB_LSQ_H 

#include "../unit/table.h"

#define BB_SQ_SIZE 96
#define BB_LQ_SIZE 94

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
        pair<bool, bbInstruction*> hasAnyCompleteLdFromAddr (ADDRS, INS_ID);
        pair<bool, bbInstruction*> hasFinishedIns (LSQ_ID);
};

#endif
