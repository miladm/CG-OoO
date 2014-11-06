/*******************************************************************************
 *  interfNode.cpp
 ******************************************************************************/

#include "interfNode.h"
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string>
#include <list>
#include <time.h>

interfNode::interfNode (long int psudoReg) {
	_reg = -1;
	_psudoReg = psudoReg;
	_edgeList_static  = new List<interfNode*>;
	_edgeList_dynamic = new List<interfNode*>;
}

interfNode::~interfNode () {
	delete _edgeList_static;
	delete _edgeList_dynamic;
}

void interfNode::assignReg (set<long int> &RFset) {
	set<long int> usedRegSet, unusedRegSet;
	for (int i = 0; i < _edgeList_static->NumElements (); i++) {
		interfNode *node = _edgeList_static->Nth (i);
		long int nodeReg = node->getReg ();
		if (nodeReg != -1) usedRegSet.insert (nodeReg);
	}
	std::set_difference (RFset.begin (), RFset.end (), usedRegSet.begin (), usedRegSet.end (), std::inserter (unusedRegSet, unusedRegSet.begin ()));
	Assert (unusedRegSet.size () > 0 && "No register value found to assign.");
	_reg = * (unusedRegSet.begin ());
	Assert (((_reg >= GRF_LO && _reg <= GRF_HI) || (_reg >= LRF_LO && _reg <= LRF_HI)) && "Invalid Register Value");
}

void interfNode::addEdge (interfNode* node) {
	if (_neighborSet.find (node->getPsudoReg ()) == _neighborSet.end ()) {
		_neighborSet.insert (node->getPsudoReg ());
		_edgeList_static->Append (node);
		_edgeList_dynamic->Append (node);
		node->addEdge (this);
	}
}

List<interfNode*>* interfNode::getEdgeList () {
	return _edgeList_dynamic;
}

long int interfNode::getPsudoReg () {
	return _psudoReg;
}

int interfNode::getNeighborSize () {
	return _neighborSet.size ();
}

void interfNode::removeFromGraph () {
	int neighborSize = _edgeList_dynamic->NumElements ();
	while (_edgeList_dynamic->NumElements () > 0) {
		_edgeList_dynamic->Nth (0)->removeAneighbor (this);
		_neighborSet.erase (_edgeList_dynamic->Nth (0)->getPsudoReg ());
		_edgeList_dynamic->RemoveAt (0);
	}
}

void interfNode::removeAneighbor (interfNode* node) {
	long int nodePsudoReg = node->getPsudoReg ();
	for (int i = 0; i < _edgeList_dynamic->NumElements (); i++) {
		if (nodePsudoReg == _edgeList_dynamic->Nth (i)->getPsudoReg ()) {
			_neighborSet.erase (nodePsudoReg);
			_edgeList_dynamic->RemoveAt (i);
			break;
		}
	}
}

long int interfNode::getReg () {
	return _reg;
}
