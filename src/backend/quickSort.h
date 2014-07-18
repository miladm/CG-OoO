#ifndef _Q_SORT_
#define _Q_SORT_
#include "instruction.h"
#include "fragment.h"

void quicksortLongestPath(List<instruction*> *list, int left, int right, int cycle, bool UPLDhoisting);
void swap(List<instruction*> *list, int x, int y);
int partitionLongestPath(List<instruction*> *list, int left, int right, int cycle, bool UPLDhoisting);

void quicksortLongestPathDyanmic(List<instruction*> *list, int left, int right, int cycle, bool UPLDhoisting);
int partitionLongestPathDynamic(List<instruction*> *list, int left, int right, int cycle, bool UPLDhoisting);

void quicksortInsList(List<instruction*> *list, int left, int right);
int partitionInsList(List<instruction*> *list, int left, int right);

void quicksort	(List<int> *list, int left, int right, int cycle);
void swap	(List<int> *list, int x, int y);
int partition	(List<int> *list, int left, int right, int cycle);

void quicksort	(List<fragment*> *list, int left, int right, int cycle);
void swap	(List<fragment*> *list, int x, int y);
int partition	(List<fragment*> *list, int left, int right, int cycle);

void quicksortFragScore(List<fragment*> *list, int left, int right, int cycle);
int partitionFragScore(List<fragment*> *list, int left, int right, int cycle);


#endif

