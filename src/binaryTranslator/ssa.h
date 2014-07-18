/*******************************************************************************
 *  ssa.h
 ******************************************************************************/

#ifndef _SSA_H
#define _SSA_H

#include <map>
#include "list.h"
#include "basicblock.h"
#include "variable.h"
#include "global.h"

void build_ssa_form(List<basicblock*> *bbList, map<int,variable*> &varList);

#endif