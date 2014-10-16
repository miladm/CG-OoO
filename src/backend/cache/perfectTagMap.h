#ifndef __PERFECT_TAGMAP__
#define __PERFECT_TAGMAP__

#include <common.h>
#include <levelmask.h>
#include <tagmap.h>
#include <map>

namespace Memory {

class PerfectTagMap : public TagMap {
    private:
        std::map<uint64_t, LevelMask> lineLevels;
        int lat;
        uint64_t tagof(uint64_t addr) {
            return addr & (~63ULL);
        }
    public:
        virtual void init(const YAML::Node &);
        virtual LevelMask lookup(uint64_t addr);
        virtual void insert_cb(uint64_t addr, int level);
        virtual void evict_cb(uint64_t addr, int level);
        virtual void reset(uint64_t addr);
        virtual int latency() const;
};

}

#endif
