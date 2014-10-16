#ifndef __ARRAYSTATS_H__
#define __ARRAYSTATS_H__

#include <stats.h>
#include <inttypes.h>
#include <cstdio>

namespace Memory {

class ArrayStats : public Stats {
    private:
        int size;

    protected:
        virtual void printValue(FILE *fp) {
//            if(size == 0)
//                return;
//            fprintf(fp, "[%" PRId64, value[0]);
//            for(int i = 1; i < size; i++) {
//                fprintf(fp, ", %" PRId64, value[i]);
//            }
//            fprintf(fp, "]");
        }

    public:
        int64_t *value;

        void init(std::string _name, Named *_p, int _size) {
            Stats::init(_name, _p);
            size = _size;
            value = new int64_t[size];
            for(int i = 0; i < size; i++) {
                value[i] = 0;
            }
        }

        ArrayStats() : value(NULL) {}
};

}

#endif
