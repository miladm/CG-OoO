/*******************************************************************************
 *  make_phraseblock.h
 ******************************************************************************/

#ifndef _MAKE_PHRASEBLOCK_H
#define _MAKE_PHRASEBLOCK_H

#include <map>
#include "phraseblock.h"
#include "loop.h"

List<basicblock*>* make_phraseblock (List<basicblock*>* bbList,
		               std::map<ADDR, double> *brBiasMap,
					   std::map<ADDR, double> *bpAccuracyMap);

#endif
