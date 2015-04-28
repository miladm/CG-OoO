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
		bb_registerRename (sysClock* clk, WIDTH, const YAML::Node& root, string rf_name);
		~bb_registerRename ();

		PR renameReg (AR a_reg);
        void squashRenameReg ();

        /* RAT's */
		void update_fRAT (AR a_reg, PR p_reg);
		void update_cRAT (AR a_reg, PR p_reg);

        /* AVAILABLE PR */
		PR getAvailablePR (BB_ID);
		void setAsAvailablePR (PR p_reg);
		int getNumAvailablePR (BB_ID);
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

        /* STAT */
        void getStat ();

    private:
        WIDTH blkIndx2APRindx (BB_ID);
        WIDTH PR2APRindx (PR pr);

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

		map<AR, bb_regElem*> _fRAT;
		map<AR, bb_regElem*> _cRAT;
		map<PR, bb_regElem*> _RF;
		List<vector<bb_regElem*>*>* _availablePRset; /* FILO */

        const WIDTH _blk_cnt;
        AR _a_rf_segmnt_size;
        PR _p_rf_segmnt_size;
        WIDTH _grf_segmnt_cnt;
        WIDTH _num_blk_per_segmnt;

        /*-- STAT --*/
        ScalarStat& s_far_segmnt_alloc_cnt;
        ScalarStat& s_loc_segmnt_alloc_cnt;
        ScalarStat& s_availablePRset_empty_cnt;
        RatioStat& s_availablePRset_avg;
};

#endif
