/*******************************************************************************
 * grfManager.h
 ******************************************************************************/

#ifndef _BB_GRF_MANAGER_H
#define _BB_GRF_MANAGER_H

#include "../unit/unit.h"
#include "../unit/bbInstruction.h"
#include "registerRename.h"

class bb_grfManager : public unit {
	public:
		bb_grfManager (sysClock*, WIDTH, const YAML::Node&, string rf_name = "bb_grfManager");
		~bb_grfManager ();

        bool canRename (bbInstruction*, BB_ID);
        void renameRegs (bbInstruction*);
        bool isReady (bbInstruction*);
        bool checkReadyAgain (bbInstruction*);
        void completeRegs (bbInstruction*);
        void commitRegs (bbInstruction*);
        void squashRenameReg ();

        /* WIRES CTRL */
        bool hasFreeWire (AXES_TYPE, WIDTH);
        void updateWireState (AXES_TYPE, WIDTH, list<string> wire_name = list<string>(), bool update_wire = false);
    
        void getStat ();

	private:
        bb_registerRename _GRF;

        /*-- ENERGY --*/
    public:
        table_energy _e_rf;
    private:
        table_energy _e_rat;
        table_energy _e_apr; /* AVIALBLE PR LIST */
        table_energy _e_arst;
        wire_energy _e_w_rr;

        /*-- STAT OBJS --*/
        ScalarStat& s_cant_rename_cnt;
        ScalarStat& s_can_rename_cnt;
        ScalarStat& s_unavailable_cnt;
};

#endif
