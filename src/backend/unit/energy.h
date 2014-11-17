/*******************************************************************************
 * energy.h
 ******************************************************************************/
#ifndef _ENERGY_H
#define _ENERGY_H

#include "../../lib/statistic.h"

class energy
{
 	public:
 		energy (string, PJ cam_energy_per_access = 0, PJ ram_energy_per_access = 0);
 		~energy ();
        void camAccess ();
        void ramAccess ();
        void fifoAccess ();

    protected:
        EnergyStat& e_cam;
        EnergyStat& e_ram;
};

#endif
