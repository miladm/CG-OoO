/*******************************************************************************
 * bbROB.h
 ******************************************************************************/
#ifndef _BBROB_H
#define _BBROB_H

#include "../lib/list.h"
#include "../global/global.h"

#define BBROB_LATENCY 1
#define READ_PORT 1
#define WRITE_PORT 1

#define BBID_SIZ 5  //assume 32 BB in flight at most
#define wGRF_SIZ 32 //a 32-bit ins
#define rGRF_SIZ 32 //a 32-bit ins
#define bbINSCNT 5  //Assume 32 ins per BB
#define BBROB_ROW_SIZ BBID_SIZ+wGRF_SIZ+rGRF_SIZ+bbINSCNT

class bbROB : private unit {
	public:
		bbROB();
		~bbROB();
		void insert(basicblock* bb);
		bool commitBB(int num);

	private:
		List<basicblock*> * bbROB;
};

#endif /*_BBROB_H*/
