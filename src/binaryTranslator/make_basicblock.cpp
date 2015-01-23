/*******************************************************************************
 *  make_basicblock.cpp
 ******************************************************************************/

#include "make_basicblock.h"
#include "listSchedule.h"


void make_basicblock  (List<instruction*> *insList,
		      		  List<basicblock*> *bbList,
					  map<int,variable*> &varList, 
		      		  set<ADDR> *brDstSet,
		      		  map<ADDR, basicblock*> *bbMap,
					  map<ADDR,instruction*> *insAddrMap,
                      LENGTH cluster_size) {
	// CONSTRUCT BB'S
	set<long int> use_set, def_set, diff_set;
	for  (int i = 0; i < insList->NumElements (); i++) {
		instruction* ins = insList->Nth (i);
		//CONSTRUCT THE DESTINATION INSTRUCTION ADDRESS SET  (STAT INFO)
		for  (int j = 0; j < ins->getNumReg (); j++) {
			if  (ins->getNthRegType (j) == READ)
				use_set.insert (ins->getNthReg (j));
			else if  (ins->getNthRegType (j) == WRITE)
				def_set.insert (ins->getNthReg (j));
		}
		//CONSTRUCT NEW BASICBLOCK OR ADD INS TO AN EXISTING ONE 
		//printf ("Type: %c\n", insList->Nth (i)->getType ());
		if  (i == 0 ||
		    brDstSet->find (ins->getInsAddr ()) != brDstSet->end ()) {
			basicblock* newBB;
			if  (i == 0 || bbList->Last ()->getBbSize () > 0) {
				newBB = new basicblock;
				newBB->setListIndx (bbList->NumElements ());
				bbList->Append (newBB);
			} else {
				//AVOID MAKING EMPTY BB'S
				newBB = bbList->Last ();
			}
			newBB->addIns (ins, BR_DST);
			bbMap->insert (pair <ADDR, basicblock*> (newBB->getID (), newBB));
			//THE CASE WITH A SINGLE BRANCH/JUMP/CALL INSTRUCTION IN BB (CLOSING BB)
			if  (ins->getType () == 'j' || ins->getType () == 'b' || ins->getType () == 'r' || 
                 ins->getType () == 'c' || ins->getType () == 's') {
			    basicblock *bb = bbList->Last ();
				bb->setBBbrHeader (ins->getInsAddr ());
				newBB = new basicblock;
				newBB->setListIndx (bbList->NumElements ());
				bbList->Append (newBB);
            } else if ((!ins->hasFallThru () && !ins->hasDst ()) || (newBB->getBbSize () >= cluster_size)) {
                newBB = new basicblock;
                newBB->setListIndx (bbList->NumElements ());
                bbList->Append (newBB);
                if  (newBB->getBbSize () == 1) {
                    bbMap->insert (pair <ADDR, basicblock*> (newBB->getID (), newBB));
                }
            }
			//printf ("%llx, %llx\n", ins->getInsAddr (), newBB->getID ());
		} else if  (ins->getType () == 'j' || ins->getType () == 'b' || ins->getType () == 'r' || 
                    ins->getType () == 'c' || ins->getType () == 's') {
			basicblock *bb = bbList->Last ();
			bb->setBBbrHeader (ins->getInsAddr ());
			bb->addIns (ins, NO_BR_DST);
			if  (bb->getBbSize () == 1) {
				bbMap->insert (pair <ADDR, basicblock*> (bb->getID (), bb));
			}
			basicblock* newBB = new basicblock;
			newBB->setListIndx (bbList->NumElements ());
			bbList->Append (newBB);
			//printf ("%llx\n", ins->getInsAddr ());
		} else if (!ins->hasFallThru () && !ins->hasDst ()) {
			basicblock *bb = bbList->Last ();
			bb->addIns (ins, NO_BR_DST);
			if  (bb->getBbSize () == 1) {
				bbMap->insert (pair <ADDR, basicblock*> (bb->getID (), bb));
			}
			basicblock* newBB = new basicblock;
			newBB->setListIndx (bbList->NumElements ());
			bbList->Append (newBB);
			//printf ("%llx\n", ins->getInsAddr ());
		} else if (bbList->Last()->getBbSize () >= cluster_size) {
			basicblock* newBB = new basicblock;
			newBB->setListIndx (bbList->NumElements ());
			bbList->Append (newBB);
			newBB->addIns (ins, BR_DST);
			if  (newBB->getBbSize () == 1) {
				bbMap->insert (pair <ADDR, basicblock*> (newBB->getID (), newBB));
			}
			//printf ("%llx\n", ins->getInsAddr ());
		} else {
			basicblock *bb = bbList->Last ();
			bb->addIns (ins, NO_BR_DST);
			if  (bb->getBbSize () == 1) {
				bbMap->insert (pair <ADDR, basicblock*> (bb->getID (), bb));
				//printf ("%llx, %llx\n", ins->getInsAddr (), bb->getID ());
			} //else
				//printf ("%llx\n", ins->getInsAddr ());
		}
	}

	/* GET RID OF EMPTY BASICBLOCKS  (SHOULDN'T FIND ANY EMPTY ONES - JUST PRECAUTION) */
	printf ("\tNum BB before Dead BB Elimination: %d\n",bbList->NumElements ());
	set<ADDR> toRemoveInsSet;
	for  (int i = bbList->NumElements () - 1; i >= 0; i--) {
        basicblock* bb = bbList->Nth (i);
		if  (bb->getBbSize () == 0) {
			// delete bbList->Nth (i);
			bbList->RemoveAt (i);
		} else if  (bb->getBbSize () == 1) {
			instruction* ins = bb->getLastIns ();
			string str (ins->getOpCode ());
			if  (str.compare ("#NOP\n") == 0) {
				toRemoveInsSet.insert (ins->getInsAddr ());
				// delete bbList->Nth (i);
				bbList->RemoveAt (i);
			}
		}
	}
	printf ("\tNum BB after Dead BB Elimination: %d\n",bbList->NumElements ());

	/* IF A BB DOES NOT YET HAVE AN ID, ASSIGN IT AN ID */
	for  (int i = bbList->NumElements () - 1; i >= 0; i--) {
        basicblock* bb = bbList->Nth (i);
        bb->forceAssignBBID ();
    }

	/* REMOVE DEAD NOP INSTRUCTIONS FROM INSLIST */
	for  (int i = insList->NumElements () - 1; i >= 0; i--) {
		instruction* ins = insList->Nth (i);
		if  (toRemoveInsSet.find (ins->getInsAddr ()) != toRemoveInsSet.end ()) {
			insList->RemoveAt (i);
			insAddrMap->erase (ins->getInsAddr ());
            // delete ins;
		}
	}

	/* SETUP BB DEPENDENCIES */
	for  (int i = 0; i < bbList->NumElements (); i++) {
		basicblock* bb = bbList->Nth (i);
		Assert (bb->getBbSize () > 0 && "Invalid BB Size.");
		instruction* ins = bb->getLastIns ();
		char type = ins->getType ();
		if  (type == 'j') { //unconditional jump 
			ADDR insDst = bb->getLastInsDst ();
			basicblock* bbDst;
			if  (bbMap->find (insDst) != bbMap->end ()) {
                if (ins->hasDst ()) {
                    bbDst =  (*bbMap)[insDst];
                    bb->setDescendent (bbDst);
                    bb->setTakenTarget (bbDst);
                }
			} else {
				printf ("\t\tERROR: Didn't find destination bb for jump, %llx  (%s, line: %d)\n", insDst , __FILE__, __LINE__);
				//exit (1);
				continue;
			}
		} else if (type == 'n' || type == 'o' || type == 'M' || type == 'c') {
			ADDR insFallThru = bb->getLastInsFallThru ();
			basicblock* bbFallThru;
			if  (bbMap->find (insFallThru) != bbMap->end ()) {
                if (ins->hasFallThru ()) {
                    bbFallThru =  (*bbMap)[insFallThru];
                    bb->setDescendent (bbFallThru);
                    bb->setFallThrough (bbFallThru);
                }
			} else {
				printf ("\t\tERROR: Didn't find destination bb for non-br op, %llx  (%s, line: %d)\n", insFallThru, __FILE__, __LINE__);
				//exit (1);
				continue;
			}
		} else if  (type == 'b') { //conditional jump
			ADDR insFallThru = bb->getLastInsFallThru ();
			basicblock* bbFallThru;
			if  (bbMap->find (insFallThru) != bbMap->end ()) {
                if (ins->hasFallThru ()) {
                    bbFallThru =  (*bbMap)[insFallThru];
                    bb->setDescendent (bbFallThru);
                    bb->setFallThrough (bbFallThru);
                }
			} else {
				printf ("\t\tERROR: Didn't find fall-thru bb for br, %llx  (%s, line: %d)\n", insFallThru, __FILE__, __LINE__);
				//exit (1);
				continue;
			}

			ADDR insDst = bb->getLastInsDst ();
			basicblock* bbDst;
			if  (bbMap->find (insDst) != bbMap->end ()) {
                if (ins->hasDst ()) {
                    bbDst =  (*bbMap)[insDst];
                    bb->setDescendent (bbDst);
                    bb->setTakenTarget (bbDst);
                }
			} else {
				printf ("\t\tERROR: Didn't find destination bb for br, %llx  (%s, line: %d)\n", insDst , __FILE__, __LINE__);
				//exit (1);
				continue;
			}
//        } else if  (type == 'c') { //func call - TODO this block should be useless - check and remove if possible
//            // Assumptions on function calls 
//            // Call does not terminate BB's
//            // The CFG links to function called is not established
//            // Call does not disturb the CFG flow and dependency b/w BB's
//            if  (i+1 < bbList->NumElements ()) {				
//                bb->setDescendent (bbList->Nth (i+1));
//                bb->setFallThrough (bbList->Nth (i+1));
//            }
        } else if  (type == 's' || type == 'r') {
			;//do nothing on syscall - no fthru, no destination
		} else {
			printf ("Wrong Ins Type: %c\n", type);
			Assert  (0 && "WRONG INSTRUTION. Terminating...");
		}
	}
	std::set_difference (use_set.begin (), use_set.end (), def_set.begin (), def_set.end (), std::inserter (diff_set, diff_set.begin ()));
	printf ("\tDiff Set: %d %d %d\n", def_set.size (), use_set.size (), diff_set.size ());

    /* PRINT THE NUMBER OF "LARGE" BB'S */
//	for  (int i = 0; i < bbList->NumElements (); i++) {
//		basicblock* bb = bbList->Nth (i);
//        if (bb->getBbSize () > 30) cout << "LARGE BB ID: " << hex << bb->getID () << endl;
//    }

    /* SETSUP BB STATS */
    dependencySetup (bbList, insList);
	for  (int i = 0; i < bbList->NumElements (); i++) {
		basicblock* bb = bbList->Nth (i);
//        bb->setsupStats ();
//        bb->reportStats ();
    }
}
