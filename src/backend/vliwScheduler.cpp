/*******************************************************************************
 *  vliwScheduler.cpp
 *  vliwScheduler object. schedules the instructions into a VLIW schedule
 ******************************************************************************/


#include "vliwScheduler.h"

extern core coreType;
extern dependencyTable *depTables;
extern int cacheLat[MEM_HIGHERARCHY];
extern int insCount;
extern int cycle;

vliwScheduler::vliwScheduler() {
	rootALUins = new List<instruction*>;
	rootMEMins = new List<instruction*>;
	outListTemp = new List<instruction*>;
	rootInsIndx = new List<int>;
	rootSize = 0;
	phID = -1;
	tempReg = -100;
}

vliwScheduler::~vliwScheduler() {
	delete rootALUins;
	delete rootMEMins;
	delete rootInsIndx;
	delete outListTemp;
}

void vliwScheduler::findMostCritIns(List<instruction*> *list, int cycle, bool UPLDhoisting) {
	if (list->NumElements() == 0) return;
	instruction* critIns = list->Nth(0);
	int critIndx = 0;
	int critPathLen = critIns->findLongestPathDynamicly(cycle, UPLDhoisting);
	for (int i = 1; i < list->NumElements(); i++) {
		int tempCritPathLen = list->Nth(i)->findLongestPathDynamicly(cycle, UPLDhoisting);
		if (critPathLen < tempCritPathLen) {
		    critIns = list->Nth(i);
		    critIndx = i;
		    critPathLen = tempCritPathLen;
		}
	}
	list->RemoveAt(critIndx);
	list->InsertAt(critIns,0);
}

void vliwScheduler::sortLists(List<instruction*> *rootALUins, List<instruction*> *rootMEMins, int cycle, bool UPLDhoisting, long int phraseID) {
	quicksortLongestPathDyanmic(rootALUins,0,rootALUins->NumElements()-1,cycle, UPLDhoisting);
	//findMostCritIns(rootALUins, cycle, UPLDhoisting); //TODO this line must be replaced wth the line above when the 1ALU unit process is done
	findMostCritIns(rootMEMins, cycle, UPLDhoisting);
	//rootSize += rootMEMins->NumElements();

	//printf("\n");
	//for (int i = 0; i < rootALUins->NumElements(); i++)
	//	printf("%d, ",rootALUins->Nth(i)->findLongestPath(rootALUins->Nth(i)->getInsID()), UPLDhoisting);
}

/* List scheduling for multiple FU (Fragments only) */
bool vliwScheduler::scheduleIns(List<instruction*>* inList, List<instruction*>* outList, int cycle, bool UPLDhoisting, long int phraseID) {
	for (int i = 0; i < inList->NumElements(); i++) {
		if (inList->Nth(i)->isReady(cycle) == true) {
			inList->Nth(i)->findLongestPath(cycle, UPLDhoisting, phraseID);
		}
	}
	quicksortLongestPath(rootALUins,0,rootALUins->NumElements()-1,cycle, UPLDhoisting);
	while (inList->NumElements() > 0 || rootALUins->NumElements() > 0 || rootMEMins->NumElements() > 0 || outListTemp->NumElements() > 0) {
		cycle++;
		for (int i = 0; i < inList->NumElements(); i++) {
			instruction* ins = inList->Nth(i);
			if (ins->isReady(cycle) == true) {
				if (ins->getType() == ALU || ins->getType() == FPU) {
					rootALUins->Append(ins);
					rootInsIndx->Append(i);
				} else if (ins->getType() == MEM) {
					rootMEMins->Append(ins);
					rootInsIndx->Append(i);
				} else {
					printf("WARNING: THERE IS A PROBLEM1!\n");
					exit(1);
				}
			}
		}
		for (int j = rootInsIndx->NumElements()-1; j >= 0; j--) {
			int indx = rootInsIndx->Nth(j);
			inList->RemoveAt(indx);
			rootInsIndx->RemoveAt(j);
		}
		Assert(rootInsIndx->NumElements() == 0);
		int j=0;
		for (j = 0; j < NUM_MEM_UNIT && rootMEMins->NumElements() > 0; j++) {
			outList->Append(rootMEMins->Nth(0));
			outListTemp->Append(rootMEMins->Nth(0));
			rootMEMins->Nth(0)->setStatus(execute, cycle, cacheLat[0]-1); //excute state has no meaning here (just an internal setting)
			rootMEMins->RemoveAt(0);
		}
		for (j = 0; j < NUM_ALU_UNIT && rootALUins->NumElements() > 0; j++) {
			outList->Append(rootALUins->Nth(0));
			outListTemp->Append(rootALUins->Nth(0));
			if (rootALUins->Nth(0)->getType() == ALU) {
				rootALUins->Nth(0)->setStatus(execute, cycle, ALU_LATENCY-1); //excute state has no meaning here (just an internal setting)
			} else if (rootALUins->Nth(0)->getType() == FPU) {
				rootALUins->Nth(0)->setStatus(execute, cycle, FPU_LATENCY-1); //excute state has no meaning here (just an internal setting)
			}
			rootALUins->RemoveAt(0);
		}
		completeIns (cycle,outListTemp,false);
	}
	if (rootALUins->NumElements() > 1000 || rootMEMins->NumElements() > 1000)
		return false;
	else
		return true;
}

