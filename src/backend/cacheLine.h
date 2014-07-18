/*******************************************************************************
 * Cache line interface and default implementation
 * Size(_lineSize) is a specified number of bytes
 * Word is the representation of a word, if you want a 64 bit representation us
 * int64_t, etc.
 * Word should be selected to be the largest granularity that will ever be
 * written to the cache otherwise, dataloss will occur
 ******************************************************************************/
#ifndef _CACHELINE_H
#define _CACHELINE_H
#include <stdint.h>
#include <stdlib.h>
#include "../global/global.h"

class CacheLine{
    public:
	CacheLine();
	~CacheLine();
	void writeLine(long int inTag, int8_t *inData);
	void readLine(int8_t*& inData);
	bool isValid();
	bool isDirty();
	void setClean();
	int getLineSize();
	ADDRS getTag();
	void setTag(long int tag);
	long int getCycle();
	void setCycle(long int cycle);
	bool getExpectData();
	void setExpectData(bool expct);

    private:
	int8_t *_data;
	ADDRS _tag;
	bool _valid;
	bool _dirty;
	int _lineSize; //In Bytes
	bool debug;
	long int _cycle;
	bool _expectData;
};

#endif
