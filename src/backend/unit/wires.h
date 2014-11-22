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
 		wires (AXES_TYPE, sysClock*, const YAML::Node&, string wire_name = "wire");
 		~wires ();
        bool hasFreeWire ();
        void updateWireState ();
        WIDTH getNumFreeWires ();

 	private: //variables
        CYCLE _cycle;
        WIDTH _num_free_wire;
        const AXES_TYPE _axes_type;
        const nMETER _wire_len;
        WIDTH _wire_cnt;

        /* STAT OBJS */
        ScalarStat& s_unavailable_cnt;
};

#endif