/* List scheduling for single FU (Fragments only) */
bool vliwScheduler::scheduleIns_1FU(List<instruction*>* inList, List<instruction*>* outList, int cycle, bool UPLDhoisting, long int phraseID) {
	for (int i = 0; i < inList->NumElements(); i++) {
		if (inList->Nth(i)->isReady(cycle) == true) {
			inList->Nth(i)->findLongestPath(cycle, UPLDhoisting, phraseID);
		}
	}
	quicksortLongestPath(rootALUins,0,rootALUins->NumElements()-1,cycle, UPLDhoisting);
	while (inList->NumElements() > 0 || rootALUins->NumElements() > 0 || outListTemp->NumElements() > 0) {
		cycle++;
		for (int i = 0; i < inList->NumElements(); i++) {
			instruction* ins = inList->Nth(i);
			if (ins->isReady(cycle) == true) {
				rootALUins->Append(ins);
				rootInsIndx->Append(i);
			}
		}
		for (int j = rootInsIndx->NumElements()-1; j >= 0; j--) {
			int indx = rootInsIndx->Nth(j);
			inList->RemoveAt(indx);
			rootInsIndx->RemoveAt(j);
		}
		Assert(rootInsIndx->NumElements() == 0);
		int j=0;
		for (j = 0; j < 1 && rootALUins->NumElements() > 0; j++) {
			outList->Append(rootALUins->Nth(0));
			outListTemp->Append(rootALUins->Nth(0));
			if (rootALUins->Nth(0)->getType() == MEM) {
				rootALUins->Nth(0)->setStatus(execute, cycle, cacheLat[0]-1); //excute state has no meaning here (just an internal setting)
			} else if (rootALUins->Nth(0)->getType() == ALU) {
				rootALUins->Nth(0)->setStatus(execute, cycle, ALU_LATENCY-1); //excute state has no meaning here (just an internal setting)
			} else if (rootALUins->Nth(0)->getType() == FPU) {
				rootALUins->Nth(0)->setStatus(execute, cycle, FPU_LATENCY-1); //excute state has no meaning here (just an internal setting)
			} else {
				Assert(rootALUins->Nth(0)->getType() == FPU || 
				       rootALUins->Nth(0)->getType() == ALU || 
				       rootALUins->Nth(0)->getType() == MEM);
			}
			rootALUins->RemoveAt(0);
		}
		completeIns (cycle,outListTemp, false);
		Assert(outListTemp->NumElements() == 0);
	}
	if (rootALUins->NumElements() > 1000)
		return false;
	else
		return true;
}

