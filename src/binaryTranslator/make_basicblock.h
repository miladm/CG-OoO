/*******************************************************************************
 *  make_basicblock.h
 ******************************************************************************/

#ifndef _MAKE_BASICBLOCK_H
#define _MAKE_BASICBLOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <set>
#include <map>
#include "basicblock.h"
#include "variable.h"
#include "global.h"
#include "dependencySetup.h"

void make_basicblock (List<instruction*> *insList,
		      List<basicblock*> *bbList,
			  map<int,variable*> &varList, 
		      std::set<ADDR> *brDstSet,
		      std::map<ADDR, basicblock*> *bbMap,
			  map<ADDR,instruction*> *insAddrMap,
              LENGTH cluster_size);

#endif
