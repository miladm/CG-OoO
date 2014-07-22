/*******************************************************************************
 * branchPred.h
 ******************************************************************************/

#ifndef _BB_BRANCHP_PRED_H
#define _BB_BRANCHP_PRED_H

#include "../unit/stage.h"

class bb_branchPred : protected stage {
	public:
		bb_branchPred (port<dynInstruction*>& fetch_to_bp_port, 
			    	port<dynInstruction*>& bp_to_fetch_port, 
			    	WIDTH bp_width,
                    sysClock* clk,
			    	string stage_name);
		~bb_branchPred ();
		void doBP ();
        void squash ();
        void regStat ();

	private:
		port<dynInstruction*>* _fetch_to_bp_port;
		port<dynInstruction*>* _bp_to_fetch_port;
};

#endif