/* List scheduling for multiple FU (instruction streaming support -  no Fragments) */
bool vliwScheduler::scheduleInsStream(List<instruction*>* inList, int cycle, bool UPLDhoisting, FILE* reScheduleFile, float unpredMemOpThreshold) {
	bool foundBR = false;
	for (int i = 0; i < inList->NumElements(); i++) {
		instruction* ins = inList->Nth(i);
		if (foundBR == false && ins->getType() == BR) {foundBR = true;}
		if (ins->isReady(cycle) == true) {
			if (ins->getType() == ALU || ins->getType() == FPU || ins->getType() == BR || ins->getType() == ASSIGN) {
				rootALUins->Append(ins);
				rootInsIndx->Append(i);
				//Inject new instructions
				//if (foundBR) {
				//	instruction *newIns = setupNewIns(ins, tempReg, cycle);
				//	tempReg--;
				//	inList->InsertAt(newIns, i+1);
				//}
			} else if (ins->getType() == MEM) {
				rootMEMins->Append(ins);
				rootInsIndx->Append(i);
				//Inject new instructions
				//if (foundBR) {
				//	instruction *newIns = setupNewIns(ins, tempReg, cycle);
				//	tempReg--;
				//	inList->InsertAt(newIns, i+1);
				//}
			} else {
				printf("WARNING: THERE IS A PROBLEM2!\n");
				exit(1);
			}
		}
	}
	for (int j = rootInsIndx->NumElements()-1; j >= 0; j--) {
		int indx = rootInsIndx->Nth(j);
		inList->RemoveAt(indx);
		rootInsIndx->RemoveAt(j);
	}
	Assert(rootInsIndx->NumElements() == 0);
	sortLists(rootALUins,rootMEMins, cycle, UPLDhoisting, -1);
	int j=0;
	for (j = 0; j < NUM_MEM_UNIT && rootMEMins->NumElements() > 0; j++) {
		rootMEMins->Nth(0)->printToFile(reScheduleFile, UPLDhoisting);//if UPLD hoisting is active, so are missRates
		outListTemp->Append(rootMEMins->Nth(0));
		if (UPLDhoisting == true && rootMEMins->Nth(0)->getMissrate() > unpredMemOpThreshold) {
			rootMEMins->Nth(0)->setStatus(execute, cycle, cacheLat[2]-1); //excute state has no meaning here (just an internal setting)
		} else {
			rootMEMins->Nth(0)->setStatus(execute, cycle, cacheLat[0]-1); //excute state has no meaning here (just an internal setting)
		}
		rootMEMins->RemoveAt(0);
	}
	for (j = 0; j < NUM_ALU_UNIT && rootALUins->NumElements() > 0; j++) {
		rootALUins->Nth(0)->printToFile(reScheduleFile, UPLDhoisting);//if UPLD hoisting is active, so are missRates
		outListTemp->Append(rootALUins->Nth(0));
		if (rootALUins->Nth(0)->getType() == ALU) {
			rootALUins->Nth(0)->setStatus(execute, cycle, ALU_LATENCY-1); //excute state has no meaning here (just an internal setting)
		} else if (rootALUins->Nth(0)->getType() == FPU) {
			rootALUins->Nth(0)->setStatus(execute, cycle, FPU_LATENCY-1); //excute state has no meaning here (just an internal setting)
		} else if (rootALUins->Nth(0)->getType() == BR) {
			rootALUins->Nth(0)->setStatus(execute, cycle, ALU_LATENCY-1); //excute state has no meaning here (just an internal setting)
		} else if (rootALUins->Nth(0)->getType() == ASSIGN) {
			rootALUins->Nth(0)->setStatus(execute, cycle, ASSIGN_LATENCY-1); //excute state has no meaning here (just an internal setting)
		}
		rootALUins->RemoveAt(0);
	}
	completeIns (cycle,outListTemp, true);
	if (rootALUins->NumElements() > 220 || rootMEMins->NumElements() > 220)
		return false;
	else
		return true;
}

