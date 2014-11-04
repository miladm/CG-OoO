/*******************************************************************************
 *  registerAllocate.cpp
 ******************************************************************************/

#include "registerAllocate.h"


int eliminatePhiFuncs (List<basicblock*> *bbList) {
	/* CAUTION:
		After phi-function elimination you must never poke into 
        the end of a BB to search for a branch of some sort.
	*/
	int numInsInsertion = 0;
	for (int i =0 ; i < bbList->NumElements (); i++) {
		numInsInsertion += bbList->Nth (i)->elimPhiFuncs ();
	}
	return numInsInsertion;
}

/* Rename from V_i format to absolute SSA format
   Rebuild the Def/Use Sets
   Eliminate phi-functions
*/
void renameAndbuildDefUseSets (List<basicblock*> *bbList) {
	for (int i =0 ; i < bbList->NumElements (); i++)
		bbList->Nth (i)->renameAllInsRegs ();
}

void findEntryPoints (List<basicblock*> *bbList, List<basicblock*> *interiorBB) {
	for (int i = 0; i < bbList->NumElements (); i++) {	
		basicblock* bb = bbList->Nth (i);
		if (bb->getNumAncestors () == 0 || bb->numNonBackEdgeAncestors () == 0) {
			interiorBB->Append (bb);
		}
	}
}

//TODO why would someone who doesn't write a variable insert itself as a bb writing into it?
void livenessAnalysis (List<basicblock*> *bbList, REG_ALLOC_MODE reg_alloc_mode) {
	bool change = true;
	while (change == true) {
		change = false;
		for (int i = bbList->NumElements () - 1; i >= 0; i--) {	
			basicblock* bb = bbList->Nth (i);
			if (bb->update_InOutSet () == true)
				change = true;
		}
	}

	//CREATE LOCAL SETS FOR IMPLEMENTING LRF REG. ALLCOATION
    if (reg_alloc_mode == LOCAL_GLOBAL) {
        for (int i = 0; i < bbList->NumElements (); i++)
            bbList->Nth (i)->updateLocalRegSet ();
    }
}


void assign_local_registers (map<long int,interfNode*> &locallIntfNodeMap, map<long int,interfNode*> &allIntfNodeMap) {
	if (locallIntfNodeMap.size () == 0) return;
	if (locallIntfNodeMap.size () > 0) {
		vector<interfNode*> removedIntfNodeVector;
		// Step 1: Eliminate Resgisters from Interference Graph
		while (locallIntfNodeMap.size () > 0) {
			int neighborCount = -1;
			map<long int,interfNode*>::iterator candidateNodeIt;
			for (map<long int,interfNode*>::iterator it = locallIntfNodeMap.begin (); it != locallIntfNodeMap.end (); it++) {
				// Remove redundant map elements
					// if ((it->second)->getNeighborSize () == 0) {
					// 	allIntfNodeMap.insert (pair<long int,interfNode*> (it->first,it->second));
					// 	globalIntfNodeMap.erase (it);
					// 	continue;
					// }
				if ((it->second)->getNeighborSize () > neighborCount &&
				    (it->second)->getNeighborSize () < LRF_SIZE) {
						neighborCount =  (it->second)->getNeighborSize ();
						candidateNodeIt = it;
				}
			}
			if (locallIntfNodeMap.size () == 0) {
				break;
			} else if (neighborCount > -1) {
				interfNode* node = candidateNodeIt->second;
				node->removeFromGraph ();
				removedIntfNodeVector.push_back (node);
				// if (allIntfNodeMap.find (candidateNodeIt->first) != allIntfNodeMap.end ()) printf ("\n============================================\n");
				#ifdef DEBUG_RA printf ("adding to ALL: %d\n",candidateNodeIt->first);
				#endif
				allIntfNodeMap.insert (pair<long int,interfNode*> (candidateNodeIt->first,candidateNodeIt->second));
				locallIntfNodeMap.erase (candidateNodeIt);
			} else {
				for (map<long int,interfNode*>::iterator it = locallIntfNodeMap.begin (); it != locallIntfNodeMap.end (); it++) {
					printf ("remaining nodes size: %d\n", (it->second)->getNeighborSize ());
				}
				Assert (0 && "no candidate neighbors found.");
			}
		}
		// Step 2: Color Registers (reg assignment)
		set<long int> LRFset;
		for (int i = LRF_LO; i <= LRF_HI; i++) LRFset.insert (i);
		for (int i = removedIntfNodeVector.size ()-1; i >= 0; i--) {
			interfNode *node = removedIntfNodeVector.at (i);
			node->assignReg (LRFset);
			// printf ("%d\n", node->getReg ());
		}
	}
	// printf ("\tDEBUG: Completed register assignment for one network.\n");
}

