#ifndef __CACHEQUEUEENTRY_H__
#define __CACHEQUEUEENTRY_H__

#include <common.h>
#include <optype.h>
#include <cstdio>
#include <array>

namespace Memory {

struct Controller;
struct Message;

struct QueueEntry {
    Controller *source;
    uint64_t address;
    OP_TYPE type;
    uint64_t messageID;
    uint64_t begin_cycle;

    int __refcnt;

    QueueEntry() : __refcnt(0) {}

    QueueEntry(Message *message);

    ~QueueEntry() {}

    void print(FILE *);
};

}

#endif
