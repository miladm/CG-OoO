#include <perfectTagMap.h>

using namespace Memory;

void PerfectTagMap::init(const YAML::Node &root) {
    root["tagmap"]["latency"] >> lat;
}

LevelMask PerfectTagMap::lookup(uint64_t addr) {
    return lineLevels[tagof(addr)];
}

void PerfectTagMap::insert_cb(uint64_t addr, int level) {
    lineLevels[tagof(addr)].set(level);
}

void PerfectTagMap::evict_cb(uint64_t addr, int level) {
    lineLevels[tagof(addr)].reset(level);
    if(lineLevels[tagof(addr)].iszero())
        lineLevels.erase(tagof(addr));
}

void PerfectTagMap::reset(uint64_t addr) {
    lineLevels.erase(tagof(addr));
}

int PerfectTagMap::latency() const {
    return lat;
}

