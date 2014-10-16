#ifndef __COMMON_H__
#define __COMMON_H__

#include <inttypes.h>
#include <cstdlib>
#include <inttypes.h>
#include <cassert>

typedef uint64_t W64;
extern uint64_t sim_cycle;

namespace Memory {
}

extern int vlog2(int x);

#ifndef PRId64
#define PRId64 "ld"
#endif

#ifndef PRIx64
#define PRIx64 "lx"
#endif

#endif
