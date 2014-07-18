/*******************************************************************************
 *  instruction.cpp
 *  Instruction object. Each instruction is placed into the instruction window for
 *  issue and then removed from the window when commited.
 ******************************************************************************/
#include <stdio.h>
#include "instruction.h"

instruction::instruction() {
	_insType   = noType;
	_insStatus = noStatus;
	_readORwrite = none;

	_r  = new List<int*>;
	_rt = new List<int*>;
	_memAddr = 0;
	
	_issueCycle  = -1;
	_latency     = -1;
	_completeCycle = -1;
}

void instruction::setMemAddr(uint64_t memAddr) {
	_memAddr = memAddr;
	//if (debug) printf("addr = %lx\n", _memAddr);
}

void instruction::setRegister ( int *r, int *rt) {
	//Assert(r1 != 0 && r2 != 0 && r3 != 0); //TODO double check this
	(_r)->Append(r);
	(_rt)->Append(rt);
	//if (debug) printf("reg = %d, type = %d\n", *r, *rt);
}

void instruction::setType (type insType) {
	//Assert(insType != NULL);
	_insType = insType;
}

void instruction::setStatus (status insStatus, 
			    int cycle, 
			    int latency) {
	//Assert(insStatus != NULL);
	_insStatus = insStatus;
	if (_insStatus == issue) {
		_issueCycle  = cycle;
		_latency     = latency;
		_completeCycle = cycle + latency;
	}
}

void instruction::setMemType (memType readORwrite) {
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

int instruction::getCompleteCycle() {
	return _completeCycle;
}

int instruction::getIssueCycle() {
	return _issueCycle;
}

int instruction::getMyReg (int i) {
	//Assert (i >= 0 && i < _r->NumElements());
	return *_r->Nth(i);
}

uint64_t instruction::getMemAddr() {
	return _memAddr;
}

memType instruction::getMemType() {
	return _readORwrite;
}


int instruction::getNumReg() {
	//Assert (_r->NumElements() == _rt->NumElements());
	return _r->NumElements();
}

int instruction::getMyRegType(int i) {
	//Assert (i >= 0 && i < _rt->NumElements());
	return *_rt->Nth(i);
}
