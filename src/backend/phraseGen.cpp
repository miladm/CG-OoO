/*******************************************************************************
 * phraseGen.cpp
 * generated phrases for each instruction thru interacting with the instruction class
 ******************************************************************************/
#include "phraseGen.h"

#define NEXT_PHRASE 1
#define FIRST_PHRASE 0

extern int phraseSizeBound;
extern core coreType;
extern int numFU;
extern float unpredMemOpThreshold;

phraseGen::phraseGen() {
	openPh = new List<phrase*>;
	closePh = new List<phrase*>;
	topPhraseID = FIRST_PHRASE;
	resetCount = 0;

	totNumPhraseAncestors = 0;
	totNumRootIns = 0;
	totNumPhUnpredMemOp = 0;
	insCount = 0;
	totNumSoftBounds = 0;
	totRootPhrases = 0;
	totNumFragments = 0;
	numCritPathViolations = 0;
}

phraseGen::~phraseGen() {
	while(openPh->NumElements() > 0) {
	    openPh->RemoveAt(0);
	}
	delete openPh;

	while(closePh->NumElements() > 0) {
	    closePh->RemoveAt(0);
	}
	delete closePh;
}
/*
int phraseGen::runPhraseGen2(List<instruction*> *iROB, List<instruction*> *iWindow, FILE* phraseFile, dependencyTable *depTables, bool eof, int ROBsize) {
	int phraseNum = -1;
	bool foundPhrase = false;
	if (iWindow->NumElements() > 0) {
		instruction* ins = iWindow->Nth(0);
		ins->findPhraseAncestors();
		int phIndx = findMyPhrase(ins);
		if (isPhraseOld(phIndx) == true) {
			//force close older phrases
			for (int i = 0; i <= phIndx; i++) {
				openPh->Nth(i)->setState(phClosed);
			}
			for (int i = 0; i <= phIndx; i++) {
				closeOldPhrase(0);
			}
		}
		//printf("ins id = %d, %d\n", iWindow->Nth(0)->getInsID(), phIndx);
		iWindow->RemoveAt(0);
	} else if (eof == true || 
		   (iWindow->NumElements() == 0 &&
	           iROB->NumElements() >= ROBsize)) {
		//Mark all phrases closed &
		//Move old phrases to closePh
		for (int i = 0; i < openPh->NumElements(); i++) {
			openPh->Nth(i)->setState(phClosed);
		}
		while (openPh->NumElements() > 0) {
			closeOldPhrase(0);
		}
		if (iWindow->NumElements() == 0 &&
		    iROB->NumElements() >= ROBsize) {
			resetCount++;
		}
	}
	findReadyPhrases(phraseFile, depTables, iROB);

}*/
int phraseGen::runPhraseGen(List<instruction*> *iROB, List<instruction*> *iWindow, FILE* phraseFile, dependencyTable *depTables, bool eof, int ROBsize) {
	if (iWindow->NumElements() > 0) {
		instruction* ins = iWindow->Nth(0);
		ins->findPhraseAncestors();
		int phIndx = findMyPhrase(ins);
		if (isPhraseOld(phIndx) == true) {
			//force close older phrases
			for (int i = 0; i <= phIndx; i++) {
				openPh->Nth(i)->setState(phClosed);
			}
			for (int i = 0; i <= phIndx; i++) {
				closeOldPhrase(0);
			}
		}
		//printf("ins id = %d, %d\n", iWindow->Nth(0)->getInsID(), phIndx);
		iWindow->RemoveAt(0);
	} else if (eof == true || 
		   (iWindow->NumElements() == 0 &&
	           iROB->NumElements() >= ROBsize)) {
		//Mark all phrases closed &
		//Move old phrases to closePh
		for (int i = 0; i < openPh->NumElements(); i++) {
			openPh->Nth(i)->setState(phClosed);
		}
		while (openPh->NumElements() > 0) {
			closeOldPhrase(0);
		}
		if (iWindow->NumElements() == 0 &&
		    iROB->NumElements() >= ROBsize) {
			resetCount++;
		}
	}
	findReadyPhrases(phraseFile, depTables, iROB);
	//for (int i = 0; i < openPh->NumElements(); i++) {
	//	if (openPh->Nth(i)->getPhraseSize() == 1) 
	//		printf("id: %d, %s\n",openPh->Nth(i)->getNthIns(0)->getInsID(), openPh->Nth(i)->getNthIns(0)->getCmdStr());
	//}

	//for (int i = 0; i < closePh->NumElements(); i++) {
	//	printf("\n");
	//	printf("ph: %d#%d - ", closePh->Nth(i)->getPhraseID(), closePh->Nth(i)->getNumAllPhraseAncestors());
	//	for (int j = 0; j < closePh->Nth(i)->getNumAllPhraseAncestors(); j++) {
	//		printf("%d#%d#s, ", closePh->Nth(i)->getNthPhraseAncestor(j)->getInsID(), closePh->Nth(i)->getNthPhraseAncestor(j)->getMyPhraseID());
	//		if (closePh->Nth(i)->getPhraseID() == 568 ||
	//		    closePh->Nth(i)->getPhraseID() == 569)
	//			printf("%s ||| ", closePh->Nth(i)->getNthPhraseAncestor(j)->getCmdStr());
	//	}
	//	printf("\nmembers:\n");
	//	for (int j = 0; j < closePh->Nth(i)->getPhraseSize(); j++) {
	//		if (closePh->Nth(i)->getPhraseID() == 568 ||
	//		    closePh->Nth(i)->getPhraseID() == 569)
	//			printf("%d, %s >>> ", closePh->Nth(i)->getNthIns(j)->getInsID(), closePh->Nth(i)->getNthIns(j)->getCmdStr());
	//	}
	//}
	return closePh->NumElements()+openPh->NumElements();
}