/* List scheduling for multiple FU (instruction streaming support -  no Fragments) */
bool vliwScheduler::scheduleInsStream_1FU(List<instruction*>* inList, int cycle, bool UPLDhoisting, FILE* reScheduleFile) {
	for (int i = 0; i < inList->NumElements(); i++) {
		instruction* ins = inList->Nth(i);
		if (ins->isReady(cycle) == true) {
			rootALUins->Append(ins);
			rootInsIndx->Append(i);
		}
	}
	for (int j = rootInsIndx->NumElements()-1; j >= 0; j--) {
		int indx = rootInsIndx->Nth(j);
		inList->RemoveAt(indx);
		rootInsIndx->RemoveAt(j);
	}
	Assert(rootInsIndx->NumElements() == 0);
	findMostCritIns(rootALUins, cycle, UPLDhoisting);
	int j=0;
	for (j = 0; j < 1 && rootALUins->NumElements() > 0; j++) {
		rootALUins->Nth(0)->printToFile(reScheduleFile, UPLDhoisting);//if UPLD hoisting is active, so are missRates
		outListTemp->Append(rootALUins->Nth(0));
		if (rootALUins->Nth(0)->getType() == MEM) {
			rootALUins->Nth(0)->setStatus(execute, cycle, cacheLat[0]-1); //excute state has no meaning here (just an internal setting)
		} else if (rootALUins->Nth(0)->getType() == ALU) {
			rootALUins->Nth(0)->setStatus(execute, cycle, ALU_LATENCY-1); //excute state has no meaning here (just an internal setting)
		} else if (rootALUins->Nth(0)->getType() == FPU) {
			rootALUins->Nth(0)->setStatus(execute, cycle, FPU_LATENCY-1); //excute state has no meaning here (just an internal setting)
		} else {
			Assert(rootALUins->Nth(0)->getType() == FPU || 
			       rootALUins->Nth(0)->getType() == ALU || 
			       rootALUins->Nth(0)->getType() == MEM);
		}
		rootALUins->RemoveAt(0);
	}
	completeIns (cycle,outListTemp,true);
	if (rootALUins->NumElements() > 220)
		return false;
	else
		return true;
}

