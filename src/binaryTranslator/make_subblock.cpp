/*******************************************************************************
 *  make_subblock.cpp
 ******************************************************************************/

#include "make_subblock.h"

void make_subblocks (List<basicblock*> *bbList) {
	for  (int i = 0; i < bbList->NumElements (); i++) {
		basicblock* bb = bbList->Nth (i);
        bb->findRootUPLD ();
        bb->markUPLDroots ();
        bb->setsupStats ();
        bb->reportStats ();
        cout << hex << bb->getID () << dec << " " << bb->getBbSize () << " ";
        bb->makeSubBlocks ();
        cout << endl;
    }
//	for  (int i = 0; i < bbList->NumElements (); i++) {
//		basicblock* bb = bbList->Nth (i);
//    }
}
