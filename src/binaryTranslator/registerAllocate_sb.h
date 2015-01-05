/*******************************************************************************
 *  registerAllocate_sb.h
 ******************************************************************************/

#ifndef _REGISTER_ALLOCATE_SB_H
#define _REGISTER_ALLOCATE_SB_H

#include <list>
#include <set>
#include <vector>
#include "global.h"
#include "interfNode.h"
#include "basicblock.h"
#include "instruction.h"
#include "listSchedule.h"

void allocate_register_sb(List<basicblock*> *bbList, List<instruction*> *insList, map<ADDR,instruction*> *insAddrMap, REG_ALLOC_MODE reg_alloc_mode);

#endif
