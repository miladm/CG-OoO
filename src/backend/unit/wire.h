/*******************************************************************************
 * wires.h
 ******************************************************************************/
#ifndef _BUS_H
#define _BUS_H

#include "unit.h"
#include "dynInstruction.h"

class wires : public unit
{
 	public:
 		wires (WIDTH, AXES_TYPE, string wire_name = "wire");
 		~wires ();
        bool hasFreeWire (sysClock&);
        void setWireBusy (sysClock&);

 	private: //variables
        CYCLE _cycle;
        WIDTH _num_free_wire;
        const AXES_TYPE _axes_type;
        const WIDTH _wire_cnt;
};

#endif
