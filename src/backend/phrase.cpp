/*******************************************************************************
 * phrase.cpp
 * the phrase class
 ******************************************************************************/
 #include "phrase.h"

extern float unpredMemOpThreshold;
extern int phraseSizeBound;
extern int cacheLat[MEM_HIGHERARCHY];
extern int numFU;

 phrase::phrase() {
	_phraseInsList      = new List<instruction*>;
	_phraseInsList_sort = new List<instruction*>;
	_phraseInsList_vliw = new List<instruction*>;
	_allAncestors       = new List<instruction*>; //All ancestors
	_memReadAncestors   = new List<instruction*>; //Only Unpredictab LD op ancestors
	_memReadAncestorsID = new List<int>;	      //Only Unpredictab LD op ancestors
	_frags              = new List<fragment*>;
	_state = phOpen;
	_id = -1; //_id not set
	_phAge = 0;
	_numUPMEMops = 0;
	_numRootIns = 0;
	_numPhraseAncestors = 0;
	_idealLat = 0;
	_numOfFragments = 0;
	_phAvgAge = -1.0;
	//TODO: ALWAYS UPDATE BOTH CONSTRUCTORS!
}

 phrase::phrase(int id) {
	_phraseInsList      = new List<instruction*>;
	_phraseInsList_sort = new List<instruction*>;
	_phraseInsList_vliw = new List<instruction*>;
	_allAncestors       = new List<instruction*>; //All ancestors
	_memReadAncestors   = new List<instruction*>; //Only Unpredictab LD op ancestors
	_memReadAncestorsID = new List<int>;	      //Only Unpredictab LD op ancestors
	_frags              = new List<fragment*>;
	_state = phOpen;
	_id = id;
	_phAge = 0;
	_numUPMEMops = 0;
	_numRootIns = 0;
	_numPhraseAncestors = 0;
	_idealLat = 0;
	_numOfFragments = 0;
	_phAvgAge = -1.0;
	//TODO: ALWAYS UPDATE BOTH CONSTRUCTORS!
}

 phrase::~phrase() {
	while(_phraseInsList->NumElements() > 0) {
	    _phraseInsList->RemoveAt(0);
	}
	while(_phraseInsList_sort->NumElements() > 0) {
	    _phraseInsList_sort->RemoveAt(0);
	}
	while(_phraseInsList_vliw->NumElements() > 0) {
	    _phraseInsList_vliw->RemoveAt(0);
	}
	while(_allAncestors->NumElements() > 0) {
	    _allAncestors->RemoveAt(0);
	}
	while(_memReadAncestors->NumElements() > 0) {
	    _memReadAncestors->RemoveAt(0);
	}
	while(_memReadAncestorsID->NumElements() > 0) {
	    _memReadAncestorsID->RemoveAt(0);
	}
	while(_frags->NumElements() > 0) {
	    delete _frags->Nth(0);
	    _frags->RemoveAt(0);
	}
	delete _phraseInsList;
	delete _phraseInsList_sort;
	delete _phraseInsList_vliw;
	delete _allAncestors;
	delete _memReadAncestors;
	delete _memReadAncestorsID;
	delete _frags;
}

void phrase::setMemReadAncestors(instruction* ins) {
	//TODO you must not wipe off the array here.... this function is wrong... fix it
	//use lists... once done... use sort and then unique functions to remove duplicates
	//also use insID
	//also only store the immediate instruction ancestors living in only the previous wavefront... this is a tricky rule

	//while (_memReadAncestors->NumElements() > 0) {
	//	_memReadAncestors->RemoveAt(0);
	//}
	//Assert(_memReadAncestors->NumElements() == 0);

	for (int i = 0; i < ins->getNumMemRdAncestors(); i++) {
		int insID = ins->getNthMemRdAncestor(i)->getInsID();
		//if (binarySearch(_memReadAncestors, insID, 0, _memReadAncestors->NumElements()-1) == -1) {
		//	_memReadAncestors->Append(ins->getNthMemRdAncestor(i));
		//	quicksortInsList(_memReadAncestors,  0, _memReadAncestors->NumElements()-1);
		//}
		_memReadAncestorsID->Append(insID);
	}
	//quicksortInsList(_memReadAncestors, 0, _memReadAncestors->NumElements()-1);

	//for (int i = 0; i < ins->getNumMemRdAncestors(); i++) {
	//	printf("%d, ", _memReadAncestors->Nth(i)->getInsID());
	//}
	//printf("\n");
}

