/*******************************************************************************
 * grfManager.h
 ******************************************************************************/

#ifndef _BB_GRF_MANAGER_H
#define _BB_GRF_MANAGER_H

#include "../unit/unit.h"
#include "../unit/bbInstruction.h"
#include "../unit/registerRename.h"

class bb_grfManager : public unit {
	public:
		bb_grfManager (sysClock*, const YAML::Node&, string rf_name = "bb_grfManager");
		~bb_grfManager ();

        bool canRename (bbInstruction* ins);
        void renameRegs (bbInstruction* ins);
        bool isReady (bbInstruction* ins);
        void completeRegs (bbInstruction* ins);
        void commitRegs (bbInstruction* ins);
        void squashRenameReg ();

        /* WIRES CTRL */
        bool hasFreeWire (AXES_TYPE, WIDTH);
        void updateWireState (AXES_TYPE, WIDTH);

	private:
        o3_registerRename _GRF;

        /*-- ENERGY --*/
    public:
        table_energy _e_rf;
    private:
        table_energy _e_rat;
        table_energy _e_apr; /* AVIALBLE PR LIST */
        table_energy _e_arst;

        /*-- STAT OBJS --*/
        ScalarStat& s_cant_rename_cnt;
        ScalarStat& s_can_rename_cnt;
        ScalarStat& s_unavailable_cnt;
};

#endif
