/*******************************************************************************
 * stage_energy.cpp
 *******************************************************************************/

#include "stage_energy.h"

stage_energy::stage_energy (string class_name, const YAML::Node& root) 
	: energy (class_name, root),
	  e_ff (g_stats.newEnergyStat (class_name, "e_ff", "Stage Energy (pJ) consumped in " + class_name, 0, NO_PRINT_ZERO))
{
    _ff_energy_per_access = 0;
    _ff_stage_cnt = 1;
    _ff_stage_width = 1;

    YAML::Iterator it;
    for(it = root.begin(); it != root.end(); ++it) {
        std::string key,value;
        it.first() >> key; it.second() >> value;

        if (key.compare ("e_ff") == 0) {
            root["e_ff"] >> _ff_energy_per_access;
        } else if (key.compare ("width") == 0) {
            root["width"] >> _ff_stage_width;
        } else if (key.compare ("latency") == 0) {
            root["latency"] >> _ff_stage_cnt;
        } else if (key.compare ("e_leak") == 0) {
            root["e_leak"] >> _leakage_energy_per_access;
        }
    }

    PJ energy_per_access = _ff_energy_per_access * _ff_stage_cnt / 2; /* ASSUME 1/2 OF BITS FLIP */
    PJ leakage_energy_per_access = _leakage_energy_per_access * _ff_stage_cnt * _ff_stage_width;
    e_ff.setEnergyPerAccess (energy_per_access);
    e_leak.setEnergyPerAccess (leakage_energy_per_access);
}

stage_energy::~stage_energy () {}

void stage_energy::ffAccess (SCALAR num_access) { e_ff += num_access; }
