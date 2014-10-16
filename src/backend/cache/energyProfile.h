#ifndef __ENERGY_PROFILE_H__
#define __ENERGY_PROFILE_H__

#include <vector>

namespace Memory {

struct EnergyProfile {
    std::vector<float> cacheEnergies;
    float missEnergy;
};

}

#endif
