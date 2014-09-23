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
                      SCH_MODE sch_mode) {
	// Construct BB's
	set<long int> use_set, def_set, diff_set;
	for  (int i = 0; i < insList->NumElements (); i++) {
		instruction* ins = insList->Nth (i);
		//Construct the destination instruction address set  (stat info)
		for  (int j = 0; j < ins->getNumReg (); j++) {
			if  (ins->getNthRegType (j) == READ)
				use_set.insert (ins->getNthReg (j));
			else if  (ins->getNthRegType (j) == WRITE)
				def_set.insert (ins->getNthReg (j));
		}
		//Construct new Basicblock or add ins to an existing one 
		//printf ("Type: %c\n", insList->Nth (i)->getType ());
		if  (i == 0 ||
		    brDstSet->find (ins->getInsAddr ()) != brDstSet->end ()) {
			basicblock* newBB;
			if  (i == 0 || bbList->Last ()->getBbSize () > 0) {
				newBB = new basicblock;
				newBB->setListIndx (bbList->NumElements ());
				bbList->Append (newBB);
			} else {
				//avoid making empty BB's
				newBB = bbList->Last ();
			}
			newBB->addIns (ins, BR_DST);
			bbMap->insert (pair <ADDR, basicblock*> (newBB->getID (), newBB));
			//The case with a single branch/jump/call instruction in BB  (closing BB)
			if  (ins->getType () == 'j' || ins->getType () == 'b' || ins->getType () == 'r') {
				bbList->Last ()->setBBbrHeader (ins->getInsAddr ());
				newBB = new basicblock;
				newBB->setListIndx (bbList->NumElements ());
				bbList->Append (newBB);
			}
			//printf ("%llx, %llx\n", ins->getInsAddr (), newBB->getID ());
		} else if  (ins->getType () == 'j' || ins->getType () == 'b' || ins->getType () == 'r') {
			bbList->Last ()->setBBbrHeader (ins->getInsAddr ());
			bbList->Last ()->addIns (ins, NO_BR_DST);
			basicblock* newBB = new basicblock;
			newBB->setListIndx (bbList->NumElements ());
			bbList->Append (newBB);
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
	printf ("\tNum BB before Dead BB Elimination: %d\n",bbList->NumElements ());
	//get rid of empty basicblocks  (shouldn't find any empty ones - just precaution)
	set<ADDR> toRemoveInsSet;
	for  (int i = bbList->NumElements () - 1; i >= 0; i--) {
		if  (bbList->Nth (i)->getBbSize () == 0) {
			// delete bbList->Nth (i);
			bbList->RemoveAt (i);
		} else if  (bbList->Nth (i)->getBbSize () == 1) {
			instruction* ins = bbList->Nth (i)->getLastIns ();
			string str (ins->getOpCode ());
			if  (str.compare ("#NOP\n") == 0) {
				toRemoveInsSet.insert (ins->getInsAddr ());
				// delete bbList->Nth (i);
				bbList->RemoveAt (i);
			}
		}
	}
	printf ("\tNum BB after Dead BB Elimination: %d\n",bbList->NumElements ());
	//Remove NOP instructions from insList
	for  (int i = insList->NumElements () - 1; i >= 0; i--) {
		instruction* ins = insList->Nth (i);
		if  (toRemoveInsSet.find (ins->getInsAddr ()) != toRemoveInsSet.end ()) {
			// delete insList->Nth (i);
			insList->RemoveAt (i);
			insAddrMap->erase (ins->getInsAddr ());
		}
	}
	// Setup BB Dependencies
	for  (int i = 0; i < bbList->NumElements (); i++) {
		basicblock* bb = bbList->Nth (i);
		Assert (bb->getBbSize () > 0 && "Invalid BB Size.");
		instruction* ins = bb->getLastIns ();
		char type = ins->getType ();
		if  (type == 'j') { //unconditional jump 
			ADDR insDst = bb->getLastInsDst ();
			basicblock* bbDst;
			if  (bbMap->find (insDst) != bbMap->end ()) {
				bbDst =  (*bbMap)[insDst];
				bb->setDescendent (bbDst);
				bb->setTakenTarget (bbDst);
			} else {
				printf ("\tERROR: Didn't find destination bb, %llx  (%s, line: %d)\n", insDst , __FILE__, __LINE__);
				//exit (1);
				continue;
			}
		} else if  (type == 'n' || type == 'o' || type == 'M') { //non-jump
			if  (i+1 < bbList->NumElements ()) {				
				bb->setDescendent (bbList->Nth (i+1));
				bb->setFallThrough (bbList->Nth (i+1));
			}
		} else if  (type == 'b') { //conditional jump
			if  (i+1 < bbList->NumElements ()) {
				bb->setDescendent (bbList->Nth (i+1));
				bb->setFallThrough (bbList->Nth (i+1));
			}
			ADDR insDst = bb->getLastInsDst ();
			basicblock* bbDst;
			if  (bbMap->find (insDst) != bbMap->end ()) {
				bbDst =  (*bbMap)[insDst];
				bb->setDescendent (bbDst);
				bb->setTakenTarget (bbDst);
			} else {
				printf ("\tERROR: Didn't find destination bb, %llx  (%s, line: %d)\n", insDst , __FILE__, __LINE__);
				//exit (1);
				continue;
			}
		} else if  (type == 'c') { //func call
			// Assumptions on function calls 
			// Call does not terminate BB's
			// The CFG links to function called is not established
			// Call does not disturb the CFG flow and dependency b/w BB's
			if  (i+1 < bbList->NumElements ()) {				
				bb->setDescendent (bbList->Nth (i+1));
				bb->setFallThrough (bbList->Nth (i+1));
			}
		} else if  (type == 'r') { //func return
			;//do nothing on return. must have been the end of function
		} else {
			printf ("Wrong Ins Type: %c\n", type);
			Assert  (true == false && "WRONG INSTRUTION. Terminating...");
		}
	}	
	// Perform List Scheduling on BB
    if (sch_mode == LIST_SCH) { //TODO put this after local register allocation
        printf ("\tRun List Scheduling\n");
        for  (int i = 0; i < bbList->NumElements (); i++) {
            basicblock* bb = bbList->Nth (i);
            listSchedule (bb); //why does this affect BB structure?
        }
    }
	std::set_difference (use_set.begin (), use_set.end (), def_set.begin (), def_set.end (), std::inserter (diff_set, diff_set.begin ()));
	printf ("\tDiff Set: %d %d %d\n", def_set.size (), use_set.size (), diff_set.size ());
}
