/*******************************************************************************
 * stage.cpp
 *******************************************************************************/

#include <iostream>
#include "stage.h"

stage::stage() {
	_latency = 1; //default
}

stage::~stage() {}

CYCLE stage::tick(CYCLE) {
	return cycle+latency;
}
