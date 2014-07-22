/*******************************************************************************
 * rfManager.h
 ******************************************************************************/

#ifndef _BB_RF_MANAGER_H
#define _BB_RF_MANAGER_H

#include "../unit/unit.h"
#include "../unit/dynInstruction.h"
#include "registerRename.h"

class bb_rfManager : public unit {
	public:
		bb_rfManager (sysClock* clk, string rf_name = "bb_rfManager");
		~bb_rfManager ();

        bool canRename (dynInstruction* ins);
        bool renameRegs (dynInstruction* ins);
        bool isReady (dynInstruction* ins);
        void completeRegs (dynInstruction* ins);
        void commitRegs (dynInstruction* ins);
        void squashRenameReg ();

        /* WIRES CTRL */
        bool hasFreeWire (AXES_TYPE);
        bool hasFreeWire (AXES_TYPE, WIDTH);
        void updateWireState (AXES_TYPE);
        void updateWireState (AXES_TYPE, WIDTH);

	private:
        bb_registerRename _GRF;
};

#endif
