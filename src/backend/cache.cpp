/*******************************************************************************
 * Cache line
 * Size is a specified number of bytes
 * Also contains state of line (per line state)
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "cache.h"
#include "latency.h"
#include "../global/global.h"

#define ADDRESS_SIZE 64

cache::cache()
{
    _sa = 1; //Direct Mapped Cache is the default
    _cacheSize = 33554432; //[Bytes]
    _lineSize = 64; //[Bytes]

    int numCacheLines = (int) _cacheSize/_lineSize;
    numRows = (int) numCacheLines/_sa;

    //Instantiate the cache
    _cacheLines = new CacheLine* [_sa];
    for(int i=0; i < _sa; i++) {
        _cacheLines[i] = new CacheLine [numRows];
    }
    cacheSpec(numCacheLines);
    debug = false;
}

cache::cache(int sa, int lineSize, int cacheSize)
{
    _sa = sa; //Direct Mapped Cache is the default
    _cacheSize = cacheSize; //[Bytes]
    _lineSize = lineSize; //[Bytes]

    int numCacheLines = (int) _cacheSize/_lineSize;
    numRows = (int) numCacheLines/_sa;

    //Instantiate the cache
    _cacheLines = new CacheLine* [_sa];
    for(int i=0; i < _sa; i++) {
        _cacheLines[i] = new CacheLine [numRows];
    }
    cacheSpec(numCacheLines);
    debug = false;
}

cache::~cache () {
    //for (int i = 0; i < numRows; i++)
    //    delete [] _cacheLines[i];
    delete [] _cacheLines;
    printf("Destroying Cache objexts...\n");
}
//bool findTag::cache(ADDRS tag) {return true;} //TODO

void cache::cacheSpec (int numCacheLines)
{
    printf("Cache size                  = %d[B]\n", _cacheSize);
    printf("Cache line size             = %d[B]\n", _lineSize);
    printf("Cache Set Associativity     = %d\n", _sa);
    printf("Number of Cache Rows/Words  = %d\n", numRows);
    printf("Number of Cache Lines       = %d\n", numCacheLines);
    printf("-----------------------------------------\n");
}

ADDRS cache::getIndex(ADDRS addr)
{
    //printf("addr = %lu,index = %lu, tag = %lu\n", addr, addr%numRows, addr-addr%numRows);
    ADDRS tempAddr = addr >> (BLOCK_OFFSET+WORD_OFFSET); //3b for each of word-offset, block-offset
    ADDRS indx = tempAddr % numRows;
    Assert(indx >= 0 && indx < (ADDRS)numRows);
    return indx;
}

ADDRS cache::getTag(ADDRS addr)
{
    ADDRS tempAddr = addr >> (BLOCK_OFFSET+WORD_OFFSET); //3b for each of word-offset, block-offset
    ADDRS tempIndex = getIndex(addr);
    ADDRS tempTag = tempAddr - tempIndex;
    Assert (tempAddr >= tempIndex);
    return tempTag;
}

bool cache::findAddr(ADDRS addr)
{
    ADDRS indx = getIndex(addr);
    ADDRS tag = getTag(addr);
    printf("TAG = %llx, INDX = %llx\n", tag, indx);
    for (int i = 0; i < _sa; i++) {
	if (tag == _cacheLines[i][indx].getTag()) {
	    printf("I FOUNT IT!!!!\n");
	    return true;
	}
    }
    printf("I DIDN'T FIND IT!!!!\n");
    return false;
}

void cache::readCache(ADDRS addr, int8_t*& outData)
{
    //ADDRS indx = getIndex(addr);
    //ADDRS tag = getTag(addr);
    //if(debug) printf("   R-Address = %x, Index = %x, tag = %x\n", addr, indx, tag);

//  if (_sa > 1) {
//      if (!findAddr(addr))
//          printf("SHIT, the address is not in the cache!\n");
//      else
//          _cacheLines[0][indx].readLine(outData); //TODO this line has a bug - fix the index
//  } else {
//      if (tag != _cacheLines[0][indx].getTag())
//          printf("SHIT, the address is not in the cache!\n");
//      else
            //_cacheLines[0][indx].readLine(outData);
//  }
}

void cache::writeCache(ADDRS addr, int8_t *data)
{
    //int selSetIndex;
    ADDRS indx = getIndex(addr);
    ADDRS tag = getTag(addr);
    if(debug) printf("   W-Address = %llx, Index = %llx, tag = %llx\n", addr, indx, tag);

//  if (_sa > 1) {
//      //TODO you may have to chack the validity & dirtiness here too.
//      selSetIndex = LRUCacheLine(indx);
//      _cacheLines[selSetIndex][indx].writeLine(tag, data);
//  } else {
	//Direct Map cache
	//printf("print addr = %lu, %lu, %lu\n", addr, tag, indx);
        _cacheLines[0][indx].writeLine(tag, data);
//  }
}

bool cache::isValid(ADDRS addr)
{
    ADDRS indx = getIndex(addr);
    return _cacheLines[0][indx].isValid();
}

bool cache::isHit(ADDRS addr)
{
    ADDRS indx = getIndex(addr);
    //Assert (_cacheLines[0][indx].getTag() != 0); //TODO valid test. bring it back through initializing the cache differently.
    ADDRS tag = getTag(addr);
    ADDRS tempTag = _cacheLines[0][indx].getTag();
    if (tag != tempTag) {
    //printf("TAG=%lu, TEMP TAG=%lu, INDX=%u\n",tag, tempTag, indx);
	//printf("cache line valid: %x",_cacheLines[0][indx].isValid()); 
	return false;
    } else {
        return true;
    }
}

bool cache::isDirty(ADDRS addr)
{
    ADDRS indx = getIndex(addr);
    return _cacheLines[0][indx].isDirty();
}

void cache::setClean(ADDRS addr) //TODO _Sa is not incorporated here
{
    ADDRS indx = getIndex(addr);
    _cacheLines[0][indx].setClean();
}

int cache::LRUCacheLine(ADDRS addr) //TODO
{
    return 0;
}

//This function is used for writeback operation
//It returns the existing mem address of a cache line
//by receiving the to-be-written address from the input
ADDRS cache::getWBAddr(ADDRS addr)
{
    //TODO this function assumes _sa = 1 (fix it)
    ADDRS indx = getIndex(addr);
    ADDRS tag = _cacheLines[0][indx].getTag();
//    Assert (tag != 0);
    return ((tag+indx) << (BLOCK_OFFSET+WORD_OFFSET)); //Construct ~full addr
}

void cache::setExpectData(ADDRS addr, bool expct) {
    ADDRS indx = getIndex(addr);
    _cacheLines[0][indx].setExpectData(expct);
}

bool cache::getExpectData(ADDRS addr) {
    ADDRS indx = getIndex(addr);
    return _cacheLines[0][indx].getExpectData();
}
