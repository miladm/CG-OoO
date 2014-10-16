#ifndef __BASEACTION_H__
#define __BASEACTION_H__

#include <array>
#include <inttypes.h>

namespace Memory {

struct BaseAction {
    virtual void execute() =0;

    virtual ~BaseAction() {};

};

}

#endif
