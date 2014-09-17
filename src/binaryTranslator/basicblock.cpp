/*******************************************************************************
 *  basicblock.cpp
 ******************************************************************************/

#include <set>
#include <vector>
#include <algorithm>
#include <iostream>
#include "basicblock.h"

basicblock::basicblock () {
	bbID = -1; //place holder
	_listIndx = -1; //place holder
	_visited = false;
	_entryPoint = false;
	_domSetIsAll = false;
	_hasBrHeader = false;
	_brHeaderAddr = 0;
	_backEdgeDest = -1;
	_fallThroughBB = NULL;
	_takenTargetBB = NULL;
	_bbListForPhraseblock = new List<ADDR>;
	_phList				  = new List<phrase*>;
	_insList              = new List<instruction*>;
	_insList_orig         = new List<instruction*>;
	_insListSchList       = new List<instruction*>;
	_ancestorBbList       = new List<basicblock*>;
	_descendantBbList     = new List<basicblock*>;
	_backEdgeSourceBbList = new List<basicblock*>;
}

basicblock::~basicblock () {
	delete _phList;
	delete _insList;
	delete _insList_orig;
	delete _insListSchList;
	delete _ancestorBbList;
	delete _descendantBbList;
	delete _backEdgeSourceBbList;
	delete _bbListForPhraseblock;
}

basicblock& basicblock::operator=  (const basicblock& bb) { //TODO: UPDATE THIS FUNCTION
	bbID = bb.bbID;
	_listIndx = bb._listIndx;
	_visited = bb._visited;
	_entryPoint = bb._entryPoint;
	_domSetIsAll = bb._domSetIsAll;
	_backEdgeDest = bb._backEdgeDest;
	_fallThroughBB = bb._fallThroughBB;
	_takenTargetBB = bb._takenTargetBB;
	_bbListForPhraseblock = bb._bbListForPhraseblock;
	_phList				  = bb._phList;
	_insList              = bb._insList;
	_insList_orig         = bb._insList_orig;
	_insListSchList		  = bb._insListSchList;
	_ancestorBbList       = bb._ancestorBbList;
	_descendantBbList     = bb._descendantBbList;
	_backEdgeSourceBbList = bb._backEdgeSourceBbList;
	_insAddrList = bb._insAddrList;
	_dominatorSet = bb._dominatorSet;
	_dominatorMap = bb._dominatorMap;
	return *this;
}

void basicblock::transferPointersToNewList (List<basicblock*>* bbList) {
	for  (int i = 0; i < _ancestorBbList->NumElements (); i++) {
		int listIndx = _ancestorBbList->Nth (i)->getListIndx ();
		_ancestorBbList->RemoveAt (i);
		_ancestorBbList->InsertAt (bbList->Nth (listIndx), i); //TODO check if the index inserts at the right spot
	}
	for  (int i = 0; i < _descendantBbList->NumElements (); i++) {
		int listIndx = _descendantBbList->Nth (i)->getListIndx ();
		_descendantBbList->RemoveAt (i);
		_descendantBbList->InsertAt (bbList->Nth (listIndx), i); //TODO check if the index inserts at the right spot
	}
	for  (int i = 0; i < _backEdgeSourceBbList->NumElements (); i++) {
		int listIndx = _backEdgeSourceBbList->Nth (i)->getListIndx ();
		_backEdgeSourceBbList->RemoveAt (i);
		_backEdgeSourceBbList->InsertAt (bbList->Nth (listIndx), i); //TODO check if the index inserts at the right spot
	}
	if  (_fallThroughBB != NULL) {
		int listIndx = _fallThroughBB->getListIndx ();
		_fallThroughBB = bbList->Nth (listIndx);	
	}
	if  (_takenTargetBB != NULL) {
		int listIndx = _takenTargetBB->getListIndx ();
		_takenTargetBB = bbList->Nth (listIndx);		
	}
}

