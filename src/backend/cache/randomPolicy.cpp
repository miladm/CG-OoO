#include <randomPolicy.h>

using namespace Memory;

int RandomPolicy::get_rand() const {
    int randnum = rand() % totalsize;
    int cumulative = 0;
    for(int i = 0; i < (int)sizes.size(); i++) {
        cumulative += sizes[i];
        if(randnum < cumulative)
            return i;
    }
    assert(0);
    return -1;
}

void RandomPolicy::init(const YAML::Node &root) {
    const YAML::Node &cachesnode = root["caches"];
    totalsize = 0;
    for(int i = 0; i < (int)cachesnode.size(); i++) {
        const YAML::Node &cachenode = cachesnode[i];
        int size;
        cachenode["size"] >> size;
        sizes.push_back(size);
        totalsize += size;
    }
}

void RandomPolicy::print() {
    printf("sizes: {%d", sizes[0]);
    for(int i = 1; i < (int)sizes.size(); i++) {
        printf(", %d", sizes[i]);
    }
    printf("} total = %d\n", totalsize);
}

Generator<PolicyGen, RandomPolicyGen> random_policy_gen("random");

