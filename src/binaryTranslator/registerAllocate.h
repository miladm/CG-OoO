/*******************************************************************************
 *  registerAllocate.h
 ******************************************************************************/

#ifndef _REGISTER_ALLOCATE_H
#define _REGISTER_ALLOCATE_H

#include <list>
#include <set>
#include <vector>
#include "global.h"
#include "interfNode.h"
#include "basicblock.h"
#include "instruction.h"

void findInteriorPoints(List<basicblock*> *bbList, List<basicblock*> *interiorBB);
void allocate_register(List<basicblock*> *bbList,List<instruction*> *insList);

#endif
