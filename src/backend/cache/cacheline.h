#ifndef __CACHELINE_H__
#define __CACHELINE_H__

#include <common.h>

namespace Memory {

enum LineState {
    LINE_VALID,
    LINE_MODIFIED,
    LINE_INVALID
};

struct CacheLine {
    uint64_t tag;
    LineState state;

    // added to create profile of number of reuses
    int reused;

    CacheLine() {
        tag = 0;
        state = LINE_INVALID;
        reused = 0;
    }
};

}

#endif
