/*******************************************************************************
 *  registerAllocate_sb.cpp
 ******************************************************************************/

#include "registerAllocate_sb.h"


/* CAUTION: 
 * AFTER PHI-FUNCTION ELIMINATION YOU MUST NEVER POKE INTO THE END OF A BB
 * TO SEARCH FOR A BRANCH OF SOME SORT.
 */
static int eliminatePhiFuncs (List<basicblock*> *bbList, map<ADDR,instruction*> *insAddrMap) {
	int numInsInsertion = 0;
    ADDR phiAddrOffset = 0;
	for (int i =0 ; i < bbList->NumElements (); i++) {
		numInsInsertion += bbList->Nth (i)->elimPhiFuncs (phiAddrOffset, insAddrMap);
	}
	return numInsInsertion;
}

/* RENAME FROM V_I FORMAT TO ABSOLUTE SSA FORMAT REBUILD THE DEF/USE SETS
 * ELIMINATE PHI-FUNCTIONS
 */
static void renameAndbuildDefUseSets (List<basicblock*> *bbList) {
	for (int i =0 ; i < bbList->NumElements (); i++)
		bbList->Nth (i)->renameAllInsRegs ();
}

static void findEntryPoints (List<basicblock*> *bbList, List<basicblock*> *interiorBB) {
	for (int i = 0; i < bbList->NumElements (); i++) {	
		basicblock* bb = bbList->Nth (i);
		if (bb->getNumAncestors () == 0 || bb->numNonBackEdgeAncestors () == 0) {
			interiorBB->Append (bb);
		}
	}
}

//TODO why would someone who doesn't write a variable insert itself as a bb writing into it?
static void livenessAnalysis (List<basicblock*> *bbList, REG_ALLOC_MODE reg_alloc_mode) {
	bool change = true;
	while (change == true) {
		change = false;
		for (int i = bbList->NumElements () - 1; i >= 0; i--) {	
			basicblock* bb = bbList->Nth (i);
			if (bb->update_InOutSet (reg_alloc_mode) == true)
				change = true;
		}
	}

	// CREATE LOCAL SETS FOR IMPLEMENTING LRF REG. ALLCOATION
//    if (reg_alloc_mode == LOCAL_GLOBAL) { TODO this should be fully removed.
//        for (int i = 0; i < bbList->NumElements (); i++)
//            bbList->Nth (i)->updateLocalRegSet ();
//    }
}

static void assign_local_registers (map<long int,interfNode*> &locallIntfNodeMap, 
                             map<long int,interfNode*> &allIntfNodeMap) {
	if (locallIntfNodeMap.size () == 0) return;
	if (locallIntfNodeMap.size () > 0) {
		vector<interfNode*> removedIntfNodeVector;

		// STEP 1: ELIMINATE RESGISTERS FROM INTERFERENCE GRAPH
		while (locallIntfNodeMap.size () > 0) {
			int neighborCount = -1;
			map<long int,interfNode*>::iterator candidateNodeIt;
            map<long int,interfNode*>::iterator it;
			for (it = locallIntfNodeMap.begin (); it != locallIntfNodeMap.end (); it++) {
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
//				#ifdef DEBUG_RA 
//                printf ("adding to ALL: %d\n", candidateNodeIt->first);
//				#endif
				allIntfNodeMap.insert (pair<long int,interfNode*> (candidateNodeIt->first,candidateNodeIt->second));
				locallIntfNodeMap.erase (candidateNodeIt);
			} else {
                map<long int,interfNode*>::iterator it;
                for (it = locallIntfNodeMap.begin (); it != locallIntfNodeMap.end (); it++) {
                    printf ("Remaining local nodes size: %d\n", (it->second)->getNeighborSize ());
                    if ((it->second)->getNeighborSize () > neighborCount) {
                        neighborCount = (it->second)->getNeighborSize ();
                        candidateNodeIt = it;
                    }
                }
                if (locallIntfNodeMap.size () == 0) {
                    break;
                } else if (neighborCount > -1) {
                    interfNode* node = candidateNodeIt->second;
                    node->removeFromGraph ();
                    removedIntfNodeVector.push_back (node);
                    allIntfNodeMap.insert (pair<long int,interfNode*> (candidateNodeIt->first,candidateNodeIt->second));
                    locallIntfNodeMap.erase (candidateNodeIt);
                }
			}
		}

		// STEP 2: COLOR REGISTERS (REG ASSIGNMENT)
		set<long int> LRFset;
		for (int i = LRF_LO; i <= LRF_HI; i++) LRFset.insert (i);
		while (removedIntfNodeVector.size () > 0) {
			interfNode *node = removedIntfNodeVector.back ();
			node->assignReg (LRFset);
            removedIntfNodeVector.pop_back ();
		}
	}
}

