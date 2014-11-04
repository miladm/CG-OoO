/*******************************************************************************
 *  variable.cpp
 ******************************************************************************/

#include "variable.h"

variable::variable (long int var) {
	Assert(var >= X86_REG_LO && var <= X86_REG_HI && "invalid x86 variable.");
	_var = var;
	_c = 0;
	_hackPushCount = 0;
	bbList = new List<basicblock*>;
}

variable::~variable() {
	delete bbList;
}

long int variable::getID() {
	return _var;
}

void variable::addBB(basicblock* bb) {
	bbList->Append(bb);
}

basicblock* variable::getNthAssignedBB(int indx) {
	return bbList->Nth(indx);
}

int variable::getNumAssignedBB() {
	return bbList->NumElements();
}

int variable::getC() {
	return _c;
}

void variable::setC(int c) {
	Assert(c >= 0 && "Invalid c value inserted.");
	_c = c;
}

void variable::popFromStack() {
	// printf("pop from stack %d_%d\n", _var, _s.back());
	_s.pop_back();
}

void variable::pushToStack(int s) {
	// printf("pushing to stack %d_%d\n", _var, s);
	Assert(s >= 0 && "Invalid s value inserted.");
	_s.push_back(s);
}

int variable::getTopStack() {
	/* The block of code here is a hack. This is not
	   aligned with how the stack is poped. In fact the
	   elements pushed here are not poped. A cleaner hack
	   would be to track this hack and pop it crrectly.
	*/
	if (_s.size() == 0) {
		int k = getC();
		_s.push_back(k);
		setC(k+1);
		_hackPushCount++;
	} //means don't assign anything
	// if (_s.size() == 0) { printf("read stack: %d_-2\n", _var); return -2; }
	// printf("read stack: %d_%d\n", _var, _s.back());
	// Assert(_s.size() > 0 && "Invalid variable stack size.");
	return _s.back();
}

void variable::popHackPushes(int counter) {
	while (counter > 0) {
        counter--;
		_hackPushCount--;
		_s.pop_back();
        Assert (_hackPushCount >= 0);
	}
}
