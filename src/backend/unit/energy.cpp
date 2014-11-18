/*******************************************************************************
 * energy.cpp
 *******************************************************************************/

#include "energy.h"

energy::energy (string class_name, const YAML::Node& root) 
	: e_cam (g_stats.newEnergyStat (class_name, "e_cam", "Energy (pJ) consumped in " + class_name, 0, NO_PRINT_ZERO)),
	  e_ram (g_stats.newEnergyStat (class_name, "e_ram", "Energy (pJ) consumped in " + class_name, 0, NO_PRINT_ZERO))
{ 
    root["e_cam"] >> _cam_energy_per_access;
    root["e_ram"] >> _ram_energy_per_access;
    e_cam.setEnergyPerAccess (_cam_energy_per_access);
    e_ram.setEnergyPerAccess (_ram_energy_per_access);
}

energy::~energy () {}

void energy::camAccess () { e_cam++; }

void energy::ramAccess () { e_ram++; }

void energy::fifoAccess () { e_ram++; } /* ALIAS FOR RAMACCESS */
