/*******************************************************************************
 *  ssa.cpp
 ******************************************************************************/

#include "ssa.h"

#define ZERO_ITER 0

void findDomEntryPoints (List<basicblock*> *bbList, List<basicblock*> *interiorBB) {
	for (int i = 0; i < bbList->NumElements (); i++) {	
		basicblock* bb = bbList->Nth (i);
		if (bb->getSDominatorSize () == 0) {
			interiorBB->Append (bb);
		}
	}
}

void buildDefUseSets (List<basicblock*> *bbList) {
	for (int i =0 ; i < bbList->NumElements (); i++)
		bbList->Nth (i)->setupDefUseSets ();
}

/* Tracks the basicblocks that define different x86 vairables in them. Each x86
 * variable is assigned the BB pointers that have it defined in them.
 */
void buildVarList (List<basicblock*>* bbList, map<int,variable*> &varList) {
	// Build the variables list for x86 registers
	for (long int i = X86_REG_LO; i <= X86_REG_HI; i++) {
		variable *var = new variable (i);
		varList.insert (pair<int,variable*> (i,var));
	}
	for (int i = 0; i < bbList->NumElements (); i++) {
		basicblock* bb = bbList->Nth (i);
		set<long int> defSet = bb->getDefSet (); //NOTE: some variables are "somehow" never defined in x86!
		for (set<long int>::iterator it = defSet.begin (); it != defSet.end (); it++) {
			if (!((*it) >= X86_REG_LO && (*it) <= X86_REG_HI))
				printf ("ERROR: %d\n",*it);
			Assert ((*it) >= X86_REG_LO && (*it) <= X86_REG_HI && "invalid x86 variable.");
			int varIndx = *it;
			varList[varIndx]->addBB (bb);
		}
		#ifdef DEBUG_SSA
		printf ("def set: %d\n",defSet.size ());
		#endif
	}
	// Stat on x86 reg definitions in a program.
	// for (map<int,variable*>::iterator it = varList.begin (); it != varList.end (); it++) {
		// printf ("%d\n", it->second->getNumAssignedBB ());
	// }
}

void phi_func_placement (List<basicblock*> *bbList, map<int,variable*> &varList) {
	map<ADDR,int> hasAlready, work;
	map<ADDR,basicblock*> W;
	for (int i = 0; i < bbList->NumElements (); i++) {
		basicblock *bb = bbList->Nth (i);
		hasAlready.insert (pair<ADDR,int> (bb->getID (),ZERO_ITER));
		work.insert (pair<ADDR,int> (bb->getID (),ZERO_ITER));
	}
	int iterCount = 0;
	for (map<int,variable*>::iterator it = varList.begin (); it != varList.end (); it++) {
		iterCount++;
		variable *var = it->second;
		for (int j = 0; j < var->getNumAssignedBB (); j++) {
			basicblock *bb = var->getNthAssignedBB (j);
			work[bb->getID ()] = iterCount;
			W.insert (pair<ADDR,basicblock*> (bb->getID (),bb));
		}
		while (W.size () != 0) {
			basicblock* X = (W.begin ())->second;
			W.erase (W.begin ());
			map<ADDR,basicblock*> xDF = X->getDF ();
			for (map<ADDR,basicblock*>::iterator Y = xDF.begin (); Y != xDF.end (); Y++) {
				if (hasAlready[Y->first] < iterCount) {
					 (Y->second)->insertPhiFunc (var->getID ());
					hasAlready[Y->first] = iterCount;
					if (work[Y->first] < iterCount) {
						work[Y->first] = iterCount;
						W.insert (pair<ADDR,basicblock*> ((Y->second)->getID (),Y->second));
					}
				}
			}
		}
	}
}

int whichPred (basicblock* Y, basicblock* X) {
	ADDR bbID = X->getID ();
	for (int i = 0; i < Y->getNumAncestors (); i++) {
		if (Y->getNthAncestor (i)->getID () == bbID)
			return i;
	}
	Assert (true == false && "CFG Fault. The BB ancesor was not found.");
}