/* List scheduling for multiple FU (instruction streaming support -  no Fragments) */
bool vliwScheduler::schedulePhraseinsStream(List<instruction*>* inList, int cycle, bool UPLDhoisting, FILE* reScheduleFile, float unpredMemOpThreshold, List<phrase*>* phList) {
	//bool foundBR = false;
	//static int tempReg = -100;
	if (phID == -1) {
		phID = 1;
		phrase* ph = new phrase(phID);
		phList->Append(ph);
	}
	//for (int i = 0; i < inList->NumElements(); i++) {
	//	instruction* ins = inList->Nth(i);
	//	if (foundBR == false && ins->getType() == BR && ins->isReady(cycle) == false) {foundBR = true;}
	//	if (ins->isReady(cycle) == true) {
	//		if (ins->getType() == ALU || ins->getType() == FPU || ins->getType() == BR || ins->getType() == ASSIGN) {
	//			rootALUins->Append(ins);
	//			rootInsIndx->Append(i);
	//			//Inject new instructions
	//			if (foundBR == true) {
	//				instruction *newIns = setupNewIns(ins, tempReg, cycle);
	//				tempReg--;
	//				inList->InsertAt(newIns, i+1);
	//			}
	//		} else if (ins->getType() == MEM) {
	//			rootMEMins->Append(ins);
	//			rootInsIndx->Append(i);
	//			//Inject new instructions
	//			if (foundBR == true) {
	//				instruction *newIns = setupNewIns(ins, tempReg, cycle);
	//				tempReg--;
	//				inList->InsertAt(newIns, i+1);
	//			}
	//		} else {
	//			printf("WARNING: THERE IS A PROBLEM3!\n");
	//			exit(1);
	//		}
	//	}
	//	//if (ins->getType() == BR) break; //Avoid useless iterations - TODO must use it in different vliw-scheduler funcs
	//}
	//for (int j = rootInsIndx->NumElements()-1; j >= 0; j--) {
	//	int indx = rootInsIndx->Nth(j);
	//	inList->RemoveAt(indx);
	//	rootInsIndx->RemoveAt(j);
	//}
	//Assert(rootInsIndx->NumElements() == 0);
	sortLists(rootALUins,rootMEMins, cycle, UPLDhoisting, -1);
	int j=0;
	for (j = 0; j < NUM_MEM_UNIT && rootMEMins->NumElements() > 0; j++) {
		if (rootMEMins->Nth(0)->getNumMemRdAncestors() > 0) {//check for immediate UPLD ops
			for (int i = 0; i < rootMEMins->Nth(0)->getNumMemRdAncestors(); i++) {
				if (phID <= rootMEMins->Nth(0)->getNthMemRdAncestor(i)->getMyPhraseID()) {
					Assert(phID == rootMEMins->Nth(0)->getNthMemRdAncestor(i)->getMyPhraseID());
					phID++; // = rootMEMins->Nth(0)->getNthMemRdAncestor(i)->getMyPhraseID() + 1;
					phrase* ph = new phrase(phID);
					phList->Append(ph);
				}
			}
		}
		//If ther is an older BR op, inject a new ins
		//if (rootMEMins->Nth(0)->getNumBrAncestors() > 0 && rootMEMins->Nth(0)->getType() != BR && rootMEMins->Nth(0)->getMemType() != WRITE && rootMEMins->Nth(0)->getType() != ASSIGN){// && rootMEMins->Nth(0)->getType() != ALU) { //TODO I think this line has the bug!
		if (rootMEMins->Nth(0)->getBrMode() == scheduleBr && rootMEMins->Nth(0)->getNumBrAncestors() > 0 && rootMEMins->Nth(0)->getMemType() == READ && rootMEMins->Nth(0)->getMissrate() > unpredMemOpThreshold) { //TODO I think this line has the bug!
			injectIns(rootMEMins->Nth(0));
		//} else {
		//	printf("ERROR: Invalid dependency chanin detected. Aborting...\n");
		//	exit(-1);
		}
		rootMEMins->Nth(0)->setMyPhraseID(phID);
		phList->Nth(phList->NumElements()-1)->addToPhrase_Light(rootMEMins->Nth(0));
		outListTemp->Append(rootMEMins->Nth(0));
		if (UPLDhoisting == true && rootMEMins->Nth(0)->getMissrate() > unpredMemOpThreshold) {
			rootMEMins->Nth(0)->setStatus(execute, cycle, cacheLat[2]-1); //excute state has no meaning here (just an internal setting)
		} else {
			rootMEMins->Nth(0)->setStatus(execute, cycle, cacheLat[0]-1); //excute state has no meaning here (just an internal setting)
		}
		rootMEMins->RemoveAt(0);
		if (phList->NumElements() > 4) {//Keep the three most outstanding phrases
			if (phList->Nth(0)->isPhraseComplete() == true) {
				phList->Nth(0)->printToFilePhrase(reScheduleFile);
				int size = phList->Nth(0)->getPhraseSize_unsort();
				for (int k = size-1; k >= 0; k--) {
					if (phList->Nth(0)->getInsList_unsort()->Nth(k)->getMissrate() > unpredMemOpThreshold) 
						phList->Nth(0)->getInsList_unsort()->Nth(k)->notifyDepICommited();
					delete phList->Nth(0)->getInsList_unsort()->Nth(k);
					phList->Nth(0)->getInsList_unsort()->RemoveAt(k);
				}
				delete phList->Nth(0);
				phList->RemoveAt(0);
			}
		}
	}
	for (j = 0; j < NUM_ALU_UNIT && rootALUins->NumElements() > 0; j++) {
		if (rootALUins->Nth(0)->getNumMemRdAncestors() > 0) {//check for immediate UPLD ops
			for (int i = 0; i < rootALUins->Nth(0)->getNumMemRdAncestors(); i++) {
				if (phID <= rootALUins->Nth(0)->getNthMemRdAncestor(i)->getMyPhraseID()) {
					Assert(phID == rootALUins->Nth(0)->getNthMemRdAncestor(i)->getMyPhraseID());
					phID++; // = rootALUins->Nth(0)->getNthMemRdAncestor(i)->getMyPhraseID() + 1;
					phrase* ph = new phrase(phID);
					phList->Append(ph);
				}
			}
		}
		//If ther is an older BR op, inject a new ins
		//if (rootALUins->Nth(0)->getNumBrAncestors() > 0 && rootALUins->Nth(0)->getType() != BR && rootALUins->Nth(0)->getMemType() != WRITE && rootALUins->Nth(0)->getType() != ASSIGN){// && rootALUins->Nth(0)->getType() != ALU) { //TODO I think this line has the bug!
		if (rootALUins->Nth(0)->getBrMode() == scheduleBr && rootALUins->Nth(0)->getNumBrAncestors() > 0 && rootALUins->Nth(0)->getMemType() == READ && rootALUins->Nth(0)->getMissrate() > unpredMemOpThreshold) { //TODO I think this line has the bug!
			injectIns(rootALUins->Nth(0));
		//} else {
		//	printf("ERROR: Invalid dependency chanin detected. Aborting...\n");
		//	exit(-1);
		}
		rootALUins->Nth(0)->setMyPhraseID(phID);
		phList->Nth(phList->NumElements()-1)->addToPhrase_Light(rootALUins->Nth(0));
		outListTemp->Append(rootALUins->Nth(0));
		if (rootALUins->Nth(0)->getType() == ALU) {
			rootALUins->Nth(0)->setStatus(execute, cycle, ALU_LATENCY-1); //excute state has no meaning here (just an internal setting)
		} else if (rootALUins->Nth(0)->getType() == FPU) {
			rootALUins->Nth(0)->setStatus(execute, cycle, FPU_LATENCY-1); //excute state has no meaning here (just an internal setting)
		} else if (rootALUins->Nth(0)->getType() == BR) {
			rootALUins->Nth(0)->setStatus(execute, cycle, ALU_LATENCY-1); //excute state has no meaning here (just an internal setting)
		} else if (rootALUins->Nth(0)->getType() == ASSIGN) {
			rootALUins->Nth(0)->setStatus(execute, cycle, ASSIGN_LATENCY-1); //excute state has no meaning here (just an internal setting)
		}
		rootALUins->RemoveAt(0);
		if (phList->NumElements() > 4) {
			if (phList->Nth(0)->isPhraseComplete() == true) {
				phList->Nth(0)->printToFilePhrase(reScheduleFile);
				int size = phList->Nth(0)->getPhraseSize_unsort();
				for (int k = size-1; k >= 0; k--) {
					if (phList->Nth(0)->getInsList_unsort()->Nth(k)->getMissrate() > unpredMemOpThreshold) 
						phList->Nth(0)->getInsList_unsort()->Nth(k)->notifyDepICommited();
					delete phList->Nth(0)->getInsList_unsort()->Nth(k);
					phList->Nth(0)->getInsList_unsort()->RemoveAt(k);
				}
				delete phList->Nth(0);
				phList->RemoveAt(0);
			}
		}
	}
	completeIns (cycle,outListTemp, false);
	if (rootALUins->NumElements() > 220 || rootMEMins->NumElements() > 220)
		return false;
	else
		return true;
}

