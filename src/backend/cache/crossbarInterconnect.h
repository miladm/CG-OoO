#ifndef __CROSSBAR_H__
#define __CROSSBAR_H__

#include <interconnect.h>
#include <controller.h>
#include <set>

namespace Memory {

struct CrossbarInterconnect : public Interconnect {

    std::set<Controller*> endpoints;

    virtual bool send(Message *message) {
        if(endpoints.count(message->dest) == 0) {
            fprintf(stderr, "%p not in endpoints\n", message->dest);
            assert(endpoints.count(message->dest));
        }

        message->dest->message_received_cb(message);
        return true;
    }

    virtual bool register_controller(Controller *controller) {
        endpoints.insert(controller);
        return true;
    }

};

}

#endif
