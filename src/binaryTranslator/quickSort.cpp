/*******************************************************************************
 *  quickSort.cpp
 ******************************************************************************/

#include <stdio.h> 
#include <stdlib.h> 
#include "quickSort.h"

/*==============================Longest Path Sort====================================*/
/****************************************
 *Sorts from the largest to the smallest
 ****************************************/
void swap(List<instruction*> *list, int x, int y) {
   instruction* tempX = list->Nth(x); 
   instruction* tempY = list->Nth(y);

   list->RemoveAt(x);
   list->InsertAt(tempY,x);
   list->RemoveAt(y);
   list->InsertAt(tempX,y);
} 
 
void quicksortLongestPath(List<instruction*> *list, int left, int right)
// Postcondition: list elements between index left and index right have been
// sorted in increasing order
{
    if( left < right )
    {
        int pivotPosition = partitionLongestPath(list, left, right);
        quicksortLongestPath(list, left, pivotPosition - 1);
        quicksortLongestPath(list, pivotPosition + 1, right);
    }
}


int partitionLongestPath(List<instruction*> *list, int left, int right) {
    int pivotValue = list->Nth(left)->getLongestPath();
    Assert(pivotValue > -1);
    int originalLeft = left;

    do {
        do {
            left++;
        } while (left < right && list->Nth(left)->getLongestPath() > pivotValue);

        while (list->Nth(right)->getLongestPath() < pivotValue)
            right--;
        if(left < right)
            swap(list, left, right);
    } while (left < right);

    swap(list, originalLeft, right);

    return right;
}