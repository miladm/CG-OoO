/*******************************************************************************
 *  phrase.cpp
 ******************************************************************************/

#include "phrase.h"

phrase::phrase() {
	_insList = new List<instruction*>;
	_ancestorPhList = new List<phrase*>;
	_descendantPhList = new List<phrase*>;
}

phrase::~phrase() {
	delete _insList;
	delete _ancestorPhList;
	delete _descendantPhList;
}

void phrase::addIns(instruction* ins) {
	_insList->Append(ins);
}

int phrase::phSize() {
	return _insList->NumElements();
}