int phraseGen::takeMax(int a, int b) {
	//printf("%d, %d, %d\t|", a,b,c);
	if (a >= b) {
		return a;
	} else {
		return b;
	}
}

int phraseGen::findMyPhrase(instruction* ins) {
	int myPhrase  = -1;
	int myPhrase1 = -1;
	int myPhrase2 = -1;
	static int myPhrase3 = 0;
	//phrase *ph;

	//bool rootPhrase = false;
	//Find the phrase
	if (ins->getNumMemRdAncestors() > 0) {
		for (int j = 0; j < ins->getNumMemRdAncestors(); j++) {
			if (myPhrase1 < ins->getNthMemRdAncestor(j)->getMyPhraseID()) {
				myPhrase1 = ins->getNthMemRdAncestor(j)->getMyPhraseID();
			}
		}
		myPhrase1 += NEXT_PHRASE;
	} 
	if (ins->getNumAncestors() > 0) {
		for (int j = 0; j < ins->getNumAncestors(); j++) {
			if (myPhrase2 < ins->getNthAncestor(j)->getMyPhraseID()) {
				myPhrase2 = ins->getNthAncestor(j)->getMyPhraseID();
			}
		}
	}
	//if (ins->getNumAncestors() == 0 &&
	//    ins->getNumMemRdAncestors() == 0) {
	//	//if (openPh->NumElements() > 0) {
	//	//	if (openPh->Nth(0)->getPhraseID() == FIRST_PHRASE)
	//	//		myPhrase = FIRST_PHRASE;
	//	//	else
	//	//		myPhrase = topPhraseID + NEXT_PHRASE;
	//	//} else {
	//	//	myPhrase = FIRST_PHRASE;
	//	//}
	//	if (myPhrase3 == -1) myPhrase = 0;
	//	else myPhrase = myPhrase3;
	//	rootPhrase = true;
	//}
	if (ins->getDepOnUPLD() == false) {
		myPhrase = myPhrase3;
	} else {
		myPhrase = takeMax(myPhrase1,myPhrase2);
		myPhrase = takeMax(myPhrase3,myPhrase);
	}
	//if (ins->isOnCritiPath(myPhrase) == true) {
	//	//if (myPhrase == myPhrase3) {
	//		myPhrase += NEXT_PHRASE;
	//	//}
	//	//else if (takeMax(myPhrase,myPhrase3) == myPhrase3) { //TODO what happens if the two were equal?
	//	//	myPhrase = myPhrase3;
	//	//} else {
	//	//	myPhrase += NEXT_PHRASE;
	//	//	/*-----STAT-----*/
	//	//	numCritPathViolations++;
	//	//	/*-----STAT-----*/
	//	//}
	//}
	//printf("my phrase id: %d, %d\n", myPhrase, ins->getInsID());
	Assert (myPhrase > -1 && myPhrase >= myPhrase2);

	bool found = false;
	int phIndx;
	while (!found) {
		if (topPhraseID > myPhrase) {
			for (int i = openPh->NumElements()-1; i >= 0; i--) {
				phrase* ph = openPh->Nth(i);
				//if (rootPhrase==true) {
				//	//if (ph->getPhraseID() == myPhrase && 
				//	//    ph->getNumAllPhraseAncestors() == 0) {
				//		phrase* ph = openPh->Nth(0);
				//		ins->setMyPhrase(ph);
				//		ph->addToPhrase(ins);
				//		ph->incPhraseAge();
				//		found = true;
				//		phIndx = 0;
				//		myPhrase3 = ph->getPhraseID();
				//		/*-----STAT-----*/
				//		insCount++;
				//		/*-----STAT-----*/
				//		break;
				//	//}
				//} else 
				if (ph->getPhraseID() == myPhrase) {
					ins->setMyPhrase(ph);
					ph->addToPhrase(ins);
					ph->incPhraseAge();
					found = true;
					phIndx = i;
					if (ins->getDepOnUPLD() == false ||
					    ins->getMissrate() > unpredMemOpThreshold) 
						myPhrase3 = ph->getPhraseID();
					/*-----STAT-----*/
					insCount++;
					/*-----STAT-----*/
					break;
				}
			}
			if (!found) {
				myPhrase += NEXT_PHRASE;
			}
		} else {
			phrase* ph = new phrase(topPhraseID);
			topPhraseID++;
			ins->setMyPhrase(ph);
			ph->addToPhrase(ins);
			ph->incPhraseAge();
			openPh->Append(ph);
			phIndx = openPh->NumElements()-1;
			found = true;
			if (ins->getDepOnUPLD() == false ||
			    ins->getMissrate() > unpredMemOpThreshold) 
				myPhrase3 = ph->getPhraseID();
			/*-----STAT-----*/
			insCount++;
			/*-----STAT-----*/
			//printf("new phrase ID = %d\n", topPhraseID);
		}
	}
	//static int prev = -1;
	//if (prev > myPhrase) printf("%d \n",myPhrase);
	//prev = myPhrase;

	//for (int i = 0; i < openPh->NumElements(); i++) {
	//	printf("%d, ", openPh->Nth(i)->getPhraseSize_unsort());
	//}
	//printf("\n");

	//Add ins to its phrase
	//int phIndx;
	//if (openPh->NumElements() > myPhrase) {
	//	ph = openPh->Nth(myPhrase);
	//	phIndx = myPhrase;
	//	Assert(myPhrase == openPh->Nth(myPhrase)->getPhraseID());
	//} else {
	//	printf("my phrase = %d, %d\n", myPhrase,openPh->NumElements());
	//	ph = new phrase(myPhrase);
	//	openPh->Append(ph);
	//	Assert(myPhrase == openPh->Nth(openPh->NumElements()-1)->getPhraseID());
	//	phIndx = openPh->NumElements()-1;
	//	//Assert(myPhrase == openPh->NumElements()-1);
	//}
	//ins->setMyPhrase(ph);
	//ph->addToPhrase(ins);
	//ph->incPhraseAge();
	//printf("phrase ID = %d\n", phIndx);
	//printf("find my phrase\n");

	return phIndx;
}

