/*******************************************************************************
 * Cache line
 * Size is a specified number of bytes
 * Also contains state of line (per line state)
 ******************************************************************************/
#include "cacheLine.h"
#include <stdio.h>


CacheLine::CacheLine ()
{
    _valid = false;
    _dirty = false;
    _tag = -1;
    _lineSize = 8; //[Bytes] 64b cache line to support double
    _data = new int8_t[_lineSize];
    for (int i = 0; i < _lineSize; i++) {_data[i] = 0;}
    if(!_data){
        //throw Fwk::MemoryException("Alloc failed in CacheLine::CacheLine"); //TODO fix this
    }
    debug = false;
}

CacheLine::~CacheLine () {
    delete [] _data;
}

void CacheLine::writeLine(long int inTag, int8_t *inData)
{
    //printf("TAG IN  = %lu\n", _tag);
    _tag = inTag;
    //printf("TAG IN  = %lu\n", _tag);
    _valid = true;
    _dirty = true;
    if(debug) printf("   Cache Content: ");
//  for (int i = 0; i < _lineSize; i++) {
//	_data[i] = inData[i];
//      if(debug) printf("%d ", _data[i]);
//  }
    if(debug) printf("\n");
}

void CacheLine::readLine(int8_t*& inData)
{
////////inData = _data; //TODO does this work?
	if(debug) printf("   Cache Content: ");
////////for (int i = 0; i < _lineSize; i++) {
////////    if(debug) printf("%d ", _data[i]);
////////}
	if(debug) printf("\n");
}

ADDRS CacheLine::getTag () {
	//printf("TAG OUT = %lu\n", _tag);
	return _tag;
}

void CacheLine::setTag (long int tag) {
	_tag = tag;
}

//uses for set associativity
long int CacheLine::getCycle() {
        return _cycle;
}

void CacheLine::setCycle(long int cycle) {
	_cycle = cycle;
}

void CacheLine::setExpectData(bool expct) {
	_expectData = expct;
}
bool CacheLine::getExpectData() {
	return _expectData;
}

bool CacheLine::isValid() {return _valid;}

bool CacheLine::isDirty() {return _dirty;}

void CacheLine::setClean() {_dirty = false;}

int  CacheLine::getLineSize() {return _lineSize;}