void basicblock::addIns (instruction* ins, REACHING_TYPE reach_type) {
	_insList->Append (ins);
	_insList_orig->Append (ins);
	_insAddrList.insert (ins->getInsAddr ());
	if  (bbID == -1 &&
         (reach_type == BR_DST || 
          ins->getType () != 'n')) {
		bbID = ins->getInsAddr ();
		Assert (bbID > 0 && "Invalid basicblock ID assigned.");
	}
	ins->setMy_BBorPB_id (getID ());
	//Update Use Set & Def Set
	// int counter = 0;
	// for  (int i = 0; i < ins->getNumReg (); i++) {
	// 	if  (ins->getNthRegType (i) == READ && _defSet.find (ins->getNthReg (i)) == _defSet.end ()) {
	// 		//second condition avoids ud-chains within a BB from propagating
	// 		updateUseSet (ins->getNthReg (i));
	// 	} else if  (ins->getNthRegType (i) == WRITE) {
	//  			updateDefSet (ins->getNthReg (i));
	// 	} else {
	// 		// updateLocalRegSet (ins->getNthReg (i));
	// 		counter++;
	// 	}
	// }
	// printf  ("%d, %d, %f\n", ins->getNumReg (), counter,  (double)counter/ (double)ins->getNumReg ());
}

void basicblock::addIns (instruction* ins, ADDR ID) {
	_insList->Append (ins);
	_insAddrList.insert (ins->getInsAddr ());
	if  (_insList->NumElements () == 1) {
		bbID = ID;
		Assert (bbID > 0 && "Invalid basicblock ID assigned.");
	}
	ins->setMy_BBorPB_id (getID ());
	//Update Use Set & Def Set
	// int counter = 0;
	// for  (int i = 0; i < ins->getNumReg (); i++) {
	// 	if  (ins->getNthRegType (i) == READ && _defSet.find (ins->getNthReg (i)) == _defSet.end ()) {
	// 		//second condition avoids ud-chains within a BB from propagating
	// 		updateUseSet (ins->getNthReg (i));
	// 	} else if  (ins->getNthRegType (i) == WRITE) {
	//  			updateDefSet (ins->getNthReg (i));
	// 	} else {
	// 		// updateLocalRegSet (ins->getNthReg (i));
	// 		counter++;
	// 	}
	// }
	// printf  ("%d, %d, %f\n", ins->getNumReg (), counter,  (double)counter/ (double)ins->getNumReg ());
}

void basicblock::printBb () {
	printf ("\n--BB ID: %llx\n", bbID);
	for  (int i = 0; i < _insList->NumElements (); i++) {
		printf ("%llx, %s\n", _insList->Nth (i)->getInsAddr (), _insList->Nth (i)->getOpCode ());
	}
}

int basicblock::getBbSize_ListSch () {
	return _insListSchList->NumElements ();
}

int basicblock::getBbSize () {
	return _insList->NumElements ();
}

void basicblock::setFallThrough (basicblock* bb) {
	Assert (_fallThroughBB == NULL);
	_fallThroughBB = bb;
}

void basicblock::setTakenTarget (basicblock* bb) {
	Assert (_takenTargetBB == NULL);
	_takenTargetBB = bb;
}

void basicblock::setDescendent (basicblock* bb) {
	_descendantBbList->Append (bb);
	bb->setAncestor (this); //Create duplex links
}

void basicblock::setAncestor (basicblock* bb) {
	_ancestorBbList->Append (bb);
}

ADDR basicblock::getID () {
	Assert (bbID > 0 && "bbID must be larger than zero.");
	return bbID;
}

ADDR basicblock::getLastInsDst () {
	Assert (_insList->NumElements ()> 0 && "BB size is zero.");
	return _insList->Nth (_insList->NumElements ()-1)->getBrDst ();
}

instruction* basicblock::getLastIns () {
	Assert (_insList->NumElements ()> 0 && "BB size is zero.");
	return _insList->Nth (_insList->NumElements ()-1);
}


List<instruction*>* basicblock::getInsList () {
	return _insList;
}

basicblock* basicblock::getNxtBB () {
	Assert  (getTakenBias () >= 0.0 && getTakenBias () <= 1.0 && "Invalid bias value\n");
	if  (getTakenBias () > WBB_LOWER_BOUND && 
		getTakenBias () < WBB_UPPER_BOUND) {
			printf ("\tWARNING: There is no clear answer for the next BB  (%s, line: %d)\n",__FILE__, __LINE__);
			printf ("%llx\n", getID ()); //TODO remove this line
			return NULL; //Best action @ this point: give up
	} else if  (getTakenBias () <= 0.1) {
		return getFallThrough ();
	} else {
		return getTakenTarget ();
	}
}

