/*******************************************************************************
 * stage_energy.h
 ******************************************************************************/
#ifndef _STAGE_ENERGY_H
#define _STAGE_ENERGY_H

#include "energy.h"

class stage_energy : public energy
{
 	public:
 		stage_energy (string, const YAML::Node& root);
 		~stage_energy ();
        void ffAccess (SCALAR num_access = 1);

    protected:
        EnergyStat& e_ff;

        PJ _ff_energy_per_access; 
        SCALAR _ff_stage_cnt; /* EACH STAGE CAN CONSIST OF MUTIPLE PIPELIE STAGES */
};

#endif