void phrase::setAllPhAncestors(instruction* ins) {
	quicksortInsList(_phraseInsList_sort, 0, _phraseInsList_sort->NumElements()-1);
	quicksortInsList(_allAncestors,  0, _allAncestors->NumElements()-1);
	for (int i = 0; i < ins->getNumAncestors(); i++) {
		int insID = ins->getNthAncestor(i)->getInsID();
		if (binarySearch(_phraseInsList_sort, insID, 0, _phraseInsList_sort->NumElements()-1) == -1) {
			if (binarySearch(_allAncestors, insID, 0, _allAncestors->NumElements()-1) == -1) {
				_allAncestors->Append(ins->getNthAncestor(i));
				ins->getNthAncestor(i)->addDepPhrase(this);
				quicksortInsList(_allAncestors,  0, _allAncestors->NumElements()-1);
				/*-----STAT-----*/
				_numPhraseAncestors++;
				/*-----STAT-----*/
			}
		}
	}
}

void phrase::addToPhrase_Light(instruction* ins) {
	Assert(ins != NULL);
	Assert(_state == phOpen);
	_phraseInsList->Append(ins);
}

void phrase::addToPhrase(instruction* ins) {
	Assert(ins != NULL);
	Assert(_state == phOpen);
	_phraseInsList->Append(ins);
	_phraseInsList_sort->Append(ins);
	/*-----STAT-----*/
	if (ins->getMissrate() > unpredMemOpThreshold) _numUPMEMops++;
	if (ins->getNumAncestors() == 0) _numRootIns++;
	/*-----STAT-----*/
	//ins->setMyPhrase(this); //TODO this func is also called in phraseGEn... which is better?
	//TODO the comment outs below must be removed
	ins->lookUpANDsetPathLen();
	computePhIdealLat(ins);
	setAllPhAncestors(ins);
	setMemReadAncestors(ins);
}

void phrase::VLIWphrase(int cycle, int numFU) {
	vliwScheduler* vliwSch = new vliwScheduler;
	long int numElem = _phraseInsList->NumElements();
	if (numFU > 1) {
		vliwSch->scheduleIns(_phraseInsList, _phraseInsList_vliw, 0, true, getPhraseID());
	} else if (numFU == 1) {
		vliwSch->scheduleIns_1FU(_phraseInsList, _phraseInsList_vliw, 0, true, getPhraseID());
	} else {
		Assert(numFU >= 1);
	}
	Assert(_phraseInsList->NumElements() == 0);
	Assert(_phraseInsList_vliw->NumElements() == numElem);
	for (int i = 0; i < _phraseInsList_vliw->NumElements(); i++) {
		_phraseInsList->Append(_phraseInsList_vliw->Nth(i));
	}
	Assert(getPhraseSize_unsort() == numElem);
	delete vliwSch;
}

int phrase::findCriticalPath() {
	int maxPathLen = -1;
	for (int i = 0; i < _phraseInsList->NumElements(); i++) {
		int pathLen = _phraseInsList->Nth(i)->findLongestPath(0, false, getPhraseID());
		if (maxPathLen < pathLen) maxPathLen = pathLen;
	}
	Assert(maxPathLen > -1);
	return maxPathLen;
}

void phrase::computePhIdealLat(instruction* ins) {
	type insType = ins->getType();
	switch (insType) {
		case ALU:
			_idealLat += ALU_LATENCY;
			break;
		case MEM:
			_idealLat += cacheLat[0];
			break;
		case FPU:
			_idealLat += FPU_LATENCY;
			break;
		case BR:
			_idealLat += ALU_LATENCY;
			break;
		case ASSIGN:
			_idealLat += ASSIGN_LATENCY;
			break;
		default:
			printf("Instruction type unrecognized. Terminating.\n");
			exit(-1);
			break;
	};
}

