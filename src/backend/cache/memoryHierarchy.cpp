#include <memoryHierarchy.h>
#include <map>
#include <crossbarInterconnect.h>

using namespace std;
using namespace Memory;

uint64_t Message::__currentid = 1;

/**
 * set up the hierarchy
 * do this in two sweeps:
 * a) first initialize the controllers and interconnects and create name->controller mapping
 * b) next connect up controllers according to the connection map
 */
void MemoryHierarchy::init(const YAML::Node &root) {
    /**
     * initialize the interconnects
     */
    map<string, Interconnect*> interconn_names;
    string name;

    const YAML::Node &inode = root["interconnects"];
    for(int i = 0; i < (int)inode.size(); i++) {
        inode[i]["name"] >> name;
        Interconnect *interconn = new CrossbarInterconnect();
        interconn_names[name] = interconn;
    }

    /**
     * initialize the CPU controllers
     * also create a mapping of the icache and dcache for each cpu controller
     */
    map<string, CPUController*> cpucont_names;
    map<string, string> cpucont_interconns;
    map<string, string> icache_conts;
    map<string, string> dcache_conts;
    const YAML::Node &cpunode = root["cpucontrollers"];

    for(int i = 0; i < (int)cpunode.size(); i++) {
        cpunode[i]["name"] >> name;
        CPUController *cont = new CPUController();
        cpucont_names[name] = cont;
        cont->set_name(name);

        string icache_name, dcache_name, interconn_name;
        cpunode[i]["icache"] >> icache_name;
        icache_conts[name] = icache_name;
        cpunode[i]["dcache"] >> dcache_name;
        dcache_conts[name] = dcache_name;
        cpunode[i]["interconn"] >> interconn_name;
        cpucont_interconns[name] = interconn_name;

        /**
         * add to the controller list
         */
        controllers.push_back(cont);

        /**
         * get core id
         */
        int coreid;
        cpunode[i]["core"] >> coreid;
        cpucontrollers[coreid] = cont;
    }


    /**
     * initialize the cache controllers
     * also create a mapping of the lower controller
     * for each controller
     */
    map<string, string> lower_controllers;
    map<string, string> cont_interconns;
    map<string, Controller*> all_cont_names;

    const YAML::Node &controllersnode = root["controllers"];
    for(int i = 0; i < (int)controllersnode.size(); i++) {
        string type;
        controllersnode[i]["type"] >> type;
        Controller *cont = Factory<Controller>::generate(type, controllersnode[i]);
        string name;
        controllersnode[i]["name"] >> name;
        all_cont_names[name] = cont;
        cont->set_name(name);

        string interconn_name;
        controllersnode[i]["interconn"] >> interconn_name;
        cont_interconns[name] = interconn_name;

        string lower_name;
        controllersnode[i]["lower"] >> lower_name;
        lower_controllers[name] = lower_name;

        /**
         * add to the controller list
         */
        controllers.push_back(cont);
    }

    /**
     * initialize the DRAM controllers
     */
    const YAML::Node &dramnode = root["drams"];
    for(int i = 0; i < (int)dramnode.size(); i++) {
        string type;
        dramnode[i]["type"] >> type;
        DRAMController *cont = Factory<DRAMController>::generate(type, dramnode[i]);
        string name;
        dramnode[i]["name"] >> name;
        all_cont_names[name] = cont;
        cont->set_name(name);

        string interconn_name;
        dramnode[i]["interconn"] >> interconn_name;
        cont_interconns[name] = interconn_name;

        /**
         * add to the controller list
         */
        controllers.push_back(cont);
    }

    /**
     * finally, create the connections
     */
    for(auto ncpair : cpucont_names) {
        /**
         * do the interconnect
         */
        Interconnect *interconn = interconn_names[cpucont_interconns[ncpair.first]];
        ((Controller*)ncpair.second)->register_interconnect(interconn);
        interconn->register_controller(ncpair.second);

        /**
         * now do the icache and dcache
         */
        ncpair.second->register_icache(all_cont_names[
                icache_conts[ncpair.first]]);
        ncpair.second->register_dcache(all_cont_names[
                dcache_conts[ncpair.first]]);
    }

    /**
     * now do the controllers
     */
    for(auto ncpair : all_cont_names) {
        /**
         * interconnects first
         */
        printf("%s %p\n", ncpair.first.c_str(), ncpair.second);
        Interconnect *interconn = interconn_names[cont_interconns[ncpair.first]];
        ncpair.second->register_interconnect(interconn);
        interconn->register_controller(ncpair.second);

        /**
         * do the lower controller if there is any
         */
        if(lower_controllers.count(ncpair.first)) {
            static_cast<CacheController*>(ncpair.second)
                ->register_lower_level(all_cont_names[
                        lower_controllers[ncpair.first]]);
        }
    }

}

void MemoryHierarchy::access(int coreid,
        bool is_instruction,
        Message *m,
        BaseAction *a) {
    cpucontrollers[coreid]->access(m, is_instruction, a);
}

std::string MemoryHierarchy::print_summary() {
    std::string sb;
    for(auto cont : controllers) {
        sb += cont->print_summary();
        sb += " ";
    }
    return sb;
}

void MemoryHierarchy::dump_state() {
    for(auto cont : controllers) {
        bool iscpucont = false;
        for(auto pair : cpucontrollers) {
            if(pair.second == cont) {
                iscpucont = true;
                break;
            }
        }
        if(!iscpucont)
            cont->dump_state();
    }
}
