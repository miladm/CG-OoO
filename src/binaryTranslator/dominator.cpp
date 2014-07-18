/*******************************************************************************
 *  dominator.cpp
 ******************************************************************************/

#include "dominator.h"


/* Setup basicblock dominator sets */
void build_dominators(List<basicblock*>* bbList) {
	for (int i = 0; i < bbList->NumElements(); i++) {
		basicblock* n = bbList->Nth(i);
		if (n->getNumAncestors() == 0 || n->numNonBackEdgeAncestors() == 0) {
			bool change = n->setDominators();
			n->markAsEntryPoint();
			n->setAllasDominators(false);
		} else {
			// bool change = n->setDominators(bbList); this approach is too mem expensive
			n->setAllasDominators(true);
			bool change = true;
		}
	}
	bool change = true;
	while(change == true) {
		change = false;
		for (int i = 0; i < bbList->NumElements(); i++) {
			basicblock* n = bbList->Nth(i);
			if (n->isEntryPoint()) continue;
			int indx;
			bool foundFirstAncestor = false;
			map<ADDR,basicblock*> intersection;
			for (indx = 0; indx < n->getNumAncestors(); indx++) {
				if (n->getNthAncestor(indx)->getAllasDominators() == true) continue;
				intersection = n->getNthAncestor(indx)->getDominators();
				if (intersection.find(n->getID()) != intersection.end()) {
					intersection.clear();
					continue;
				}
				foundFirstAncestor = true;
				break;
			}
			if (foundFirstAncestor == true) {
				for (int j = indx+1; j < n->getNumAncestors(); j++) {
					if (n->getNthAncestor(j)->getAllasDominators() == true) continue;
					map<ADDR,basicblock*> out, nthAncestorDom;
					nthAncestorDom = n->getNthAncestor(j)->getDominators();
					if (nthAncestorDom.find(n->getID()) != nthAncestorDom.end()) {
						nthAncestorDom.clear();
						continue;
					}
					set_intersection(intersection.begin(),intersection.end(),nthAncestorDom.begin(),nthAncestorDom.end(),std::inserter(out, out.begin()));
					intersection = out;
				}
				if (change == false) change = n->setDominators(intersection);
				else 			  bool temp = n->setDominators(intersection);
			} else {
				if (change == false) change = n->setDominators();
				else		      bool temp = n->setDominators();
			}
			n->setAllasDominators(false);
		}
	} //TODO: I must have a start node (the start node at the moment may always have an ancestor - due to loops)
	#ifdef DEBUG_DOM
	for (int i = 0; i < bbList->NumElements(); i++) {
		map<ADDR,basicblock*> dom = bbList->Nth(i)->getDominators();
		printf("dom of %llx: ",bbList->Nth(i)->getID());
		for (map<ADDR,basicblock*>::iterator it = dom.begin(); it != dom.end(); it++) {
			printf("%llx, ",it->first);
		}
		printf("\n");
	}
	#endif
}

void build_strict_dominators(List<basicblock*>* bbList) {
	for (int i = 0; i < bbList->NumElements(); i++)
		bbList->Nth(i)->buildSDominators();
	#ifdef DEBUG_DOM
	for (int i = 0; i < bbList->NumElements(); i++) {
		map<ADDR,basicblock*> dom = bbList->Nth(i)->getDominators();
		printf("strict dom of %llx: ",bbList->Nth(i)->getID());
		for (map<ADDR,basicblock*>::iterator it = dom.begin(); it != dom.end(); it++) {
			if (bbList->Nth(i)->isASDominator(it->first)) printf("%llx, ",it->first);
		}
		printf("\n");
	}
	#endif
}

/* 
 * Setup Dominator Tree 
 * This pass requires build_dominators() to run first
 */
void build_idom(List<basicblock*>* bbList) {
	for (int i = 0; i < bbList->NumElements(); i++)
		bbList->Nth(i)->buildImmediateDominators();
}

/* Setup dominator tree */
void build_dominator_tree(List<basicblock*>* bbList) {
	for (int i = 0; i < bbList->NumElements(); i++)
		bbList->Nth(i)->buildDomTree();
	#ifdef DEBUG_DOM
	for (int i = 0; i < bbList->NumElements(); i++) {
		map<ADDR,basicblock*> dom = bbList->Nth(i)->getDominators();
		printf("idom of %llx: ",bbList->Nth(i)->getID());
		for (map<ADDR,basicblock*>::iterator it = dom.begin(); it != dom.end(); it++) {
			if (bbList->Nth(i)->isInIDom(it->first)) printf("%llx, ",it->first);
		}
		printf("\n");
	}
	#endif
}

void findDomTreeBottomSet (List<basicblock*>* bbList, map<ADDR, basicblock*> &domTree_Bottoms) {
	for (int i = 0; i < bbList->NumElements(); i++) {
		basicblock *bb = bbList->Nth(i);
		if (bb->getChildrenSize() == 0)
			domTree_Bottoms.insert(pair<ADDR,basicblock*>(bb->getID(), bb));
	}
}

void build_dominance_frontier (List<basicblock*>* bbList) {
	map<ADDR, basicblock*> domTree_Bottoms;
	findDomTreeBottomSet(bbList, domTree_Bottoms);
	for (map<ADDR, basicblock*>::iterator it = domTree_Bottoms.begin(); it != domTree_Bottoms.end(); it++) {
		basicblock *bb = it->second;
		 /* local DF Detection */
		for (int i = 0; i < bb->getNumDescendents(); i++) {
			basicblock* descendent = bb->getNthDescendent(i);
			if (descendent->isInIDom(bb->getID()) == false)
				bb->addToDFset(descendent);
		}
		 /* up DF Detection */
		map<ADDR,basicblock*> children = bb->getChildren();
		for (map<ADDR,basicblock*>::iterator it1 = children.begin(); it1 != children.end(); it1++) {
			basicblock *child = it1->second;
			map<ADDR,basicblock*> childDF = child->getDF();
			for (map<ADDR,basicblock*>::iterator it2 = childDF.begin(); it2 != childDF.end(); it2++) {
				basicblock* y = it2->second;
				if (y->isInIDom(bb->getID()) == false)
					bb->addToDFset(y);
			}
		}
	}
}

void setup_dominance_frontier(List<basicblock*>* bbList) {
	printf("\tBuild Dominators\n");
	build_dominators(bbList);
	printf("\tBuild Strict Dominators\n");
	build_strict_dominators(bbList);
	printf("\tBuild Immediate Dominators\n");
	build_idom(bbList);
	printf("\tBuild Dominance Tree\n");
	build_dominator_tree(bbList);
	printf("\tBuild Dominance Frontier\n");
	build_dominance_frontier(bbList);
}
