/*******************************************************************************
 *  registerRename.cpp
 ******************************************************************************/

#include "registerRename.h"
#include "instruction.h"

registerRename::registerRename() {
	//Initialize all tables
	int PR_counter = GPRF_LO;
	// Initialize Architectural Register domain
	for (int i = GARF_LO; i <= GARF_HI; i++) {
		_fRAT.insert(pair<AR,PR>(i,PR_counter));
		_cRAT.insert(pair<AR,PR>(i,PR_counter));
		_PRFSM.insert(pair<PR,REG_REN_STATE>(PR_counter,ARCH_REG));
		PR_counter++;
	}
	// Initialize Rename Register domain
	while (PR_counter <= GPRF_HI) {
		_availablePRset.push_back(PR_counter);
		_PRFSM.insert(pair<PR,REG_REN_STATE>(PR_counter,AVAILABLE));
		PR_counter++;
	}
	// Initialize the whole Physical Register domain
	for (int i = GPRF_LO; i <= GPRF_HI; i++) {
		_PRvalid.insert(pair<PR,VALID>(i,false));
		_writeInstructions.insert(pair<PR,instruction*>(i,NULL));
	}
	Assert(_writeInstructions.size() == GPRF_SIZE && "Invalid Rename table structure initialization.");
	Assert(_availablePRset.size() == GRRF_SIZE && "Invalid Rename table structure initialization.");
	Assert(_PRFSM.size() == GPRF_SIZE && "Invalid Rename table structure initialization.");
	Assert(_fRAT.size() == GARF_SIZE && "Invalid Rename table structure initialization.");
	Assert(_cRAT.size() == GARF_SIZE && "Invalid Rename table structure initialization.");
}

registerRename::registerRename(int arf_lo, int arf_hi) {
	unsigned int arf_size = arf_hi-arf_lo+1;
	int prf_lo = GPRF_LO, prf_hi = prf_lo+arf_size+GRRF_SIZE-1;
	unsigned int prf_size = prf_hi-prf_lo+1;
	//Initialize all tables
	int PR_counter = prf_lo;
	// Initialize Architectural Register domain
	for (int i = arf_lo; i <= arf_hi; i++) {
		_fRAT.insert(pair<AR,PR>(i,PR_counter));
		_cRAT.insert(pair<AR,PR>(i,PR_counter));
		_PRFSM.insert(pair<PR,REG_REN_STATE>(PR_counter,ARCH_REG));
		PR_counter++;
	}
	// Initialize Rename Register domain
	while (PR_counter <= prf_hi) {
		_availablePRset.push_back(PR_counter);
		_PRFSM.insert(pair<PR,REG_REN_STATE>(PR_counter,AVAILABLE));
		PR_counter++;
	}
	// Initialize the whole Physical Register domain
	for (int i = prf_lo; i <= prf_hi; i++) {
		_PRvalid.insert(pair<PR,VALID>(i,false));
		_writeInstructions.insert(pair<PR,instruction*>(i,NULL));
	}
	Assert(_writeInstructions.size() == prf_size && "Invalid Rename table structure initialization.");
	Assert(_availablePRset.size() == GRRF_SIZE && "Invalid Rename table structure initialization.");
	Assert(_PRFSM.size() == prf_size && "Invalid Rename table structure initialization.");
	Assert(_fRAT.size() == arf_size && "Invalid Rename table structure initialization.");
	Assert(_cRAT.size() == arf_size && "Invalid Rename table structure initialization.");
}

registerRename::~registerRename() {
	; //nothing to be done
}

PR registerRename::getRenamedReg(AR a_reg) {
	if (_fRAT.find(a_reg) == _fRAT.end()) {printf("1) %u\n", a_reg);}
	Assert(_fRAT.find(a_reg) != _fRAT.end() && "The arch. reg was not found in RAT!");
	return _fRAT[a_reg];
}

void registerRename::update_fRAT(AR a_reg, PR p_reg) {
	if (_fRAT.find(a_reg) != _fRAT.end()) {
		// printf("erasing %d->%d to %d->%d\n",a_reg,_fRAT[a_reg],a_reg,p_reg);
		_fRAT.erase(a_reg);
	}
	_fRAT.insert(pair<AR,PR>(a_reg,p_reg));
}

void registerRename::update_cRAT(AR a_reg, PR p_reg) {
	Assert(_cRAT.find(a_reg) != _cRAT.end());
	Assert(_PRFSM[_cRAT[a_reg]] == AVAILABLE);
	Assert(_PRFSM[p_reg] == ARCH_REG);
	_cRAT.erase(a_reg);
	_cRAT.insert(pair<AR,PR>(a_reg,p_reg));
}

void registerRename::squashRAT(AR a_reg) {
	Assert(_fRAT.find(a_reg) != _fRAT.end());
	Assert(_PRFSM[_cRAT[a_reg]] == ARCH_REG);
	_fRAT.erase(a_reg);
	_fRAT.insert(pair<AR,PR>(a_reg,_cRAT[a_reg]));
}