basicblock* basicblock::getFallThrough () {
	Assert (_fallThroughBB != NULL && "_fallThroughBB must not be NULL");
	return _fallThroughBB;
}

basicblock* basicblock::getTakenTarget () {
	Assert (_takenTargetBB != NULL && "_takenTargetBB must not be NULL");
	return _takenTargetBB;
}

basicblock* basicblock::getNthDescendent (int indx) {
	Assert (_descendantBbList->NumElements () > indx && indx >= 0 && "BB size is smaller than indx.");
	return _descendantBbList->Nth (indx);
}

basicblock* basicblock::getNthAncestor (int indx) {
	Assert (_ancestorBbList->NumElements () > indx && indx >= 0 && "BB size is smaller than indx.");
	return _ancestorBbList->Nth (indx);
}

int basicblock::getNumDescendents () {
	return _descendantBbList->NumElements ();
}

int basicblock::getNumAncestors () {
	return _ancestorBbList->NumElements ();
}

void basicblock::setAsVisited () {
	Assert (_visited == false && "BB must not have been visited");
	_visited = true;
}

void basicblock::setAsUnvisited () {
	_visited = false;
}

bool basicblock::isVisited () {
	return _visited;
}

bool basicblock::setDominators () {
	// Assert (_dominatorMap.size () == 0 && "set size must be zero initially.");
	if  (_dominatorMap.find (getID ()) == _dominatorMap.end ()) {
		_dominatorSet.insert (getID ());
		_dominatorMap.insert (pair<ADDR, basicblock*> (getID (), this));
	} else {
		return false;//Assert (true == false && "Invalid dominator set.");
	}
	return true;
}

/* Set of Strict Dominators */
void basicblock::buildSDominators () {
	map<ADDR,basicblock*> self;
	self.insert (pair<ADDR,basicblock*> (getID (),this));	
	set_difference (_dominatorMap.begin (),_dominatorMap.end (),self.begin (),self.end (),std::inserter (_sDominatorMap, _sDominatorMap.begin ()));
}


void basicblock::buildImmediateDominators () {
	map<ADDR,basicblock*> _notParentsMap;
	for  (map<ADDR,basicblock*>::iterator it1 = _sDominatorMap.begin (); it1 != _sDominatorMap.end (); it1++) {
		for  (map<ADDR,basicblock*>::iterator it2 = _sDominatorMap.begin (); it2 != _sDominatorMap.end (); it2++) {
			if  (it1->first == it2->first) continue;
			if  (it2->second->isASDominator (it1->first) == true)
				_notParentsMap.insert (pair<ADDR,basicblock*> (it1->first, it1->second));
		}
	}
	set_difference (_sDominatorMap.begin (),_sDominatorMap.end (),_notParentsMap.begin (),_notParentsMap.end (),std::inserter (_parentsMap, _parentsMap.begin ()));
	for  (map<ADDR,basicblock*>::iterator it1 = _parentsMap.begin (); it1 != _parentsMap.end (); it1++)
		_idomSet.insert (it1->first);
}

bool basicblock::isASDominator (ADDR nodeID) {
	if  (_sDominatorMap.find (nodeID) == _sDominatorMap.end ())
		return false;
	else
		return true;
}

int basicblock::getSDominatorSize () {
	return _sDominatorMap.size ();
}

/* 
 * NOTE: _idomMap provides the bottom-up link for dom tree.
 *       This module makes the top-down link  (a little bit countrary to the func name).
 */
void basicblock::buildDomTree () {
	for  (map<ADDR,basicblock*>::iterator it = _parentsMap.begin (); it != _parentsMap.end (); it++) {
		basicblock *myIdom = it->second;
		myIdom->addChild (this);
	}
}

void basicblock::addChild (basicblock *child) {
	_childrenMap.insert (pair<ADDR,basicblock*> (child->getID (), child));
}

int basicblock::getChildrenSize () {
	return _childrenMap.size ();
}

map<ADDR,basicblock*> basicblock::getChildren () {
	return _childrenMap;
}

