/*******************************************************************************
 *  loop.h
 ******************************************************************************/

#ifndef _LOOP_H
#define _LOOP_H

#include <stdint.h>
#include <stdlib.h>
#include "list.h"
#include "basicblock.h"

class loop : public basicblock {
	public:
		loop(ADDR loopEntryID, ADDR loopExitID);
		~loop();
		void addBB(basicblock* bb);
		List<basicblock*>* getLoop();
		ADDR getLoopEntryID();
		basicblock* getLoopEntry();
		bool isInnerLoop(loop* lp);
		int getNumBB();
		basicblock* getNthBB(int indx);
		bool isBbInLoop(ADDR bbID);
		void resetVisitBits();
		void findFallThroughBBs();
		List<basicblock*>* getFallThroughBBs();
		bool isBbFallThrough(ADDR bbID);

	private:
		void setOuterLoop(loop* lp);

		List<basicblock*> *_loop;
		List<basicblock*> *_fallThroughBBList;
		ADDR _loopEntryID;
		ADDR _loopExitID;
		List<loop*> *_innerLoops;
		List<loop*> *_ourerLoops;
};

#endif