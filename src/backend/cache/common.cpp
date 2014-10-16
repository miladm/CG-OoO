#include <common.h>

using namespace Memory;

int vlog2(int x) {
    if(x == 1)
        return 0;
    else
        return vlog2(x/2)+1;
    assert(0);
}
