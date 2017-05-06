/*******************************************************************************
 * table_energy.h
 ******************************************************************************/
#ifndef _TABLE_ENERGY_H
#define _TABLE_ENERGY_H

#include "energy.h"

class table_energy : public energy
{
 	public:
 		table_energy (string, const YAML::Node& root);
 		~table_energy ();
        void camAccess (SCALAR num_access = 1);
        void cam2Access (SCALAR num_access = 1);
        void ramAccess (SCALAR num_access = 1);
        void ram2Access (SCALAR num_access = 1);
        void fifoAccess (SCALAR num_access = 1);

    protected:
        EnergyStat& e_cam;
        EnergyStat& e_cam2;
        EnergyStat& e_ram;
        EnergyStat& e_ram2;

        PJ _cam_energy_per_access; 
        PJ _cam2_energy_per_access; 
        PJ _ram_energy_per_access;
        PJ _ram2_energy_per_access;
};

#endif