void basicblock::addToDFset (basicblock *node) {
	_dominanceFrontier.insert (pair<ADDR,basicblock*> (node->getID (), node));
}

bool basicblock::isInIDom (ADDR nodeID) {
	if  (_idomSet.find (nodeID) == _idomSet.end ())
		return false;
	else
		return true;
}

map<ADDR,basicblock*> basicblock::getDF () {
	// printf ("DEBUG: dom frontier: %d\n", _dominanceFrontier.size ());
	return _dominanceFrontier;
}

void basicblock::setAllasDominators (bool domSetIsAll) {
	_domSetIsAll = domSetIsAll;
}

bool basicblock::getAllasDominators () {
	return _domSetIsAll;
}

bool basicblock::setDominators (List<basicblock*>* bbList) {
	Assert (_dominatorMap.size () == 0 && "set size must be zero initially.");
	if  (_dominatorMap.find (getID ()) == _dominatorMap.end ()) {
		_dominatorSet.insert (getID ());
		_dominatorMap.insert (pair<ADDR, basicblock*> (getID (), this));
	}
	for  (int i = 0; i < bbList->NumElements (); i++) {
		basicblock* bb = bbList->Nth (i);
		if  (_dominatorMap.find (bb->getID ()) == _dominatorMap.end ()) {
			_dominatorSet.insert (bb->getID ());
			_dominatorMap.insert (pair<ADDR, basicblock*> (bb->getID (), bb));
		}
	}
	return true;
}

bool basicblock::setDominators (map<ADDR,basicblock*> &intersection) {
	map<ADDR,basicblock*> oldMap = _dominatorMap;
	_dominatorMap.clear ();
	_dominatorMap = intersection;
	if  (_dominatorMap.find (getID ()) == _dominatorMap.end ()) {
		_dominatorSet.insert (getID ());
		_dominatorMap.insert (pair<ADDR, basicblock*> (getID (), this));
	}
	for  (map<ADDR,basicblock*>::iterator it = _dominatorMap.begin (); it != _dominatorMap.end (); it++) {
		_dominatorSet.insert (it->first);
	}
	if  (oldMap.size () != _dominatorMap.size ()) {
		return true;
	} else {
		map<ADDR,basicblock*>::iterator it1, it2;
		for  (it1 = _dominatorMap.begin (), it2 = oldMap.begin (); it1 != _dominatorMap.end (); it1++, it2++) {
			if  (*it1 != *it2) {
				return true;
			}
		}
		return false;
	}
	//it = set_intersection  (oldSet.begin (), oldSet.end (), _dominatorSet.begin (), _dominatorSet.end (), out.begin ());
	//if  (int (it - out.begin ()) != 0) {
	//	return true;
	//} else {
	//	return false;
	//}
}

map<ADDR,basicblock*>  basicblock::getDominators () {
	return _dominatorMap;
}

void basicblock::insertPhiFunc (long int var) {
	Assert (var >= X86_REG_LO && var <= X86_REG_HI && "invalid x86 variable.");
	// Add as many variables as there are ancestors for this basicblock
	if  (_phiFuncMap.find (var) == _phiFuncMap.end ()) {
		vector<long int> phiVector; //one vector per variable
		vector<basicblock*> phiBbVector; //one vector per variable
		_phiFuncMap.insert (pair<long int, vector<long int> > (var,phiVector));
		for  (int i = 0; i < getNumAncestors (); i++) {
			_phiFuncMap[var].push_back (var);
		}
	}
}

map<long int, vector<long int> > basicblock::getPhiFuncs () {
	return _phiFuncMap;
}

void basicblock::replaceNthPhiOperand (long int var, int indx, long int subscript) {
		_phiFuncMap[var].at (indx) = subscript;
}

void basicblock::setPhiWriteVar (long int var, long int subscript) {
	_phiDestMap.insert (pair<long int, long int> (var,subscript));
}

/* done at register allocation phase
   Here we don't actually "eliminate" phi functions, but we will add code to represent them in real program */
