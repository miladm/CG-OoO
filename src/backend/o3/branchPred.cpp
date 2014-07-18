/*******************************************************************************
 * branchPred.cpp
 *******************************************************************************/

#include "branchPred.h"

o3_branchPred::o3_branchPred (port<dynInstruction*>& fetch_to_bp_port, 
			    		port<dynInstruction*>& bp_to_fetch_port, 
	    				WIDTH bp_width,
	    				string stage_name) 
	: stage (bp_width, stage_name)
{ 
    _fetch_to_bp_port = &fetch_to_bp_port;
    _bp_to_fetch_port = &bp_to_fetch_port;
}

o3_branchPred::~o3_branchPred () {}

void o3_branchPred::doBP (sysClock& clk) {
    /* STAT */
    regStat (clk);
}

void o3_branchPred::squash (sysClock& clk) {
}

void o3_branchPred::regStat (sysClock& clk) {
    _fetch_to_bp_port->regStat (clk.now ());
}