long int phrase::getPhIdealLat() {
	Assert(_idealLat > 0);
	return _idealLat;
}

void phrase::removeFromPhrase(int indx) {
	_phraseInsList_sort->RemoveAt(indx);
}

void phrase::removeAncestorIns(instruction* ins) {
	//TODO would be nice if the ins had a map of dependent phraes + index
	//Assume there no duplicaetes of an instructin in list
	for (int i = _allAncestors->NumElements()-1; i >= 0; i--) {
		if (_allAncestors->Nth(i)->getInsID() == ins->getInsID()) {
			_allAncestors->RemoveAt(i);
			//break;
		}
	}
}

int phrase::getPhraseSize() {
	return _phraseInsList_sort->NumElements();
}

int phrase::getPhraseSize_unsort() {
	return _phraseInsList->NumElements();
}

int phrase::getNumAllPhraseAncestors() {
	return _allAncestors->NumElements();
}

instruction* phrase::getNthPhraseAncestor(int i) {
	return _allAncestors->Nth(i);
}

instruction* phrase::getNthIns(int i) {
	return _phraseInsList_sort->Nth(i);
}

instruction* phrase::getNthIns_unsort(int i) {
	return _phraseInsList->Nth(i);
}

List<instruction*>* phrase::getInsList_unsort() {
	return _phraseInsList;
}

int phrase::getPhraseID() {
	return _id;
}

void phrase::incPhraseAge() {
	_phAge++;
}

int phrase::getPhraseAge() {
	return _phAge;
}

phraseState phrase::getState() {
	return _state;
}

void phrase::setState(phraseState state) {
	_state = state;
}

void phrase::printToFilePhrase(FILE* phraseFile) {
	fprintf(phraseFile, "\n{(WF: %d)", getPhraseID());//Phrase Separator
	for (int i = 0; i < _phraseInsList->NumElements(); i++) {
		//unsorted list is stored
		_phraseInsList->Nth(i)->printToFile(phraseFile, true);
	}
	fprintf(phraseFile, "\n} (WF: %d)", getPhraseID());//Phrase Separator
}

void phrase::printToFileFragment(FILE* phraseFile) {
	fprintf(phraseFile, "\n{(WF: %d)", getPhraseID());//Phrase Separator
	for (int i = 0; i < _frags->NumElements(); i++) {
		long int score = _frags->Nth(i)->getFrScore() * COEFF;
		fprintf(phraseFile, "\n<,%ld,(FR: %llu)", score, _frags->Nth(i)->getFragID());//Phrase Separator
		_frags->Nth(i)->VLIWfrag(0, numFU);
		Assert(_frags->Nth(i)->getFragSize() > 0);
		for (int j = 0; j < _frags->Nth(i)->getFragSize(); j++) {
			_frags->Nth(i)->getNthIns(j)->printToFile(phraseFile, true);
		}
		fprintf(phraseFile, "\n>(FR: %llu)", _frags->Nth(i)->getFragID());//Phrase Separator
	}
	fprintf(phraseFile, "\n}(WF: %d)", getPhraseID());//Phrase Separator
}

long int phrase::getNumUPLDops() {
	return _numUPMEMops;
}

int phrase::getNumRootIns() {
	return _numRootIns;
}

int phrase::getNumPhraseAncestors() {
	return _numPhraseAncestors;
}

long int phrase::getNumUPLDancestors() {
	return _memReadAncestorsID->NumElements();
}

