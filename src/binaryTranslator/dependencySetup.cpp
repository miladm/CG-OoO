/*******************************************************************************
 *  dependencySetup.h
 ******************************************************************************/

#include "dependencySetup.h"

static void findEntryPoints (List<basicblock*> *bbList, List<basicblock*> *interiorBB) {
	for (int i = 0; i < bbList->NumElements (); i++) {	
		basicblock* bb = bbList->Nth (i);
		if (bb->getNumAncestors () == 0 || bb->numNonBackEdgeAncestors () == 0) {
			interiorBB->Append (bb);
		}
	}
}

static void dependency (basicblock* bb) {
    if (bb->isVisited ()) return;
	dependencyTable* depTables = new dependencyTable; /* DISABLED */
    bb->setAsVisited ();
    List<instruction*>* insList = bb->getInsList ();
    for (int i = 0; i < insList->NumElements (); i++) {
        instruction* ins  = insList->Nth (i);
        ins->dependencyTableCheck (depTables); /* DISABLED */
    }
    bb->brDependencyTableCheck ();
    delete depTables;
    for (int i = 0; i < bb->getNumDescendents (); i++)
        dependency (bb->getNthDescendent (i));
    for (int i = 0; i < bb->getNumAncestors (); i++)
        dependency (bb->getNthAncestor (i));
}

void dependencySetup (List<basicblock*>* bbList) {
	List<basicblock*>* interiorBB = new List<basicblock*>;

	printf ("\tFind Graph Entry Points\n");
	findEntryPoints (bbList, interiorBB);

	for (int i = 0; i < bbList->NumElements (); i++)
		bbList->Nth (i)->setAsUnvisited ();
	for (int i = 0; i < interiorBB->NumElements (); i++) {
		basicblock* bbHead = interiorBB->Nth (i);
		dependency (bbHead);
	}
	for (int i = 0; i < bbList->NumElements (); i++) {
		if (!bbList->Nth (i)->isVisited ()) 
            printf ("Unvisited BB ID: %llx,%d,%d\n", bbList->Nth (i)->getID (), 
                                                     bbList->Nth (i)->getNumAncestors (), 
                                                     bbList->Nth (i)->numNonBackEdgeAncestors ());
		bbList->Nth (i)->setAsUnvisited ();
	}

	delete interiorBB;
}
