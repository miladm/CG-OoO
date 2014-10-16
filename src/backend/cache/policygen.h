#ifndef __POLICYGEN_H__
#define __POLICYGEN_H__

#include <vector>
#include <common.h>
#include <yamlInit.h>

namespace Memory {

struct Policy {
    int id;

    virtual int get_insert_level() const =0;
    virtual int get_next_level(int) const =0;

    virtual void print() =0;
};

struct PolicyGen {
    virtual Policy *get_policy(uint64_t address, bool &shouldSample) =0;
    virtual void register_access(uint64_t address, bool hit) =0;
};

}

#endif
