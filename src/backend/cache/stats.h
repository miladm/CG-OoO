#ifndef __STATS_H__
#define __STATS_H__

#include <string>
#include <cstdio>
#include <named.h>

namespace Memory {

class Stats {
    private:
        Named *parent;
        std::string name;

    protected:
        virtual void printValue(FILE *fp) =0;

    public:
        void print(FILE *fp) {
            fprintf(fp, "%s.%s = ", parent->get_name().c_str(), name.c_str());
            printValue(fp);
            fprintf(fp, "\n");
        }

        void init(std::string _name, Named *_p) {
            name = _name;
            parent = _p;
        }
};

}

#endif
