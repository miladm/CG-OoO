/*******************************************************************************
 *  phraseblock.h
 ******************************************************************************/

#ifndef _PHRASEBLOCK_H
#define _PHRASEBLOCK_H

#include "basicblock.h"
#include "loop.h"

class phraseblock : public basicblock {
	public:
		phraseblock();
		~phraseblock();
		void loopToPhraseblock(loop* lp);
		void PhraseblockToBB(List<basicblock*>* bbList_new, loop *lp, ADDR* phID);

	private:
		int _numPhraseblocks;
		List<basicblock*> **_bBLists;
		List<basicblock*> *_phraseBBLists;
		List<phraseblock*> *_ancestorPbList;
		List<phraseblock*> *_descendantPbList;
};

#endif