//Inject new instructions into trace
instruction* vliwScheduler::setupNewIns(instruction* ins, int cycle) {
	instruction* newIns = new instruction;
	newIns->setType(ASSIGN);
	newIns->setInsID(ins->getInsID());
	newIns->setInsAddr(ins->getInsAddr());
	newIns->setPiepelineLat(ASSIGN_LATENCY);
	newIns->setStatus(FETCH, cycle, -1);
	newIns->setStatus(FETCH, cycle, -1);
	newIns->setBrMode(scheduleBr);
	newIns->setReadyLists(rootALUins, rootMEMins);
	newIns->setVliwScheduler(this);
	//Setup Registers
	ins->updateDepTableEntris(depTables, coreType, newIns);
	for (int i = ins->getNumReg()-1; i >= 0; i--) {
		//Transfer write registers
		if (ins->getNthRegType(i) == WRITE) {
			long int temp1 = ins->getNthReg(i);
			int temp2 = ins->getNthRegType(i);
			newIns->setRegister(&temp1, &temp2);
			ins->removeNthRegister(i);
		}
		//Create register dependency link between ins & newIns 
		int type = WRITE;
		ins->setRegister(&tempReg, &type);
		type = READ;
		newIns->setRegister(&tempReg, &type);
	}
	//Setup Dependencies
	while(ins->getNumBrAncestors() > 0) {
		newIns->setAsAncestor(ins->getNthBrAncestor(0));
		newIns->setAsBrAncestor(ins->getNthBrAncestor(0));
		newIns->addDep();
		ins->getNthBrAncestor(0)->setAsDependent(newIns);
		ins->getNthBrAncestor(0)->delBrDependent(ins);
		ins->getNthBrAncestor(0)->setAsBrDependent(newIns);
		ins->delNthBrAncestor(0);
	}
	ins->notifyDepICommited();
	ins->notifyAllDepICompleted_light();
	while(ins->getDependents()->NumElements() > 0) {
		newIns->setAsDependent(ins->getDependents()->Nth(0));
		ins->getDependents()->Nth(0)->setAsAncestor(newIns);
		ins->getDependents()->Nth(0)->addDep();
		ins->getDependents()->RemoveAt(0);
	}
	if (newIns->isRepeated(ins, newIns->_ancestors) == false) {
		newIns->setAsAncestor(ins);
		newIns->addDep();
	}
	ins->setAsDependent(newIns);
	newIns->findPhraseAncestors();
	newIns->setCmdStr("T, "); //TODO incomplete
	if (newIns->isReady(cycle) == true) newIns->goToReadyList(); //Catch root instructions
	tempReg--;
	return newIns;
}

