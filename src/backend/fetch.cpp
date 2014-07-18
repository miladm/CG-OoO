/*******************************************************************************
 * fetch.cpp
 *******************************************************************************/

#include <iostream>
#include "fetch.h"

fetch::fetch() {
	_insBuff = new List<instruction*>;
}

fetch::~fetch() {
	delete _insBuff;
}

CYCLE fetch::tick(CYCLE) {
	return cycle+latency;
}
