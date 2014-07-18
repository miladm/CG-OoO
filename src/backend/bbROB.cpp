/*******************************************************************************
 * bbROB.cpp
 *******************************************************************************/

#include <iostream>
#include "bbROB.h"


bbROB::bbROB(int rob_size, int commit_width) 
      :unit(READ_PORT, WRITE_PORT, rob_size, BBROB_ROW_SIZ, BBROB_LATENCY, CAM_ARRAY)
{
	bbROB = new List<basicblock*>;
}

bbROB::~bbROB() {
	delete bbROB;
}

void bbROB::insert(basicblock* bb) {
	bbROB->Append(bb);
}

/* commit a number of BB's*/
void bbROB::cmmitBBROB(int num) {
	Assert(num > 0 && num <= 
	Assert(bbROB->Nth(0)->isBBcomplete() == true);
	delete bbROB->Nth(0);
	bbROB->RemoveAt(0);
}

void bbROB::
