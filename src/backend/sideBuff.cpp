#include <stdio.h>
#include "sideBuff.h"
#include "utility.h"

sideBuff::sideBuff()
{
	_sideBuff = new List<instruction*>;
	_free = true; //it is free
	_expCycle = -1;
}

sideBuff::~sideBuff() {
	delete _sideBuff;
}

int sideBuff::NumElements() {
	Assert (_sideBuff->NumElements() >= 0);
	return _sideBuff->NumElements();
}

bool sideBuff::isFree() {
	return _free;
}

instruction* sideBuff::Nth(int indx) {
	//printf("indx = %d,%d\n",indx,NumElements());
	Assert (indx >= 0 && indx < NumElements());
	return _sideBuff->Nth(indx);
}

void sideBuff::setFree() {
	_free = true;
}

void sideBuff::setBusy() {
	_free = false;
}

void sideBuff::Append(instruction * ins) {
	Assert(ins != NULL);
	_sideBuff->Append(ins);
}

void sideBuff::InsertAt(instruction * ins, int index) {
	Assert(ins != NULL && index >= 0);
	_sideBuff->InsertAt(ins,index);
}

void sideBuff::setExpiration(int expCycle) {
	_expCycle = expCycle;
}

int sideBuff::getExpiration() {
	Assert(_expCycle > 0);
	return _expCycle;
}

void sideBuff::RemoveAt(int indx) {
	Assert (indx >= 0 && indx < NumElements());
	_sideBuff->RemoveAt(indx);
}
