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
		o3_rfManager (sysClock* clk, string rf_name = "o3_rfManager");
		~o3_rfManager ();

        bool canRename (dynInstruction* ins);
        bool renameRegs (dynInstruction* ins);
        bool isReady (dynInstruction* ins);
        void completeRegs (dynInstruction* ins);
        void commitRegs (dynInstruction* ins);
        void squashRenameReg ();

        /*-- WIRES CTRL --*/
        bool hasFreeWire (AXES_TYPE);
        bool hasFreeWire (AXES_TYPE, WIDTH);
        void updateWireState (AXES_TYPE);
        void updateWireState (AXES_TYPE, WIDTH);

	private:
        o3_registerRename _GRF;

        /*-- STAT --*/
        ScalarStat& s_rf_not_ready_cnt;
        ScalarStat& s_cant_rename_cnt;
        ScalarStat& s_can_rename_cnt;
};

extern o3_rfManager* g_GRF_MGR;

#endif
