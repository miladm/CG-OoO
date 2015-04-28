/*******************************************************************************
 * wire_energy.cpp
 *******************************************************************************/

#include "wire_energy.h"

wire_energy::wire_energy (string class_name, const YAML::Node& root) 
	: energy (class_name, root),
      _class_name (class_name)
{ 
    e_leak.setEnergyPerAccess (_leakage_energy_per_access); /* MUST PASS ZERO */
    getWires (root);
}

void wire_energy::getWires (const YAML::Node& root) {
    YAML::Iterator it;
    for(it = root.begin(); it != root.end(); ++it) {
        std::string key,value;
        it.first() >> key; it.second() >> value;

        PJ energy_val = 0;
        WIDTH num_wires = 0;
        root["w_wire"] >> num_wires;

        /* GENERAL CASES */
        if (key.compare ("e_w_cache2rr") == 0) {
            string name = "e_w_cache2rr";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        }

        /* INO */
        if (key.compare ("e_w_cache2win") == 0) {
            string name = "e_w_cache2win";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_bp2cache_ino") == 0) {
            string name = "e_w_bp2cache_ino";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_win2eu") == 0) {
            string name = "e_w_win2eu";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_eu2simplerf") == 0) {
            string name = "e_w_eu2simplerf";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_cache2rf") == 0) {
            string name = "e_w_cache2rf";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_l22l1") == 0) {
            string name = "e_w_l22l1";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        }

        /* O3 */
        if (key.compare ("e_w_rs2lsq") == 0) {
            string name = "e_w_rs2lsq";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_cache2lsq_o3") == 0) {
            string name = "e_w_cache2lsq_o3";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_eu2lsq_o3") == 0) {
            string name = "e_w_eu2lsq_o3";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_bp2cache_o3") == 0) {
            string name = "e_w_bp2cache_o3";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_rr2lsq_o3") == 0) {
            string name = "e_w_rr2lsq_o3";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_rs2eu") == 0) {
            string name = "e_w_rs2eu";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_eu2rob") == 0) {
            string name = "e_w_eu2rob";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_eu2rf") == 0) {
            string name = "e_w_eu2rf";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_lsq2rf") == 0) {
            string name = "e_w_lsq2rf";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_lsq2rob") == 0) {
            string name = "e_w_lsq2rob";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_rr2rob") == 0) {
            string name = "e_w_rr2rob";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_rr2rs") == 0) {
            string name = "e_w_rr2rs";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        }

        /* BB */
        if (key.compare ("e_w_iq2lsq") == 0) {
            string name = "e_w_iq2lsq";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_cache2lsq_bb") == 0) {
            string name = "e_w_cache2lsq_bb";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_eu2lsq_bb") == 0) {
            string name = "e_w_eu2lsq_bb";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_bp2cache_bb") == 0) {
            string name = "e_w_bp2cache_bb";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_rr2lsq_bb") == 0) {
            string name = "e_w_rr2lsq_bb";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_iq2eu") == 0) {
            string name = "e_w_iq2eu";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_eu2brob") == 0) {
            string name = "e_w_eu2brob";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_eu2grf") == 0) {
            string name = "e_w_eu2grf";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_eu2lrf") == 0) {
            string name = "e_w_eu2lrf";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_lsq2lrf") == 0) {
            string name = "e_w_lsq2lrf";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_lsq2grf") == 0) {
            string name = "e_w_lsq2grf";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_lsq2brob") == 0) {
            string name = "e_w_lsq2brob";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_rr2brob") == 0) {
            string name = "e_w_rr2brob";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        } else if (key.compare ("e_w_rr2iq") == 0) {
            string name = "e_w_rr2iq";
            root[name] >> energy_val;
            addWireComponent (energy_val, num_wires, name);
        }
    }
}

void wire_energy::addWireComponent (PJ energy_val, WIDTH num_wires, string wire_name) {
    PJ energy_per_access = num_wires * energy_val;
    EnergyStat* e_wire = &g_stats.newEnergyStat (_class_name, wire_name, "Wire energy (pJ) consumped on " + _class_name + " " + wire_name, 0, NO_PRINT_ZERO);
    e_wire->setEnergyPerAccess (energy_per_access);
    _wires.insert(pair<string, EnergyStat*>(wire_name, e_wire));
}

wire_energy::~wire_energy () {
    map<string, EnergyStat*>::iterator it;
    for (it = _wires.begin (); it != _wires.end (); it++) {
        delete it->second;
    }
}

void wire_energy::wireAccess (string wire_name) { 
    if (_wires.find (wire_name) != _wires.end ()) 
        (*_wires[wire_name])++;
}
