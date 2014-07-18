/*******************************************************************************
 * stage.h
 ******************************************************************************/
#ifndef _STAGE_H
#define _STAGE_H

#include <map>
#include <string.h>
#include <string>
#include "../lib/list.h"
#include "../global/global.h"

class stage {
	public:
		stage();
		~stage();
		CYCLE tick(CYCLE cycle);

	private:
		CYCLE _latency;
};


#endif /*_STAGE_H*/
