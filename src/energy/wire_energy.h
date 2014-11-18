/*******************************************************************************
 * wire_energy.h
 ******************************************************************************/
#ifndef _WIRE_ENERGY_H
#define _WIRE_ENERGY_H

#include "energy.h"

class wire_energy : public energy
{
 	public:
 		wire_energy (string, const YAML::Node& root);
 		~wire_energy ();
        void wireAccess ();

    private:
        EnergyStat& e_wire;

        PJ _wire_energy_per_access; 
        WIDTH _num_wires;
};

#endif
