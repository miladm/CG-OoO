/*******************************************************************************
 *  phrase.h
 ******************************************************************************/

#ifndef _PHRASE_H
#define _PHRASE_H

#include "instruction.h"

class phrase {
	public:
		phrase();
		~phrase();
		void addIns(instruction* ins);
		int phSize();
		//TODO add code for setting up dependenncies betweeen phrases

	private:
		List<instruction*> *_insList;
		List<phrase*> *_ancestorPhList;
		List<phrase*> *_descendantPhList;
};

#endif
