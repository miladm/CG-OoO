/*******************************************************************************
 * registerRename.h
 ******************************************************************************/

#ifndef _O3_REGISTER_RENAME_H
#define _O3_REGISTER_RENAME_H

#include "dynInstruction.h"
#include "unit.h"
#include "wires.h"

#define RD_TO_WR_WIRE_CNT_RATIO 2

struct o3_regElem {
    o3_regElem (PR reg, REG_REN_STATE state) 
        : _reg (reg)
    {
        Assert (state == AVAILABLE || state == ARCH_REG);
        _reg_state = state;
        _prev_pr = NULL;
    }
    const PR _reg;
    REG_REN_STATE _reg_state;
    o3_regElem* _prev_pr;
};

class o3_registerRename : public unit {
	public:
		o3_registerRename (sysClock* clk, const YAML::Node& root, string rf_name);
		~o3_registerRename ();

		PR renameReg (AR a_reg);
        void squashRenameReg ();

        /* RAT's */
		void update_fRAT (AR a_reg, PR p_reg);
		void update_cRAT (AR a_reg, PR p_reg);

        /* AVAILABLE PR */
		bool isAnyPRavailable ();
		PR getAvailablePR ();
		void setAsAvailablePR (PR p_reg);
		int getNumAvailablePR ();
		PR getPrevPR (PR p_reg);

        /* PR STATE */
		void updatePR (PR new_pr, PR prev_pr, REG_REN_STATE state);
		void updatePRstate (PR new_pr, REG_REN_STATE state);
		REG_REN_STATE getPRstate (PR p_reg);
		void squashPRstate (PR new_pr);
        bool isPRvalid (PR p_reg);

        /* WIRE CTRL */
        void updateWireState (AXES_TYPE, list<string> wire_name = list<string>(), bool update_wire = false);
        bool hasFreeWire (AXES_TYPE);
        WIDTH getNumFreeWires (AXES_TYPE);

        void getStat ();

	private:
        CYCLE _cycle;

        AR _a_rf_size;
        AR _a_rf_hi;
        AR _a_rf_lo;

        PR _p_rf_size;
        PR _p_rf_hi;
        PR _p_rf_lo;

        PR _r_rf_size;
        PR _r_rf_hi;
        PR _r_rf_lo;

        wires _wr_port;
        wires _rd_port;

		map<AR, o3_regElem*> _fRAT;
		map<AR, o3_regElem*> _cRAT;
		map<PR, o3_regElem*> _RF;
		vector<o3_regElem*> _availablePRset; /* FILO */

        /*-- STAT --*/
        ScalarStat& s_availablePRset_empty_cnt;
        RatioStat& s_availablePRset_avg;
};

#endif
