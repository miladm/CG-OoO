#ifndef __CACHECONTROLLER_H__
#define __CACHECONTROLLER_H__

#include <controller.h>

namespace Memory {

struct CacheController : public Controller {
    private:
        Controller *lower_level;

    public:
        Controller *get_lower_level() {
            return lower_level;
        }

        void register_lower_level(Controller *cont) {
            lower_level = cont;
        }

};

}

#endif
