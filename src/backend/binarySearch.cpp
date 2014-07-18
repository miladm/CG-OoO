/*******************************************************************************
 * binarySearch.cpp
 * Performs binary search
 ******************************************************************************/
#include "binarySearch.h"


int binarySearch(List<instruction*> *list, INS_ID id, int low, int high) {
	if (high < low)
		return -1; // not found

	int mid = low + (high - low) / 2;

	if (list->Nth(mid)->getInsID() > id)
		return binarySearch(list, id, low, mid-1);
	else if (list->Nth(mid)->getInsID() < id)
		return binarySearch(list, id, mid+1, high);
	else
		return mid; // found - array index is returned
}

int binarySearch(List<int> *list, int id, int low, int high) {
	if (high < low)
		return -1; // not found

	int mid = low + (high - low) / 2;

	if (list->Nth(mid) > id)
		return binarySearch(list, id, low, mid-1);
	else if (list->Nth(mid) < id)
		return binarySearch(list, id, mid+1, high);
	else
		return mid; // found - array index is returned
}
