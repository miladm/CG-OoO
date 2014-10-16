#ifndef __CPUCONTROLLER_H__
#define __CPUCONTROLLER_H__

#include <controller.h>
#include <common.h>
#include <cassert>
#include <baseAction.h>
#include <intStats.h>

namespace Memory {

class CPUController : public Controller {

    private:
        Controller *icache;
        Controller *dcache;

        IntStats accesses;

    public:
        CPUController() {
            accesses.init("accesses", this);
        }

        /**
         * no requests can be made to this controller
         */
        virtual void request_cb(Message *m) {
            assert(0);
        }

        virtual void reply_cb(Message *m) {}

        virtual void dump_stats(FILE *fp) {
            accesses.print(fp);
        }

        void access(Message *m, bool is_instruction, BaseAction *a);

        struct ReplyAction : public Action {
            BaseAction *a;
            ReplyAction(QueueEntry *_e, BaseAction *_a) {
                Action::e = _e;
                a = _a;
            }

            virtual void execute();
        };

        void register_icache(Controller *cont) {
            icache = cont;
        }

        void register_dcache(Controller *cont) {
            dcache = cont;
        }
};

}

#endif
