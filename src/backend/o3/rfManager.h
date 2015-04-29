/*******************************************************************************
 * rfManager.h
 ******************************************************************************/

#ifndef _O3_RF_MANAGER_H
#define _O3_RF_MANAGER_H

#include "../unit/unit.h"
#include "../unit/dynInstruction.h"
#include "../unit/registerRename.h"

class o3_rfManager : public unit {
	public:
		o3_rfManager (sysClock*, const YAML::Node&, string rf_name = "o3_rfManager");
		~o3_rfManager ();

        bool canRename (dynInstruction* ins);
        bool renameRegs (dynInstruction* ins);
        bool checkReadyAgain (dynInstruction* ins);
        bool isReady (dynInstruction* ins);
        void completeRegs (dynInstruction* ins);
        void commitRegs (dynInstruction* ins);
        void squashRenameReg ();
        void regStat ();

        /*-- WIRES CTRL --*/
        bool hasFreeWire (AXES_TYPE, WIDTH);
        void updateWireState (AXES_TYPE, WIDTH, bool update_wire = false);

	private:
        o3_registerRename _GRF;

        /*-- ENERGY --*/
        table_energy _e_rf;
        table_energy _e_rat;
        table_energy _e_apr; /* AVIALBLE PR LIST */
        table_energy _e_arst;
        wire_energy _e_w_rr;

        /*-- STAT --*/
        ScalarStat& s_rf_not_ready_cnt;
        ScalarStat& s_cant_rename_cnt;
        ScalarStat& s_can_rename_cnt;
        ScalarStat& s_unavailable_cnt;
};

extern o3_rfManager* g_GRF_MGR;

#endif
