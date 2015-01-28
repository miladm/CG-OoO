/*******************************************************************************
 *  listSchedule.cpp
 ******************************************************************************/

#include "listSchedule.h"

int findLongestPath(instruction* ins, basicblock* bb) {
	List<instruction*> *_dependents = ins->getDependents();
	int _critPathLen = -1;
	for (int i = 0; i < _dependents->NumElements(); i++) {
		instruction* dep = _dependents->Nth(i);
		int pathLength;
		if (bb->isInsAddrInBB(dep->getInsAddr()) == false) continue;
		if (dep->isLongestPathSet() == true) pathLength = dep->getLongestPath();
		else pathLength = findLongestPath(dep, bb);
		if (_critPathLen < pathLength) _critPathLen = pathLength;
	}
	if (_critPathLen == -1) {
		_critPathLen = ins->getLatency();
	} else {
		_critPathLen += ins->getLatency();
	}
	Assert (_critPathLen > 0);
	return _critPathLen;
}


/* LIST SCHEDULING (FRAGMENTS ONLY) */
void listSchedule(basicblock* bb) {
	List<instruction*> *sortList      = new List<instruction*>;
	List<instruction*> *listSchList   = new List<instruction*>;
	List<instruction*> *toBeSchedList = new List<instruction*>;
	List<instruction*> *insList       = bb->getInsList();
	set<ADDR> scheduledIns;
	bool scheduleIt = false;
	// REPLICATE THE BB INSTRUCTION LIST
	for (int i = 0; i < insList->NumElements(); i++) {
		insList->Nth(i)->resetLongestPath(); //Added to alow Phraesblock listscheduling (not useful for BB-list scheduling)
		sortList->Append(insList->Nth(i));
	}
	// FIND THE LONGEST PATH FOR EVERY INSTRUCTION IN BB
	for (int i = sortList->NumElements()-1; i >= 0; i--) {
		int lp = findLongestPath(sortList->Nth(i), bb);
		sortList->Nth(i)->setLongestPath(lp);
		// printf("lp for ins %llu: %d\n", sortList->Nth(i)->getInsAddr(), lp);
    }
	// printf("\n");
	// SORT INSTRUCTION LIST BASED ON CRITICAL PATH LENGTH (HIGH TO LOW)
	quicksortLongestPath(sortList,0,sortList->NumElements()-1);
	// LIST SCHEDULE INSTRUCTIONS BASED ON THEIR DATA-DEPENDENCY CHAINS.
	while (sortList->NumElements() != scheduledIns.size()) {
		int scheduleCounter = 0;
	    for (int i = 0; i < sortList->NumElements() && scheduleCounter < NUM_EU; i++) {
			instruction* ins = sortList->Nth(i);
			if (scheduledIns.find(ins->getInsAddr()) != scheduledIns.end()) continue;
			List<instruction*>* ancestors = ins->getAncestors();
			scheduleIt = true;
			for (int j = 0; j < ancestors->NumElements(); j++) {
				instruction* anc = ancestors->Nth(j);
				if (bb->isInsAddrInBB(anc->getInsAddr()) == true &&
				    scheduledIns.find(anc->getInsAddr()) == scheduledIns.end()) {
						// printf("hey ");
						scheduleIt = false;
						continue;
				}
			}
			// printf("\n");
			if (scheduleIt == true) {
				listSchList->Append(ins);
				toBeSchedList->Append(ins);
				scheduleCounter++;
			}
	    }
		// printf("To SChedule #: %d\n", toBeSchedList->NumElements());
		for (int i = 0; i < toBeSchedList->NumElements(); i++)
			scheduledIns.insert(toBeSchedList->Nth(i)->getInsAddr());
		toBeSchedList->RemoveAll();
	}
	// STORE THE LIST-SCHEDULED LIST TO BB CLASS
    for (int i = 0; i < listSchList->NumElements(); i++) {
		bb->addToBB_ListSchedule(listSchList->Nth(i));
		// printf("lp for ins %llu: %d\n", listSchList->Nth(i)->getInsAddr(), listSchList->Nth(i)->getLongestPath());
    }
	// printf("\n---------\n");
	delete toBeSchedList;
	delete listSchList;
	delete sortList;
}

