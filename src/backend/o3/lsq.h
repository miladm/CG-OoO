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
		o3_lsqCAM (LENGTH len = 1, 
                   WIDTH rd_port_cnt = 1, 
                   WIDTH wr_port_cnt = 1,
                   string table_name = "LSQtable");
		~o3_lsqCAM ();

        dynInstruction* findPendingMemIns (LSQ_ID);
        void setTimer (dynInstruction*, CYCLE, CYCLE);
        void squash (INS_ID);
        void delFinishedMemAxes (sysClock&);
        bool hasCommit ();
        bool hasAnyCompleteLdFromAddr (INS_ID, dynInstruction*);
        pair<bool, dynInstruction*> hasFinishedIns (LSQ_ID, CYCLE);
};

#endif
