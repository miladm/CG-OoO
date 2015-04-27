/*******************************************************************************
 * wire_energy.cpp
 *******************************************************************************/

#include "wire_energy.h"

wire_energy::wire_energy (string class_name, const YAML::Node& root) 
	: energy (class_name, root),
      e_wire (g_stats.newEnergyStat (class_name, "e_wire", "Wire energy (pJ) consumped on " + class_name, 0, NO_PRINT_ZERO))
{ 
    root["e_wire"] >> _wire_energy_per_access;
    root["w_wire"] >> _num_wires;
    PJ energy_per_access = _num_wires * _wire_energy_per_access;
    e_wire.setEnergyPerAccess (energy_per_access);
    e_leak.setEnergyPerAccess (_leakage_energy_per_access); /* MUST PASS ZERO */
}

wire_energy::~wire_energy () {}

void wire_energy::wireAccess () { e_wire++; }
