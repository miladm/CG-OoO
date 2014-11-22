/*******************************************************************************
 * lsq.h
 * -----
 * LOAD-STORE QUEUEE UNIT DATA STRUCTURES
 ******************************************************************************/

#ifndef _O3_LSQ_H
#define _O3_LSQ_H 

#include "../unit/table.h"
#include "../../global/g_variable.h"

#define NO_WAW_ST_INS 0

class o3_lsqCAM : public CAMtable<dynInstruction*> {
	public:
		o3_lsqCAM (sysClock*, const YAML::Node&, string);
		~o3_lsqCAM ();

        dynInstruction* findPendingMemIns (LSQ_ID);
        void setTimer (dynInstruction*, CYCLE);
        void squash (INS_ID);
        void delFinishedMemAxes ();
        bool hasCommit ();
        bool hasMemAddr (ADDRS, INS_ID);
        INS_ID hasAnyCompleteStFromAddr (ADDRS, INS_ID);
        pair<bool, dynInstruction*> hasAnyCompleteLdFromAddr (ADDRS, INS_ID, INS_ID);
        pair<bool, dynInstruction*> hasFinishedIns (LSQ_ID);
};

#endif