void assign_global_registers (map<long int,interfNode*> &locallIntfNodeMap, map<long int,interfNode*> &globalIntfNodeMap, map<long int,interfNode*> &allIntfNodeMap) {
	if (globalIntfNodeMap.size () == 0 && locallIntfNodeMap.size () == 0) return;
	// printf ("2 map size: %d\n", globalIntfNodeMap.size ());
	if (globalIntfNodeMap.size () > 0) {
		vector<interfNode*> removedIntfNodeVector;
		// Step 1: Eliminate Resgisters from Interference Graph
		while (globalIntfNodeMap.size () > 0) {
			int neighborCount = -1;
			map<long int,interfNode*>::iterator candidateNodeIt;
			for (map<long int,interfNode*>::iterator it = globalIntfNodeMap.begin (); it != globalIntfNodeMap.end (); it++) {
				// Remove redundant map elements
				// if ((it->second)->getNeighborSize () == 0) {
				// 	allIntfNodeMap.insert (pair<long int,interfNode*> (it->first,it->second));
				// 	globalIntfNodeMap.erase (it);
				// 	continue;
				// }
				if ((it->second)->getNeighborSize () > neighborCount &&
				    (it->second)->getNeighborSize () < GRF_SIZE) {
						neighborCount = (it->second)->getNeighborSize ();
						candidateNodeIt = it;
				}
			}
			if (globalIntfNodeMap.size () == 0) {
				break;
			} else if (neighborCount > -1) {
				interfNode* node = candidateNodeIt->second;
				node->removeFromGraph ();
				removedIntfNodeVector.push_back (node);
				#ifdef DEBUG_RA printf ("adding to ALL: %d\n",candidateNodeIt->first); 
				#endif
				allIntfNodeMap.insert (pair<long int,interfNode*> (candidateNodeIt->first,candidateNodeIt->second));
				globalIntfNodeMap.erase (candidateNodeIt);
			} else {
				for (map<long int,interfNode*>::iterator it = globalIntfNodeMap.begin (); it != globalIntfNodeMap.end (); it++) {
					printf ("remaining nodes size: %d\n", (it->second)->getNeighborSize ());
				}
				Assert (0 && "no candidate neighbors found.");
			}
		}
		// Step 2: Color Registers (reg assignment)
		set<long int> GRFset;
		for (int i = GRF_LO; i <= GRF_HI; i++) GRFset.insert (i);
		for (int i = removedIntfNodeVector.size ()-1; i >= 0; i--) {
			interfNode *node = removedIntfNodeVector.at (i);
			node->assignReg (GRFset);
			// printf ("%d\n", node->getReg ());
		}
	}
}

