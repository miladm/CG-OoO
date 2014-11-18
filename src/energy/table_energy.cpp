/*******************************************************************************
 * table_energy.cpp
 *******************************************************************************/

#include "table_energy.h"

table_energy::table_energy (string class_name, const YAML::Node& root) 
	: energy (class_name, root),
	  e_cam (g_stats.newEnergyStat (class_name, "e_cam", "Energy (pJ) consumped in " + class_name, 0, NO_PRINT_ZERO)),
	  e_ram (g_stats.newEnergyStat (class_name, "e_ram", "Energy (pJ) consumped in " + class_name, 0, NO_PRINT_ZERO))
{
    root["e_cam"] >> _cam_energy_per_access;
    root["e_ram"] >> _ram_energy_per_access;
    e_cam.setEnergyPerAccess (_cam_energy_per_access);
    e_ram.setEnergyPerAccess (_ram_energy_per_access);
}

table_energy::~table_energy () {}

void table_energy::camAccess (SCALAR num_access) { e_cam += num_access; }

void table_energy::ramAccess (SCALAR num_access) { e_ram += num_access; }

void table_energy::fifoAccess (SCALAR num_access) { e_ram += num_access; } /* ALIAS FOR RAMACCESS */
