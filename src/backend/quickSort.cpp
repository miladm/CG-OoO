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
 
void quicksortLongestPath(List<instruction*> *list, int left, int right, int cycle, bool UPLDhoisting)
// Postcondition: list elements between index left and index right have been
// sorted in increasing order
{
    if( left < right )
    {
        int pivotPosition = partitionLongestPath(list, left, right, cycle, UPLDhoisting);
        quicksortLongestPath(list, left, pivotPosition - 1, cycle, UPLDhoisting);
        quicksortLongestPath(list, pivotPosition + 1, right, cycle, UPLDhoisting);
    }
}


int partitionLongestPath(List<instruction*> *list, int left, int right, int cycle, bool UPLDhoisting) {
    int pivotValue = list->Nth(left)->getLongestPath(); //list[left];
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

 
void quicksortLongestPathDyanmic(List<instruction*> *list, int left, int right, int cycle, bool UPLDhoisting)
// Postcondition: list elements between index left and index right have been
// sorted in increasing order
{
    if( left < right )
    {
        int pivotPosition = partitionLongestPathDynamic(list, left, right, cycle, UPLDhoisting);
        quicksortLongestPathDyanmic(list, left, pivotPosition - 1, cycle, UPLDhoisting);
        quicksortLongestPathDyanmic(list, pivotPosition + 1, right, cycle, UPLDhoisting);
    }
}


int partitionLongestPathDynamic(List<instruction*> *list, int left, int right, int cycle, bool UPLDhoisting) {
    int pivotValue = list->Nth(left)->findLongestPathDynamicly(cycle, UPLDhoisting); //list[left];
    int originalLeft = left;

    do {
        do {
            left++;
        } while (left < right && list->Nth(left)->findLongestPathDynamicly(cycle, UPLDhoisting) > pivotValue);

        while (list->Nth(right)->findLongestPathDynamicly(cycle, UPLDhoisting) < pivotValue)
            right--;
        if(left < right)
            swap(list, left, right);
    } while (left < right);

    swap(list, originalLeft, right);

    return right;
}
//TODO how does this sorting algorithleft break a tie? should do oldest first - implement it



/*==============================Ins Lost Sort====================================*/
/****************************************
 *Sorts from the smallest to the largest
 ****************************************/

void quicksortInsList(List<instruction*> *list, int left, int right)
// Postcondition: list elements between index left and index right have been
// sorted in increasing order
{
    if( left < right )
    {
        int pivotPosition = partitionInsList(list, left, right);
        quicksortInsList(list, left, pivotPosition - 1);
        quicksortInsList(list, pivotPosition + 1, right);
    }
}


int partitionInsList(List<instruction*> *list, int left, int right) {
    INS_ID pivotValue = list->Nth(left)->getInsID(); //list[left];
    int originalLeft = left;

    do {
        do {
            left++;
        } while (left < right && list->Nth(left)->getInsID() < pivotValue);

        while (list->Nth(right)->getInsID() > pivotValue)
            right--;
        if(left < right)
            swap(list, left, right);
    } while (left < right);

    swap(list, originalLeft, right);

    return right;
}
//TODO how does this sorting algorithleft break a tie? should do oldest first - implement it





/*==============================List Sort====================================*/
/****************************************
 *Sorts from the smallest to the largest
 ****************************************/
void swap(List<int> *list, int x, int y) {
   int tempX = list->Nth(x); 
   int tempY = list->Nth(y);

   list->RemoveAt(x);
   list->InsertAt(tempY,x);
   list->RemoveAt(y);
   list->InsertAt(tempX,y);
} 
 
void quicksort(List<int> *list, int left, int right, int cycle)
// Postcondition: list elements between index left and index right have been
// sorted in increasing order
{
    if( left < right )
    {
        int pivotPosition = partition(list, left, right, cycle);
        quicksort(list, left, pivotPosition - 1, cycle);
        quicksort(list, pivotPosition + 1, right, cycle);
    }
}


int partition(List<int> *list, int left, int right, int cycle) {
    int pivotValue = list->Nth(left); //list[left];
    int originalLeft = left;

    do {
        do {
            left++;
        } while (left < right && list->Nth(left) < pivotValue);

        while (list->Nth(right) > pivotValue)
            right--;
        if(left < right)
            swap(list, left, right);
    } while (left < right);

    swap(list, originalLeft, right);

    return right;
}



/*==============================Fragment Sort====================================*/
/****************************************
 *Sorts from the smallest to the largest fragment number of bits of dependency id list
 ****************************************/
void swap(List<fragment*> *list, int x, int y) { 
   fragment* tempX = list->Nth(x); 
   fragment* tempY = list->Nth(y);

   list->RemoveAt(x);
   list->InsertAt(tempY,x);
   list->RemoveAt(y);
   list->InsertAt(tempX,y);
} 
 
void quicksort(List<fragment*> *list, int left, int right, int cycle)
// Postcondition: list elements between index left and index right have been
// sorted in increasing order
{
    if( left < right )
    {
        int pivotPosition = partition(list, left, right, cycle);
        quicksort(list, left, pivotPosition - 1, cycle);
        quicksort(list, pivotPosition + 1, right, cycle);
    }
}


int partition(List<fragment*> *list, int left, int right, int cycle) {
    int pivotValue = list->Nth(left)->getNumBits(); //list[left];
    int originalLeft = left;

    do {
        do {
            left++;
        } while (left < right && list->Nth(left)->getNumBits() < pivotValue);

        while (list->Nth(right)->getNumBits() > pivotValue)
            right--;
        if(left < right)
            swap(list, left, right);
    } while (left < right);

    swap(list, originalLeft, right);

    return right;
}


/****************************************
 * Sorts fragments based on their score
 ****************************************/
void quicksortFragScore(List<fragment*> *list, int left, int right, int cycle)
// Postcondition: list elements between index left and index right have been
// sorted in increasing order
{
    if( left < right )
    {
        int pivotPosition = partitionFragScore(list, left, right, cycle);
        quicksortFragScore(list, left, pivotPosition - 1, cycle);
        quicksortFragScore(list, pivotPosition + 1, right, cycle);
    }
}


int partitionFragScore(List<fragment*> *list, int left, int right, int cycle) {
    double pivotValue = list->Nth(left)->getFrScore(); //list[left];
    int originalLeft = left;

    do {
        do {
            left++;
        } while (left < right && list->Nth(left)->getFrScore() > pivotValue);

        while (list->Nth(right)->getFrScore() < pivotValue)
            right--;
        if(left < right)
            swap(list, left, right);
    } while (left < right);

    swap(list, originalLeft, right);

    return right;
}





/*


void quicksort(List<instruction*> *list,int left,int right)
{ 
   int pivot,i,j,k; 
   if( left < right) 
   {
      k = choose_pivot(left,right); 
      swap(list,left,k); 
      pivot = list->Nth(left)->findLongestPath(list->Nth(left)->getInsID());
      i = left+1; 
      j = right;
      while(i <= j) 
      { 
         while((i <= right) && (list->Nth(i)->findLongestPath(list->Nth(i)->getInsID()) <= pivot)) 
                i++; 
         while((j >= left) && (list->Nth(j)->findLongestPath(list->Nth(j)->getInsID()) > pivot))
                j--; 
         if( i < j) 
                swap(list,i,j); 
      }
      // swap two elements 
      swap(list,left,j); 
      // recursively sort the lesser list 
      quicksort(list,left,j-1);
      quicksort(list,j+1,right); 
   }
}
*/