int liveMaxSize = 0;
void make_interference_nodes_network (basicblock* bb, map<long int,interfNode*> &globalIntfNodeMap, map<long int,interfNode*> &locallIntfNodeMap, map<long int,interfNode*> &allIntfNodeMap, REG_ALLOC_MODE reg_alloc_mode) {
    if (!bb->isVisited ()) {
        bb->setAsVisited ();
        List<instruction*>* insList = bb->getInsList ();
        for (int i = 0; i <  insList->NumElements (); i++) {
            instruction* ins = insList->Nth (i);
            set<long int> defSet = ins->getDefSet ();
            set<long int> inSet = ins->getInSet ();
            set<long int> localSet = ins->getLocalRegSet ();
            set<long int> liveSet, defSet_noLocal;
            // printf ("%llx - inSet size: %d\n", bb->getID (), inSet.size ());
            if (reg_alloc_mode == LOCAL_GLOBAL) {
                set_difference (defSet.begin (), defSet.end (), localSet.begin (), localSet.end (), std::inserter (defSet_noLocal, defSet_noLocal.begin ()));
                set_union (defSet_noLocal.begin (), defSet_noLocal.end (), inSet.begin (), inSet.end (), std::inserter (liveSet, liveSet.begin ()));
            } else {
                set_union (defSet.begin (), defSet.end (), inSet.begin (), inSet.end (), std::inserter (liveSet, liveSet.begin ()));
            }
            //TODO - debug - to remove
            // set<long int> test;
            // set_intersection (liveSet.begin (), liveSet.end (), localSet.begin (), localSet.end (), std::inserter (test, test.begin ()));
            // printf ("test set: %d, %d, %d\n", liveSet.size (), localSet.size (), test.size ());
            // For each live value, connect the node to all other live nodes at that BB
            if (liveSet.size () > 75) {
                cout << hex << bb->getID () << dec << " " << liveSet.size () << " " << inSet.size () << " " << defSet.size () << " " << bb->getBbSize () << endl;
            }
            if (liveMaxSize < liveSet.size ()) liveMaxSize = liveSet.size ();
            for (set<long int>::iterator it = liveSet.begin (); it != liveSet.end (); it++) {
                if (globalIntfNodeMap.find (*it) == globalIntfNodeMap.end ()) {
                    interfNode *IntfNd = new interfNode (*it);
                    globalIntfNodeMap.insert (pair<long int,interfNode*> (*it,IntfNd));
                }
                interfNode *defNode = globalIntfNodeMap[*it];
                for (set<long int>::iterator it_live = liveSet.begin (); it_live != liveSet.end (); it_live++) {
                    if (*it != *it_live) { /* AVOID EDGES TO SELF */
                        if (globalIntfNodeMap.find (*it_live) == globalIntfNodeMap.end ()) {
                            interfNode *IntfNd = new interfNode (*it_live);
                            globalIntfNodeMap.insert (pair<long int,interfNode*> (*it_live,IntfNd));
                        }
                        interfNode *node = globalIntfNodeMap[*it_live];
                        defNode->addEdge (node);
                    }
                }
            }
            if (reg_alloc_mode == LOCAL_GLOBAL) {
                // For each local value, connect the node to all other local nodes at that BB
                for (set<long int>::iterator it = localSet.begin (); it != localSet.end (); it++) {
                    if (locallIntfNodeMap.find (*it) != locallIntfNodeMap.end ()) 
                        printf ("OMG. This value already exists: %llx\n", *it);
                }
                for (set<long int>::iterator it = localSet.begin (); it != localSet.end (); it++) {
                    if (locallIntfNodeMap.find (*it) == locallIntfNodeMap.end ()) {
                        interfNode *IntfNd = new interfNode (*it);
                        locallIntfNodeMap.insert (pair<long int,interfNode*> (*it,IntfNd));
                    }
                    interfNode *defNode = locallIntfNodeMap[*it];
                    for (set<long int>::iterator it_live = localSet.begin (); it_live != localSet.end (); it_live++) {
                        if (*it != *it_live) { /* AVOID EDGES TO SELF */
                            if (locallIntfNodeMap.find (*it_live) == locallIntfNodeMap.end ()) {
                                interfNode *IntfNd = new interfNode (*it_live);
                                locallIntfNodeMap.insert (pair<long int,interfNode*> (*it_live,IntfNd));
                            }
                            interfNode *node = locallIntfNodeMap[*it_live];
                            defNode->addEdge (node);
                        }
                    }
                }
                //=======
                //Do register allocation for local registerrs
                //Problem to solve: it looks like for some strange reason
                //local registers do conflict across BB's this solution, avoids that.
                // (I don't yet know what the cause of the conflict is)
                assign_local_registers (locallIntfNodeMap,allIntfNodeMap);
                locallIntfNodeMap.clear ();
                //=======
                for (map<long int,interfNode*>::iterator it = globalIntfNodeMap.begin (); it != globalIntfNodeMap.end (); it++) {
                    if (locallIntfNodeMap.find (it->first) != locallIntfNodeMap.end ()) printf ("-SHIIIIIIIIT+++++++ %d\n", it->first);
                }
            }
            // printf ("CALLING DESCENDENTS\n");
            for (int i = 0; i < bb->getNumDescendents (); i++)
                make_interference_nodes_network (bb->getNthDescendent (i), globalIntfNodeMap, locallIntfNodeMap, allIntfNodeMap, reg_alloc_mode);//TODO should it not be a BFS instead of DFS? 
            for (int i = 0; i < bb->getNumAncestors (); i++)
                make_interference_nodes_network (bb->getNthAncestor (i), globalIntfNodeMap, locallIntfNodeMap, allIntfNodeMap, reg_alloc_mode);//TODO should it not be a BFS instead of DFS? 
        }
    }
	// printf ("1 map size: %d\n", globalIntfNodeMap.size ());
}

