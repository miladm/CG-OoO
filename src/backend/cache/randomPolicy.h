#ifndef __RANDOM_POLICY_H__
#define __RANDOM_POLICY_H__

#include <policygen.h>
#include <cassert>

namespace Memory {

class RandomPolicy : public Policy, public YAMLInitable {
    private:
        std::vector<int> sizes;
        int totalsize;

        int get_rand() const;

    public:
        RandomPolicy() {
            id = 0;
        }

        virtual void init(const YAML::Node &root);

        virtual int get_insert_level() const {
            return get_rand();
        }

        virtual int get_next_level(int level) const {
            return -1;
        }

        virtual void print();
};

class RandomPolicyGen : public PolicyGen, public YAMLInitable {
    private:
        RandomPolicy policy;

    public:
        RandomPolicyGen() {}

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
