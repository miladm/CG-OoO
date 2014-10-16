#ifndef __INTERCONNECT_H__
#define __INTERCONNECT_H__

namespace Memory {

struct Message;
struct Controller;

struct Interconnect {

    virtual bool send(Message *) =0;

    virtual bool register_controller(Controller *) =0;

};

}

#endif