void set_arch_reg_for_all_ins (basicblock* bb, map<long int,interfNode*> &globalIntfNodeMap) {
    if (!bb->isRegAllocated ()) {
        // printf ("3 map size: %d\n", globalIntfNodeMap.size ());
        List<instruction*>* insList = bb->getInsList ();
        for (int i = 0; i <  insList->NumElements (); i++) {
            instruction* ins = insList->Nth (i);
            bool isAlreadyAssigned = false;
            for (int j = 0; j < ins->getNumReg (); j++) {
                // printf (" (%llx) reg type: %d, %d, ", bb->getID (), ins->getNthRegType (j), ins->getNthReg (j));
                isAlreadyAssigned = ins->isAlreadyAssignedArcRegs ();
                if (isAlreadyAssigned) break;
                if (globalIntfNodeMap.find (ins->getNthReg (j)) == globalIntfNodeMap.end ()) break; //TODO replace this with next line
                // Assert (globalIntfNodeMap.find (ins->getNthReg (j)) != globalIntfNodeMap.end ());
                long int reg = ((globalIntfNodeMap.find (ins->getNthReg (j)))->second)->getReg ();
                // printf ("arch reg: %d\n", reg);
                ins->setArchReg (reg);
                //assign reg to proper instruction obj. if already assigned, break out of bb
                //at dump time make sure no reg is left unassigned
            }
            if (isAlreadyAssigned) return; //skip bb
        }
        bb->setRegAllocated ();
        for (int i = 0; i < bb->getNumDescendents (); i++)
            set_arch_reg_for_all_ins (bb->getNthDescendent (i), globalIntfNodeMap);//TODO should it not be a BFS instead of DFS? 
        for (int i = 0; i < bb->getNumAncestors (); i++)
            set_arch_reg_for_all_ins (bb->getNthAncestor (i), globalIntfNodeMap);//TODO should it not be a BFS instead of DFS? 
    }
}

void allocate_register (List<basicblock*> *bbList, List<instruction*> *insList, REG_ALLOC_MODE reg_alloc_mode) {
	List<basicblock*> *interiorBB = new List<basicblock*>;
	map<long int,interfNode*> locallIntfNodeMap, globalIntfNodeMap, allIntfNodeMap;
	printf ("\tPhi-Function Elimination\n");
	int numInsInsertion = eliminatePhiFuncs (bbList);
	printf ("\tSSA Rename & Build Def/Use Set\n");
	renameAndbuildDefUseSets (bbList); //TODO: make sure this step does not impact next step
	printf ("\tLiveness Analysis\n");
	livenessAnalysis (bbList, reg_alloc_mode);
	printf ("\tFind Graph Entry Points\n");
	findEntryPoints (bbList, interiorBB);
	//TODO is the block below okay? needed?
	for (int i = 0; i < bbList->NumElements (); i++)
		bbList->Nth (i)->setAsUnvisited ();
	printf ("\tBuild Interference Graph & Run Register Assignment\n");
	for (int i = 0; i < interiorBB->NumElements (); i++) {
		basicblock* bbHead = interiorBB->Nth (i);
		locallIntfNodeMap.clear ();
		globalIntfNodeMap.clear ();
		allIntfNodeMap.clear ();
		make_interference_nodes_network (bbHead, globalIntfNodeMap, locallIntfNodeMap,allIntfNodeMap, reg_alloc_mode);
		for (map<long int,interfNode*>::iterator it = locallIntfNodeMap.begin (); it != locallIntfNodeMap.end (); it++) {
			if (globalIntfNodeMap.find (it->first) != globalIntfNodeMap.end ()) printf ("*SHIIIIIIIIT+++++++\n");
		}
		for (map<long int,interfNode*>::iterator it = globalIntfNodeMap.begin (); it != globalIntfNodeMap.end (); it++) {
			if (locallIntfNodeMap.find (it->first) != locallIntfNodeMap.end ()) printf ("SHIIIIIIIIT+++++++\n");
		}
		// printf ("sizes: %d,%d,%d\n",allIntfNodeMap.size (),globalIntfNodeMap.size (),locallIntfNodeMap.size ());
		assign_global_registers (locallIntfNodeMap, globalIntfNodeMap, allIntfNodeMap);
		// printf ("sizes: %d,%d,%d\n",allIntfNodeMap.size (),globalIntfNodeMap.size (),locallIntfNodeMap.size ());
		set_arch_reg_for_all_ins (bbHead, allIntfNodeMap);
	}
	for (int i = 0; i < bbList->NumElements (); i++) {	
		basicblock* bb = bbList->Nth (i);
		// Assert (bb->isVisited ()); //all BB must be visited by now //TODO put this back ASAP
		bb->setAsUnvisited ();
		// printf ("%llx, %d\n", bb->getID (),	bb->getLiveVarSize ());
		// bb->getLiveVarSize ();
	}
	printf ("\tNumber of Phi MOV instructions: %d added to the %d static program instructions\n",numInsInsertion,insList->NumElements ());
    cout << "max number of live values: " << dec << liveMaxSize << endl;
	delete interiorBB;
}
