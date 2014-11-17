/*******************************************************************************
 * unit.h
 ******************************************************************************/
#ifndef _UNIT_H
#define _UNIT_H

#include <map>
#include <list>
#include <vector>
#include <string>
#include <string.h>
#include <stdio.h>
#include <utility>
#include <iostream>
#include <sstream>
#include "energy.h"
#include "sysClock.h"
#include "../../global/global.h"
#include "../../global/g_objs.h"
#include "../../lib/timer.h"
#include "../../lib/utility.h"
#include "../../lib/list.h"
#include "../../lib/debug.h"
#include "../../lib/statistic.h"


class unit : public energy {
    public:
        unit (string class_name, sysClock* clk = NULL, PJ cam_energy_per_access = 0, PJ ram_energy_per_access = 0);
        ~unit ();

        const string _c_name;
        sysClock* _clk; /* PROGRAM CLOCK */
};

#endif
