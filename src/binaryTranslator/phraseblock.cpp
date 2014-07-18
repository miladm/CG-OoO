/*******************************************************************************
 *  phraseblock.cpp
 ******************************************************************************/

#include <math.h>
#include "phraseblock.h"

phraseblock::phraseblock() : basicblock() {
	_numPhraseblocks = 0;
	_phraseBBLists = new List<basicblock*>;
	_ancestorPbList = new List<phraseblock*>;
	_descendantPbList = new List<phraseblock*>;
}

phraseblock::~phraseblock() {
	delete _bBLists;
	delete _phraseBBLists;
	delete _ancestorPbList;
	delete _descendantPbList;
}

void phraseblock::loopToPhraseblock(loop *lp) {
	//Count the number of WBB's in loop
	int wbbCount = 0;
	List<ADDR> wbbSet;
	printf("\n");
	for (int i = 0; i < lp->getNumBB(); i++) {
		basicblock* bb = lp->getNthBB(i);
		printf("%f, ", bb->getTakenBias());
		if (bb->getNumDescendents() >= 2 && //sometimes not all descendents are captured
			bb->getTakenBias() > WBB_LOWER_BOUND && 
		    bb->getTakenBias() < WBB_UPPER_BOUND &&
		    lp->isBbInLoop(bb->getFallThrough()->getID()) &&
			lp->isBbInLoop(bb->getTakenTarget()->getID())) { //TODO create macros for this boundires
			Assert(bb->getNumDescendents() == 2 && "Invalid number of basicblock descendents\n");
			wbbSet.Append(bb->getID());
			wbbCount++;
		}
	}
	int numPhraseBlks = pow(2,wbbCount); //TODO this is very bad model, must be changed
	printf("\nnumber of WBBs in loop %llx: %d, %d\n", lp->getLoopEntryID(), numPhraseBlks, lp->getNumBB());
	// lp->setNumWbb(wbbCount);
	if (numPhraseBlks <= 4) {
		_bBLists = new List<basicblock*>* [numPhraseBlks];
		for (int i = 0; i < numPhraseBlks; i++) {
			_bBLists[i] = new List<basicblock*>;
		}
		for (int i = 0; i < numPhraseBlks; i++) {
			List<basicblock*>* bbList = _bBLists[i];
			basicblock* bbHead = lp->getLoopEntry();
			lp->resetVisitBits();
			basicblock* bb = bbHead;
			bool break_loop = false;

			while(1) {
				bbList->Append(bb);
				bb->setAsVisited();
				int elemIndx = wbbSet.findElement(bb->getID());
				//printf("indx = %llx\n", bb->getID());
				if (elemIndx > -1) {
					switch(elemIndx) {
						case 0:
							if (i%2 == 0 && 
							    lp->isBbInLoop(bb->getNthDescendent(0)->getID()) == true &&
								bb->getNthDescendent(0)->isVisited() == false && 
								bb->getNthDescendent(0)->getID() != bb->getID())
								bb = bb->getNthDescendent(0);
							else if (lp->isBbInLoop(bb->getNthDescendent(1)->getID()) == true && 
							         bb->getNthDescendent(1)->isVisited() == false && 
									 bb->getNthDescendent(1)->getID() != bb->getID())
								bb = bb->getNthDescendent(1);
							else
								break_loop = true;
								//TODO: what do we do here?
							break;
						case 1: 
							if (i < 2 && 
							    lp->isBbInLoop(bb->getNthDescendent(0)->getID()) == true && 
							    bb->getNthDescendent(0)->isVisited() == false && 
							    bb->getNthDescendent(0)->getID() != bb->getID())
								bb = bb->getNthDescendent(0); 
							else if (lp->isBbInLoop(bb->getNthDescendent(1)->getID()) == true && 
							         bb->getNthDescendent(1)->isVisited() == false && 
							         bb->getNthDescendent(1)->getID() != bb->getID())
								bb = bb->getNthDescendent(1);
							else
								break_loop = true;
								//TODO: what do we do here?
							break;
						default:
							Assert("Invalid number of WBB's. Aborting execution.");
					}
				} else if (bb->getNumDescendents() >= 2 && (lp->isBbFallThrough(bb->getFallThrough()->getID()) || lp->isBbFallThrough(bb->getTakenTarget()->getID()))) {
					if (lp->isBbFallThrough(bb->getFallThrough()->getID()) &&
							   bb->getTakenTarget()->isVisited() == false &&
							   bb->getTakenTarget()->getID() != bb->getID()) {
						bb = bb->getTakenTarget();
					} else if (lp->isBbFallThrough(bb->getTakenTarget()->getID()) &&
							   bb->getFallThrough()->isVisited() == false &&
							   bb->getFallThrough()->getID() != bb->getID()) {
						bb = bb->getFallThrough();
					} else {
						break; //TODO validate that this is correct functionality
					}
				} else if (bb->getNumDescendents() > 0 &&
						   bb->getNxtBB() != NULL &&
				           bb->getNxtBB()->isVisited() == false && 
						   lp->isBbInLoop(bb->getNxtBB()->getID()) == true &&
				           bb->getNxtBB()->getID() != bb->getID()) {
					bb = bb->getNxtBB();
				} else {
					printf("case 4\n");
					break; //graph is completed
				}
				if (break_loop == true) break;
			}
			printf("\nPHRASE (%d): ", i);
			for (int i = 0; i < bbList->NumElements(); i++) {
				printf("%llx, ", bbList->Nth(i)->getID());
			}
			printf("\n");
		}
	} else {
		printf("DEBUG: too many WBB's in the loop. Skipping the loop for now.\n");
	}
	_numPhraseblocks = numPhraseBlks;
}

//Convert phraseblocks to BB's and 
//replace them with the BB's in the program
void phraseblock::PhraseblockToBB (List<basicblock*>* bbList_new, loop* lp, ADDR* phID) {
	if (_numPhraseblocks <= 4) {
		for (int i = 0; i < _numPhraseblocks; i++) {
			List<basicblock*>* bbList = _bBLists[i];
			basicblock* bb = new basicblock;
			_phraseBBLists->Append(bb);
			//Contruct the phraseblock BB
			//TODO add the position index to the BB
			(*phID)++;
			for (int j = 0; j < bbList->NumElements(); j++) {
				basicblock* nthBB = bbList->Nth(j);
				bb->addBBtoPBList(nthBB->getID());
				for (int k = 0; k < nthBB->getInsList()->NumElements(); k++) {
					bb->addIns(nthBB->getInsList()->Nth(k), *phID);
				}
			}
			bb->setListIndx(bbList_new->NumElements());
			bbList_new->Append(bb);
			//Contruct input/loop/output edges
			List<basicblock*>* lpHeadAncestors = lp->getLoopEntry()->getAncestorList();
			for (int j = 0; j < lpHeadAncestors->NumElements(); j++) {
				//lpHeadAncestors->Nth(j)->setDescendent(bb); //TODO this part is missing - must be completed
			}
			/* link fall through paths HERE */
			for (int j = 0; j < lp->getFallThroughBBs()->NumElements(); j++) {
				bb->setDescendent(lp->getFallThroughBBs()->Nth(j));
			}
		}
		//Link phraseblocks together
		Assert(_phraseBBLists->NumElements() == _numPhraseblocks && "Phrase-basic blocks are not made correctly");
		_numPhraseblocks = _phraseBBLists->NumElements();
		for (int i = 0; i < _numPhraseblocks; i++) {
			for (int j = 0; j < _numPhraseblocks; j++) {
				_phraseBBLists->Nth(i)->setDescendent(_phraseBBLists->Nth(j));
			}
		}
	}
}