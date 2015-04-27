/*******************************************************************************
 * branchPred.cpp
 *******************************************************************************/

#include "branchPred.h"

branchPred::branchPred (port<dynInstruction*>& fetch_to_bp_port, 
			    		port<dynInstruction*>& bp_to_fetch_port, 
	    				WIDTH bp_width,
                        sysClock* clk,
	    				string stage_name) 
	: stage (bp_width, stage_name, g_cfg->_root["cpu"]["backend"]["pipe"]["bp"], clk)
{ 
    _fetch_to_bp_port = &fetch_to_bp_port;
    _bp_to_fetch_port = &bp_to_fetch_port;
}

branchPred::~branchPred () {}

void branchPred::doBP () {
    /* STAT */
    regStat ();
}

void branchPred::squash () {
    _e_stage.ffAccess ();
}

void branchPred::regStat () {
    _fetch_to_bp_port->regStat ();
    _e_stage.ffAccess (); //READ
    _e_stage.ffAccess (); //WRITE
}


