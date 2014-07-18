/*******************************************************************************
 *  stat.h
 ******************************************************************************/

#ifndef _STAT_H
#define _STAT_H

#include <map>
#include "basicblock.h"
#include "list.h"
#include <string.h>
#include <string>

void StatBBSizeStat(List<basicblock*> *bbList, string *program_name);
void DynBBSizeStat(map<int,int> &bbSizeHist, string *program_name);
void StatNum_interBB_and_intra_BB_regs(List<basicblock*> *bbList, string *program_name);

#endif