/*******************************************************************************
 *  instruction.cpp
 *  Instruction object. Each instruction is placed into the instruction window for
 *  issue and then removed from the window when commited.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "instruction.h"
#include "../lib/utility.h"
#include "../global/global.h"
#include "dependencyTable.h"
#include "registerRename.h"
#include "regFile.h"
#include "phrase.h"
#include "lsq.h"


//Register Renaming
long int nextRenReg = INIT_RENAME_REG_NUM;
extern regFile *RF;
extern float unpredMemOpThreshold;
extern int cacheLat[MEM_HIGHERARCHY];
extern int phraseSizeBound;

instruction::instruction() {
	_insType   = noType;
	_insStatus = NO_STAGE;
	_readORwrite = none;

	_r  = new List<long int>;
	_pr  = new List<long int>;
	_rt = new List<int>;
	_numWriteReg = 0;
	_memAddr = 0;
	_insAddr = 0;
	_memAccessSize = 0;

	_brTarget = 0;
	_missPred = true; //stay conservative in case of error
	_brTaken = false;
	_brForward = false;
	
	_executeCycle  = -1;
	_latency     = -1;
	_completeCycle = -1;
	_memAddrCompCompleteCycle = -1;
	_pipeLineLat = -1;
	_fetchEndCycle = -1;

	_guardian = 0;
	_dependents	 = new List<instruction*>;
	_ancestors	 = new List<instruction*>;
	_depPhrases	 = new List<phrase*>;
	_depFrags	 = new List<fragment*>;
	_phraseAncestors = new List<instruction*>;
	_phraseAncestorsID = new List<int>;
	_brAncestors	 = new List<instruction*>;
	_brDependents	 = new List<instruction*>;
	_branchMode = noBrMode;
	_brBias = 1.0; //assume taken
	_brAccuracy = 0.5; //assume half way accurate
	_brPred = false;

	_inSideBuff = false;
	_mySBnum  = new List<int>;
	_causeOfSBinsID = new List<INS_ID>;
	_currentMySBnum = -1;
	_currentCauseOfSBinsID = 0;
	_causeOfFragInsID = -1;

	_addFlag = -1;
	_delFlag = -1;
	_findLPflag = -1;
	_phraseAddFlag = -1;

	_critPathLen = -1;
	_pathLen = -1;
	_hitLevel = -2;

	_myPhraseID = -1;
	_myFragID   = -1;

	_missRate = 0.0;
	_hasUPLDancestor = false;

	_bp_hist = NULL;

	_bbTail = false;
	_bbHead = false;
	_brHeaderAddr = 0;
}

instruction::~instruction() {
	while(_r->NumElements() > 0) {
	    _r->RemoveAt(0);
	}
	while(_rt->NumElements() > 0) {
	    _rt->RemoveAt(0);
	}
	while(_depPhrases->NumElements() > 0) {
	    _depPhrases->RemoveAt(0);
	}
	while(_depFrags->NumElements() > 0) {
	    _depFrags->RemoveAt(0);
	}
	//TODO chekc this out later
	//while(_dependents->NumElements() > 0) {
	//    _dependents->RemoveAt(0);
	//}
	delete _r;
	delete _pr;
	delete _rt;
	delete _dependents;
	delete _ancestors;
	delete _phraseAncestors;
	delete _phraseAncestorsID;
	delete _mySBnum;
	delete _causeOfSBinsID;
	delete _brAncestors;
	delete _brDependents;
	delete _depPhrases;
	delete _depFrags;
}

void instruction::setMemAddr(ADDRS memAddr) {
	Assert (memAddr >= 0);
	_memAddr = memAddr;
	//if (debug) printf("addr = %lx\n", _memAddr);
}

void instruction::setInsAddr(ADDRS insAddr) {
	if (insAddr < 0) printf("%s\n",getCmdStr());
	Assert (insAddr >= 0);
	_insAddr = insAddr;
	//if (debug) printf("ins addr = %lx\n", _insAddr);
}

void instruction::setMemAccessSize(long int memAccessSize) {
	Assert (memAccessSize > 0);
	_memAccessSize = memAccessSize;
}

void instruction::setRegister (long int *r, int *rt) {
	long int tempR = *r;
	int tempRT = *rt;
	//Assert(tempR >= 1); //TODO this is remved to enable instruction injection
	Assert(tempRT >= READ && tempRT <= WRITE);
	(_r)->Append(tempR);
	(_rt)->Append(tempRT);
	if (tempRT == WRITE) _numWriteReg++;
	//if (debug) printf("reg = %d, type = %d\n", tempR, tempRT);
}

void instruction::removeNthRegister(int i) {
	Assert(i < _r->NumElements() && i >= 0);
	_r->RemoveAt(i);
	_rt->RemoveAt(i);
}

long int instruction::getNthReg(int i) {
	Assert(i < _r->NumElements() && i >= 0);
	return _r->Nth(i);
}

int instruction::getNthRegType(int i) {
	Assert(i < _r->NumElements() && i >= 0);
	return _rt->Nth(i);
}

int instruction::getNumReg() {
	Assert (_r->NumElements() == _rt->NumElements());
	return _r->NumElements();
}

int instruction::getNumLRFreg() {
	int numReg = 0;
	for (int i = 0; i < _r->NumElements(); i++) {
		if (_r->Nth(i) >= LARF_LO && _r->Nth(i) <= LARF_HI)
			numReg++;
	}
	return numReg;
}

int instruction::getNumWrReg() {
	int numWrReg = 0;
	for (int i = 0; i < _r->NumElements(); i++) {
		if (_rt->Nth(i) == WRITE)
			numWrReg++;
	}
	return numWrReg;
}

void instruction::setType (type insType) {
	Assert(insType != noType);
	_insType = insType;
}

void instruction::setStatus (status insStatus, 
			    long int cycle, 
			    int latency) {
	Assert(insStatus != NO_STAGE);
	_insStatus = insStatus;
	if (_insStatus == execute) {
		_executeCycle  = cycle;
		_latency     = latency;
		_completeCycle = cycle + (long int)latency;
		if (getType() == MEM) { //This block is used for LSQ (Naive, Store Set, etc)
			Assert(latency >= ADDR_COMPUTE_LATENCY);
			_memAddrCompCompleteCycle = cycle + ADDR_COMPUTE_LATENCY;
		}
	} else if (_insStatus == FETCH) {
		_fetchEndCycle = cycle + FETCH_STATE_LATENCY;
	}
}

void instruction::updateLatency(long int cycle, int latency) {
	Assert(_executeCycle > -1 && _executeCycle < cycle);
	_latency       = (latency-(ADDR_COMPUTE_LATENCY+CAM_ACCESS_LATENCY))+(cycle-_executeCycle+1);
	_completeCycle = _executeCycle + _latency;
}

void instruction::setMemType (memType readORwrite) {
	Assert(readORwrite >= none && readORwrite <= WRITE);
	_readORwrite = readORwrite;
}
type instruction::getType () {
	return _insType;
}

status instruction::getStatus () {
	return _insStatus;
}

int instruction::getLatency() {
	return _latency;
}

long int instruction::getCompleteCycle() {
	Assert(_completeCycle > -1);
	return _completeCycle;
}

long int instruction::getMemAddrCompCompleteCycle() {
	Assert(_memAddrCompCompleteCycle > -1);
	return _memAddrCompCompleteCycle;
}

long int instruction::getExecuteCycle() {
	return _executeCycle;
}

long int instruction::getMyReg (int i) { //TODO this func may return wrong data (register renaming)
	Assert (i >= 0 && i < _r->NumElements());
	//Assert (_r->Nth(i) >= 1 && _r->Nth(i) <= RF->getNumberOfRegs());
	Assert (_r->Nth(i) >= 1);
	return _r->Nth(i);
}

ADDRS instruction::getMemAddr() {
	return _memAddr;
}

ADDRS instruction::getInsAddr() {
	return _insAddr;
}

long int instruction::getMemAccessSize() {
	Assert(_memAccessSize > 0 and getType() == MEM);
	return _memAccessSize;
}

memType instruction::getMemType() {
	return _readORwrite;
}


int instruction::getMyRegType(int i) {
	Assert (i >= 0 && i < _rt->NumElements());
	Assert (_rt->Nth(i) >= READ && _rt->Nth(i) <= WRITE);
	return _rt->Nth(i);
}

char* instruction::getCmdStr() {
	return _command;
}

void instruction::setCmdStr(const char * cmd) {
	strcpy(_command, cmd);
}

void instruction::setInsID(INS_ID id) {
	_id = id;
}

INS_ID instruction::getInsID() {
	return _id;
}

void instruction::setBrMode(brMode branchMode) {
	_branchMode = branchMode;
}

brMode instruction::getBrMode() {
	Assert(_branchMode != noBrMode);
	return _branchMode;
}

void instruction::setAsBrAncestor(instruction* ins) {
	Assert(ins->getInsID() < _id);
	_brAncestors->Append(ins);
}

void instruction::setAsBrDependent(instruction* ins) {
	Assert(ins->getInsID() > _id);
	_brDependents->Append(ins);
}

void instruction::delBrDependent(instruction* ins) {
	Assert(ins->getInsID() > _id      && 
	       //ins->getMemType() != WRITE &&
	       ////ins->getType()    != ALU   &&
	       //ins->getType()    != BR    &&
	       //ins->getType()    != ASSIGN);
	       ins->getMemType() == READ && 
	       ins->getMissrate() > unpredMemOpThreshold);

	for (int i = _brDependents->NumElements()-1; i >= 0 ; i--) {
		if (ins->getInsID() == _brDependents->Nth(i)->getInsID() &&
		    ins->getType()  == _brDependents->Nth(i)->getType()) {
			_brDependents->RemoveAt(i);
		}
	}
}

int instruction::getNumBrAncestors() {
	return _brAncestors->NumElements();
}

instruction* instruction::getNthBrAncestor(int i) {
	return _brAncestors->Nth(i);
}

void instruction::delNthBrAncestor(int i) {
	_brAncestors->RemoveAt(i);
}

void instruction::setAsAncestor(instruction* ins) {
	//if (ins->getInsID() > _id) printf("this ins id %llx < its descendent %llx\n",_id,ins->getInsID());
	Assert(ins->getInsID() <= _id);
	_ancestors->Append(ins);
}

int instruction::getNumAncestors() {
	return _ancestors->NumElements();
}

instruction* instruction::getNthAncestor(int i) {
	return _ancestors->Nth(i);
}

void instruction::setAsDependent(instruction* ins) {
	//if (_insStatus == complete) printf("-ins id: %d, type: %d\n", getInsID(), getType());
	Assert(_insStatus != complete);
	_dependents->Append(ins);
}

List<instruction*>* instruction::getDependents() {
	return _dependents;
}

void instruction::notifyAllDepICompleted() {
	for (int i = 0; i < _dependents->NumElements(); i++) {
		_dependents->Nth(i)->releaseDep(this);
	}
	for (int i = 0; i < _brDependents->NumElements(); i++) {
		_brDependents->Nth(i)->releaseBrDep(this);
	}
}

void instruction::notifyAllAncISquashed() {
	for (int i = 0; i < _ancestors->NumElements(); i++) {
		_ancestors->Nth(i)->squashDep(this);
		_guardian--;
	}
	for (int i = 0; i < _brAncestors->NumElements(); i++) {
		_brAncestors->Nth(i)->squashBrDep(this);
	}
}

void instruction::notifyAllDepICompleted_light() {
	for (int i = 0; i < _dependents->NumElements(); i++) {
		_dependents->Nth(i)->releaseDep_light(this);
	}
	for (int i = 0; i < _brDependents->NumElements(); i++) {
		_brDependents->Nth(i)->releaseBrDep(this);
	}
}

void instruction::notifyAllBrAncestorsICompleted() {
	if (getBrMode() == scheduleBr) {
		for (int i = 0; i < _brAncestors->NumElements(); i++) {
			_brAncestors->Nth(i)->releaseBrAncestors(this);
		}
	}
}

void instruction::addDep() {
	_guardian++;
	//TODO might need upper bound assert
}


void instruction::releaseDep_light(instruction* ins) {
	for (int i = _ancestors->NumElements()-1; i >= 0 ; i--) {
		if (ins->getInsID() == _ancestors->Nth(i)->getInsID()) {
			_ancestors->RemoveAt(i);
		}
	}
	_guardian--;
}

void instruction::releaseDep(instruction* ins) {
	for (int i = _ancestors->NumElements()-1; i >= 0 ; i--) {
		if (ins->getInsID() == _ancestors->Nth(i)->getInsID()) {
			_ancestors->RemoveAt(i);
		}
	}
	_guardian--;
	//Assert(_guardian >=0);
	//if ((getBrMode() == scheduleBr || getBrMode() == allBr || getBrMode() == noBr) && _ancestors->NumElements() == 0) {
	if ((getBrMode() == scheduleBr) && _ancestors->NumElements() == 0) {
		goToReadyList();
		removeFromInsMap();
		//If ther is an older BR op, inject a new ins
		//if (getNumBrAncestors() > 0 && getType() != BR && getMemType() != WRITE && getType() != ASSIGN && getType() != ALU) { //TODO I think this line has the bug!
		////if (getNumBrAncestors() > 0 && getMemType() == READ && getMissrate() > unpredMemOpThreshold) { //TODO I think this line has the bug!
		//	_scheduler->injectIns(this);
		////} else {
		////	printf("ERROR: Invalid dependency chanin detected. Aborting...\n");
		////	exit(-1);
		//}
	}
}

void instruction::squashDep(instruction* ins) {
	for (int i = _dependents->NumElements()-1; i >= 0 ; i--) {
		if (ins->getInsID() == _dependents->Nth(i)->getInsID()) {
			_dependents->RemoveAt(i);
		}
	}
}

void instruction::releaseBrAncestors(instruction* ins) {
	Assert(getType() == BR);
	for (int i = _brDependents->NumElements()-1; i >= 0 ; i--) {
		if (ins->getInsID() == _brDependents->Nth(i)->getInsID()) {
			_brDependents->RemoveAt(i);
		}
	}
}

void instruction::releaseBrDep(instruction* ins) {
	for (int i = _brAncestors->NumElements()-1; i >= 0 ; i--) {
		if (ins->getInsID() == _brAncestors->Nth(i)->getInsID()) {
			_brAncestors->RemoveAt(i);
		}
	}
}

void instruction::squashBrDep(instruction* ins) {
	Assert(getType() == BR);
	for (int i = _brDependents->NumElements()-1; i >= 0 ; i--) {
		if (ins->getInsID() == _brDependents->Nth(i)->getInsID()) {
			_brDependents->RemoveAt(i);
		}
	}
}

void instruction::setVliwScheduler(vliwScheduler *scheduler) {
      _scheduler = scheduler;
}

void instruction::insertToInsMap() {
      _scheduler->insertToInsMap(this);
}


void instruction::removeFromInsMap() {
      _scheduler->delFromInsMap(this);
}

//Append myself to the ready instructions list
void instruction::goToReadyList() {
	_rootList->Append(this);
}

void instruction::setReadyLists(List<instruction*>* rootALUins, List<instruction*>* rootMEMins) {
	if (getType() == MEM) {
		_rootList = rootMEMins;
	} else {
		_rootList = rootALUins;
	}
}

bool instruction::isReady(long int cycle) {
	Assert(_guardian >= 0); // && 
	       //_fetchEndCycle > 0);
	//printf("number of dependents = %d\n", _guardian);
	//if (_guardian > 0)	return false;
	//if (_fetchEndCycle > cycle ||	//TODO if you uncomment, be sure all func calls pass correct cycle val
	if(_ancestors->NumElements() > 0) return false;
	else return true;
}

bool instruction::isRepeated(instruction* temp, List<instruction*>*_ancestors) {
	Assert (temp != NULL);
	for (int i = 0; i < _ancestors->NumElements(); i++)
		if (_ancestors->Nth(i)->getInsID() == temp->getInsID()) {
			return true;
		}
	return false;
}

bool instruction::renameRegs(registerRename *GRF, int coreType) {
	Assert((coreType == OUT_OF_ORDER || coreType == PHRASEBLOCK) && "The specified core type is not supported for register renaming.");
	//Do we have enough registers to rename?
	if (GRF->getNumAvailablePR() < _numWriteReg)
		return true; //STALL FETCH

	// Process Read Refisters FIRST
	for (int i = 0; i < getNumReg(); i++) {
		if (coreType == PHRASEBLOCK && !(_r->Nth(i) >= GARF_LO && _r->Nth(i) <= GARF_HI)) continue;
		if (_rt->Nth(i) == READ) {
			PR p_reg = GRF->getRenamedReg(_r->Nth(i));
			switch (GRF->getPRFSM(p_reg)) {
				case AVAILABLE:
					Assert(true == false && "Unexpected register state");
					//This can happen after some squashed
					break;
				case RENAMED_VALID:
					// _r->RemoveAt(i);
					// _r->InsertAt(p_reg,i); //don't have to do it
					break;
				case RENAMED_INVALID:
					//need to impose dependency here
					instruction* ancestor;
					ancestor = GRF->getWriterIns(p_reg);
					if (isRepeated(ancestor,_ancestors)==false) {
						addDep();
						ancestor->setAsDependent(this);
						setAsAncestor(ancestor);
					}
					// _r->RemoveAt(i);
					// _r->InsertAt(p_reg,i); //don't have to do it
					break;
				case ARCH_REG:
					// _r->RemoveAt(i);
					// _r->InsertAt(p_reg,i); //don't have to do it
					break;
				default:
					Assert(true == false && "Invalid PR state");
			};
		}
	}
	// Process Write Refisters SECOND
	for (int i = 0; i < getNumReg(); i++) {
		if (coreType == PHRASEBLOCK && !(_r->Nth(i) >= GARF_LO && _r->Nth(i) <= GARF_HI)) continue;
		if (_rt->Nth(i) == WRITE) {
			AR a_reg = _r->Nth(i);
			if (GRF->isAnyPRavailable()) {
				PR old_pr = GRF->getRenamedReg(a_reg);
				PR new_pr = GRF->getAvailablePR();
				GRF->update_fRAT(a_reg,new_pr);
				GRF->setARST(new_pr,old_pr);
				GRF->updatePRFSM(new_pr,RENAMED_INVALID,this);
				_pr->Append(new_pr);
				_pTOaRegMap.insert(pair<PR,AR>(new_pr,a_reg));
				// _r->RemoveAt(i);
				// _r->InsertAt(new_pr,i); //don't have to do it
			} else {
				Assert(true == false && "An avialable physical register must have been available!");
			}
		}
	}
	return false; //DON'T STALL FETCH
}

void instruction::commitRegs(registerRename *GRF) {
	// Process Write Refisters @complete
	for (int i = 0; i < _pr->NumElements(); i++) {
		PR p_reg = _pr->Nth(i);
		AR a_reg = _pTOaRegMap[p_reg];
		Assert(_pTOaRegMap.find(p_reg) != _pTOaRegMap.end());
		PR old_pr = GRF->getARST(p_reg);
		GRF->updatePRFSM(p_reg,ARCH_REG);
		GRF->updatePRFSM(old_pr,AVAILABLE);
		GRF->update_cRAT(a_reg,p_reg);
		GRF->setAsAvailablePR(old_pr);
		GRF->eraseARST(p_reg);
	}
}

void instruction::completeRegs(registerRename *GRF) {
	// Process Write Refisters @complete
	for (int i = 0; i < _pr->NumElements(); i++) {
		PR p_reg = _pr->Nth(i);
		GRF->updatePRFSM(p_reg,RENAMED_VALID);
	}
}

void instruction::squashRenameReg(registerRename *GRF) {
	for (int i = 0; i < _pr->NumElements(); i++) {
		PR p_reg = _pr->Nth(i);
		Assert(_pTOaRegMap.find(p_reg) != _pTOaRegMap.end());
		GRF->squashRAT(_pTOaRegMap[p_reg]);
		GRF->squashARST(p_reg);
		GRF->squashPRFSM(p_reg);
		GRF->setAsAvailablePR(p_reg);
	}
}

void instruction::noRRdependencyTable (dependencyTable *depTables, int coreType) {
	Assert((coreType == IN_ORDER || coreType == PHRASEBLOCK) && "The specified core type is not supported for register renaming.");
	//Register true dependency check for all instruction types
	for (int i = 0; i < _r->NumElements(); i++) {
		if (coreType == PHRASEBLOCK && !(_r->Nth(i) >= LARF_LO && _r->Nth(i) <= LARF_HI)) continue;
		if (coreType == IN_ORDER    && !(_r->Nth(i) >= LARF_LO && _r->Nth(i) <= GARF_HI)) Assert(true == false && "Invalid register value.");
		if (_rt->Nth(i) == READ) { //TODO Does this line make sense? (int vs. memType)
			instruction *temp = depTables->regLookup(_r->Nth(i),REG_WRITE);//RAW
			if (temp != NULL) {
				if (isRepeated(temp,_ancestors)==false) {
					addDep();
					temp->setAsDependent(this);
					setAsAncestor(temp);
				}
			}
		} else if (_rt->Nth(i) == WRITE) {
			//only for non-OOO cores
			instruction *temp = depTables->regLookup(_r->Nth(i),REG_READ);//WAR
			if (temp != NULL && isRepeated(temp,_ancestors)==false) {
				addDep();
				temp->setAsDependent(this);
				setAsAncestor(temp);
			}
			temp = depTables->regLookup(_r->Nth(i),REG_WRITE);//WAW
			if (temp != NULL && isRepeated(temp,_ancestors)==false) {
				addDep();
				temp->setAsDependent(this);
				setAsAncestor(temp);
			}
		}
	}
	//Update write register table (must be done last to avoid deadlock/wrong dependency)
	for (int i = 0; i < _r->NumElements(); i++) {
		if (coreType == PHRASEBLOCK && !(_r->Nth(i) >= LARF_LO && _r->Nth(i) <= LARF_HI)) continue;
		if (coreType == IN_ORDER    && !(_r->Nth(i) >= LARF_LO && _r->Nth(i) <= GARF_HI)) Assert(true == false && "Invalid register value.");
		if (_rt->Nth(i) == WRITE) {
			depTables->addReg(_r->Nth(i), this, REG_WRITE); //overwrites existing table entry for reg
		} else if (_rt->Nth(i) == READ) { //TODO register renaming breaks this block of code
			depTables->addReg(_r->Nth(i), this, REG_READ); //overwrites existing table entry for reg
		}
	}
}

void instruction::infRegdependencyTable (dependencyTable *depTables, int coreType) {
	//Register true dependency check for all instruction types
	for (int i = 0; i < _r->NumElements(); i++) {
		if (_rt->Nth(i) == READ) { //TODO Does this line make sense? (int vs. memType)
			instruction *temp = depTables->regLookup(_r->Nth(i),REG_WRITE);//RAW
			if (temp != NULL) {
				if (isRepeated(temp,_ancestors)==false) {
					addDep();
					temp->setAsDependent(this);
					setAsAncestor(temp);
				}
			}
		}
	}
	//Update write register table (must be done last to avoid deadlock/wrong dependency)
	for (int i = 0; i < _r->NumElements(); i++) {
		if (_rt->Nth(i) == WRITE) {
			depTables->addReg(_r->Nth(i), this, REG_WRITE); //overwrites existing table entry for reg
		}
	}
}

void instruction::totalOrder_MemDependencyTable (lsq* totalOrderLSQ) {
	if (getMemType() == READ && !totalOrderLSQ->isSQempty()) {
		instruction* ancestor = totalOrderLSQ->getSQtail();
		if (ancestor->getStatus() != complete) { //WAR and RAW
			addDep();
			ancestor->setAsDependent(this);
			setAsAncestor(ancestor);
		}
	} else if (getMemType() == WRITE) {
		if (totalOrderLSQ->isSQfull()) {
			//STALL (add code here)
		} else {
			if (!totalOrderLSQ->isSQempty()) {
				instruction* ancestor = totalOrderLSQ->getSQtail();
				if (ancestor->getStatus() != complete) { //WAW
					addDep();
					ancestor->setAsDependent(this);
					setAsAncestor(ancestor);
				}
			}
			totalOrderLSQ->pushBackSQ(this);
		}
	}
}

void instruction::storeOrder_MemDependencyTable (instruction* ancestor) {
	//Enforce INO execution of ST operations
	if (ancestor == NULL) printf("empty\n");
	if (getMemType() == WRITE) {
		if (ancestor->getStatus() != complete) { //WAW
			addDep();
			ancestor->setAsDependent(this);
			setAsAncestor(ancestor);
		}
	}
}

void instruction::br_dependencyTable (dependencyTable *depTables) {
	// Branch dependency check for all instruction types
	if (getBrMode() != noBr) {
	//if (getBrMode() == lowBiasBr && (getBrBias() >= 0.05 && getBrBias() <= 0.95)) {
		//if (getBrMode() == lowBiasBr && (getBrBias() < 0.05 || getBrBias() > 0.95)) {
		//}
		List<instruction*>* brList = depTables->brLookup();
		for (int i = 0; i < brList->NumElements(); i++) {
			instruction *tempBr = brList->Nth(i);
			if ( getBrMode() == allBr	   || 
			     getBrMode() == dynPredBr  || 
			     getBrMode() == statPredBr || 
			    (getBrMode() == lowBiasBr) || // && (getBrBias() >= 0.05 && getBrBias() <= 0.95)) || 
			    (getBrMode() == scheduleBr && !(getMemType() == READ && getMissrate() > unpredMemOpThreshold))) {
			    //(getBrMode() == scheduleBr && (getMemType() == WRITE || getType() == BR || getType() == ASSIGN))) {// || getType() == ALU))) {
				tempBr->setAsDependent(this);
				addDep();
				setAsAncestor(tempBr);
			}
			setAsBrAncestor(tempBr);
			tempBr->setAsBrDependent(this);
		}
		if (getBrMode() == statPredBr || getBrMode() == dynPredBr) {
			if (getType() == BR && _missPred == true) {depTables->addBr(this);} //Static branch prediction
		} else if (getBrMode() == allBr || getBrMode() == scheduleBr || (getBrMode() == lowBiasBr&& (getBrBias() >= 0.05 && getBrBias() <= 0.95))) {
			if (getType() == BR) {depTables->addBr(this);} //No Static branch prediction
		}
	}
}

void instruction::perfect_MemDependencyTable (dependencyTable *depTables, int coreType, int numSideBuffs) {
	// Dependency check for mem instructions
	if (_readORwrite == READ) { //Mem Read
		instruction *temp = depTables->addrLookup(_memAddr, MEM_WRITE);//RAW
		if (temp != NULL && isRepeated(temp,_ancestors)==false) {
			addDep();
			temp->setAsDependent(this);
			setAsAncestor(temp);
			if (temp->isGotoSideBuff() == true) {
				//goToSideBuff();
				if (coreType != ONE_LEVE_DEEP_STAT) addAsMySB(temp->getSideBuffNum(),temp->getCauseOfSBinsID(),numSideBuffs);
			}
		}
		depTables->addAddr(_memAddr, this, MEM_READ);
	} else if (_readORwrite == WRITE) { //Mem Write
		instruction *temp = depTables->addrLookup(_memAddr, MEM_READ);//WAR
		if (temp != NULL && isRepeated(temp,_ancestors)==false) {
			addDep();
			temp->setAsDependent(this);
			setAsAncestor(temp);
			if (temp->isGotoSideBuff() == true) {
				//goToSideBuff();
				if (coreType != ONE_LEVE_DEEP_STAT) addAsMySB(temp->getSideBuffNum(),temp->getCauseOfSBinsID(),numSideBuffs);
			}
		}
		temp = depTables->addrLookup(_memAddr, MEM_WRITE);//WAW
		if (temp != NULL && isRepeated(temp,_ancestors)==false) {
			addDep();
			temp->setAsDependent(this);
			setAsAncestor(temp);
			if (temp->isGotoSideBuff() == true) {
				//goToSideBuff();
				if (coreType != ONE_LEVE_DEEP_STAT) addAsMySB(temp->getSideBuffNum(),temp->getCauseOfSBinsID(),numSideBuffs);
			}
		}
		depTables->addAddr(_memAddr, this, MEM_WRITE);
	}
}

void instruction::delDepTableEntris(dependencyTable *depTables, int coreType, bool perfectRegRen) {
	for (int i = 0; i < _r->NumElements(); i++) {
		if (perfectRegRen == false && coreType == PHRASEBLOCK && (_r->Nth(i) >= LARF_LO && _r->Nth(i) <= LARF_HI)) continue;
		if (coreType == IN_ORDER    && !(_r->Nth(i) >= LARF_LO && _r->Nth(i) <= GARF_HI)) Assert(true == false && "Invalid register value.");
		if (_rt->Nth(i) == WRITE && (coreType == PHRASEBLOCK || coreType == IN_ORDER || perfectRegRen == true)) {
			depTables->delReg(_r->Nth(i), this, REG_WRITE);
		} else if ( _rt->Nth(i) == READ && coreType == IN_ORDER) {
			depTables->delReg(_r->Nth(i), this, REG_READ);
		}
	}
	if (_readORwrite == READ) {
		depTables->delAddr(_memAddr, this, MEM_READ);
	} else if (_readORwrite == WRITE) {
		depTables->delAddr(_memAddr, this, MEM_WRITE);
	}

	if ((getBrMode() == statPredBr || getBrMode() == dynPredBr) && (getType() == BR && _missPred == true)) {
		//Static branch prediction
		depTables->delBr(this);
	} else if ((getBrMode() == allBr || getBrMode() == scheduleBr || (getBrMode() == lowBiasBr&& (getBrBias() >= 0.05 && getBrBias() <= 0.95))) && getType() == BR) {
		//No Static branch prediction
		depTables->delBr(this);
	}
}

void instruction::delDepTableEntris_LRF(dependencyTable *depTables, int coreType, bool perfectRegRen) {
	Assert(coreType == PHRASEBLOCK);
	if (perfectRegRen == true) return; //don't use this func in perfectRR mode
	for (int i = 0; i < _r->NumElements(); i++) {
		if (!(_r->Nth(i) >= LARF_LO && _r->Nth(i) <= LARF_HI)) continue;
		if (_rt->Nth(i) == WRITE) {
			depTables->delReg(_r->Nth(i), this, REG_WRITE);
		} else if (_rt->Nth(i) == READ) {
			depTables->delReg(_r->Nth(i), this, REG_READ);
		}
	}
}

//Update the map for write-registers
void instruction::updateDepTableEntris(dependencyTable *depTables, int coreType, instruction* replaceIns) {
	//printf("command = %s\n", _command);
	Assert(getMemType() == READ && getMissrate() > unpredMemOpThreshold);
	//Assert(getMemType() != WRITE && getType() != BR && getType() != ASSIGN); // && getType() != ALU);
	for (int i = 0; i < _r->NumElements(); i++) {
		if (_rt->Nth(i) == WRITE) {
			depTables->addReg(_r->Nth(i), replaceIns, REG_WRITE);
		}
	}
}

void instruction::goToSideBuff() {
	_inSideBuff = true;
}

void instruction::getOutSideBuff() {
	_inSideBuff = false;
}

bool instruction::isGotoSideBuff() {
	if (_mySBnum->NumElements() > 0) return true;
	else return false;
}

void instruction::addAsMySB(int sb, INS_ID causeOfSBinsID, int numSideBuffs) {
	Assert (sb >= 0 && sb < numSideBuffs);
	Assert (_mySBnum->NumElements() == _causeOfSBinsID->NumElements());
	if (_currentCauseOfSBinsID < causeOfSBinsID) {
		_currentMySBnum = sb;
		_currentCauseOfSBinsID = causeOfSBinsID;
	}
	_mySBnum->Append(sb);
	_causeOfSBinsID->Append(causeOfSBinsID);
}

void instruction::delMySB(int sb, INS_ID causeOfSBinsID, int numSideBuffs) {
	Assert (sb >= 0 && sb < numSideBuffs);
	Assert (_mySBnum->NumElements() == _causeOfSBinsID->NumElements());
	bool findNewVal = false;
	if (_currentMySBnum == sb && _currentCauseOfSBinsID == causeOfSBinsID) {
		findNewVal = true;
		_currentMySBnum = -1;
		_currentCauseOfSBinsID = 0;
		//find a new current value
	}
	for (int i = _causeOfSBinsID->NumElements()-1; i>= 0; i--) {
		if (_causeOfSBinsID->Nth(i) == causeOfSBinsID && _mySBnum->Nth(i) == sb) {
			//Remove the SB that just completed
			_mySBnum->RemoveAt(i);
			_causeOfSBinsID->RemoveAt(i);
		} else if (findNewVal==true && _currentCauseOfSBinsID < _causeOfSBinsID->Nth(i)) {
			//Find the youngest long lat instruction & its SB
			_currentMySBnum = _mySBnum->Nth(i);
			_currentCauseOfSBinsID = _causeOfSBinsID->Nth(i);
		}
	}
}

void instruction::notifyAllDepGoToSideBuff(int sb, INS_ID causeOfSBinsID, int numSideBuffs) {
	Assert (sb >= 0 && sb < numSideBuffs);

	if (_addFlag != causeOfSBinsID) { //avoid infinite calls
		//This guarantees the youngest long latency op set the SB of an ins in fetch state
		if (_insStatus == FETCH || 
		   (_insStatus == sideBuffer && _currentMySBnum == sb)||
		   //(_insStatus == complete && _currentMySBnum == sb)
		   (_id == causeOfSBinsID))
		{
			addAsMySB(sb, causeOfSBinsID, numSideBuffs);
		}
		for (int i = 0; i < _dependents->NumElements(); i++) {
			_dependents->Nth(i)->notifyAllDepGoToSideBuff(sb,causeOfSBinsID,numSideBuffs);
		}
		_addFlag = causeOfSBinsID;
	}
}

long int instruction::notifyAllDepGetOutSideBuff(int sb, INS_ID causeOfSBinsID, int numSideBuffs) {
	Assert (sb >= 0 && sb < numSideBuffs);
	long int _numDepInOtherSBs = 0;
	if (_delFlag != causeOfSBinsID) { //avoid infinite calls
		if (_insStatus == FETCH) {
			delMySB(sb,causeOfSBinsID,numSideBuffs);
		} else if (_insStatus == sideBuffer && _currentMySBnum != sb) {
			_numDepInOtherSBs++;
		}
		for (int i = 0; i < _dependents->NumElements(); i++) {
			_numDepInOtherSBs += _dependents->Nth(i)->notifyAllDepGetOutSideBuff(sb, causeOfSBinsID,numSideBuffs);
		}
		_delFlag = causeOfSBinsID;
		return _numDepInOtherSBs;
	}
	return 0;
}

int instruction::getSideBuffNum() {
	Assert (_currentMySBnum != -1);
	return _currentMySBnum;
}

INS_ID instruction::getCauseOfSBinsID() {
	Assert(_currentCauseOfSBinsID != 0);
	return _currentCauseOfSBinsID;
}

void instruction::setPiepelineLat (int pipeLineLat) {
	Assert (pipeLineLat > 0);
	_pipeLineLat = pipeLineLat;
}

int instruction::getPipelineLat () {
	Assert (_pipeLineLat != -1);
	return _pipeLineLat;
}

int instruction::findLongestPath(long int cycle, bool UPLDhoisting, long int myPhraseID) {
	//only access the children one, per cycle to aviud runtime performance loss
	if (_myPhraseID == myPhraseID) {
		if (_findLPflag != (INS_ID) cycle) {
			//printf ("%d - %s", getInsID(), getCmdStr());
			_critPathLen = -1;
			for (int i = 0; i < _dependents->NumElements(); i++) {
				int pathLength = _dependents->Nth(i)->findLongestPath(cycle, UPLDhoisting, myPhraseID);
				if (_critPathLen < pathLength) {
					_critPathLen = pathLength;
				}
			}
			if (_critPathLen == -1) {
				if (UPLDhoisting == true
				    && getMissrate() > unpredMemOpThreshold
				    && _readORwrite == READ) {
					_critPathLen = cacheLat[1];
				} else {
					_critPathLen = getPipelineLat();
				}
			} else { 
				if (UPLDhoisting == true
				    && getMissrate() > unpredMemOpThreshold
				    && _readORwrite == READ) {
					_critPathLen += cacheLat[1];
				} else {
					_critPathLen += getPipelineLat();
				}
			}
			_findLPflag = cycle;
		}
		Assert (_findLPflag != 0 && _critPathLen > 0);
		return _critPathLen;
	} else {
		return 0;
	}
}

int instruction::findLongestPathDynamicly(long int cycle, bool UPLDhoisting) {
	//only access the children one, per cycle to aviud runtime performance loss
	if (_findLPflag != (INS_ID)cycle) {
		_critPathLen = -1;
		for (int i = 0; i < _dependents->NumElements(); i++) {
			int pathLength = _dependents->Nth(i)->findLongestPathDynamicly(cycle, UPLDhoisting);
			if (_critPathLen < pathLength) {
				_critPathLen = pathLength;
			}
		}
		if (_critPathLen == -1) {
			if (UPLDhoisting == true
			    && getMissrate() > unpredMemOpThreshold
			    && _readORwrite == READ) {
				_critPathLen = cacheLat[1];
			} else {
				_critPathLen = getPipelineLat();
			}
		} else {
			if (UPLDhoisting == true
			    && getMissrate() > unpredMemOpThreshold
			    && _readORwrite == READ) {
				_critPathLen += cacheLat[1];
			} else {
				_critPathLen += getPipelineLat();
			}
		}
		//TODO an experiment ------------------
		//if (_critPathLen < _dependents->NumElements()) {
		//	//printf("%d, %d, %d\n",_id, _critPathLen,_dependents->NumElements());
		//	_critPathLen = _dependents->NumElements();
		//}
		//TODO an experiment ------------------
		_findLPflag = cycle;
	}
	Assert (_findLPflag != 0 && _critPathLen > 0);
	return _critPathLen;
}

int instruction::getLongestPath() {
	//if (_critPathLen == -1)
	//	printf ("%d - %s", getInsID(), getCmdStr());
	Assert(_critPathLen != -1);
	return _critPathLen;
}

//set _pathLen to the longest path of phrase
void instruction::lookUpANDsetPathLen() {
	Assert(_pathLen == -1);
	//Find the max path-length of the ancestors
	for (int i = 0; i < _ancestors->NumElements(); i++) { //TODO this can be made efficient (see isOnCritiPath())
		if (_ancestors->Nth(i)->getMyPhraseID() == getMyPhraseID() && 
		    _pathLen < _ancestors->Nth(i)->getPathLen()) {
			_pathLen = _ancestors->Nth(i)->getPathLen();
		}
	}
	//add my own path length
	if (_pathLen == -1) {_pathLen = 0;}
	_pathLen += getMyPathLen();
	Assert(_pathLen > -1);
}

int instruction::getPathLen() {
	Assert(_pathLen > -1);
	return _pathLen;
}

bool instruction::isOnCritiPath(int candidatePhID) {
	Assert(candidatePhID > -1);
	int maxPathLen=-1;
	for (int i = 0; i < _ancestors->NumElements(); i++) {
		if (candidatePhID == _ancestors->Nth(i)->getMyPhraseID() && maxPathLen < _ancestors->Nth(i)->getPathLen()) {
			maxPathLen = _ancestors->Nth(i)->getPathLen();
		}
	}
	if (maxPathLen == -1) {maxPathLen = 0;}
	maxPathLen += getMyPathLen();
	int critPathThreshold = phraseSizeBound / NUM_FUNC_UNIT;
	if (maxPathLen > critPathThreshold) return true;
	else return false;
}

int instruction::getMyPathLen() {
	if (_insType == ALU) {
		return ALU_LATENCY;
	} else if (_insType == MEM) {
		return cacheLat[0];
	} else if (_insType == FPU) {
		return FPU_LATENCY;
	} else if (_insType == BR) {
		return ALU_LATENCY; //TODO is this a correct latency?
	} else if (_insType == ASSIGN) {
		return ASSIGN_LATENCY; //TODO is this a correct latency?
	} else {
		printf("ERROR: invalid instruction type.\n");
		exit(1);
	}
}

void instruction::renameWriteReg(long int reg) {
	nextRenReg++;
	if (!(reg < NUM_REGISTERS && nextRenReg > INIT_RENAME_REG_NUM)) printf("cmd = %s --- %lld\n", getCmdStr(),_id);
	Assert(reg < NUM_REGISTERS && nextRenReg > INIT_RENAME_REG_NUM);
	Assert(nextRenReg > INIT_RENAME_REG_NUM);
	writeRegRenMap.insert(pair<long int,long int>(reg,nextRenReg));
}

void instruction::renameReadReg(int indx, long int renReg) {
	Assert(_rt->Nth(indx) == READ);
	int num1 = _r->NumElements();
	if (renReg != -1) {
		_r->RemoveAt(indx);
		_r->InsertAt(renReg,indx);
	}
	int num2 = _r->NumElements();
	Assert (num1 == num2);
}

long int instruction::getRenamedReg(long int reg) {
	if (writeRegRenMap.find(reg) != writeRegRenMap.end())
		return writeRegRenMap.find(reg)->second;
	else
		return -1; //reg is not returned
}

void instruction::printToFile(FILE *reScheduleFile, bool recordHitMiss) {
	if (_insType == ALU) {
		fprintf(reScheduleFile, "\nA,%llu,",getInsAddr());
		//printf("\nA,%ld,",getInsAddr());
	} else if (_insType == FPU) {
		fprintf(reScheduleFile, "\nF,%llu,",getInsAddr());
		//printf("\nF,%ld,",getInsAddr());
	} else if (_insType == BR) {
		fprintf(reScheduleFile, "\nB,%llu,%d,%ld,",getInsAddr(),_brTaken,_brTarget);
		//printf("\nB,%ld,%d,%ld,",getInsAddr(),_brTaken,_brTarget);
	} else if (_insType == MEM && _readORwrite == READ) {
		fprintf(reScheduleFile, "\nR,%llu,%llu,",getMemAddr(),getInsAddr());
		//printf("\nR,%ld,%ld,",getMemAddr(),getInsAddr());
	} else if (_insType == MEM && _readORwrite == WRITE) {
		fprintf(reScheduleFile, "\nW,%llu,%llu,",getMemAddr(),getInsAddr());
		//printf("\nW,%ld,%ld,",getMemAddr(),getInsAddr());
	} else if (_insType == ASSIGN) {
		fprintf(reScheduleFile, "\nT,%llu,",getInsAddr());
		//printf("\nT,%ld,",getInsAddr());
	}

	if (recordHitMiss == true) {
		//COEFF used for conveient parsing later on
		long int temp = COEFF * getMissrate();
		Assert(temp <= COEFF && temp >= 0);
		fprintf(reScheduleFile, "%ld,", temp);
		//printf("%ld,", temp);
		//fprintf(reScheduleFile, "%d,", _hitLevel); //TODO remove - old version
		//printf("%f | %s", getMissrate(), getCmdStr());
		Assert((getMissrate() >= 0.0 && getMissrate() <= 1.0));// &&  _readORwrite == READ) ||
		       //(getMissrate() == 0.0 && (_readORwrite == WRITE || _insType == ALU || _insType == FPU)));
	}

	Assert(_r->NumElements() == _rt->NumElements());
	for (int i = 0; i < _r->NumElements(); i++) {
		if (_rt->Nth(i) == READ) {
			fprintf(reScheduleFile, "%ld#%d,",_r->Nth(i),_rt->Nth(i));
			//printf("%ld#%d,",_r->Nth(i),_rt->Nth(i));
		} else if (_rt->Nth(i) == WRITE) {
			long int r = getRenamedReg(_r->Nth(i));
			if (r != -1) {
				fprintf(reScheduleFile, "%ld#%d,",r,_rt->Nth(i));
				//printf("%ld#%d,",r,_rt->Nth(i));
			} else { 
				fprintf(reScheduleFile, "%ld#%d,",_r->Nth(i),_rt->Nth(i));
				//printf("%ld#%d,",_r->Nth(i),_rt->Nth(i));
			}
		}
	}
}

void instruction::findPhraseAncestors() {
	genPhraseAncestorsList(_id, this);
	//Check if 'any' of its ancestors has ever been a UPLD
	for (int i = 0; i < _ancestors->NumElements(); i++) {
		if (_ancestors->Nth(i)->getDepOnUPLD() == true) {
			setDepOnUPLD();
		}
	}
}

void instruction::genPhraseAncestorsList(int causeOfPhraseinsID, instruction* ins) {
	//Find the immediate UPLD ops
	for (int i = 0; i < _ancestors->NumElements(); i++) {
		if (_ancestors->Nth(i)->getMemType() == READ && 
		    _ancestors->Nth(i)->getMissrate() > unpredMemOpThreshold) {
			addAsPhraseAncestor(_ancestors->Nth(i));
		}
	}
}

//This function is for UPLD ops to realse their dependents
void instruction::notifyDepICommited() {
	for (int i = 0; i < _dependents->NumElements(); i++)
		_dependents->Nth(i)->releaseDepFromUPLD(this);
}

//This function is for UPLD ops to realse their dependents
void instruction::releaseDepFromUPLD(instruction* ins) {
	for (int i = _phraseAncestors->NumElements()-1; i >= 0 ; i--) {
		if (ins->getInsID() == _phraseAncestors->Nth(i)->getInsID() && ins->getMemType() == READ) {
			_phraseAncestors->RemoveAt(i);
			_phraseAncestorsID->RemoveAt(i);
		}
	}
}

//This function is not used anywhere at the moment
void instruction::genAllPhraseAncestorsList(INS_ID causeOfPhraseinsID, instruction* ins) {
	if (_phraseAddFlag != causeOfPhraseinsID) {
		//printf("hit level = %d\n", _hitLevel);
		if (getMemType() == READ && _hitLevel > 1 && _id != ins->getInsID()) {
			ins->addAsPhraseAncestor(this);
		}
		//if (_ancestors->NumElements()==0) printf("no aestor %d\n", _id);
		//else printf("yes aestor %d\n",_id);
		for (int i = 0; i < _ancestors->NumElements(); i++) {
			_ancestors->Nth(i)->genAllPhraseAncestorsList(causeOfPhraseinsID, ins);
		}
		_phraseAddFlag = causeOfPhraseinsID;
	}
	Assert(_phraseAddFlag != 0);
}

void instruction::addAsPhraseAncestor(instruction* ins) {
	Assert(ins->getMemType() == READ);
	//Assert(ins->getInsID() < getInsID());
	_phraseAncestors->Append(ins);
	_phraseAncestorsID->Append(ins->getInsID());
}

int instruction::getNumMemRdAncestors() {
	//NOTE: read the note for getNthMemRdAncestor()
	Assert(_phraseAncestorsID->NumElements() == _phraseAncestors->NumElements());
	return _phraseAncestorsID->NumElements();
}

instruction* instruction::getNthMemRdAncestor(int i) {
	//NOTE: this function can accessed anytime as opposed to getNthMemRdAncestor
	return _phraseAncestors->Nth(i);
}

int instruction::getNthMemRdAncestorID(int i) {
	//NOTE: this function can only be accessed only when the instruction is joining a wavefront
	//accessing it later (in time) may cause seg fault as the ancestors may leave ROB
	return _phraseAncestorsID->Nth(i);
}

//Only used for MEM ops. Other ops are assumed to have hitLat=-1
void instruction::setCacheHitLevel(int hitLat) {
	Assert(getType() == MEM);

	//Assume a 3-level cache system
	if (hitLat <=  cacheLat[0])
		_hitLevel = 1;
	else if (hitLat <=  cacheLat[1])
		_hitLevel = 2;
	else if (hitLat <=  cacheLat[2])
		_hitLevel = 3;
	else
		_hitLevel = 4;
}

void instruction::importCacheHitLevel(int hitLevel) {
	Assert(getType() == MEM);
	_hitLevel = hitLevel;
}


int instruction::getCacheHitLevel() {
	return _hitLevel;
}

void instruction::setMyPhrase(phrase* ph) {
	Assert(_myPhraseID == -1 && ph->getPhraseID() > -1);
	_myPhraseID = ph->getPhraseID();
	_myPhrase = ph;
}

void instruction::setMyPhraseID(int id) {
	if (_myPhraseID != -1) printf("id: %d, type: %d", _myPhraseID, getType());
	Assert(_myPhraseID == -1);
	_myPhraseID = id;
}

phrase* instruction::getMyPhrase() {
	return _myPhrase;
}

int instruction::getMyPhraseID() {
	if (!(_myPhraseID >= 0))printf("%d, %s", getStatus(), getCmdStr());
	Assert(_myPhraseID >= 0);
	return _myPhraseID;
}

void instruction::addDepPhrase(phrase* ph) {
	_depPhrases->Append(ph);
}

void instruction::notifyMyDepPhrasesICompleted() {
	for (int i = 0; i < _depPhrases->NumElements(); i++) {
		_depPhrases->Nth(i)->removeAncestorIns(this);
	}
}

void instruction::addDepFrag(fragment* fr) {
	_depFrags->Append(fr);
}

void instruction::notifyMyDepFragsICompleted() {
	for (int i = 0; i < _depFrags->NumElements(); i++) {
		_depFrags->Nth(i)->removeAncestorIns(this);
	}
}

void instruction::setMissRate(double missRate) {
	//printf("missRate = %f\n",missRate);
	Assert(missRate >= 0.0 && missRate <= 1.0); 
	_missRate = missRate;
	if (getMemType() == READ && 
	    _missRate > unpredMemOpThreshold) {
		setDepOnUPLD();
	}
}

double instruction::getMissrate() {
	return _missRate;
}

void instruction::setCauseOfFragInsID(INS_ID causeOfFragInsID) {
	Assert(causeOfFragInsID != 0);
	_causeOfFragInsID = causeOfFragInsID;

}
INS_ID instruction::getCauseOfFragInsID() {
	//Assert(_causeOfFragInsID != -1);
	return _causeOfFragInsID;
}

void instruction::setMyFrag(fragment* fr) {
	Assert(_myFragID == -1 && fr->getFragID() >= 0);
	_myFragID = fr->getFragID();
	_myFrag = fr;
}

fragment* instruction::getMyFrag() {
	return _myFrag;
}

int instruction::getMyFragID() {
	return _myFragID;
}

void instruction::setDepOnUPLD() {
	//Assert(_hasUPLDancestor == false); //TODO u should remove it i think
	_hasUPLDancestor = true;
}

bool instruction::getDepOnUPLD() {
	return _hasUPLDancestor;
}

float instruction::getBrBias () {
	Assert(_brBias >= 0.0 && _brBias <= 1.0);
	return _brBias;
}

float instruction::getBrAccuracy () {
	Assert(_brAccuracy >= 0.0 && _brAccuracy <= 1.0);
	return _brAccuracy;
}

void instruction::setBrTarget(long int brTarget) {
	if (brTarget <= 0) printf("Branch Target Addr: %ld, ins = %s", brTarget, getCmdStr());
	Assert(brTarget > 0);
	_brTarget = brTarget;
}

void instruction::setBrSide(long int brTaken) {
	Assert(brTaken == 0 || brTaken == 1);
	brTaken == 0 ? _brTaken = false : _brTaken = true;
}

bool instruction::getBrSide() {
	return _brTaken;
}

void instruction::setBrForward() {
	Assert(_brTarget > 0 || _insAddr > 0);
	//long long int diff = _insAddr - _brTarget;
	//if      (diff < 0 && _insAddr < _brTarget) _brForward = true;
	//else if (diff > 0 && _insAddr > _brTarget) _brForward = false;
	//else Failure ("Compute overflow occured!");
}

void instruction::setBrBias (float brBias) {
	Assert(brBias >= 0.0 && brBias <= 1.0);
	_brBias = brBias;
}

void instruction::setBrAccuracy (float brAccuracy) {
	Assert(brAccuracy >= 0.0 && brAccuracy <= 1.0);
	_brAccuracy = brAccuracy;
}

void instruction::findMissPrediction(bool missPred) {
	if (getBrMode() == dynPredBr) { //dynamic pred
		_missPred = missPred;
	} else { //static pred
		if ((_brForward == true  && _brTaken == true) ||
		    (_brForward == false && _brTaken == false)) {
			_missPred = true; //Static predictor failed
		} else {
			_missPred = false;//Correct prediction
		}
	}
}

bool instruction::getMissPrediction() {
	return _missPred;
}

void instruction::setPrediction(bool brPred) {
	Assert(getBrMode() == dynPredBr && "wrong usage of the function");
	Assert(getType() == BR);
	_brPred = brPred;
}

bool instruction::getPrediction() {
	Assert(_bp_hist != NULL && "_bp_hist object must not be null");
	return _bp_hist;
}

void instruction::setPredHistObj(void* bp_hist) {
	Assert(bp_hist != NULL && "bp_hist object must not be null");
	_bp_hist = bp_hist;
}

void* instruction::getPredHistObj() {
	Assert(_bp_hist != NULL && "_bp_hist object must not be null");
	return _bp_hist;
}

void instruction::setBBtail() {
	_bbTail = true;
}

void instruction::setBBhead() {
	_bbHead = true;
}

void instruction::setBrHeaderAddr(INS_ADDR brAddr) {
	Assert(_bbHead == true);
	_brHeaderAddr = brAddr;
}

bool instruction::isBBtail() {
	return _bbTail;
}

bool instruction::isBBhead() {
	return _bbHead;
}

INS_ADDR instruction::getBrHeaderAddr() {
	//NOTE: this assert does not work because no branch header
	//is constructed when a mid-BB squash is reconstructed
	//(because H is not reconstucted after squash)
	//Assert(_bbHead == true && _brHeaderAddr > 0);
	Assert(_bbHead == true);
	return _brHeaderAddr;
}

	//if (_inSideBuff == false) { //avoid infinite loop calls
	//	//send myself to side buffer
	//	goToSideBuff();
	//	//Only notify if either in fetch state or already belong to the same SB
	//	//This guarantees that the youngest long latency op set the SB of an ins in fetch state
	//	if (_insStatus==fetch || _mySBnum == sb) { //TODO triple check this condition!
	//		_mySBnum = sb;
	//		_causeOfSBinsID = causeOfSBinsID;
	//		// send dependents to side buffer 
	//		// (future dependents will not know - 
	//		// need special treatment for those)
	//		for (int i = 0; i < _dependents->NumElements(); i++) {
	//			_dependents->Nth(i)->notifyAllDepGoToSideBuff(sb,_causeOfSBinsID,numSideBuffs);
	//		}
	//	}
	//}
	////printf("num elements: %d\n", _dependents->NumElements());

	//if (_inSideBuff == true && _mySBnum == sb &&
	//   (_insStatus == fetch || _insStatus == complete || _insStatus == sideBuffer)) { //avoid infinite loop calls
	//	//send myself to side buffer
	//	getOutSideBuff(); 
	//	// send dependents to side buffer 
	//	// (future dependents will not know - 
	//	// need special treatment for those)
	//	_causeOfSBinsID = -1; //TODO double check this one for correctnes
	//	for (int i = 0; i < _dependents->NumElements(); i++)
	//		_dependents->Nth(i)->notifyAllDepGetOutSideBuff(sb, causeOfSBinsID,numSideBuffs);
	//}