//Assume that this function is called after all its ancestor phrases are commited
//(i.e. no inter-phrase ancestors) and the phrase itself is closed
long int phrase::getNumSoftBounds () {
//TODO this function does not work right... fix it
	List<instruction*>* tempList = new List<instruction*>;
	for (int i = 0; i < _phraseInsList->NumElements(); i++) {
		tempList->Append(_phraseInsList->Nth(i));
	}
	while (tempList->NumElements() > 0) {
		instruction* endIns = tempList->Nth(tempList->NumElements()-1);
		tempList->RemoveAt(tempList->NumElements()-1);
		for (int i = endIns->getNumAncestors()-1; i >= 0; i--) {
			for (int j = tempList->NumElements()-1; j >= 0; j--) {
				if (endIns->getNthAncestor(i)->getInsID() == tempList->Nth(j)->getInsID()) {
					tempList->RemoveAt(j);
				}
			}
		}
		if (endIns->getNumAncestors() > 0) _numOfFragments++;
	}
	delete tempList;
	return _numOfFragments;
}

void phrase::makeFragment() {
	bool* fragID = new bool[_memReadAncestorsID->NumElements()];
	for (int i = 0; i < _memReadAncestorsID->NumElements(); i++) {
		fragID[i] = 0;
	}
	map<long int,fragment*> fragTable;
	long int fID = -1;
	quicksort(_memReadAncestorsID,0,_memReadAncestorsID->NumElements()-1,-1);
	unique(_memReadAncestorsID);
	computePhAvgAge();
	for (int i = 0; i < _phraseInsList->NumElements(); i++) {
		int numBits = findFragID(_phraseInsList->Nth(i), fragID, _phraseInsList->Nth(i));
		fID = encodeAndReset(fragID, _memReadAncestorsID->NumElements());
		if (fragTable.count(fID) > 0) {
			fragTable.find(fID)->second->addToFrag(_phraseInsList->Nth(i));
		} else {
			fragment* fr = new fragment;
			fragTable.insert(pair<long int,fragment*>(fID,fr));
			fr->setFragID(fID);
			fr->setMyPhID(getPhraseID());
			fr->addToFrag(_phraseInsList->Nth(i));
			fr->setNumBits(numBits);
			fr->setPhAvgAge(getPhAvgAge());
			//fr->setPhNumDepLinksToNxtPh();
			fr->setPhNumUPLDops(getNumUPLDops());
			fr->setPhSize(getPhraseSize_unsort());
			_frags->Append(fr);
			_phraseInsList->Nth(i)->setMyFrag(fr);
		}
	}
	for (int i = 0; i < _frags->NumElements(); i++) {
		_frags->Nth(i)->computeFrScore();
		//if (_frags->NumElements() == 1) {
		//	breakFragmentInHalf();
		//}
	}
	//Sort to conserve true data dependency (program ordering correctness)
	quicksort(_frags,0,_frags->NumElements()-1,-1);
	delete [] fragID;
}

long int phrase::encodeAndReset(bool* fragID, int size) {
	long int id  = 0;
	long int pow = 1;
	for (int i = 0; i < size; i++) {
		id += fragID[i]*pow;
		fragID[i] = 0; //reset
		pow *= 2;
	}
	return id;
}

long int phrase::getNumFrags() {
	return _frags->NumElements();
}

/*
 *This function is run when the previous wavefront is stored in file. 
 *Thus, each instruction only sees its ancestors from within the same fragment.
 */
int phrase::findFragID(instruction* ins, bool* fragID, instruction* root) {
	Assert(ins->getMyPhraseID() == getPhraseID());
	int numBits = 0;
	if (root->getInsID() != ins->getCauseOfFragInsID()) {
		ins->setCauseOfFragInsID(root->getInsID());
		for(int i = 0; i < ins->getNumMemRdAncestors(); i++) {
			int UPLDindx = binarySearch(_memReadAncestorsID, 
						    ins->getNthMemRdAncestorID(i), 
						    0, 
						    _memReadAncestorsID->NumElements()-1); 
			Assert(UPLDindx > -1);
			if (fragID[UPLDindx] != 1) numBits++;
			fragID[UPLDindx] = 1;
		}
		for (int i = 0; i < ins->getNumAncestors(); i++) { //TODO is the upper bound correct?
			numBits += findFragID(ins->getNthAncestor(i), fragID, root);
		}
	}
	return numBits;
	//TODO any exponential access pattern? seems not
}

