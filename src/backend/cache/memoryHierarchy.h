#ifndef __MEMORYHIERARCHY_H__
#define __MEMORYHIERARCHY_H__

#include <controller.h>
#include <cpuController.h>
#include <DRAMController.h>
#include <interconnect.h>
#include <cacheController.h>
#include <yaml/yaml.h>
#include <vector>
#include <baseAction.h>

namespace Memory {

class MemoryHierarchy : public YAMLInitable {

    private:
        /**
         * list of all clockable controllers
         * this includes cpu controllers, cache controllers and dram controllers
         */
        std::vector<Controller*> controllers;

        std::map<int, CPUController*> cpucontrollers;

    public:
        /**
         * clock all the controllers
         */
        void clock() {
            for(auto cont : controllers) {
                cont->clock();
            }
        }

        /*-- SHOWS CACHE STATS --*/
        void dump_stats(FILE* statfp) {
             for(auto cont : controllers) {
                 cont->dump_stats(statfp);
             }
             fflush(statfp);
        }

        void init(const YAML::Node &);

        void access(int coreid, bool is_instruction, Message *m, BaseAction *a);

        bool is_cache_available(uint8_t, uint8_t, bool) {
            return true;
        }

        bool grab_lock(uint64_t, uint8_t) {
            return true;
        }

        void invalidate_lock(uint64_t, uint8_t) {}

        bool probe_lock(uint64_t, uint8_t) {
            return true;
        }

        /**
         * a "summary printout" of all the cache hit rates
         * a summary must not contain any newlines
         * and should ideally be within the line size of the
         * terminal
         */
        std::string print_summary();

        void dump_state();
};

} 

#endif
