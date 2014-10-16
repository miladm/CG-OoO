#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <common.h>
#include <optype.h>
#include <cstdio>
#include <array>

namespace Memory {

struct Controller;

struct Message {
    uint64_t address;
    OP_TYPE type;
    uint64_t id;
    uint64_t replyTo;

    Controller *source;
    Controller *dest;

    static uint64_t __currentid;

    Message(Controller *_source, Controller *_dest, uint64_t _address, OP_TYPE _type) {
        source = _source;
        dest = _dest;
        address = _address;
        type = _type;
        id = __currentid;
        replyTo = 0;
        __currentid++;
    }

    Message(Controller *_source, Controller *_dest, uint64_t _replyTo) {
        source = _source;
        dest = _dest;
        replyTo = _replyTo;
        id = __currentid;
        type = REPLY;
        __currentid++;
    }

    Message(uint64_t _address, OP_TYPE _type) {
        address = _address;
        type = _type;
    }

    void print(FILE *fp);
    void print();
};


}

#endif
