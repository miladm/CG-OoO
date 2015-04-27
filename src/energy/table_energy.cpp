/*******************************************************************************
 * table_energy.cpp
 *******************************************************************************/

#include "table_energy.h"

table_energy::table_energy (string class_name, const YAML::Node& root) 
	: energy (class_name, root),
	  e_cam (g_stats.newEnergyStat (class_name, "e_cam", "Energy (pJ) consumped in " + class_name, 0, NO_PRINT_ZERO)),
	  e_cam2 (g_stats.newEnergyStat (class_name, "e_cam2", "Energy (pJ) consumped in " + class_name, 0, NO_PRINT_ZERO)),
	  e_ram (g_stats.newEnergyStat (class_name, "e_ram", "Energy (pJ) consumped in " + class_name, 0, NO_PRINT_ZERO))
{
    _cam_energy_per_access = 0;
    _cam2_energy_per_access = 0;
    _ram_energy_per_access = 0;

    YAML::Iterator it;
    for(it = root.begin(); it != root.end(); ++it) {
        std::string key,value;
        it.first() >> key; it.second() >> value;

        if (key.compare ("e_cam") == 0) {
            root["e_cam"] >> _cam_energy_per_access;
            e_cam.setEnergyPerAccess (_cam_energy_per_access);
        } else if (key.compare ("e_cam2") == 0) {
            root["e_cam2"] >> _cam2_energy_per_access;
            e_cam2.setEnergyPerAccess (_cam2_energy_per_access);
        } else if (key.compare ("e_ram") == 0) {
            root["e_ram"] >> _ram_energy_per_access;
            e_ram.setEnergyPerAccess (_ram_energy_per_access);
        } else if (key.compare ("e_leak") == 0) {
            root["e_leak"] >> _leakage_energy_per_access;
            e_leak.setEnergyPerAccess (_leakage_energy_per_access);
        }
    }
}

table_energy::~table_energy () {}

void table_energy::camAccess (SCALAR num_access) { e_cam += num_access; }

void table_energy::cam2Access (SCALAR num_access) { e_cam2 += num_access; }

void table_energy::ramAccess (SCALAR num_access) { e_ram += num_access; }

void table_energy::fifoAccess (SCALAR num_access) { e_ram += num_access; } /* ALIAS FOR RAMACCESS */
