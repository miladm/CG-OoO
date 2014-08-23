/*******************************************************************************
 * grfManager.h
 ******************************************************************************/

#ifndef _BB_GRF_MANAGER_H
#define _BB_GRF_MANAGER_H

#include "../unit/unit.h"
#include "../unit/dynInstruction.h"
#include "../o3/registerRename.h"

class bb_grfManager : public unit {
	public:
		bb_grfManager (sysClock* clk, string rf_name = "bb_grfManager");
		~bb_grfManager ();

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
        o3_registerRename _GRF;
};

#endif
