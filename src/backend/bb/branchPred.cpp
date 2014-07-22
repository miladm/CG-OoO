/*******************************************************************************
 * branchPred.cpp
 *******************************************************************************/

#include "branchPred.h"

bb_branchPred::bb_branchPred (port<dynInstruction*>& fetch_to_bp_port, 
			    		port<dynInstruction*>& bp_to_fetch_port, 
	    				WIDTH bp_width,
                        sysClock* clk,
	    				string stage_name) 
	: stage (bp_width, stage_name, clk)
{ 
    _fetch_to_bp_port = &fetch_to_bp_port;
    _bp_to_fetch_port = &bp_to_fetch_port;
}

bb_branchPred::~bb_branchPred () {}

void bb_branchPred::doBP () {
    /*-- STAT --*/
    regStat ();
}

void bb_branchPred::squash () {
}

void bb_branchPred::regStat () {
    _fetch_to_bp_port->regStat ();
}


