/*******************************************************************************
 *  dependencySetup.h
 ******************************************************************************/

#include "dependencySetup.h"

static dependencyTable* depTables;

static void findEntryPoints (List<basicblock*> *bbList, List<basicblock*> *interiorBB) {
	for (int i = 0; i < bbList->NumElements (); i++) {	
		basicblock* bb = bbList->Nth (i);
		if (bb->getNumAncestors () == 0 || bb->numNonBackEdgeAncestors () == 0) {
			interiorBB->Append (bb);
		}
	}
}

static void ctrl_dependency (basicblock* bb) {
    if (bb->isVisited ()) return;
    bb->setAsVisited ();
    bb->brDependencyTableCheck ();
    for (int i = 0; i < bb->getNumDescendents (); i++)
        ctrl_dependency (bb->getNthDescendent (i));
    for (int i = 0; i < bb->getNumAncestors (); i++)
        ctrl_dependency (bb->getNthAncestor (i));
}

static void data_dependency (basicblock* bb) {
    if (bb->isVisited ()) return;
    bb->setAsVisited ();
    List<instruction*>* insList = bb->getInsList ();
    for (int i = 0; i < insList->NumElements (); i++) {
        instruction* ins  = insList->Nth (i);
        ins->dependencyTableCheck (depTables); /* DISABLED */
    }
    bb->brDependencyTableCheck ();
    for (int i = 0; i < bb->getNumDescendents (); i++)
        data_dependency (bb->getNthDescendent (i));
    for (int i = 0; i < bb->getNumAncestors (); i++)
        data_dependency (bb->getNthAncestor (i));
}

static void upld_annotation (List<instruction*>* insList) {
    bool change = true;
    while (change) {
        change = false;
        for (int i = 0; i < insList->NumElements (); i++) {
            instruction* ins = insList->Nth (i);
            bool changed = ins->updateUPLDbit ();
            if (changed) change = true;
        }
    }
}

void dependencySetup (List<basicblock*>* bbList, List<instruction*>* insList) {
	List<basicblock*>* interiorBB = new List<basicblock*>;
	depTables = new dependencyTable; /* DISABLED */

	printf ("\tFind Graph Entry Points\n");
	findEntryPoints (bbList, interiorBB);

    /* DEPENDENCY TABLE SETUP */
	printf ("\tSetup data dependency tables\n");
	for (int i = 0; i < bbList->NumElements (); i++)
		bbList->Nth (i)->setAsUnvisited ();
	for (int i = 0; i < interiorBB->NumElements (); i++) {
		basicblock* bbHead = interiorBB->Nth (i);
		data_dependency (bbHead);
	}
	for (int i = 0; i < bbList->NumElements (); i++) {
		if (!bbList->Nth (i)->isVisited ()) 
            printf ("Unvisited BB ID: %llx,%d,%d\n", bbList->Nth (i)->getID (), 
                                                     bbList->Nth (i)->getNumAncestors (), 
                                                     bbList->Nth (i)->numNonBackEdgeAncestors ());
		bbList->Nth (i)->setAsUnvisited ();
	}

    /* ANNOTATION OF UPLD OPERATIONS */
	printf ("\tAnnotate UPLD's\n");
	upld_annotation (insList);

    /* DEPENDENCY TABLE SETUP */
	printf ("\tSetup control dependency tables\n");
	for (int i = 0; i < interiorBB->NumElements (); i++) {
		basicblock* bbHead = interiorBB->Nth (i);
		ctrl_dependency (bbHead);
	}
	for (int i = 0; i < bbList->NumElements (); i++) {
		if (!bbList->Nth (i)->isVisited ()) 
            printf ("Unvisited BB ID: %llx,%d,%d\n", bbList->Nth (i)->getID (), 
                                                     bbList->Nth (i)->getNumAncestors (), 
                                                     bbList->Nth (i)->numNonBackEdgeAncestors ());
		bbList->Nth (i)->setAsUnvisited ();
	}

    delete depTables;
	delete interiorBB;
}
