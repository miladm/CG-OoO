/*******************************************************************************
 * branchPred.h
 ******************************************************************************/

#ifndef _BRANCHP_PRED_H
#define _BRANCHP_PRED_H

#include "../unit/stage.h"

class branchPred : protected stage {
	public:
		branchPred (port<dynInstruction*>& fetch_to_bp_port, 
			    	port<dynInstruction*>& bp_to_fetch_port, 
			    	WIDTH bp_width,
			    	string stage_name);
		~branchPred ();
		void doBP (sysClock& clk);
        void squash (sysClock& clk);
        void regStat (sysClock& clk);

	private:
		port<dynInstruction*>* _fetch_to_bp_port;
		port<dynInstruction*>* _bp_to_fetch_port;
};

#endif
