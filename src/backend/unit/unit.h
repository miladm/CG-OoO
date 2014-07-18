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
#include "../../global/global.h"
#include "../../lib/timer.h"
#include "../../lib/utility.h"
#include "../../lib/list.h"
#include "../../lib/debug.h"
#include "../../lib/statistic.h"


class unit {
    public:
        unit(string class_name);
        ~unit();
        string _c_name;
};

#endif