bool registerRename::isAnyPRavailable() {
	if (_availablePRset.size() == 0)
		return false;
	else
		return true;
}

PR registerRename::getAvailablePR() {
	Assert(_availablePRset.size() > 0 && "Invalid use-set size.");
	PR p_reg = _availablePRset.back();
	Assert(getPRFSM(p_reg) == AVAILABLE && "Register State is Invalid - Register must be in Available State");
	_availablePRset.pop_back();
	return p_reg;
}

void registerRename::setAsAvailablePR(PR p_reg) {
	_availablePRset.push_back(p_reg);
	Assert(_availablePRset.size() <= GRRF_SIZE && "Rename table has grown too large (size violation).");
}

int registerRename::getNumAvailablePR() {
	return _availablePRset.size();
}

void registerRename::setARST(PR new_pr,PR old_pr) {
	Assert(_ARST.find(new_pr) == _ARST.end() && "The arch. reg was found in ARST!");
	_ARST.insert(pair<PR,PR>(new_pr,old_pr));
} //TODO any checks for old_pr as key?

PR registerRename::getARST(PR p_reg) {
	Assert(_ARST.find(p_reg) != _ARST.end() && "The arch. reg was not found in ARST!");
	return _ARST[p_reg];
}

void registerRename::eraseARST(PR p_reg) {
	Assert(_ARST.find(p_reg) != _ARST.end() && "The arch. reg was not found in ARST!");
	_ARST.erase(p_reg);
}

void registerRename::squashARST(PR p_reg) {
	_ARST.erase(p_reg);
}

void registerRename::flush_fRAT() {
	_fRAT.clear();
	for (map<AR,PR>::iterator it = _cRAT.begin(); it != _cRAT.end(); it++) {
		AR a_reg = it->first;
		PR p_reg = it->second;
		_fRAT.insert(pair<AR,PR>(a_reg,p_reg));
	}
	Assert(_fRAT.size() == _cRAT.size());
}

void registerRename::updatePRFSM(PR p_reg, REG_REN_STATE state) {
	//This function does NOT handle instruction cancellation
	Assert(_PRFSM.find(p_reg) != _PRFSM.end() && "Physical register state value was not found!");
	Assert(((_PRFSM[p_reg] == RENAMED_INVALID && state == RENAMED_VALID) ||
			(_PRFSM[p_reg] == RENAMED_VALID && state == ARCH_REG) ||
			(_PRFSM[p_reg] == ARCH_REG && state == AVAILABLE)) &&
			"Phys Reg. State Transition Violation.");
	_PRFSM[p_reg] = state;
	if (state == RENAMED_VALID) {
		Assert(_writeInstructions[p_reg] != NULL && "A writer instruction must have been assigned to phys reg.");
		_writeInstructions[p_reg] = NULL;
	}
}

void registerRename::updatePRFSM(PR p_reg, REG_REN_STATE state, instruction* writerIns) {
	//This function does NOT handle instruction cancellation
	Assert(_PRFSM.find(p_reg) != _PRFSM.end() && "Physical register state value was not found!");
	Assert((_PRFSM[p_reg] == AVAILABLE && state == RENAMED_INVALID) && "Phys Reg. State Transition Violation.");
	_PRFSM[p_reg] = state;
	Assert(_writeInstructions[p_reg] == NULL && "No writer instruction must have been assigned to phys reg.");
	_writeInstructions.erase(p_reg);
	_writeInstructions.insert(pair<PR,instruction*>(p_reg,writerIns));
}

void registerRename::squashPRFSM(PR p_reg) {
	//This function does NOT handle instruction cancellation
	Assert(_PRFSM.find(p_reg) != _PRFSM.end() && "Physical register state value was not found!");
	Assert(_PRFSM[p_reg] == RENAMED_INVALID || _PRFSM[p_reg] == RENAMED_VALID);
	REG_REN_STATE state = _PRFSM[p_reg];
	_PRFSM[p_reg] = AVAILABLE;
	if (state == RENAMED_INVALID) {
		Assert(_writeInstructions[p_reg] != NULL && "A writer instruction must have been assigned to phys reg.");
		_writeInstructions[p_reg] = NULL;
	}
}

instruction* registerRename::getWriterIns(PR p_reg) {
	Assert(_PRFSM[p_reg] == RENAMED_INVALID && "Phys Reg. State Transition Violation.");
	instruction *writerIns = _writeInstructions[p_reg];
	Assert(writerIns != NULL && "The physical register has no pending writer!");
	return writerIns;
}

REG_REN_STATE registerRename::getPRFSM(PR p_reg) {
	if (_PRFSM.find(p_reg) == _PRFSM.end()) {printf("2) %u\n", p_reg);}
	Assert(_PRFSM.find(p_reg) != _PRFSM.end() && "Physical register state value was not found!");
	return _PRFSM[p_reg];
}
