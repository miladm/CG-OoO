/*******************************************************************************
 *  make_phraseblock.cpp
 ******************************************************************************/

#include <algorithm>
#include "make_phraseblock.h"
#include "listSchedule.h"

void replicateBBList(List<basicblock*>* bbList, List<basicblock*>* newBbList) {
	for (int i = 0; i < bbList->NumElements(); i++) {
		basicblock* newBB = new basicblock;
		*newBB = *(bbList->Nth(i));
		newBbList->Append(newBB);
	}
	for (int i = 0; i < newBbList->NumElements(); i++) {
		newBbList->Nth(i)->transferPointersToNewList(newBbList);
	}
}

void resetVisitBits(List<basicblock*> *bbList) {
	for (int i = 0; i < bbList->NumElements(); i++) {
		bbList->Nth(i)->setAsUnvisited();
	}
}

void findLoop(basicblock* bb, loop* lp, map<ADDR,basicblock*> &domList, ADDR startLoopID, ADDR endLoopID) {
	if (bb->isVisited() == true) return;
	bb->setAsVisited();
	//if (domList.find(bb->getID()) != domList.end()) {
		printf("%llx, ", bb->getID());
		lp->addBB(bb);
	//}
	if (bb->getID() != startLoopID) {
		for (int i = 0; i < bb->getNumAncestors(); i++) {
			basicblock *parent = bb->getNthAncestor(i);
			findLoop(parent, lp, domList, startLoopID, endLoopID);
		}
	} else {
		//printf("==%llx, %d\n", bb->getID(), bb->getNumBackEdgeSource());
		for (int i = 0; i < bb->getNumBackEdgeSource(); i++) {
			basicblock *parent = bb->getNthBackEdgeSource(i);
			findLoop(parent, lp, domList, startLoopID, endLoopID);
		}
	}
}

void makeNaturalLoops(List<basicblock*>* bbList, List<loop*>* loopList, set<ADDR> &loopHeaders) {
	printf("\tstart of natural loops\n");
	//Generate backedges
	for (int i = 0; i < bbList->NumElements(); i++) {
		basicblock* bb = bbList->Nth(i);
		if (bb->getBbSize() == 0) continue;
		bb->setupBackEdge();
	}

	//Find loops
	for (int i = 0; i < bbList->NumElements(); i++) {
		basicblock* bb = bbList->Nth(i);
		if (bb->getBbSize() == 0) continue;
		if (bb->isBackEdge() == true) {
			ADDR startLoopID = bb->getBackEdgeDest();
			ADDR endLoopID = bb->getLastIns()->getInsAddr();
			if (loopHeaders.find(startLoopID) != loopHeaders.end()) 
				continue;
			else
				loopHeaders.insert(startLoopID);
			loop* lp = new loop(startLoopID, endLoopID);
			loopList->Append(lp);
			printf("(%llx, %llx): ", startLoopID, endLoopID);
			map<ADDR,basicblock*> domList = bb->getDominators();
			resetVisitBits(bbList);
			findLoop(bb, lp, domList, startLoopID, endLoopID);
			printf("\n");
		}
	}
	
	//Find loop fallthrough paths
	for (int i = 0; i < loopList->NumElements(); i++) {
		loopList->Nth(i)->findFallThroughBBs();
	}
}

void makeLoopNests(List<loop*> *loopList, set<ADDR> &loopHeaders) {
	for (int i = 0; i < loopList->NumElements(); i++) {
		loop* lp = loopList->Nth(i);
		for (int j = 0; j < loopList->NumElements(); j++) {
			if (i == j) continue;
			if (lp->isInnerLoop(loopList->Nth(j)) == true) {
				//TODO do something here nest loops or something?
			}
		}
	}
}

void mk_phraseblock(List<phraseblock*>* pbList, List<loop*> *loopList, set<ADDR> &loopHeaders, List<basicblock*>* newBbList) {
	//TODO: must sort the loops based on nest level - LATER
	//Phraseblocks from loops
	ADDR* phID = new ADDR;
	(*phID) = 0;
	for (int i = 0; i < loopList->NumElements(); i++) {
		loop* lp = loopList->Nth(i);
		phraseblock* pb = new phraseblock ();
		pb->loopToPhraseblock(lp);
		pb->PhraseblockToBB(newBbList, lp, phID);
		pbList->Append(pb);
	}
	//Phraseblock from the rest of program
}

void mk_phrase(List<basicblock*>* newBbList) {
	for (int i = 0; i < newBbList->NumElements(); i++) {
		basicblock* bb = newBbList->Nth(i);
		bb->basicblockToPhrase();
	}
}

void listSchedule_phraseblock(List<basicblock*> *pbList) {
	//Perform list Scheduling on PB's only. BB's are already list-scheduled
	for (int i = 0; i < pbList->NumElements(); i++) {
		basicblock* bb = pbList->Nth(i);
		if (bb->isAPhraseblock())
			listSchedule(bb);
	}
}
//===============================//////////////////
//TODOOOOO: make sure each instruction knows about the ID of its phraeblock (IMPORTANT - I have tried to do it in PhraseblockToBB func.. is that all I must do?)
////////////////////////////////
List<basicblock*>* make_phraseblock(List<basicblock*>* bbList,
									map<ADDR, double> *brBiasMap,
									map<ADDR, double> *bpAccuracyMap) {
	List<phraseblock*>* pbList   = new List<phraseblock*>;
	List<basicblock*>* newBbList = new List<basicblock*>;
	List<loop*>* loopList        = new List<loop*>;
	set<ADDR> loopHeaders;
	printf("\treplicate basiblocks\n");
	replicateBBList(bbList, newBbList);
	printf("\tmake natural loops\n");
	makeNaturalLoops(newBbList, loopList, loopHeaders);
	printf("\tmake loop nests\n");
	makeLoopNests(loopList, loopHeaders);
	printf("\tmake phraseblock\n");
	resetVisitBits(newBbList);
	mk_phraseblock(pbList, loopList, loopHeaders, newBbList);
	printf("\tlist schedule phraseblock\n");
	listSchedule_phraseblock(newBbList);
	printf("\tmake phrase\n");
	mk_phrase(newBbList);
	delete loopList;
	return newBbList;
}

