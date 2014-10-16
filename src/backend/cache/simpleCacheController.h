#ifndef __SIMPLECACHECONTROLLER_H__
#define __SIMPLECACHECONTROLLER_H__

#include <cacheController.h>
#include <controller.h>
#include <cache.h>
#include <tagmap.h>
#include <policygen.h>
#include <perfectTagMap.h>

#include <intStats.h>
#include <arrayStats.h>

#include <list>
#include <vector>
#include <map>

namespace Memory {

class SimpleCacheController : public CacheController, public YAMLInitable {
    private:
        BaseCache* cacheLines_;
        Controller *lower_level;

        typedef std::list<QueueEntry*> DependencyList;
        std::map<uint64_t, DependencyList> per_entry_deps;
        std::map<uint64_t, QueueEntry*> original_entry;

        uint64_t timestamp;
        std::map<uint64_t, uint64_t> last_accessed;

        float get_hit_rate() {
            return hits.value/(float)(misses.value + hits.value);
        }

    public:

        /**
         * stats for cache controller
         */
        IntStats accesses;
        IntStats demand_accesses;
        IntStats hits;
        IntStats demand_hits;
        IntStats misses;
        IntStats demand_misses;
        IntStats inserts;
        IntStats writebacks;

        ArrayStats reuse_frequencies;
        ArrayStats reuse_distances;

        SimpleCacheController() {}

        virtual void init(const YAML::Node &root);

        virtual void request_cb(Message*);
        virtual void reply_cb(Message*) {}

        virtual void dump_stats(FILE *fp);

        void handle_deps(QueueEntry*);

        void register_reuse(int reused);
        void register_reuse_distance(uint64_t address);

        BaseCache *get_cache() {
            return cacheLines_;
        }

        virtual std::string print_summary() {
            std::string sb(get_name());
            sb += ": ";
            char hit_rate_buf[20];
            sprintf(hit_rate_buf, "%.2f%% ", get_hit_rate()*100);
            sb += hit_rate_buf;
            return sb;
        }

        /**
         * Various Actions
         */
        struct SimpleAction : public Action {
            SimpleCacheController *ctrl;

            virtual ~SimpleAction() {}
        };

        struct AccessAction : public SimpleAction {
            virtual void execute();
            AccessAction(
                    SimpleCacheController *_ctrl,
                    QueueEntry *_e) {
                SimpleAction::ctrl = _ctrl;
                Action::e = _e;
            }
        };

        struct SendAction : public SimpleAction {
            Message *message;
            virtual void execute();
            SendAction(
                    SimpleCacheController *_ctrl,
                    QueueEntry *_e,
                    Message *_message) :
                message(_message) {
                    SimpleAction::ctrl = _ctrl;
                    Action::e = _e;
                }
        };

        struct RequestReplyAction : public SimpleAction {
            virtual void execute();
            RequestReplyAction(
                    SimpleCacheController *_ctrl,
                    QueueEntry *_e) {
                SimpleAction::ctrl = _ctrl;
                Action::e = _e;
            }
        };

        struct HandleDependencyAction : public SimpleAction {
            QueueEntry *de;
            virtual void execute();
            HandleDependencyAction(
                    SimpleCacheController *_ctrl,
                    QueueEntry *_e,
                    QueueEntry *_de) {
                SimpleAction::ctrl = _ctrl;
                Action::e = _e;
                de = _de;
            }
        };


        struct HitAction : public SimpleAction {
            CacheLine *line;
            virtual void execute();
            HitAction(
                    SimpleCacheController *_ctrl,
                    QueueEntry *_e,
                    CacheLine *_line) :
                line(_line) {
                    SimpleAction::ctrl = _ctrl;
                    Action::e = _e;
                }
        };

        struct MissAction : public SimpleAction {
            virtual void execute();
            MissAction(
                    SimpleCacheController *_ctrl,
                    QueueEntry *_e) {
                SimpleAction::ctrl = _ctrl;
                Action::e = _e;
            }
        };

        struct MissReplyAction : public SimpleAction {
            virtual void execute();
            MissReplyAction(
                    SimpleCacheController *_ctrl,
                    QueueEntry *_e) {
                SimpleAction::ctrl = _ctrl;
                Action::e = _e;
            }
        };

        struct InsertAction : public SimpleAction {
            virtual void execute();
            InsertAction(
                    SimpleCacheController *_ctrl,
                    QueueEntry *_e) {
                SimpleAction::ctrl = _ctrl;
                Action::e = _e;
            }
        };

        struct EvictAction : public SimpleAction {
            uint64_t address;
            virtual void execute();
            EvictAction(
                    SimpleCacheController *_ctrl,
                    QueueEntry *_e,
                    uint64_t _address) :
                address(_address) {
                SimpleAction::ctrl = _ctrl;
                Action::e = _e;
            }
        };

};

}

#endif
