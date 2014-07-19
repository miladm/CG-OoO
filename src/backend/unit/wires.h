/*******************************************************************************
 * wires.h
 ******************************************************************************/
#ifndef _WIRES_H
#define _WIRES_H

#include "unit.h"
#include "sysClock.h"

class wires : public unit
{
 	public:
 		wires (WIDTH, AXES_TYPE, sysClock*, string wire_name = "wire");
 		~wires ();
        bool hasFreeWire ();
        void updateWireState ();
        WIDTH getNumFreeWires ();

 	private: //variables
        CYCLE _cycle;
        WIDTH _num_free_wire;
        const AXES_TYPE _axes_type;
        const WIDTH _wire_cnt;
};

#endif
