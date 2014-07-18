/*******************************************************************************
 *  dominator.h
 ******************************************************************************/

#ifndef _DOMINATOR_H_
#define _DOMINATOR_H_

#include <set>
#include <map>
#include "basicblock.h"
#include "utility.h"
#include "global.h"
#include "list.h"

void setup_dominance_frontier(List<basicblock*>* bbList);

#endif