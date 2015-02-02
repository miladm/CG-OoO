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
		bb_rfManager (WIDTH, sysClock*, const YAML::Node&, string rf_name = "bb_rfManager");
		~bb_rfManager ();

        // GRF ONLY
//        void completeRegs (bbInstruction* ins);
//        void squashRenameReg ();
        bool canRename (bbInstruction*, BB_ID);
        void renameRegs (bbInstruction*);
        void commitRegs (bbInstruction*);

        //LRF ONLY
//        void resetRF ();
//        void writeToRF (bbInstruction* ins);
        void reserveRF (bbInstruction*);
        bool canReserveRF (bbInstruction*);

        //LRF & GRF
        bool isReady (bbInstruction*);
        bool checkReadyAgain (bbInstruction*);
        void completeRegs (bbInstruction*);
        void squashRegs ();

        /* WIRES CTRL */
        bool hasFreeWire (AXES_TYPE, bbInstruction*);
        void updateWireState (AXES_TYPE, bbInstruction*);

        void getStat ();

	private:
        bb_grfManager _GRF_MGR;
        map<WIDTH, bb_lrfManager*> _LRF_MGRS;

        /*-- STAT --*/
        ScalarStat& s_rf_not_ready_cnt;
        ScalarStat& s_lrf_not_ready_cnt;
        ScalarStat& s_grf_not_ready_cnt;
        ScalarStat& s_lrf_busy_cnt;
};

#endif

