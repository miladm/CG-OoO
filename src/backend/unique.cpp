/*******************************************************************************
 * unique.cpp
 * Remove duplicates in a sorted list (from smallest insID to the highest insID)
 ******************************************************************************/

#include "unique.h"

void  unique (List<int>* list) {
	int listSize = list->NumElements();
	for (int i = listSize-2; i >= 0; i--) {
		int prevIns = list->Nth(i+1);
		int thisIns = list->Nth(i);
		Assert(prevIns >= thisIns);
		if (prevIns == thisIns) {
			list->RemoveAt(i+1);
		}
	}
}