int basicblock::elimPhiFuncs () {
	map<long int, vector<long int> >::iterator it0;
	int temp = 0;
	for  (it0 = _phiFuncMap.begin (); it0 != _phiFuncMap.end (); it0++) {
		int var = it0->first;
		vector<long int> phiVector = it0->second;
		Assert (var <= X86_REG_HI && var >= X86_REG_LO && "Invalid register value");
		Assert (_phiDestMap.find (var) != _phiDestMap.end ());
		int renameToSub = _phiDestMap[var];
		Assert (phiVector.size () == getNumAncestors () && "The two vectors must have equal size.");
		int indx = 0;
		vector<long int>::iterator it1;
		for  (it1 = phiVector.begin (); it1 != phiVector.end (); it1++) {
			int subscript = *it1;
			basicblock* NthAncestor = getNthAncestor (indx);
			//printf ("values: %d=>%d, %d\n",var,renameToSub,subscript);
			NthAncestor->insertMOVop (var,renameToSub,var,subscript);
			indx++;
		}
		temp = phiVector.size ();
	}
	//repost the number of MOV isntructions inserted.
	return temp*_phiFuncMap.size ();
}

/* 
	These instructions
	- are not part of insList
	- do not have their asm variable setup
	- do not have their ins address setup
*/ 
void basicblock::insertMOVop (long int dst_var, long int dst_subs, long int src_var, long int src_subs) {
	instruction* newIns = new instruction;
	//TODO do all the bells and whistles for adding an ins.
	newIns->setType ('o');
	newIns->setOpCode ("#MOV\n");
	newIns->setInsAddr (PHI_INS_ADDR); //this is just a psudo instruction address.
	int type = READ;
	newIns->setRegister (&src_var, &type);
	newIns->setReadVar (src_var,src_subs);
	type = WRITE;
	newIns->setRegister (&dst_var, &type);
	newIns->setWriteVar (dst_var,dst_subs);
	// newIns->makeUniqueRegs (); //THIS STEP is done later  (don't put back)

	// If the last BB instruction is a branch/jump/return, hist to the secodn to last position
	//TODO: check if the phi-function is associated with neither of branch/jump/return
	//Since I run a trace and dump the whole BB during BB-annotation, I don't have to worry about
	//the points made above.
	addIns (newIns, NO_BR_DST); // _insList->Append (newIns);
}

void basicblock::setupBackEdge () {
	for  (int i = 0; i < getNumDescendents (); i++) {
		if  (_dominatorMap.find (getNthDescendent (i)->getID ()) != _dominatorMap.end ()) {
			_backEdgeDest = getNthDescendent (i)->getID ();
			Assert (_backEdgeDest > 0 && "Invalid back edge destination assignment.");
			getNthDescendent (i)->setAsBackEdgeSource (this);
		}
	}
}

bool basicblock::isBackEdge () {
	for  (int i = 0; i < getNumDescendents (); i++) {
		if  (_dominatorMap.find (getNthDescendent (i)->getID ()) != _dominatorMap.end ()) {
			return true;
		} else {
			return false;
		}
	}
	return false;
}

ADDR basicblock::getBackEdgeDest () {
	Assert (_backEdgeDest > 0 && "Back edge destination address is invalid");
	return _backEdgeDest;
}

int basicblock::numNonBackEdgeAncestors () {
	int count = 0;
	//NOTE: this is only a hack to get some loops recognized. should not be a universal solution
	if  (_ancestorBbList->NumElements () == _descendantBbList->NumElements () &&
	    _ancestorBbList->NumElements () == 1 &&
        _ancestorBbList->Nth (0)->getID () == _descendantBbList->Nth (0)->getID ()) {
		return 0;
	}
	//NOTE: this is only a hack to get some loops recognized. should not be a universal solution
	for  (int i = 0; i < _ancestorBbList->NumElements (); i++) {
		if  (_ancestorBbList->Nth (i)->getID () < getID ()) {
			count++;
		}
	}
	// printf ("%llx, %d\n", getID (), count);
	return count;
}

void basicblock::markAsEntryPoint () {
	_entryPoint = true;
}

bool basicblock::isEntryPoint () {
	return _entryPoint;
}

void basicblock::setAsBackEdgeSource (basicblock* bb) {
	_backEdgeSourceBbList->Append (bb);
}

basicblock* basicblock::getNthBackEdgeSource (int i) {
	return _backEdgeSourceBbList->Nth (i);
}

