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
        bool hasFreeWrPort (CYCLE now);
        bool hasFreeRdPort (CYCLE now, WIDTH numRegRdPorts);
        void updateReg (PR reg);

	private:
        registerFile _RF;
};

extern rfManager* g_RF_MGR;

#endif
