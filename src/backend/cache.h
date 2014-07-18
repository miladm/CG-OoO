/*******************************************************************************
 * Cache.h implements a cache object with configurable size, set associativity,
 * and write policy.
 ******************************************************************************/
#ifndef _CACHE_H
#define _CACHE_H

#include <stdint.h>
#include <stdlib.h>
#include "cacheLine.h"
#include "../global/global.h"


class cache{
    public:
	//-------VARIABLES--------//
	bool findWord(ADDRS addr);

	//-------FUNCTIONS--------//
	cache();
	cache(int sa, int lineSize, int cacheSize);
	~cache();
	bool findAddr(ADDRS addr);
	void writeCache(ADDRS addr, int8_t *data);
	void readCache(ADDRS addr, int8_t*& outData);
	bool isValid(ADDRS addr);
	bool isHit(ADDRS addr);
	bool isDirty(ADDRS addr);
	void setClean(ADDRS addr);
	ADDRS getWBAddr(ADDRS addr);
	ADDRS getTag(ADDRS addr);
	bool getExpectData(ADDRS addr);
	void setExpectData(ADDRS addr, bool expct);
	

    protected:
	int _lineSize; //In Bytes

    private:
	//-------VARIABLES--------//
	CacheLine **_cacheLines;
	int _sa;
	unsigned _cacheSize; //In Bytes
	int numRows;
	bool debug;


	//-------FUNCTIONS--------//
	ADDRS getIndex(ADDRS addr);
	void cacheSpec(int numCacheLines);
	int LRUCacheLine(ADDRS addr);
};


#endif
