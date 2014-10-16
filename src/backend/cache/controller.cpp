#include <controller.h>
#include <cassert>
#include <cstdio>
#include <inttypes.h>

using namespace std;
using namespace Memory;

void Controller::wait(uint64_t messageID, Action *action) {
//    printf("controller %s  waiting for message %d with action %p\n",
//            name.c_str(), messageID, action);
    assert(waitingq.count(messageID) == 0);
    waitingq[messageID] = action;
    action->e->__refcnt++;
}

void Controller::message_received_cb(Message *message) {
//    printf("controller %s  received message ",
//            name.c_str());
//    message->print();

    if(message->type == REPLY) {

        /**
         * for a reply find the corresponding entry in waitingq
         * and execute the action
         */
        if(waitingq.count(message->replyTo) == 0) {
            printf("ERROR dump\n");
            printf("%s, replyTo: %" PRId64 "\n", name.c_str(), message->replyTo);
            for(auto wpair : waitingq) {
                printf("%" PRId64 "  :  %p\n", wpair.first, wpair.second);
            }
        }

        assert(waitingq.count(message->replyTo));
        Action *a = waitingq[message->replyTo];

        a->execute();
        a->e->__refcnt--;
        if(a->e->__refcnt == 0)
            delete a->e;
        delete a;

        waitingq.erase(message->replyTo);

        reply_cb(message);

    } else {

        request_cb(message);

    }

    /**
     * delete the message
     */
    delete message;

}

void Controller::schedule(Action *a, int latency) {

    /**
     * if latency == 0, simply execute the action
     */
    if(latency == 0) {
        a->e->__refcnt++;
        a->execute();
        a->e->__refcnt--;
        if(a->e->__refcnt == 0)
            delete a->e;
        delete a;
        return;
    }

    /**
     * else, we can safely add to the map
     * since event will not be executed at the same clock cycle
     * just add the action to the map of actions at each tick
     */
    timeActionMap[sim_cycle + latency].push_back(a);
    a->e->__refcnt++;

}

void Controller::clock() {

    /**
     * get all the events at the current sim_cycle
     */
    list<Action*> &l = timeActionMap[sim_cycle];
    for(list<Action*>::iterator it = l.begin(); it != l.end(); ++it) {
        Action *a = *it;
        a->execute();

        a->e->__refcnt--;
        if(a->e->__refcnt == 0)
            delete a->e;
        delete a;
    }

    timeActionMap.erase(sim_cycle);

}

Message *Controller::create_request(uint64_t address, OP_TYPE type, Controller *dest) {
    return new Message(this, dest, address, type);
}

Message *Controller::create_reply(uint64_t messageID, Controller *dest) {
    return new Message(this, dest, messageID);
}

bool Controller::send(Message *message) {
//    printf("controller %s  sending message ",
//            name.c_str());
//    message->print();

    return interconnect->send(message);
}

void Controller::register_interconnect(Interconnect *_interconn) {
    interconnect = _interconn;
}

void Controller::dump_state() {
    fprintf(stderr, "%s\n", name.c_str());
    fprintf(stderr, "  waitingq:\n");
    for(auto &apair : waitingq) {
        fprintf(stderr, "    ");
        apair.second->e->print(stderr);
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "  actionq:\n");
    for(auto &lpair : timeActionMap) {
        for(auto a : lpair.second) {
            fprintf(stderr, "    ");
            a->e->print(stderr);
            fprintf(stderr, "\n");
        }
    }
}
