/*******************************************************************************
 * basicblock.cpp
 *******************************************************************************/

#include "basicblock.h"

basicblock::basicblock() {
	_s_insList = new List<string*>;
	_o_insList = new List<instruction*>;
	_insAddrList = new List<ADDRS>;
	_BBheader = NULL;
	_hasBBheader = false;
	_bbAddr = 0;
}

basicblock::~basicblock() {
	if (_s_insList->NumElements() != _insAddrList->NumElements()) 
		cout << _s_insList->NumElements() << " " <<  _insAddrList->NumElements() << endl;
	Assert(_s_insList->NumElements() == _insAddrList->NumElements());
	for (int i = _s_insList->NumElements()-1; i >= 0; i--) {
		delete _s_insList->Nth(i);
	}
	for (int i = _o_insList->NumElements()-1; i >= 0; i--) {
		delete _o_insList->Nth(i);
	}
	delete _s_insList;
	delete _o_insList;
	delete _insAddrList;
}

void basicblock::addToBB(string* ins, ADDRS insAddr) {
	Assert(ins != NULL);
	_s_insList->Append(ins);
	_insAddrList->Append(insAddr);
}

void basicblock::addToBB(instruction* ins) {
	Assert(ins != NULL);
	_o_insList->Append(ins);
}

void basicblock::addBBheader(string* header, ADDRS bbAddr) {
	Assert(header != NULL);
	_BBheader = header;
	_hasBBheader = true;
	_bbAddr = bbAddr;
}

bool basicblock::hasBBheader() {
	return _hasBBheader;
}

int basicblock::getNumBBIns_s() {
	return _s_insList->NumElements();
}

int basicblock::getNumBBIns_o() {
	return _o_insList->NumElements();
}

string* basicblock::getNthBBIns_s(int indx) {
	return _s_insList->Nth(indx);
}

instruction* basicblock::getNthBBIns_o(int indx) {
	return _o_insList->Nth(indx);
}

bool basicblock::isBBcomplete() {
	for (int i = 0; i < getNumBBIns_o(); i++) {
		if (getNthBBIns_o(i)->getStatus() != complete) return false;
	}
	return true;
}