void phrase::computePhAvgAge() {
	long int phAge = 0;
	for (int i = 0; i < _phraseInsList->NumElements(); i++) {
		phAge += _phraseInsList->Nth(i)->getInsID();
	}
	Assert(phAge > 0);
	_phAvgAge = (long double)phAge; //(double)phAge / (double)_phraseInsList->NumElements();
}

long double phrase::getPhAvgAge() {
	Assert(_phAvgAge != -1.0);
	return _phAvgAge;
}

void phrase::breakFragmentInHalf() {
	int longestPathRoots[2];
	longestPathRoots[0]=-1;
	longestPathRoots[1]=-1;

	for (int i = 0; i < _phraseInsList->NumElements(); i++) {
		if (_phraseInsList->Nth(i)->isReady(-1) == true) {
			instruction* ins = _phraseInsList->Nth(i);
			int LPval = ins->findLongestPath(0, true, getPhraseID());
			//printf("%d, ", LPval);
			if (LPval > longestPathRoots[0]) {
				longestPathRoots[0] = i;
			} else if (LPval > longestPathRoots[1]) {
				longestPathRoots[1] = i;
			}
		}
	}
	//printf("\n(%d) %d, %d \n", _id, longestPathRoots[0], longestPathRoots[1]);
}
















bool phrase::chkMemRdAncestors(instruction* ins) {
        int foundMatch = 0;
        int innerBound;
        int outerBound;
        if ((_memReadAncestors->NumElements() != 0 && ins->getNumMemRdAncestors() == 0) ||
            (_memReadAncestors->NumElements() == 0 && ins->getNumMemRdAncestors() != 0)) 
                return false;

        if (_memReadAncestors->NumElements() > ins->getNumMemRdAncestors()) {
                outerBound = ins->getNumMemRdAncestors();
                innerBound = _memReadAncestors->NumElements();
		//quicksortInsList(_memReadAncestors, 0, _memReadAncestors->NumElements()-1);
		for (int i = 0; i < outerBound; i++) {
		        if (binarySearch(_memReadAncestors, ins->getNthMemRdAncestor(i)->getInsID(), 0, _memReadAncestors->NumElements()-1) != -1) {
		                foundMatch++;
		        } else {
				break;
		        }
		}
        } else {
                outerBound = _memReadAncestors->NumElements();
                innerBound = ins->getNumMemRdAncestors();
		quicksortInsList(ins->_phraseAncestors, 0, ins->getNumMemRdAncestors()-1);
		for (int i = 0; i < outerBound; i++) {
		        if (binarySearch(ins->_phraseAncestors, _memReadAncestors->Nth(i)->getInsID(), 0, ins->getNumMemRdAncestors()-1) != -1) {
		                foundMatch++;
		        } else {
				break;
			}
		}
        }
        Assert(outerBound <= innerBound);


        if (foundMatch == outerBound) {
                if (_memReadAncestors->NumElements() < ins->getNumMemRdAncestors()) {
                        setMemReadAncestors(ins);
                }
                return true;
        } else {return false;}
}


