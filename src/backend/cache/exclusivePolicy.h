#ifndef __EXCLUSIVE_POLICY_H__
#define __EXCLUSIVE_POLICY_H__

#include <vector>
#include <common.h>
#include <yamlInit.h>
#include <policygen.h>

namespace Memory {

class ExclusivePolicy : public Policy, public YAMLInitable {
    private:
        int numLevels;

    public:
        ExclusivePolicy() {}

        virtual void init(const YAML::Node &root) {
            const YAML::Node &cachesnode = root["caches"];
            numLevels = cachesnode.size();
        }

        virtual int get_insert_level() const {
            return 0;
        }

        virtual int get_next_level(int level) const {
            if(level < numLevels-1)
                return level+1;
            else
                return -1;
        }

        virtual void print() {}

};

class ExclusivePolicyGen : public PolicyGen, public YAMLInitable {
    private:
        ExclusivePolicy policy;

    public:
        ExclusivePolicyGen() {}

        void init(const YAML::Node &root) {
            policy.init(root);
        }

        virtual Policy *get_policy(uint64_t address, bool &shouldSample) {
            shouldSample = true;
            return &policy;
        }

        virtual void register_access(uint64_t address, bool hit) {}
};

}

#endif
