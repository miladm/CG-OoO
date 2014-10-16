#ifndef __NAMED_H__
#define __NAMED_H__

#include <string>

namespace Memory {

class Named {
    public:
        virtual std::string get_name() =0;
};

}

#endif
