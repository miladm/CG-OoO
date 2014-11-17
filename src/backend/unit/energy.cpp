/*******************************************************************************
 * energy.cpp
 *******************************************************************************/

#include "energy.h"

energy::energy (string class_name, PJ cam_energy_per_access, PJ ram_energy_per_access) 
	: e_cam (g_stats.newEnergyStat (cam_energy_per_access, class_name, "e_cam", "Energy (pJ) consumped in " + class_name, 0, NO_PRINT_ZERO)),
	  e_ram (g_stats.newEnergyStat (ram_energy_per_access, class_name, "e_ram", "Energy (pJ) consumped in " + class_name, 0, NO_PRINT_ZERO))
      
{ }

energy::~energy () {}

void energy::camAccess () { e_cam++; }

void energy::ramAccess () { e_ram++; }

void energy::fifoAccess () { e_ram++; } /* ALIAS FOR RAMACCESS */
