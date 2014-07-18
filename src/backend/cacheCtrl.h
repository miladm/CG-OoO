/*******************************************************************************
 * CacheCtrl.h 
 ******************************************************************************/
#ifndef _CACHECTRL_H
#define _CACHECTRL_H
#include <stdint.h>
#include <stdlib.h>
#include "cache.h"
#include "latency.h"
#include "instruction.h"
#include "../global/global.h"

int getLatency(int memCode);
void writeUp(int memCode1, int memCode2, ADDRS addr, int*& latency, int8_t*& data, char type, memType rORw);
void writeBack(int memCode1, int memCode2, ADDRS addr, int*& latency, memType rORw);
int doRead(cache *L, ADDRS addr, int8_t*& data, int*& latency, int memCode, char type, memType rORw);
int doWrite(cache *L, ADDRS addr, int8_t*& data, int*& latency, int memCode, memType rORw);
void report (ADDRS addr, int8_t*& data, int*& latency, memType rORw);
int cacheCtrl (memType rORw, ADDRS addr, BYTES memAxesSize, cache *_L1, cache *_L2, cache *_L3); //TODO is data declared correctly?
void initCaches();

#endif