static void assign_global_registers (map<long int,interfNode*> &locallIntfNodeMap, 
                              map<long int,interfNode*> &globalIntfNodeMap, 
                              map<long int,interfNode*> &allIntfNodeMap) {
	if (globalIntfNodeMap.size () == 0 && locallIntfNodeMap.size () == 0) return;
	if (globalIntfNodeMap.size () > 0) {
		vector<interfNode*> removedIntfNodeVector;

		// STEP 1: ELIMINATE RESGISTERS FROM INTERFERENCE GRAPH
		while (globalIntfNodeMap.size () > 0) {
			int neighborCount = -1;
			map<long int,interfNode*>::iterator candidateNodeIt;
            map<long int,interfNode*>::iterator it;
			for (it = globalIntfNodeMap.begin (); it != globalIntfNodeMap.end (); it++) {
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
//				#ifdef DEBUG_RA 
//                printf ("adding to ALL: %d\n", candidateNodeIt->first); 
//				#endif
				allIntfNodeMap.insert (pair<long int,interfNode*> (candidateNodeIt->first,candidateNodeIt->second));
				globalIntfNodeMap.erase (candidateNodeIt);
			} else {
                map<long int,interfNode*>::iterator it;
                for (it = globalIntfNodeMap.begin (); it != globalIntfNodeMap.end (); it++) {
                    printf ("Remaining global nodes size: %d\n", (it->second)->getNeighborSize ());
                    if ((it->second)->getNeighborSize () > neighborCount) {
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
                    allIntfNodeMap.insert (pair<long int,interfNode*> (candidateNodeIt->first,candidateNodeIt->second));
                    globalIntfNodeMap.erase (candidateNodeIt);
                }
			}
		}

		// STEP 2: COLOR REGISTERS (REG ASSIGNMENT)
		set<long int> GRFset;
		for (int i = GRF_LO; i <= GRF_HI; i++) GRFset.insert (i);
		while (removedIntfNodeVector.size () > 0) {
			interfNode *node = removedIntfNodeVector.back ();
			node->assignReg (GRFset);
            removedIntfNodeVector.pop_back ();
		}
	}
}

static int liveGlbMaxSize = 0, liveLclMaxSize = 0;
static void make_interference_nodes_network (basicblock* bb, map<long int,interfNode*> &globalIntfNodeMap, map<long int,interfNode*> &locallIntfNodeMap, map<long int,interfNode*> &allIntfNodeMap, REG_ALLOC_MODE reg_alloc_mode) {
    if (!bb->isVisited ()) {
        bb->setAsVisited ();
        List<instruction*>* insList = bb->getInsList ();
        for (int i = 0; i <  insList->NumElements (); i++) {
            instruction* ins = insList->Nth (i);
            set<long int> inSet = ins->getInSet ();
            set<long int> outSet = ins->getOutSet ();
            set<long int> defSet = ins->getDefSet ();
            set<long int> localSet = ins->getLocalRegSet ();
            set<long int> liveSet, defSet_noLocal, inSet_noLocal;

            /* BUILD THE LIVESET */
            if (reg_alloc_mode == LOCAL_GLOBAL) {
                set_difference (defSet.begin (), defSet.end (), 
                                localSet.begin (), localSet.end (), 
                                std::inserter (defSet_noLocal, defSet_noLocal.begin ()));
                set_difference (inSet.begin (), inSet.end (), 
                                localSet.begin (), localSet.end (), 
                                std::inserter (inSet_noLocal, inSet_noLocal.begin ()));
                set_difference (outSet.begin (), outSet.end (), 
                                localSet.begin (), localSet.end (), 
                                std::inserter (liveSet, liveSet.begin ()));
            } else {
                liveSet = ins->getOutSet ();
            }

            /* BUILD THE INTERFERENCE GRAPH OF A NODE */
            Assert (liveSet.size () <= GRF_SIZE && "Global register allocation spilling in not supported");
            if (liveGlbMaxSize < liveSet.size ()) liveGlbMaxSize = liveSet.size ();
            for (set<long int>::iterator it = liveSet.begin (); it != liveSet.end (); it++) {
                if (globalIntfNodeMap.find (*it) == globalIntfNodeMap.end ()) {
                    interfNode *IntfNd = new interfNode (*it);
                    globalIntfNodeMap.insert (pair<long int,interfNode*> (*it,IntfNd));
                }
                interfNode *defNode = globalIntfNodeMap[*it];
                set<long int>::iterator it_live;
                for (it_live = liveSet.begin (); it_live != liveSet.end (); it_live++) {
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
                /* HANDLE LONELY NODES TO AVOID PROGRAM FAILURE - A HACK */
                for (set<long int>::iterator it = inSet_noLocal.begin (); it != inSet_noLocal.end (); it++) {
                    if (globalIntfNodeMap.find (*it) == globalIntfNodeMap.end ()) {
                        interfNode *IntfNd = new interfNode (*it);
                        globalIntfNodeMap.insert (pair<long int,interfNode*> (*it,IntfNd));
                    }
                }
                for (set<long int>::iterator it = defSet_noLocal.begin (); it != defSet_noLocal.end (); it++) {
                    if (globalIntfNodeMap.find (*it) == globalIntfNodeMap.end ()) {
                        interfNode *IntfNd = new interfNode (*it);
                        globalIntfNodeMap.insert (pair<long int,interfNode*> (*it,IntfNd));
                    }
                    interfNode *defNode = globalIntfNodeMap[*it];
                    set<long int>::iterator it_def;
                    for (it_def = defSet_noLocal.begin (); it_def != defSet_noLocal.end (); it_def++) {
                        if (*it != *it_def) { /* AVOID EDGES TO SELF */
                            if (globalIntfNodeMap.find (*it_def) == globalIntfNodeMap.end ()) {
                                interfNode *IntfNd = new interfNode (*it_def);
                                globalIntfNodeMap.insert (pair<long int,interfNode*> (*it_def,IntfNd));
                            }
                            interfNode *node = globalIntfNodeMap[*it_def];
                            defNode->addEdge (node);
                        }
                    }
                }

                if (localSet.size () > LRF_SIZE ) cout << "local: " << bb->getID () << " " << localSet.size () << endl;
                if (liveLclMaxSize < localSet.size ()) liveLclMaxSize = liveSet.size ();
                Assert (localSet.size () <= LRF_SIZE && "Local register allocation spilling in not supported");
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
            } else {
                for (set<long int>::iterator it = inSet.begin (); it != inSet.end (); it++) {
                    if (globalIntfNodeMap.find (*it) == globalIntfNodeMap.end ()) {
                        interfNode *IntfNd = new interfNode (*it);
                        globalIntfNodeMap.insert (pair<long int,interfNode*> (*it,IntfNd));
                    }
                }
                for (set<long int>::iterator it = defSet.begin (); it != defSet.end (); it++) {
                    if (globalIntfNodeMap.find (*it) == globalIntfNodeMap.end ()) {
                        interfNode *IntfNd = new interfNode (*it);
                        globalIntfNodeMap.insert (pair<long int,interfNode*> (*it,IntfNd));
                    }
                    interfNode *defNode = globalIntfNodeMap[*it];
                    set<long int>::iterator it_def;
                    for (it_def = defSet.begin (); it_def != defSet.end (); it_def++) {
                        if (*it != *it_def) { /* AVOID EDGES TO SELF */
                            if (globalIntfNodeMap.find (*it_def) == globalIntfNodeMap.end ()) {
                                interfNode *IntfNd = new interfNode (*it_def);
                                globalIntfNodeMap.insert (pair<long int,interfNode*> (*it_def,IntfNd));
                            }
                            interfNode *node = globalIntfNodeMap[*it_def];
                            defNode->addEdge (node);
                        }
                    }
                }
            }
        }

        if (reg_alloc_mode == LOCAL_GLOBAL) {
            /* CHECK FOR CONFLICTS IN THE TWO INTEREFERENCE NETWORKS */
            map<long int,interfNode*>::iterator it;
            for (it = locallIntfNodeMap.begin (); it != locallIntfNodeMap.end (); it++) {
                if (globalIntfNodeMap.find (it->first) != globalIntfNodeMap.end ())
                    printf ("\t\tERROR: register conflict between local & global interference networks %d\n", it->first);
            }
            for (it = globalIntfNodeMap.begin (); it != globalIntfNodeMap.end (); it++) {
                if (locallIntfNodeMap.find (it->first) != locallIntfNodeMap.end ())
                    printf ("\t\tERROR: register conflict between global & local interference networks %d\n", it->first);
            }

            /*
             * DO REGISTER ALLOCATION FOR LOCAL REGISTERRS THIS MUST BE DONE HERE
             * TO AVOID EDGES BETWEEN LOCAL REGISTERS OF DIFFERNT BBs 
             */ 
            assign_local_registers (locallIntfNodeMap,allIntfNodeMap);
            locallIntfNodeMap.clear ();
        }

        /* CHECK DESCENDENTS AND ANCESTORS */
        for (int i = 0; i < bb->getNumDescendents (); i++)
            make_interference_nodes_network (bb->getNthDescendent (i), globalIntfNodeMap, locallIntfNodeMap, allIntfNodeMap, reg_alloc_mode);//TODO should it not be a BFS instead of DFS? 
        for (int i = 0; i < bb->getNumAncestors (); i++)
            make_interference_nodes_network (bb->getNthAncestor (i), globalIntfNodeMap, locallIntfNodeMap, allIntfNodeMap, reg_alloc_mode);//TODO should it not be a BFS instead of DFS? 
    }
}

static void set_arch_reg_for_all_ins (basicblock* bb, map<long int,interfNode*> &globalIntfNodeMap) {
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
//        bb->globalToLocalXform ();
        bb->setRegAllocated ();
        for (int i = 0; i < bb->getNumDescendents (); i++)
            set_arch_reg_for_all_ins (bb->getNthDescendent (i), globalIntfNodeMap);//TODO should it not be a BFS instead of DFS? 
        for (int i = 0; i < bb->getNumAncestors (); i++)
            set_arch_reg_for_all_ins (bb->getNthAncestor (i), globalIntfNodeMap);//TODO should it not be a BFS instead of DFS? 
    }
}

void allocate_register_sb (List<basicblock*> *bbList, 
                        List<instruction*> *insList, 
                        map<ADDR,instruction*> *insAddrMap, 
                        REG_ALLOC_MODE reg_alloc_mode) {
	List<basicblock*> *interiorBB = new List<basicblock*>;
	map<long int,interfNode*> locallIntfNodeMap, globalIntfNodeMap, allIntfNodeMap;

	printf ("\tPhi-Function Elimination\n");
	int numInsInsertion = eliminatePhiFuncs (bbList, insAddrMap);

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
		assign_global_registers (locallIntfNodeMap, globalIntfNodeMap, allIntfNodeMap);
		set_arch_reg_for_all_ins (bbHead, allIntfNodeMap);
	}

	for (int i = 0; i < bbList->NumElements (); i++) {	
		basicblock* bb = bbList->Nth (i);
		// Assert (bb->isVisited ()); //all BB must be visited by now //TODO put this back ASAP
		bb->setAsUnvisited ();
	}

	printf ("\tNumber of Phi MOV instructions: %d added to the %d static program instructions\n", numInsInsertion, insList->NumElements ());
	printf ("\tMAX number of global/local live variables: %d / %d\n", liveGlbMaxSize, liveLclMaxSize);

	delete interiorBB;
}
