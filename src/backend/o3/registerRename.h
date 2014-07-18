/*******************************************************************************
 * registerRename.h
 ******************************************************************************/

#ifndef _O3_REGISTER_RENAME_H
#define _O3_REGISTER_RENAME_H

#include "../unit/dynInstruction.h"
#include "../unit/unit.h"

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
		o3_registerRename (string rf_name);
		o3_registerRename (AR a_rf_lo, AR a_rf_hi, 
                           WIDTH rd_port_cnt, WIDTH wr_port_cnt, 
                           string rf_name);
		~o3_registerRename ();

		PR renameReg (AR a_reg);
        void squashRenameReg ();
        bool hasFreeRdPort (CYCLE, WIDTH);
        bool hasFreeWrPort (CYCLE);

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

	private:
        CYCLE _cycle;

        const PR _a_rf_size;
        const PR _a_rf_hi;
        const PR _a_rf_lo;

        const AR _p_rf_size;
        const AR _p_rf_hi;
        const AR _p_rf_lo;

        const WIDTH _wr_port_cnt;
        const WIDTH _rd_port_cnt;
        WIDTH _num_free_wr_port;
        WIDTH _num_free_rd_port;

		map<AR, o3_regElem*> _fRAT;
		map<AR, o3_regElem*> _cRAT;
		map<PR, o3_regElem*> _RF;
		vector<o3_regElem*> _availablePRset; /* FILO */
};

#endif
