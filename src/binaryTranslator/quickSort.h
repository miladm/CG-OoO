/*******************************************************************************
 *  quickSort.h
 ******************************************************************************/

#ifndef _Q_SORT_
#define _Q_SORT_

#include "instruction.h"

void quicksortLongestPath(List<instruction*> *list, int left, int right);
void swap(List<instruction*> *list, int x, int y);
int partitionLongestPath(List<instruction*> *list, int left, int right);



#endif

