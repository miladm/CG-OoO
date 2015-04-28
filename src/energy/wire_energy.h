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
        void wireAccess (list<string>);

    private:
        void getWires (const YAML::Node& root);
        void addWireComponent (PJ, WIDTH, string);

    private:
        string _class_name;
        PJ _wire_energy_per_access; 
        map<string, EnergyStat*> _wires;
};

#endif
