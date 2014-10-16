#include <cpuController.h>

using namespace Memory;

//void access(MemmoryRequest *req) {
//
//    req->incRefCounter();
//
//    Controller *dest;
//    if(req->is_instruction())
//        dest = icache;
//    else
//        dest = dcache;
//
//    Message *m = create_request(req->get_physical_address(),
//            req->type, dest);
//    QueueEntry *e = new QueueEntry(m);
//    Action *a = new ReplyAction(e, req);
//
//    Controller::send(message);
//    Controller::wait(message->id, a);
//}
//
//
//void CPUController::ReplyAction::execute() {
//    req->get_coreSignal()->emit(req);
//    req->decRefCounter();
//}

void CPUController::access(Message *m, bool is_instruction, BaseAction *a) {
    Controller *dest = is_instruction?icache:dcache;
    Message *req_m = create_request(m->address, m->type, dest);
    QueueEntry *e = new QueueEntry(req_m);
    Action *ra = new ReplyAction(e, a);

    Controller::wait(req_m->id, ra);
    Controller::send(req_m);

    delete m;

    accesses.value++;
}

void CPUController::ReplyAction::execute() {
    a->execute();
    delete a;
}
