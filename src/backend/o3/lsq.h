/*******************************************************************************
 * lsq.h
 * -----
 * LOAD-STORE QUEUEE UNIT DATA STRUCTURES
 ******************************************************************************/

#ifndef _O3_LSQ_H
#define _O3_LSQ_H 

#include "../unit/table.h"

#define SQ_SIZE 36
#define LQ_SIZE 64

typedef enum {LD_QU, ST_QU} LSQ_ID;

class o3_lsqCAM : public CAMtable<dynInstruction*> {
	public:
		o3_lsqCAM (LENGTH len, 
                   WIDTH rd_port_cnt, 
                   WIDTH wr_port_cnt,
                   sysClock* clk,
                   string table_name);
		~o3_lsqCAM ();

        dynInstruction* findPendingMemIns (LSQ_ID);
        void setTimer (dynInstruction*, CYCLE);
        void squash (INS_ID);
        void delFinishedMemAxes ();
        bool hasCommit ();
        bool hasMemAddr (ADDRS, INS_ID);
        pair<bool, dynInstruction*> hasAnyCompleteLdFromAddr (ADDRS, INS_ID);
        pair<bool, dynInstruction*> hasFinishedIns (LSQ_ID);
};

#endif