void phraseGen::closeOldPhrase(int phIndx) {
	//Add closed phrase to closePh &
	//Remove close phrase from openPh
	phrase* ph = openPh->Nth(phIndx);
	//printf("close old phrase %d\n", ph->getPhraseID());
	if (ph->getState() == phClosed) {
		closePh->Append(ph);
		openPh->RemoveAt(phIndx);
	}
}

void phraseGen::findReadyPhrases(FILE* phraseFile, dependencyTable *depTables, List<instruction*> *iROB) {
	bool hadReadyPh=true;
	while (hadReadyPh==true) {
		hadReadyPh=false;
		for (int i = 0; i < closePh->NumElements();) {
			//printf("find ready phraes\n");
			if (isPhraseReady(i) == true) {
				hadReadyPh = true;
				//printf("found ready phraes (id:%d)\n", closePh->Nth(i)->getPhraseID());
				if (coreType == FRAGMENT || coreType == FRAGMENT2) {
					closePh->Nth(i)->makeFragment();
				}
				genPhraseStat(i);
				storeToFile(i, phraseFile, depTables);
				commitIns(-1,iROB);
				break; //TODO remove this? this is oldest ready first
			} else {
				i++;
			}
		}
	}
}

void phraseGen::storeToFile(int indx, FILE* phraseFile, dependencyTable *depTables) {
	phrase* ph = closePh->Nth(indx);
	if (coreType == FRAGMENT || coreType == FRAGMENT2) {
		ph->printToFileFragment(phraseFile);
		while (ph->getPhraseSize() > 0) {
			ph->getNthIns(0)->notifyMyDepPhrasesICompleted();
			ph->getNthIns(0)->setStatus(complete, -1, -1);
			ph->removeFromPhrase(0);
		}
		delete closePh->Nth(indx);
		closePh->RemoveAt(indx);
	} else if (coreType == PHRASE) {
		//ph->VLIWphrase(0,numFU);
		ph->printToFilePhrase(phraseFile);
		while (ph->getPhraseSize() > 0) {
			ph->getNthIns(0)->notifyAllDepICompleted(); //NOTE: doesn't remove unpred. ld op ancestors
			ph->getNthIns(0)->notifyMyDepPhrasesICompleted();
			ph->getNthIns(0)->delDepTableEntris(depTables, -1, true); //assuming always perfect register renaming (04/24/13)
			ph->getNthIns(0)->setStatus(complete, -1, -1);
			//delete ph->getNthIns(0);
			ph->removeFromPhrase(0);
		}
		delete closePh->Nth(indx);
		closePh->RemoveAt(indx);
		//printf("store to file\n");
	}
}

