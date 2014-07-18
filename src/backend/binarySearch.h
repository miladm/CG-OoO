/*******************************************************************************
 * binarySearch.h
 * Performs binary search
 ******************************************************************************/
#ifndef _BIN_SEARCH_H
#define _BIN_SEARCH_H

#include "../lib/list.h"
#include "../global/global.h"
#include "instruction.h"

//Search for an instruction id
int binarySearch(List<instruction*> *list, INS_ID id, int low, int high);
int binarySearch(List<int> *list, int id, int low, int high);

#endif
