#ifndef __DRAMCONTROLLER_H__
#define __DRAMCONTROLLER_H__

#include <controller.h>
#include <cassert>
#include <yamlInit.h>
#include <intStats.h>

namespace Memory {

struct DRAMController : public Controller, public YAMLInitable {
    int latency;
    IntStats accesses;
    IntStats demand_accesses;

    DRAMController() {}

    DRAMController(int _latency) : latency(_latency) {}

    virtual void init(const YAML::Node &root) {
        root["latency"] >> latency;
        accesses.init("accesses", this);
        demand_accesses.init("demand_accesses", this);
    }

    struct SendReplyAction : public Action {
        DRAMController *cont;
        SendReplyAction(DRAMController *_cont, QueueEntry *_e) {
            cont = _cont;
            Action::e = _e;
        }
        virtual void execute();
    };

    virtual void request_cb(Message *m);
    virtual void reply_cb(Message *m) {
        assert(0);
    }

    virtual void dump_stats(FILE *fp);
};

}

#endif