int basicblock::getNumBackEdgeSource () {
	return _backEdgeSourceBbList->NumElements ();
}

float basicblock::getTakenBias () {
	return getLastIns ()->getBrTakenBias ();
}

List<basicblock*>* basicblock::getAncestorList () {
	return _ancestorBbList;
}

List<basicblock*>* basicblock::getDescendentList () {
	return _descendantBbList;
}

List<basicblock*>* basicblock::getBackEdgeSourceList () {
	return _backEdgeSourceBbList;
}

int basicblock::getListIndx () {
	Assert (_listIndx >= 0 && "Invlid list index value");
	return _listIndx;
}

void basicblock::setListIndx (int listIndx) {
	Assert (listIndx >= 0 && "Invlid list index value");
	_listIndx = listIndx;
}

bool basicblock::isAPhraseblock () {
	if  (_bbListForPhraseblock->NumElements () == 0) {
		return false;
	} else {
		return true;
	}
}

void basicblock::addBBtoPBList (ADDR bbID) {
	_bbListForPhraseblock->Append (bbID);
}

List<ADDR>* basicblock::getBBListForPB () {
	return _bbListForPhraseblock;
}

void basicblock::basicblockToPhrase () {
	phrase* ph = NULL;
	for  (int i = 0; i < _insList->NumElements (); i++) {
		instruction* ins = _insList->Nth (i);
		if  (ins->getNumAncestors () > 1 || ins->getNumDependents () > 1 || _phList->NumElements () == 0) {
			ph = new phrase;
			_phList->Append (ph);
			ph->addIns (ins);
		} else {
			ph->addIns (ins);
		}
	}
	printf ("debug: BB %llx Size: %d - Phrases  (%d): ", getID (), _insList->NumElements (), _phList->NumElements ());
	for  (int i = 0; i < _phList->NumElements (); i++) {
		printf ("%d, ", _phList->Nth (i)->phSize ());
	}
	printf ("\n");
}

bool basicblock::isInsAddrInBB (ADDR insAddr) {
	Assert (_insAddrList.size () > 0);
	if  (_insAddrList.find (insAddr) != _insAddrList.end ()) return true;
	else return false;
}

void basicblock::addToBB_ListSchedule (instruction* ins) {
	Assert (isInsAddrInBB (ins->getInsAddr ()) == true && "Instruction does not belong to this BB"); //this can be a heavy check. Remove it? Useful?
	_insListSchList->Append (ins);
}

List<instruction*>* basicblock::getInsList_ListSchedule () {
//	Assert (_insListSchList->NumElements () == _insList->NumElements () && "List-scheduled list is incomplete"); //this can be a heavy check. Remove it? Useful?
	return _insListSchList;
}

void basicblock::updateDefSet (long int reg) {
	_defSet.insert (reg);
}

void basicblock::updateUseSet (long int reg) {
	_useSet.insert (reg);
}

void basicblock::updateLocalRegSet () {
	Assert (_localRegSet.size () == 0 && "The local register set must be empty at this point.");
	//The commented code makes write-registers that are never read be assigned in GRF	
	//std::set<long int> _defUseIntersection;
	//std::set_intersection (_defSet.begin (), _defSet.end (), _useSet.begin (), _useSet.end (), std::inserter (_defUseIntersection, _defUseIntersection.begin ()));
	//std::set_difference (_defUseIntersection.begin (), _defUseIntersection.end (), _outSet.begin (), _outSet.end (), std::inserter (_localRegSet, _localRegSet.begin ()));
	std::set_difference (_defSet.begin (), _defSet.end (), _outSet.begin (), _outSet.end (), std::inserter (_localRegSet, _localRegSet.begin ()));
/* debug	
	printf ("local set ratio:, %f\n",  (double)_localRegSet.size ()/ (double) (_inSet.size ()+_defSet.size ()));
	std::set<long int> test1,test2;
	std::set_union (_outSet.begin (), _outSet.end (), _useSet.begin (), _useSet.end (), std::inserter (test1, test1.begin ()));
	std::set_difference (_defSet.begin (), _defSet.end (), test1.begin (), test1.end (), std::inserter (test2, test2.begin ()));
	for  (set<long int>::iterator it = test2.begin (); it != test2.end (); it++) {
		printf ("stale register: %d\n", (*it)%100);
	}
*/
}

