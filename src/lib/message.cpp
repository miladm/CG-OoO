/*******************************************************************************
 * message.cpp
 *******************************************************************************/

#include <iostream>
#include "message.h"

message::message() {
	_simStepCnt = 0;
}

message::~message() {}

void message::heading() {
	cout << ">>>>>>>>>>>>>>> >>>>>>>>>>>>>>\n";
}

void message::simStep(const char* msg) {
	heading();
	cout << ">> SIM STEP " << _simStepCnt << ": " << msg << endl;
	heading();
	_simStepCnt++;
}

message g_msg;
