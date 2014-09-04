/*******************************************************************************
 * rfManager.h
 ******************************************************************************/

#ifndef _BB_RF_MANAGER_H
#define _BB_RF_MANAGER_H

#include "../unit/unit.h"
#include "../unit/bbInstruction.h"
#include "lrfManager.h"
#include "grfManager.h"

class bb_rfManager : public unit {
	public:
		bb_rfManager (WIDTH num_bbWin, sysClock* clk, string rf_name = "bb_rfManager");
		~bb_rfManager ();

        // GRF ONLY
//        void completeRegs (bbInstruction* ins);
//        void squashRenameReg ();
        bool canRename (bbInstruction* ins);
        void renameRegs (bbInstruction* ins);
        void commitRegs (bbInstruction* ins);

        //LRF ONLY
//        void resetRF ();
//        void writeToRF (bbInstruction* ins);
        void reserveRF (bbInstruction* ins);
        bool canReserveRF (bbInstruction* ins);

        //LRF & GRF
        bool isReady (bbInstruction* ins);
        void completeRegs (bbInstruction* ins);
        void squashRegs ();

        /* WIRES CTRL */
        bool hasFreeWire (AXES_TYPE);
        bool hasFreeWire (AXES_TYPE, WIDTH);
        void updateWireState (AXES_TYPE);
        void updateWireState (AXES_TYPE, WIDTH);

	private:
        bb_grfManager _GRF_MGR;
        map<WIDTH, bb_lrfManager*> _LRF_MGRS;

        /*-- STAT --*/
        ScalarStat& s_rf_not_ready_cnt;
        ScalarStat& s_lrf_not_ready_cnt;
        ScalarStat& s_grf_not_ready_cnt;

};

#endif

