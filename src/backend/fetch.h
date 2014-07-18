/*******************************************************************************
 * fetch.h
 * This stage grabs instructions from Pin frontend. Later on an instruction
 * cache may be added
 * The number of instructions fetched depends on hardware limits such as:
 *		- fetch-width
 *		- depth of fetch pipeline
 ******************************************************************************/
#ifndef _FETCH_H
#define _FETCH_H

#include <map>
#include <string.h>
#include <string>
#include "../lib/list.h"
#include "../global/global.h"

class fetch {
	public:
		fetch();
		~fetch();
		void 

	private:
		List<instruction*> * _insBuff;
};


#endif /*_FETCH_H*/
