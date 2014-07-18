/*******************************************************************************
 *  loop.cpp
 ******************************************************************************/

#include "loop.h"

loop::loop(ADDR loopEntryID, ADDR loopExitID) : basicblock() {
	_fallThroughBBList = new List<basicblock*>;
	_loop = new List<basicblock*>;
	_innerLoops = new List<loop*>;
	_ourerLoops = new List<loop*>;
	_loopEntryID = loopEntryID;
	_loopExitID  = loopExitID;
}

loop::~loop() {
	delete _loop;
	delete _innerLoops;
	delete _ourerLoops;
	delete _fallThroughBBList;
}

void loop::addBB (basicblock* bb) {
	for (int i = _loop->NumElements()-1; i >= 0; i--) {
		if (bb->getID() == _loop->Nth(i)->getID())
			return;
	}
	_loop->Append(bb);
}

List<basicblock*>* loop::getLoop() {
	return _loop;
}

ADDR loop::getLoopEntryID() {
	Assert(_loopEntryID >= 0 && "loop Entry ID is invalid");
	return _loopEntryID;
}

basicblock* loop::getLoopEntry() {
	Assert(_loopEntryID >= 0 && "loop Entry ID is invalid");
	for (int i = 0; i < _loop->NumElements(); i++) {
		if (_loop->Nth(i)->getID() == getLoopEntryID())
			return _loop->Nth(i);
	}
	Assert("The loop entry was not found. Aborting execution.");
}

//Setup the innner and outer loop pointers here
bool loop::isInnerLoop(loop* lp) {
	for (int i = 0; i < _loop->NumElements(); i++) {
		if (_loop->Nth(i)->getID() == lp->getLoopEntryID()) {
			_innerLoops->Append(lp);
			lp->setOuterLoop(this);
			printf("%llx -> %llx\n", getLoopEntryID(), _loop->Nth(i)->getID());
			return true;
		}
	}
	return false;
}

void loop::setOuterLoop(loop* lp) {
	_ourerLoops->Append(lp);
}

int loop::getNumBB() {
	return _loop->NumElements();
}

basicblock* loop::getNthBB(int indx) {
	return _loop->Nth(indx);
}

bool loop::isBbInLoop(ADDR bbID) {
	for (int i = 0; i < _loop->NumElements(); i++) {
		if (_loop->Nth(i)->getID() == bbID)
			return true;
	}
	return false;
}

bool loop::isBbFallThrough(ADDR bbID) {
	for (int i = 0; i < _fallThroughBBList->NumElements(); i++) {
		if (_fallThroughBBList->Nth(i)->getID() == bbID) return true;
	}
	return false;
}

void loop::resetVisitBits() {
	for (int i = 0; i < _loop->NumElements(); i++) {
		_loop->Nth(i)->setAsUnvisited();
	}
}

void loop::findFallThroughBBs() {
	Assert(_loop->NumElements() > 0 && "Loop is empty\n");
	printf("Loop %llx Fallthroughs: ", getLoopEntryID());
	for (int i = 0; i < _loop->NumElements(); i++) {
		for (int j = 0; j < _loop->Nth(i)->getNumDescendents(); j++) {
			bool descendentIsInLoop = false;
			for (int k = 0; k < _loop->NumElements(); k++) {
				if (_loop->Nth(i)->getNthDescendent(j)->getID() == _loop->Nth(k)->getID()) {
					descendentIsInLoop = true;
					break;
				}				
			}
			if (descendentIsInLoop == false) {
				printf("%llx, ", _loop->Nth(i)->getNthDescendent(j)->getID());
				_fallThroughBBList->Append(_loop->Nth(i)->getNthDescendent(j));
			}
		}
	}
	printf("\n");
}

List<basicblock*>* loop::getFallThroughBBs() {
	//Assert(_fallThroughBBList->NumElements() > 0 && "No loop fallthrough found");
	if (_fallThroughBBList->NumElements() <= 0) printf("\tWARNING: No loop fallthrough found in loop %llx\n", getLoopEntryID()); //TOOD bring this back to an assert soon
	return _fallThroughBBList;
}