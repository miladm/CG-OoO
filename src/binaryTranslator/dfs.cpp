/*******************************************************************************
 *  dfs.cpp
 ******************************************************************************/

#include <set>
#include "dfs.h"

set<ADDR> path_visit_set;

basicblock* findBB(List<basicblock*>* bbList, ADDR addr) {
	for (int i = 0; i < bbList->NumElements(); i++) {
		if (bbList->Nth(i)->getID() == addr) {
			return bbList->Nth(i);
		}
	}
	printf("\tERROR: didn't find the requested BB\n");
	exit(1);
}

void findPath(basicblock* bb, ADDR loopEntryID) {
	if (bb->getID() == loopEntryID ||
	    bb->getNumDescendents() == 0 ||
	    path_visit_set.find(bb->getID()) != path_visit_set.end()) {
		printf("%llx\n", bb->getID());
		path_visit_set.insert(bb->getID());
		return;
	} else {
		for (int i = 0; i < bb->getNumDescendents(); i++) {
			printf("%llx, ", bb->getNthDescendent(i)->getID());
			path_visit_set.insert(bb->getNthDescendent(i)->getID());
			findPath(bb->getNthDescendent(i), loopEntryID);
		}
	}
}

void dfs(List<basicblock*>* bbList, ADDR bbID_seed) {
	basicblock* bb_seed = findBB(bbList, bbID_seed);
	for (int i = 0; i < bb_seed->getNumDescendents(); i++) {
		printf("(%llx)  %llx, ", bbID_seed, bb_seed->getNthDescendent(i)->getID());
		path_visit_set.insert(bb_seed->getID());
		findPath(bb_seed->getNthDescendent(i), bbID_seed);
	}
}
