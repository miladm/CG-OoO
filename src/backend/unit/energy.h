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
 		energy (string, PJ cam_energy_per_access = ZERO_ENERGY, PJ ram_energy_per_access = ZERO_ENERGY);
 		~energy ();
        void camAccess ();
        void ramAccess ();
        void fifoAccess ();

    protected:
        EnergyStat& e_cam;
        EnergyStat& e_ram;
};

#endif