void vliwScheduler::completeIns (int cycle,List<instruction*>* list, bool del) {
	for (int i = list->NumElements()-1; i >= 0; i--) {
		if ((list->Nth(i))->getStatus() == execute && 
		    (list->Nth(i))->getCompleteCycle() <= cycle) {
			//Release dependencies
			list->Nth(i)->notifyAllBrAncestorsICompleted();
			list->Nth(i)->notifyAllDepICompleted();
			list->Nth(i)->delDepTableEntris(depTables, coreType, true); //assuming always perfect register renaming (04/24/13)
			list->Nth(i)->setStatus(complete, -1, -1); //TODO double check this line. Is it compatible with the rest of the program?
			if (del == true) delete list->Nth(i);
			list->RemoveAt(i);
		}
	}
}

void vliwScheduler::delFromInsMap(instruction* ins) {
	if (insMap.count(ins->getInsID()) == 1) {
		insMap.erase(ins->getInsID());
	} else {
		printf("\nERROR: Instruction Map Detected an Invalid Conflict\n");
		exit(-1);
	}
}

void vliwScheduler::insertToInsMap(instruction* newIns) {
	if (insMap.count(newIns->getInsID()) == 0) {
		insMap.insert(pair<long int, instruction*>(newIns->getInsID(),newIns));
	} else {
		printf("\nERROR: Instruction Map Detected an Invalid Conflict\n");
		exit(-1);
	}
}

int vliwScheduler::getSizeOfInsMap() {
	return (int) insMap.size();
}

//Interface for parsing instructions
void vliwScheduler::parseIns (int ROBsize, parser* parse, int cycle) {
	if (insMap.size() < (unsigned int)ROBsize) {
		int diff = ROBsize - insMap.size();
		for (int i = 0; i < diff; i++) {
			instruction *newIns = new instruction;
			if (parse->parseIns(newIns) == false) break; //EOF
			newIns->setReadyLists(rootALUins, rootMEMins);
			newIns->setVliwScheduler(this);
			if (newIns->isReady(cycle) == true) newIns->goToReadyList(); //Catch root instructions
			else insertToInsMap(newIns);
			insCount++;
		}
	}
}

void vliwScheduler::injectIns(instruction* ins) {
	instruction *newIns = setupNewIns(ins, cycle);
	if (newIns->isReady(cycle) == true) newIns->goToReadyList(); //Catch root instructions
	else insertToInsMap(newIns);
}
