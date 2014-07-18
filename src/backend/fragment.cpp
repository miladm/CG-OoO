/*******************************************************************************
 * fragment.cpp
 * the fragment class
 ******************************************************************************/
 #include "fragment.h"

extern float unpredMemOpThreshold;
extern int cacheLat[MEM_HIGHERARCHY];
 
fragment::fragment() {
	_id      = -1;
	_frNum   = -1;
	_numBits = -1;
	_fragmentInsList_sort = new List<instruction*>;
	_fragmentInsList_vliw = new List<instruction*>;
	_memReadAncestorsID   = new List<int>;
	_fragmentInsList      = new List<instruction*>;
	_allAncestors         = new List<instruction*>; //All ancestors
	_frScore      = -1.0;
	_phSize       = -1.0;
	_phAvgAge     = -1.0;
	_idealLat     = 0;
	_numUPMEMops  = 0;
	_phNumUPLDops = -1.0;
	_phNumDepLinksToNxtPh = -1.0;
	_startCycle   = -1;
	_endCycle     = -1;
	_latency      = -1;

}

fragment::~fragment() {
	while(_fragmentInsList_sort->NumElements() > 0) {
	    _fragmentInsList_sort->RemoveAt(0);
	}
	while(_fragmentInsList_vliw->NumElements() > 0) {
	    _fragmentInsList_vliw->RemoveAt(0);
	}
	while(_fragmentInsList->NumElements() > 0) {
	    _fragmentInsList->RemoveAt(0);
	}
	while(_allAncestors->NumElements() > 0) {
	    _allAncestors->RemoveAt(0);
	}
	delete _fragmentInsList_sort;
	delete _fragmentInsList_vliw;
	delete _memReadAncestorsID;
	delete _fragmentInsList;
	delete _allAncestors;
}

void fragment::setFragID(long int id) {
	if (id <= -1) printf("bad id: %ld\n", id);
	Assert(id > -1);
	_id = id;
}

INS_ID fragment::getFragID() {
	Assert(_id >= 0);
	return _id;
}

void fragment::setFragNum(long int num) {
	if (num <= -1) printf("bad num: %ld\n", num);
	Assert(num > -1);
	_frNum = num;
}

long int fragment::getFragNum() {
	Assert(_frNum > -1);
	return _frNum;
}

void fragment::addToFrag(instruction* ins) {
	_fragmentInsList->Append(ins);
	_fragmentInsList_sort->Append(ins);
	computeFrIdealLat(ins);
	setAllFrAncestors(ins);
	if (ins->getMissrate() > unpredMemOpThreshold) _numUPMEMops++;
}

instruction* fragment::getNthIns(int i) {
	return _fragmentInsList->Nth(i);
}

int fragment::getFragSize() {
	return _fragmentInsList->NumElements();
}

void fragment::setNumBits(int numBits) {
	Assert(numBits > -1);
	_numBits = numBits;
}

int fragment::getNumBits() {
	Assert(_numBits > -1);
	return _numBits;
}
void fragment::computeFrIdealLat(instruction* ins) {
	type insType = ins->getType();
	switch (insType) {
		case ALU:
			_idealLat += ALU_LATENCY;
			break;
		case MEM:
			_idealLat += 1; //cacheLat[0];
			break;
		case FPU:
			_idealLat += FPU_LATENCY;
			break;
		default:
			printf("Instruction type unrecognized. Terminating.\n");
			exit(-1);
			break;
	};
}

long int fragment::getFrIdealLat() {
	Assert(_idealLat > 0);
	return _idealLat;
}

List<instruction*>* fragment::getInsList() {
	return _fragmentInsList;
}

bool fragment::isReady() {
	if (_allAncestors->NumElements() == 0)
		return true;
	else
		return false;
}

void fragment::setAllFrAncestors(instruction* ins) {
	quicksortInsList(_fragmentInsList_sort, 0, _fragmentInsList_sort->NumElements()-1);
	quicksortInsList(_allAncestors,  0, _allAncestors->NumElements()-1);
	for (int i = 0; i < ins->getNumAncestors(); i++) {
		int insID = ins->getNthAncestor(i)->getInsID();
		if (binarySearch(_fragmentInsList_sort, insID, 0, _fragmentInsList_sort->NumElements()-1) == -1) {
			if (binarySearch(_allAncestors, insID, 0, _allAncestors->NumElements()-1) == -1) {
				_allAncestors->Append(ins->getNthAncestor(i));
				ins->getNthAncestor(i)->addDepFrag(this);
				quicksortInsList(_allAncestors,  0, _allAncestors->NumElements()-1);
				///*-----STAT-----*/
				//_numPhraseAncestors++;
				///*-----STAT-----*/
			}
		}
	}
}

