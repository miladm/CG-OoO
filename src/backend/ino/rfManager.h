/*******************************************************************************
 * rfManager.h
 ******************************************************************************/

#ifndef _RF_MANAGER_H
#define _RF_MANAGER_H

#include "../unit/unit.h"
#include "../unit/dynInstruction.h"
#include "registerFile.h"

class rfManager : public unit {
	public:
		rfManager (sysClock* clk, string rf_name = "rfManager");
		~rfManager ();
        void resetRF ();
        bool isReady (dynInstruction* ins);
        void reserveRF (dynInstruction* ins);
        bool canReserveRF (dynInstruction* ins);
        void writeToRF (dynInstruction* ins);
        void updateReg (PR reg);

        /* WIRES CTRL */
        bool hasFreeWire (AXES_TYPE, WIDTH);
        void updateWireState (AXES_TYPE, WIDTH);

	private:
        registerFile _RF;

        /*-- STAT --*/
        ScalarStat& s_rf_not_ready_cnt;
        ScalarStat& s_lrf_busy_cnt;
        ScalarStat& s_unavailable_cnt;
};

extern rfManager* g_RF_MGR;

#endif
