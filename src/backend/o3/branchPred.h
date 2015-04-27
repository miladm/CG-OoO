/*******************************************************************************
 * branchPred.h
 ******************************************************************************/

#ifndef _O3_BRANCHP_PRED_H
#define _O3_BRANCHP_PRED_H

#include "../unit/stage.h"

class o3_branchPred : protected stage {
	public:
		o3_branchPred (port<dynInstruction*>& fetch_to_bp_port, 
			    	port<dynInstruction*>& bp_to_fetch_port, 
			    	WIDTH bp_width,
                    sysClock* clk,
			    	string stage_name);
		~o3_branchPred ();
		void doBP ();
        void squash ();
        void regStat ();

	private:
		port<dynInstruction*>* _fetch_to_bp_port;
		port<dynInstruction*>* _bp_to_fetch_port;

        /* ENERGY */
        stage_energy _e_stage;
};

#endif
