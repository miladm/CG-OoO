/*******************************************************************************
 * registerRename.h
 ******************************************************************************/

#ifndef _BB_REGISTER_RENAME_H
#define _BB_REGISTER_RENAME_H

#include "../unit/dynInstruction.h"
#include "../unit/unit.h"
#include "../unit/wires.h"

#define RD_TO_WR_WIRE_CNT_RATIO 2

struct bb_regElem {
    bb_regElem (PR reg, REG_REN_STATE state) 
        : _reg (reg)
    {
        Assert (state == AVAILABLE || state == ARCH_REG);
        _reg_state = state;
        _prev_pr = NULL;
    }
    const PR _reg;
    REG_REN_STATE _reg_state;
    bb_regElem* _prev_pr;
};

class bb_registerRename : public unit {
	public:
		bb_registerRename (sysClock* clk, string rf_name);
		bb_registerRename (AR a_rf_lo, AR a_rf_hi, 
                           WIDTH rd_port_cnt, WIDTH wr_port_cnt, 
                           sysClock* clk, string rf_name);
		~bb_registerRename ();

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
        void updateWireState (AXES_TYPE);
        bool hasFreeWire (AXES_TYPE);
        WIDTH getNumFreeWires (AXES_TYPE);

	private:
        CYCLE _cycle;

        const PR _a_rf_size;
        const PR _a_rf_hi;
        const PR _a_rf_lo;

        const AR _p_rf_size;
        const AR _p_rf_hi;
        const AR _p_rf_lo;

        wires _wr_port;
        wires _rd_port;

		map<AR, bb_regElem*> _fRAT;
		map<AR, bb_regElem*> _cRAT;
		map<PR, bb_regElem*> _RF;
		vector<bb_regElem*> _availablePRset; /* FILO */
};

#endif
