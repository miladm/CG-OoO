/*******************************************************************************
 * branchPred.cpp
 *******************************************************************************/

#include "branchPred.h"

o3_branchPred::o3_branchPred (port<dynInstruction*>& fetch_to_bp_port, 
			    		port<dynInstruction*>& bp_to_fetch_port, 
	    				WIDTH bp_width,
                        sysClock* clk,
	    				string stage_name) 
	: stage (bp_width, stage_name, clk)
{ 
    _fetch_to_bp_port = &fetch_to_bp_port;
    _bp_to_fetch_port = &bp_to_fetch_port;
}

o3_branchPred::~o3_branchPred () {}

void o3_branchPred::doBP () {
    /* STAT */
    regStat (clk);
}

void o3_branchPred::squash () {
}

void o3_branchPred::regStat () {
    _fetch_to_bp_port->regStat (_clk->now ());
}


