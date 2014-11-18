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
        void camAccess ();
        void ramAccess ();
        void fifoAccess ();

    protected:
        EnergyStat& e_cam;
        EnergyStat& e_ram;

        PJ _cam_energy_per_access; 
        PJ _ram_energy_per_access;
};

#endif
