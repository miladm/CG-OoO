#ifndef _H_SIMPLE_DRIVER
#define  _H_SIMPLE_DRIVER

#include <cpuController.h>
#include <controller.h>
#include <memoryHierarchy.h>
#include <baseAction.h>
#include <common.h>
#include <message.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;
using namespace Memory;

struct Driver {
    MemoryHierarchy *hier;
    bool done;
    FILE* cache_stat;

    Driver (const char *configfile) {
        ifstream fin (configfile);
        YAML::Parser parser (fin);
        YAML::Node doc;
        parser.GetNextDocument (doc);

        hier = new MemoryHierarchy ();
        hier->init (doc);

        /* FOR DEBUGGING PURPOSES */
        cache_stat = fopen ("/scratch/milad/cache_debug", "w");
    }

    struct ReplyAction : public BaseAction {
        Driver *dr;
        uint64_t addr;
        ReplyAction (Driver *_dr, uint64_t _addr) : dr (_dr), addr (_addr) {}

        virtual void execute () {
            dr->done = true;
        }
    };

    int request (uint64_t address, bool is_instruction, OP_TYPE type) {
        int numcycles = 0;
        Message *m = new Message (address, type);

        /* ATTENTION: HERE WE DO A WHILE LOOP UNTIL THIS ACCESS COMES BACK,
         * THIS NEEDS TO BE MODIFIED FOR A BETTER CYCLE ACCURATE MODEL */
        done = false;

        hier->access (0, is_instruction, m, new ReplyAction (this, address));
        while (!done) {
            numcycles++;
            hier->clock ();
            if (numcycles > 1000)
                assert (0 && "deadlock here");
            sim_cycle++;
        }

        /* FOR DEBUGGING PURPOSES */
//        hier->dump_stats(cache_stat);
//        hier->dump_state();

        return numcycles;
    }
};

#endif
