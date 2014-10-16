#include <DRAMController.h>

using namespace Memory;

void DRAMController::SendReplyAction::execute() {
    Message *m = ((Controller*)cont)->create_reply(e->messageID, e->source);
    ((Controller*)cont)->send(m);
}

void DRAMController::request_cb(Message *m) {
    if(m->type != REQUEST_UPDATE) {
        QueueEntry *e = new QueueEntry(m);
        Action *a = new SendReplyAction(this, e);
        ((Controller*)this)->schedule(a, latency);
        demand_accesses.value++;
    }

    accesses.value++;
}

void DRAMController::dump_stats(FILE *fp) {
    accesses.print(fp);
    demand_accesses.print(fp);
}

Generator<DRAMController, DRAMController> dram_gen("dram");
