#ifndef __INTSTATS_H__
#define __INTSTATS_H__

#include <stats.h>
#include <inttypes.h>
#include <cstdio>

namespace Memory {

class IntStats : public Stats {
    protected:
        virtual void printValue(FILE *fp) {
//            fprintf(fp, "%" PRId64, value);
        }

    public:
        int64_t value;

        void init(std::string _name, Named *_p) {
            value = 0;
            Stats::init(_name, _p);
        }
};

}

#endif