bool phraseGen::isPhraseOld(int phIndx) {
	//printf("is phrase old?\n");
	phrase* ph = openPh->Nth(phIndx);
	Assert(ph->getState() == phOpen);

	if (ph->getPhraseAge() == phraseSizeBound) {
		ph->setState(phClosed);
		for (int i = 0; i <= phIndx; i++) {
			openPh->Nth(i)->setState(phClosed);
		}
		return true;
	} else {
		return false;
	}
	printf("is phrase old?2\n");
}

bool phraseGen::isPhraseReady(int phIndx) {
	phrase* ph = closePh->Nth(phIndx);
	Assert(ph->getState() == phClosed);
	//printf(">>> %d\n", ph->getPhraseID());

	if (ph->getNumAllPhraseAncestors() == 0) {
		ph->setState(phReady);
		return true;
	} else {
		return false;
	}
}

int phraseGen::commitIns (int cycle, List<instruction*> *iROB) {
	if (iROB->NumElements() > 0) {
		for (int i = iROB->NumElements()-1; i >= 0; i--) {
			if ((iROB->Nth(i))->getStatus() == complete) {
				//printf("commiting instructions\n");
				//Report execution timing
				//if (reportTrace == true) reportInsTiming(0);
				//if (reportTraceAndHitMiss == true) createTraceAndHitMiss(0);
				//Remove item
				delete iROB->Nth(i);
				iROB->RemoveAt(i); //Commit
				//if (debug) printf("completed something - Window Size = %d\n", iROB->NumElements());
			} //else {
			//	//if (debug) 
			//	break;
			//}
		}
		return 0;
	} else {
		printf("The iROB is empty\n");
		return -1;
	}
}

