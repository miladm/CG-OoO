/*******************************************************************************
 * energy.cpp
 *******************************************************************************/

#include "energy.h"

energy::energy (string class_name, const YAML::Node& root) 
	  : e_leak (g_stats.newLeakageEnergyStat (class_name, "e_leak", "Stage Leakage Energy (pJ) consumped in " + class_name, 0, NO_PRINT_ZERO))
{}

energy::~energy () {}
