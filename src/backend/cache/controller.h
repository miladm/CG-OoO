#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include <common.h>
#include <map>
#include <list>
#include <interconnect.h>
#include <baseAction.h>
#include <cacheQueueEntry.h>
#include <string>
#include <message.h>
#include <named.h>

namespace Memory {

struct QueueEntry;
struct BaseAction;
struct Message;

struct Action : public BaseAction {
    QueueEntry *e;

    virtual ~Action() {}
};

struct Controller : public Named {

    private:
        std::map<uint64_t, Action*> waitingq;
        std::map<uint64_t, std::list<Action*> > timeActionMap;

        Interconnect *interconnect;

        Controller *lower;

        std::string name;

    public:
        virtual void request_cb(Message *) =0;
        virtual void reply_cb(Message *) =0;

        virtual void dump_stats(FILE *fp) {}

        Message *create_request(uint64_t address, OP_TYPE type, Controller *dest);

        Message *create_reply(uint64_t messageID, Controller *dest);

        bool send(Message *message);

        void register_interconnect(Interconnect *_interconn);

        void set_name(const std::string &_name) {
            name = _name;
        }

        virtual std::string get_name() {
            return name;
        }

        void wait(uint64_t, Action *);

        void message_received_cb(Message *);

        void schedule(Action *, int);

        void clock();

        virtual std::string print_summary() {return std::string();}

        virtual void dump_state();
};

}

#endif
