/*******************************************************************************
 *  make_superblock.h
 ******************************************************************************/

#ifndef _MAKE_SUPERBLOCK_H
#define _MAKE_SUPERBLOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <set>
#include <map>
#include "basicblock.h"
#include "variable.h"
#include "global.h"

void make_superblock (List<instruction*> *insList,
        List<basicblock*> *bbList,
        map<int,variable*> &varList, 
        std::set<ADDR> *brDstSet,
        std::map<ADDR, basicblock*> *bbMap,
        map<ADDR,instruction*> *insAddrMap);

#endif
