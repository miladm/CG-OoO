#ifndef __TAGMAP_H__
#define __TAGMAP_H__

#include <common.h>
#include <levelmask.h>
#include <yaml/yaml.h>

namespace Memory {

// Base class for tagmaps
class TagMap {
public:
	virtual void init(const YAML::Node &) = 0;
	virtual LevelMask lookup(uint64_t addr) = 0;
    virtual void insert_cb(uint64_t addr, int level) =0;
    virtual void evict_cb(uint64_t addr, int level) =0;
	virtual void reset(uint64_t addr) = 0;
    virtual int latency() const =0;
};

}

#endif