bool basicblock::isInLocalRegSet (long int reg) {
	if  (_localRegSet.find (reg) == _localRegSet.end ())
		return false;
	else
		return true;
}

set<long int> basicblock::getInSet () {
	return _inSet;
}

set<long int> basicblock::getDefSet () {
	return _defSet;
}

set<long int> basicblock::getLocalRegSet () {
	return _localRegSet;
}

bool basicblock::update_InOutSet () {
	int outSetSize = _outSet.size ();
	int inSetSize = _inSet.size ();
	// Update _outSet
	for  (int i = 0; i < getNumDescendents (); i++) {
		set<long int> tempSet;
		tempSet.clear ();
		basicblock *successor = getNthDescendent (i);
		set<long int> succInSet;
		vector<long int> result;
		succInSet = successor->getInSet ();
		std::set_union (succInSet.begin (), succInSet.end (), _outSet.begin (), _outSet.end (), std::inserter (tempSet, tempSet.begin ()));
		_outSet = tempSet;
		// printf ("%d,%d\n",succInSet.size (),_outSet.size ());
	}
	// Update _inSet
	set<long int> result1,result2;
	std::set_difference (_outSet.begin (), _outSet.end (), _defSet.begin (), _defSet.end (), std::inserter (result1, result1.begin ()));
	std::set_difference (_useSet.begin (), _useSet.end (), _defSet.begin (), _defSet.end (), std::inserter (result2, result2.begin ()));
	std::set_union (result1.begin (), result1.end (), result2.begin (), result2.end (), std::inserter (_inSet, _inSet.begin ()));
	// printf ("%d,%d,%d\n",_useSet.size (),_inSet.size (),_outSet.size ());

	// Has there been a change in the BB sets?
	bool change;
	if  (outSetSize != _outSet.size () || inSetSize != _inSet.size ())
		change = true;
	else
		change = false;
	return change;
}

/* Construct def/use sets from x86 format of registers */
void basicblock::setupDefUseSets () {
	for  (int i =0 ; i < _insList->NumElements (); i++) {
		instruction* ins = _insList->Nth (i);
		for  (int j = 0; j < ins->getNumReg (); j++) {
			if  (ins->getNthRegType (j) == READ) {// && _defSet.find (ins->getNthReg (i)) == _defSet.end ()) { TODO: what to do with this line?
				//second condition avoids ud-chains within a BB from propagating
				updateUseSet (ins->getNthReg (j));
			} else if  (ins->getNthRegType (j) == WRITE) {
	 			updateDefSet (ins->getNthReg (j));
			} else {
				Assert (true == false && "Invalid register type");
			}
		}
	}
}

/* Re-Construct def/use sets from SSA format of registers */
void basicblock::renameAllInsRegs () {
	_defSet.clear ();
	_useSet.clear ();
	for  (int i =0 ; i < _insList->NumElements (); i++) {
		instruction* ins = _insList->Nth (i);
		ins->makeUniqueRegs ();
		//printf ("ins %llx\n",ins->getInsAddr ());
		for  (int j = 0; j < ins->getNumReg (); j++) {
			long int reg = ins->getNthReg (j);
			//printf ("reg: %ld\n",reg);
			if  (ins->getNthRegType (j) == READ) {// && _defSet.find (ins->getNthReg (i)) == _defSet.end ()) { TODO: what to do with this line?
				//second condition avoids ud-chains within a BB from propagating
				updateUseSet (reg);
			} else if  (ins->getNthRegType (j) == WRITE) {
	 			updateDefSet (reg);
			} else {
				Assert (true == false && "Invalid register type");
			}
		}
	}
}

ADDR basicblock::getBBbrHeader () {
	Assert (_hasBrHeader == true && "You forgot to check if BB has a header in the first place.");
	return _brHeaderAddr;
}

bool basicblock::hasHeader () {
	return _hasBrHeader;
}

ADDR basicblock::getBBtail  () {
    return _insList->Last ()->getInsAddr ();
}

void basicblock::setBBbrHeader (ADDR brAddr) {
	Assert (brAddr > 0);
	_brHeaderAddr = brAddr;
	_hasBrHeader = true;
}
