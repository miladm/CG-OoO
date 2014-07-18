/*******************************************************************************
 *  dfs.h
 ******************************************************************************/

#ifndef _DFS_H
#define _DFS_H

#include <stdint.h>
#include <stdlib.h>
#include "list.h"
#include "instruction.h"
#include "basicblock.h"

void dfs(List<basicblock*>* bbList, ADDR bbID_seed);

#endif
