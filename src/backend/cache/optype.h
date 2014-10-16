#ifndef __OPTYPE_H__
#define __OPTYPE_H__

namespace Memory {

enum OP_TYPE {
    REQUEST_READ,
    REQUEST_WRITE,
    REQUEST_UPDATE,
    REPLY,
    NUM_OP
};

} 

#endif