bool phrase::chkMemRdAncestorsV1(instruction* ins) {
	//TODO optimize this block (create a code for eahc instruction's parent set)
	if (_phAge == phraseSizeBound) {
		setState(phClosed);
		return false; //TODO you might want to remove this line
	}
        int foundMatch = 0;
        int innerBound;
        int outerBound;
        if ((_memReadAncestors->NumElements() != 0 && ins->getNumMemRdAncestors() == 0) ||
            (_memReadAncestors->NumElements() == 0 && ins->getNumMemRdAncestors() != 0)) 
                return false;

        if (_memReadAncestors->NumElements() > ins->getNumMemRdAncestors()) {
                outerBound = ins->getNumMemRdAncestors();
                innerBound = _memReadAncestors->NumElements();
		//quicksortInsList(_memReadAncestors, 0, _memReadAncestors->NumElements()-1);
		for (int i = 0; i < outerBound; i++) {
		        if (binarySearch(_memReadAncestors, ins->getNthMemRdAncestor(i)->getInsID(), 0, _memReadAncestors->NumElements()-1) != -1) {
		                foundMatch++;
		        } else {
				break;
		        }
		}
        } else {
                outerBound = _memReadAncestors->NumElements();
                innerBound = ins->getNumMemRdAncestors();
		quicksortInsList(ins->_phraseAncestors, 0, ins->getNumMemRdAncestors()-1);
		for (int i = 0; i < outerBound; i++) {
		        if (binarySearch(ins->_phraseAncestors, _memReadAncestors->Nth(i)->getInsID(), 0, ins->getNumMemRdAncestors()-1) != -1) {
		                foundMatch++;
		        } else {
				break;
			}
		}
        }
        Assert(outerBound <= innerBound);


        if (foundMatch == outerBound) {
                if (_memReadAncestors->NumElements() < ins->getNumMemRdAncestors()) {
                        setMemReadAncestors(ins);
                }
                return true;
        } else {return false;}
}

bool phrase::isPhraseComplete() {
	for (int i = _phraseInsList->NumElements()-1; i >= 0; i--) {
		if (_phraseInsList->Nth(i)->getStatus() != complete) {
			return false;
		}
	}
	return true;
}

/* 

TODO this version of the function generates more phrases... should find out why
bool phrase::chkMemRdAncestors(instruction* ins) {
	//TODO optimize this block (create a code for eahc instruction's parent set)
        bool found = false;
        int foundMatch = 0;
        int innerBound;
        int outerBound;
	if (_phraseInsList->NumElements() > 99) return false;
        if ((_memReadAncestors->NumElements() != 0 && ins->getNumMemRdAncestors() == 0) ||
            (_memReadAncestors->NumElements() == 0 && ins->getNumMemRdAncestors() != 0)) 
                return false;

        if (_memReadAncestors->NumElements() > ins->getNumMemRdAncestors()) {
                outerBound = ins->getNumMemRdAncestors();
                innerBound = _memReadAncestors->NumElements();
		for (int i = 0; i < outerBound; i++) {
		        found = false;
		        for (int j = 0; j < innerBound; j++) {
		                if (ins->getNthMemRdAncestor(i)->getInsID() == _memReadAncestors->Nth(j)->getInsID()) {
		                        foundMatch++;
		                        found = true;
		                        break;
		                }
		        }
		        if (found == false) break;
		}
        } else {
                outerBound = _memReadAncestors->NumElements();
                innerBound = ins->getNumMemRdAncestors();
		for (int i = 0; i < outerBound; i++) {
		        found = false;
		        for (int j = 0; j < innerBound; j++) {
		                if (ins->getNthMemRdAncestor(j)->getInsID() == _memReadAncestors->Nth(i)->getInsID()) {
		                        foundMatch++;
		                        found = true;
		                        break;
		                }   
		        }
		        if (found == false) break;
		}
        }
        Assert(outerBound <= innerBound);


        if (foundMatch == outerBound) {
                if (_memReadAncestors->NumElements() < ins->getNumMemRdAncestors()) {
                        setMemReadAncestors(ins);
                }   
                return true;
        } else {return false;}

	//int foundMatch = 0;
	//if (_phraseInsList->NumElements() > 99) return false;
	//if (_memReadAncestors->NumElements() != ins->getNumMemRdAncestors()) return false;
	//for (int i = 0; i < _memReadAncestors->NumElements(); i++) {
	//	for (int j = 0; j < ins->getNumMemRdAncestors(); j++)
	//		if (ins->getNthMemRdAncestor(j)->getInsID() == _memReadAncestors->Nth(i)->getInsID()) {
	//			foundMatch++;
	//			break;
	//		}
	//}
	//if (foundMatch == _memReadAncestors->NumElements()) return true;
	//else return false;
}



*/