void phraseGen::genPhraseStat(int indx) {
	phrase* ph = closePh->Nth(indx);
	totNumPhraseAncestors += ph->getNumPhraseAncestors(); //TODO this line is broken
	totNumRootIns	      += ph->getNumRootIns();
	totNumPhUnpredMemOp   += ph->getNumUPLDops();
	totNumSoftBounds      += ph->getNumSoftBounds();
	totNumFragments       += ph->getNumFrags();
	//printf("%ld, %ld, %ld, %ld, %ld, %ld\n", ph->getNumFrags(), 
	//					 ph->getNumUPLDancestors(), 
	//					 ph->getNumUPLDops(), 
	//					 ph->getNumRootIns(),
	//					 ph->getPhraseSize(),
	//					 ph->getNumPhraseAncestors());
	if (ph->getNumUPLDancestors() == 0) totRootPhrases++;

	//printf("*************PHRASE SUMMARY*******************\n");
	//printf("Num Phrases: %d\n", topPhraseID);
	//printf("Num of Resetting Times: %d\n", resetCount);
	//printf("Phrase Sizes: \n");
	//for (int i = 0; i < openPh->NumElements(); i++) {
	//	printf("%d, ", openPh->Nth(i)->getPhraseSize());
	//}
	//for (int i = 0; i < closePh->NumElements(); i++) {
	//	printf("%d, ", closePh->Nth(i)->getPhraseSize());
	//}

	//printf("\nPhrase Num Unpred-Mem Op Ancestors: \n");
	//for (int i = 0; i < openPh->NumElements(); i++) {
	//	printf("%d, ", openPh->Nth(i)->getNumPhraseAncestors());
	//}
	//for (int i = 0; i < closePh->NumElements(); i++) {
	//	printf("%d, ", closePh->Nth(i)->getNumPhraseAncestors());
	//}
	//
	//printf("\nPhrase Num All Ancestors: \n");
	//for (int i = 0; i < openPh->NumElements(); i++) {
	//	printf("%d, ", openPh->Nth(i)->getNumAllPhraseAncestors());
	//}
	//for (int i = 0; i < closePh->NumElements(); i++) {
	//	printf("%d, ", closePh->Nth(i)->getNumAllPhraseAncestors());
	//}
	//printf("\n******************************************\n");
}

long int phraseGen::getTotNumFrags () {
	return totNumFragments;
}

long int phraseGen::getTotNumRootPh () {
	return totRootPhrases;
}

long int phraseGen::getTotNumPhrases() {
	return topPhraseID;
}

long int phraseGen::getTotNumSoftBound() {
	return totNumSoftBounds;
}

long int phraseGen::getTotNumPhraseUPLD() {
	return totNumPhUnpredMemOp;
}

long int phraseGen::getTotNumRootIns() {
	return totNumRootIns;
}

long int phraseGen::getTotNumPhraseAncestors() {
	return totNumPhraseAncestors;
}

long int phraseGen::getNumPhGenReset() {
	return resetCount;
}

long int phraseGen::getTotNumIns() {
	return insCount;
}

long int phraseGen::getTotNumCritPathViolations() {
	return numCritPathViolations;
}
/*
int phraseGen::runPhraseGen_MixANDcap(List<instruction*> *iROB, List<instruction*> *iWindow, FILE* phraseFile, dependencyTable *depTables) {
	while(iWindow->NumElements() > 0) {
		int phraseNum = -1;
		bool foundPhrase = false;
		instruction* ins = iWindow->Nth(0);
		ins->findPhraseAncestors();

		//Oldest phrase first scheme (TODO must be changed)
		for (int j =  0; j < openPh->NumElements(); j++) {
			phrase *ph = openPh->Nth(j);
			ph->insVisitCounter();
			setupClosePh(ph, j);
			if (ph->chkMemRdAncestors(ins) == true) {
				ph->addToPhrase(ins);
				foundPhrase = true;
				phraseNum = j; //TODO remove
				setupClosePh(ph, j);
				break;
			}
			setupClosePh(ph, j);
		}
		if (foundPhrase == false) {
			phrase *ph = new phrase(openPh->NumElements()+1);
			ph->addToPhrase(ins);
			ph->insVisitCounter();
			openPh->Append(ph);
			phraseNum = openPh->NumElements()-1;
		}
		refreshPhraseList();
		printf("ins id = %d, %d\n", iWindow->Nth(0)->getInsID(), phraseNum);
		iWindow->RemoveAt(0);
	}
	for (int i = 0; i < openPh->NumElements(); i++) {
		if (openPh->Nth(i)->getPhraseSize() == 1) 
			printf("id: %d, %s\n",openPh->Nth(i)->getNthIns(0)->getInsID(), openPh->Nth(i)->getNthIns(0)->getCmdStr());
	}
	return openPh->NumElements();

}
*/
