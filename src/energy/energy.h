/*******************************************************************************
 * energy.h
 ******************************************************************************/
#ifndef _ENERGY_H
#define _ENERGY_H

#include "../../lib/statistic.h"
#include "../../global/global.h"

class energy
{
 	public:
 		energy (string, const YAML::Node& root);
 		~energy ();
    protected:
        LeakageEnergyStat& e_leak;

        PJ _leakage_energy_per_access;
};

#endif
