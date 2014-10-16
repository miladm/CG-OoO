#ifndef __LEVELMASK_H__
#define __LEVELMASK_H__

#include <common.h>
#include <cassert>
#include <cstdio>

namespace Memory {

struct LevelMask {
    uint64_t mask;
    LevelMask() {
        mask = 0;
    }

    int popcount() {
        return __builtin_popcountll(mask);
    }

    int lsb() {
        assert(mask != 0);
        return 63 - __builtin_clzll(mask);
    }

    void reset(int pos) {
        mask &= (~(1ULL << pos));
    }

    void set(int pos) {
        mask |= (1ULL << pos);
    }

    bool iszero() {
        return (mask == 0);
    }

    void print(FILE *fp) {
        for(int i = 63; i >= 0; i--) {
            fprintf(fp, "%d", (int)(bool)(mask & (1ULL << i)));
        }
    }

    void print() {
        print(stdout);
    }

};

}

#endif
