/*******************************************************************************
 * eu_energy.h
 ******************************************************************************/
#ifndef _EU_ENERGY_H
#define _EU_ENERGY_H

#include "energy.h"

class eu_energy : public energy
{
 	public:
 		eu_energy (string, const YAML::Node& root);
 		~eu_energy ();
        void euAccess ();

    private:
        EnergyStat& e_eu;

        PJ _eu_energy_per_compute; 
};

#endif