int counter = 0; //TODO for debug
void search (basicblock* bb, map<int,variable*> &varList) {
	// if (!bb->isVisited ()) {
		bb->setAsVisited ();
		// printf ("================================%d\n", counter++);
		List<instruction*> *insList = bb->getInsList ();
		//TODO: check if the block is visited
		//Process assignment operations
		//Process phi operations
		map<long int, vector<long int> > phiFuncs = bb->getPhiFuncs ();
        map<long int, vector<long int> >::iterator it;
		for (it = phiFuncs.begin (); it != phiFuncs.end (); it++) {
			int var = it->first;
			Assert (var <= X86_REG_HI && var >= X86_REG_LO && "Invalid register value");
			int k = varList[var]->getC ();
			bb->setPhiWriteVar (var, k);
			varList[var]->pushToStack (k);
			varList[var]->setC (k+1);
		}
		for (int i = 0; i < insList->NumElements (); i++) {
			instruction *ins = insList->Nth (i);
			for (int j = 0; j < ins->getNumReadReg (); j++) {
				int var = ins->getNthReadReg (j);
				Assert (var <= X86_REG_HI && var >= X86_REG_LO && "Invalid register value");
				ins->setReadVar (var, varList[var]->getTopStack ());
			}
			for (int j = 0; j < ins->getNumWriteReg (); j++) {
				int var = ins->getNthWriteReg (j);
				Assert (var <= X86_REG_HI && var >= X86_REG_LO && "Invalid register value");
				int k = varList[var]->getC ();
				ins->setWriteVar (var, k);
				varList[var]->pushToStack (k);
				varList[var]->setC (k+1);
			}
		}
		for (int i = 0; i < bb->getNumDescendents (); i++) {
			basicblock* Y = bb->getNthDescendent (i);
			int j = whichPred (Y, bb);
			map<long int, vector<long int> > phiFuncs = Y->getPhiFuncs ();
			for (map<long int, vector<long int> >::iterator it = phiFuncs.begin (); it != phiFuncs.end (); it++) {
				int var = it->first;
				Assert (var <= X86_REG_HI && var >= X86_REG_LO && "Invalid register value");
				//TODO see if j ever larger than 0
				//we want to have as many elemenet in teh phi-vector as the number of ancestors of the BB. Then every ancestor must come and fill in the hole
				Y->replaceNthPhiOperand (var,j,varList[var]->getTopStack ()); //TODO correct?
			}
		}
		map<ADDR,basicblock*> children = bb->getChildren ();
		for (map<ADDR,basicblock*>::iterator it = children.begin (); it != children.end (); it++) {
		// for (int i = 0; i < bb->getNumChildren (); i++) {
			// printf ("children\n");
			basicblock* Y = it->second;// bb->getNthChild (i);
			search (Y,varList);
		}
		for (int i = 0; i < insList->NumElements (); i++) {
			instruction *ins = insList->Nth (i);
			for (int j = 0; j < ins->getNumWriteReg (); j++) {
				int var = ins->getNthOldWriteReg (j);
				Assert (var <= X86_REG_HI && var >= X86_REG_LO && "Invalid register value");
				varList[var]->popFromStack ();
			}
			// This is another part of the stack hack
			for (int j = 0; j < ins->getNumReadReg (); j++) {
				int var = ins->getNthReadReg (j);
				Assert (varList.find (var) != varList.end ());
				varList[var]->popHackPushes ();
			}
		}
		for (map<long int, vector<long int> >::iterator it = phiFuncs.begin (); it != phiFuncs.end (); it++) {
			int var = it->first;
			Assert (var <= X86_REG_HI && var >= X86_REG_LO && "Invalid register value");
			varList[var]->popFromStack ();
		}
	// }
}
// TODO: I don't know if the process of pushing and popping from the "variable" stack is well synchronized 
//       (without any mis-push or mis-pop). What is going on?

void ssa_renaming (List<basicblock*> *bbList, map<int,variable*> &varList) {
	List<basicblock*> *interiorBB = new List<basicblock*>;
	findDomEntryPoints (bbList, interiorBB); //TODO: this is done twice (another time in reg-allocation - fix it)
	for (int i = 0; i < bbList->NumElements (); i++)
		bbList->Nth (i)->setAsUnvisited ();
	for (int i = 0; i < interiorBB->NumElements (); i++) {
		basicblock* bbHead = interiorBB->Nth (i);
		search (bbHead, varList);
	}
	for (int i = 0; i < bbList->NumElements (); i++) {
		if (bbList->Nth (i)->isVisited () == false) printf ("unvisited BB ID: %llx,%d,%d\n", bbList->Nth (i)->getID (), bbList->Nth (i)->getNumAncestors (), bbList->Nth (i)->numNonBackEdgeAncestors ());
		bbList->Nth (i)->setAsUnvisited ();
	}
}

void build_ssa_form (List<basicblock*> *bbList, map<int,variable*> &varList) {
	printf ("\tBuild Def-Use Sets\n");
	buildDefUseSets (bbList);
	printf ("\tBuild Variable Def Lists From DefSets\n");
	buildVarList (bbList,varList);
	printf ("\tBuild Phi-Funcations\n");
	phi_func_placement (bbList, varList);
	printf ("\tRun SSA Renaming\n");
	ssa_renaming (bbList, varList);
	printf ("\tDistroy VarList\n");
	for (map<int,variable*>::iterator it = varList.begin (); it != varList.end (); it++) {
		delete it->second;
	}
}
