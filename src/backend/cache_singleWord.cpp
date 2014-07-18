/*******************************************************************************
 * Cache line
 * Size is a specified number of bytes
 * Also contains state of line (per line state)
 ******************************************************************************/
#include <stdio.h>
#include "cache.h"
#include "latency.h"

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
    for(int i=0; i < _sa; i++)
        _cacheLines[i] = new CacheLine [numRows];
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
    for(int i=0; i < _sa; i++)
        _cacheLines[i] = new CacheLine [numRows];
    cacheSpec(numCacheLines);
    debug = false;
}

//bool findTag::cache(uint64_t tag) {return true;} //TODO

void cache::cacheSpec (int numCacheLines)
{
    printf("Cache size                  = %d[B]\n", _cacheSize);
    printf("Cache line size             = %d[B]\n", _lineSize);
    printf("Cache Set Associativity     = %d\n", _sa);
    printf("Number of Cache Rows/Words  = %d\n", numRows);
    printf("Number of Cache Lines       = %d\n", numCacheLines);
    printf("-----------------------------------------\n");
}

uint64_t cache::getIndex(uint64_t addr)
{
    //printf("addr = %lu,index = %lu, tag = %lu\n", addr, addr%numRows, addr-addr%numRows);
    uint64_t tempAddr = addr; //3b for each of byte-offset, word-offset, block-offset
    return (tempAddr % numRows);
}

uint64_t cache::getTag(uint64_t addr)
{
    uint64_t tempAddr = addr; //3b for each of byte-offset, word-offset, block-offset
    tempAddr = tempAddr;
    uint64_t tempIndex = getIndex(addr);
    tempIndex = tempIndex;
    return (tempAddr - tempIndex);
}

bool cache::findAddr(uint64_t addr)
{
    uint64_t indx = getIndex(addr);
    uint64_t tag = getTag(addr);
    printf("TAG = %x, INDX = %x\n", tag, indx);
    for (int i = 0; i < _sa; i++) {
	if (tag == _cacheLines[i][indx].getTag()) {
	    printf("I FOUNT IT!!!!\n");
	    return true;
	}
    }
    printf("I DIDN'T FIND IT!!!!\n");
    return false;
}

void cache::readCache(uint64_t addr, int8_t*& outData)
{
    uint64_t indx = getIndex(addr);
    uint64_t tag = getTag(addr);
    if(debug) printf("   R-Address = %x, Index = %x, tag = %x\n", addr, indx, tag);

//  if (_sa > 1) {
//      if (!findAddr(addr))
//          printf("SHIT, the address is not in the cache!\n");
//      else
//          _cacheLines[0][indx].readLine(outData); //TODO this line has a bug - fix the index
//  } else {
//      if (tag != _cacheLines[0][indx].getTag())
//          printf("SHIT, the address is not in the cache!\n");
//      else
            _cacheLines[0][indx].readLine(outData);
//  }
}

void cache::writeCache(uint64_t addr, int8_t *data)
{
    int selSetIndex;
    uint64_t indx = getIndex(addr);
    uint64_t tag = getTag(addr);
    if(debug) printf("   W-Address = %x, Index = %x, tag = %x\n", addr, indx, tag);

//  if (_sa > 1) {
//      //TODO you may have to chack the validity & dirtiness here too.
//      selSetIndex = LRUCacheLine(indx);
//      _cacheLines[selSetIndex][indx].writeLine(tag, data);
//  } else {
	//Direct Map cache
        _cacheLines[0][indx].writeLine(tag, data);
//  }
}

bool cache::isValid(uint64_t addr)
{
    uint64_t indx = getIndex(addr);
    return _cacheLines[0][indx].isValid();
}

bool cache::isHit(uint64_t addr)
{
    uint64_t indx = getIndex(addr);
    uint64_t tag = getTag(addr);
    if (tag == -1) {printf("\nERROR: the tag is corrupt\n");}

    if (tag != _cacheLines[0][indx].getTag())
	return false;
    else
        return true;
}

bool cache::isDirty(uint64_t addr)
{
    uint64_t indx = getIndex(addr);
    return _cacheLines[0][indx].isDirty();
}

void cache::setClean(uint64_t addr) //TODO _Sa is not incorporated here
{
    uint64_t indx = getIndex(addr);
    _cacheLines[0][indx].setClean();
}

int cache::LRUCacheLine(uint64_t addr) //TODO
{
    return 0;
}

//This function is used for writeback operation
//It returns the existing mem address of a cache line
//by receiving the to-be-written address from the input
uint64_t cache::getWBAddr(uint64_t addr)
{
    //TODO this function assumes _sa = 1 (fix it)
    uint64_t indx = getIndex(addr);
    uint64_t tag = _cacheLines[0][indx].getTag();
    return (tag+indx);
}
