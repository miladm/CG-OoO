/*******************************************************************************
 * eu_energy.cpp
 *******************************************************************************/

#include "eu_energy.h"

eu_energy::eu_energy (string class_name, const YAML::Node& root) 
	: energy (class_name, root),
      e_eu (g_stats.newEnergyStat (class_name, "e_eu", "EU energy (pJ) consumped in " + class_name, 0, NO_PRINT_ZERO))
{ 
    root["e_eu"] >> _eu_energy_per_compute;
    e_eu.setEnergyPerAccess (_eu_energy_per_compute);
}

eu_energy::~eu_energy () {}

void eu_energy::euAccess () { e_eu++; }