void fragment::removeAncestorIns(instruction* ins) {
	//TODO would be nice if the ins had a map of dependent phraes + index
	//Assume there no duplicaetes of an instructin in list
	for (int i = _allAncestors->NumElements()-1; i >= 0; i--) {
		if (_allAncestors->Nth(i)->getInsID() == ins->getInsID()) {
			_allAncestors->RemoveAt(i);
			//break;
		}
	}
}

void fragment::setScore(double frScore) {
	Assert(frScore >= 0.0 && frScore <= 4.0);
	_frScore = frScore;
}

double fragment::getFrScore() {
	Assert(_frScore != -1.0 && _frScore <= 4.0);
	return _frScore;
}

void fragment::computeFrScore() {
	//printf("%f, %f, %f, %f\n",	getFrRelNumDepLinksToNxtPh(), 
	//				getFrRelNumUPLDops(), 
	//				getFrRelAvgAge(), 
	//				getFrRelSize());
	_frScore = getFrRelNumDepLinksToNxtPh() + 
		   getFrRelNumUPLDops() + 
		   getFrRelAvgAge() + 
		   getFrRelSize();
}

long double fragment::getFrRelAvgAge() {
	Assert(_phAvgAge != -1.0);
	long double totalAge = 0.0;
	for (int i = 0; i < _fragmentInsList_sort->NumElements(); i++) {
		totalAge += _fragmentInsList_sort->Nth(i)->getInsID();
	}
	long double avgAge = totalAge;// / (double)_fragmentInsList_sort->NumElements();
	if (avgAge > _phAvgAge) printf("%Lf,%Lf",avgAge,_phAvgAge);
	Assert(avgAge <= _phAvgAge);
	return (1.0 - avgAge / _phAvgAge); //Smaller value means older
}

double fragment::getFrRelNumDepLinksToNxtPh() {
	//Assert(_phNumDepLinksToNxtPh != -1.0);
	//double numDepLinksToNxtPh = (double) !;
	//return numDepLinksToNxtPh / _phNumDepLinksToNxtPh;
	return 0.0;
}

double fragment::getFrRelNumUPLDops() {
	Assert(_phNumUPLDops != -1.0);
	double numUPLDops = (double) _numUPMEMops;
	Assert(numUPLDops <= _phNumUPLDops);
	if (_phNumUPLDops == 0.0) {
		return 0.0;
	} else {
		return (numUPLDops / _phNumUPLDops);
	}
}

double fragment::getFrRelSize() {
	Assert(_phSize > 0.0);
	double fragSize = (double) _fragmentInsList_sort->NumElements();
	Assert(fragSize <= _phSize);
	return (fragSize / _phSize);
}

void fragment::setPhAvgAge(long double phAvgAge) {
	_phAvgAge = phAvgAge;
}

void fragment::setPhNumDepLinksToNxtPh(int phNumDepLinksToNxtPh) {
	_phNumDepLinksToNxtPh = 0.0; //(double) phNumDepLinksToNxtPh; //TODO to be implemented
}

void fragment::setPhNumUPLDops(int phNumUPLDops) {
	_phNumUPLDops = (double) phNumUPLDops;
}

void fragment::setPhSize(int phSize) {
	_phSize = (double) phSize;
}

void fragment::VLIWfrag(int cycle, int numFU) {
	vliwScheduler* vliwSch = new vliwScheduler;
	long int numElem = _fragmentInsList->NumElements();
	if (numFU > 1) {
		vliwSch->scheduleIns(_fragmentInsList, _fragmentInsList_vliw, 0, true, _muPhID);
	} else if (numFU == 1) {
		vliwSch->scheduleIns_1FU(_fragmentInsList, _fragmentInsList_vliw, 0, true, _muPhID);
	} else {
		Assert(numFU >= 1);
	}
	Assert(_fragmentInsList->NumElements() == 0);
	Assert(_fragmentInsList_vliw->NumElements() == numElem);
	for (int i = 0; i < _fragmentInsList_vliw->NumElements(); i++) {
		_fragmentInsList->Append(_fragmentInsList_vliw->Nth(i));
	}
	Assert(getFragSize() == numElem);
	delete vliwSch;
}

void fragment::setMyPhID(long int id) {
	Assert(id >= 0);
	_muPhID = id;
}

void fragment::setStart(long int start) {
	Assert(start > 0 && _startCycle == -1);
	_startCycle = start;
}

void fragment::setEnd(long int end) {
	Assert(end > _startCycle && _endCycle == -1 && _latency == -1);
	_endCycle = end;
	_latency = _endCycle - _startCycle;
}

long int fragment::getLat() {
	Assert(_latency > 0);
	return _latency;
}
