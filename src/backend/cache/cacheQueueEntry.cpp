#include <cacheQueueEntry.h>
#include <message.h>
#include <cstdio>

using namespace Memory;
using namespace std;

QueueEntry::QueueEntry(Message *message) {
    address = message->address;
    type = message->type;
    messageID = message->id;
    source = message->source;
    begin_cycle = sim_cycle;
    __refcnt = 0;
}

void QueueEntry::print(FILE *fp) {
    fprintf(fp, "addr %016" PRIx64 " type %3d begin_cycle %8" PRId64,
            address,
            type,
            begin_cycle);
}

