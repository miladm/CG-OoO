/*******************************************************************************
 * rfManager.h
 ******************************************************************************/

#ifndef _O3_RF_MANAGER_H
#define _O3_RF_MANAGER_H

#include "../unit/unit.h"
#include "../unit/dynInstruction.h"
#include "registerRename.h"

class o3_rfManager : public unit {
	public:
		o3_rfManager (string rf_name = "o3_rfManager");
		~o3_rfManager ();

        bool hasFreeWrPort (CYCLE now);
        bool hasFreeRdPort (CYCLE now, WIDTH numRegRdPorts);
        bool canRename (dynInstruction* ins);
        bool renameRegs (dynInstruction* ins);
        bool isReady (dynInstruction* ins);
        void completeRegs (dynInstruction* ins);
        void commitRegs (dynInstruction* ins);
        void squashRenameReg ();

	private:
        o3_registerRename _GRF;
};

extern o3_rfManager g_GRF_MGR;

#endif
