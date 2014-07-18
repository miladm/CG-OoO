/*******************************************************************************
 * Cache controller
 * cacheCtrl.cpp
 ******************************************************************************/
#include <stdio.h>
#include <time.h>

#include "cacheCtrl.h"
#include "../global/global.h"
#include "../lib/utility.h"

//cache L1(1, 8, 256);
//cache L2(1, 8, 2048);
//cache L3(1, 8, 8192);
cache *L1;
cache *L2;
cache *L3;


bool debug = true;
bool debug1 = false;
int hitLevel;
//TODO we should get rid of the variable "latency"

void writeUp(int memCode1, int memCode2, ADDRS addr, int*& latency, int8_t*& data, memType rORw)
{
 	int8_t *tempData = (int8_t *) malloc(sizeof(int8_t)*8);

	if (memCode1 == 1 && memCode2 == 2) {
		if(debug1) printf("   WU, memcCode:%d, addr = %llx\n", memCode1, addr);
		doRead (L2, addr, data, latency, 2, 'R', rORw);
		doWrite (L1, addr, data, latency, 1, rORw);
		L1->setClean(addr); //reset the dirty flag
	} else if (memCode1 == 2 && memCode2 == 3) {
		if(debug1) printf("   WU, memcCode:%d, addr = %llx\n", memCode1, addr);
		doRead (L3, addr, data, latency, 3, 'R', rORw);
		doWrite (L2, addr, data, latency, 2, rORw);
		L2->setClean(addr); //reset the dirty flag
	} else if (memCode1 == 3 && memCode2 == 4) {
		if(debug1) printf("   WU, memcCode:%d\n", memCode1);
		//Skip actually writing into the memory (to avoid crazy mem. consumption)
		doRead (L3, addr, data, latency, 4, 'R', rORw); //L2 is dummy... not being used or modified here
		//Generate Random Data (TODO: this should be removed)
		doWrite (L3, addr, data, latency, 3, rORw); //TODO this is WRONG and must be fixed
		L3->setClean(addr); //reset the dirty flag
	}
	delete [] tempData;
}

void writeBack(int memCode1, int memCode2, ADDRS addr, int*& latency, memType rORw) {
 	int8_t *tempData = (int8_t *) malloc(sizeof(int8_t)*8);

	if (memCode1 == 1 && memCode2 == 2) {
		ADDRS tempAddr = L1->getWBAddr(addr);
		if(debug1) printf("   WB, memcCode:%d - ADDRS:%llx\n", memCode1, addr);
		doRead (L1, tempAddr, tempData, latency, 1, 'W', rORw);
		doWrite (L2, tempAddr, tempData, latency, 2, rORw);
		L1->setClean(addr); //reset the dirty flag
		delete [] tempData;
	} else if (memCode1 == 2 && memCode2 == 3) {
		ADDRS tempAddr = L2->getWBAddr(addr);
		if(debug1) printf("   WB, memcCode:%d\n", memCode1);
		doRead (L2, tempAddr, tempData, latency, 2, 'W', rORw);
		doWrite (L3, tempAddr, tempData, latency, 3, rORw);
		L2->setClean(addr); //reset the dirty flag
		delete [] tempData;
	} else if (memCode1 == 3 && memCode2 == 4) {
		//ADDRS tempAddr = L3->getWBAddr(addr);
		if(debug1) printf("   WB, memcCode:%d\n", memCode1);
		//Skip actually writing into the memory (to avoid crazy mem. consumption)
		//doRead (L2, tempAddr, tempData, latency, 2, 'W', rORw);
		//doWrite (L3, tempAddr, tempData, latency, 3, rORw);
		L3->setClean(addr); //reset the dirty flag
		delete [] tempData;
	}
}


int doRead(cache *L, ADDRS addr, int8_t*& data, int*& latency, int memCode, char type, memType rORw)
{
	//if(memCode > hitLevel) hitLevel = memCode;
	if(debug1) printf("Onto Reading L%d - ADDRS=%llx\n", memCode,addr); 
	if (memCode <= 3) {
		if (L->isValid(addr)) {
			if (L->isHit(addr)) {
				if(debug1) printf("   R, V, H\n");
				if(memCode > hitLevel && rORw == READ) hitLevel = memCode;
				L->readCache(addr, data);
				return 0; //Cache Hit
			} else {
				if(L->isDirty(addr)) {
					if(debug1) printf("   R, V, M, D\n");
					//if(debug1) printf("   ** why dirty: %x\n", L->getTag(addr)); //TODO getTag must go back to being provate...
					if (type == 'W') printf("      ADDRESSSSSSSS %llx\n", addr);
					writeBack(memCode, memCode+1, addr, latency, rORw);
					if (type == 'R') {writeUp(memCode, memCode+1, addr, latency, data, rORw);}
				} else {
					if(debug1) printf("   R, V, M, C\n");
					writeUp(memCode, memCode+1, addr, latency, data, rORw);
				}
				L->readCache(addr, data);
				return 0; //Cache miss (conflict) - check for dirtiness
			}
		} else {
			if(debug1) printf("   R, I\n");
			writeUp(memCode, memCode+1, addr, latency, data, rORw);
			if (memCode == 1) L->readCache(addr, data);
			return 0; //Cache miss (compulsary) (fetch-> read)
		}
	} else {
		if(memCode > hitLevel && rORw == READ) hitLevel = memCode; //TODO can a write ever make it here?
		//We don't care about the number from DRAM. So gen a rand val
		if(debug1) printf("   R, L4\n");
		if(debug1) printf("   Data to Write: ");
		//for (int i = 0; i < 8; i++)
		//{
		//    data[i] = rand() % 100 + 1;
		//    if(debug1) printf("%d ", data[i]);
		//}
		if(debug1) printf("\n");
	}
	return 0;
}


int doWrite(cache *L, ADDRS addr, int8_t*& data, int*& latency, int memCode, memType rORw)
{
	//if(memCode > hitLevel) hitLevel = memCode;
	if(debug1) printf("Onto Writing L%d - ADDRS=%llx\n", memCode, addr); 
	if (memCode <= 3) {
		if (L->isValid(addr)) {
			if (L->isHit(addr)) {
				if(debug1) printf("   W, V, H\n");
				L->writeCache(addr, data);
				return 0; //Cache Hit (no care for dirty flag)
			} else {
				//int8_t *inData = (int8_t *) malloc(sizeof(int8_t)*8);
				//for (int i = 0; i < 8; i++) {inData[i] = data[i];}
				if (L->isDirty(addr)) {
					if(debug1) printf("   W, V, M, D\n");
					if(debug1) printf("   ** why dirty: %llx\n", L->getTag(addr));
					writeBack(memCode, memCode+1, addr, latency, rORw);
					writeUp(memCode, memCode+1, addr, latency, data, rORw); //TODO fix? need it?
					//Cache Miss (writeback -> fetch -> write)
					L->writeCache(addr, data);
				} else {
					if(debug1) printf("   W, V, M, C\n");
					if (L->getExpectData(addr) == false) {
						L->setExpectData(addr,true);
						writeUp(memCode, memCode+1, addr, latency, data, rORw);
						L->setExpectData(addr,false);
						L->writeCache(addr, data);
					} else {
						L->writeCache(addr, data);
					}
					//Cache Miss (fetch -> write)
				}
				return 0;
			}
		} else {
			if(debug1) printf("   W, I\n");
					//writeUp(memCode, memCode+1, addr, latency, data, rORw); //TODO: should not need it
			if (L->getExpectData(addr) == false) {
				L->setExpectData(addr,true);
				writeUp(memCode, memCode+1, addr, latency, data, rORw);
				L->setExpectData(addr,false);
				L->writeCache(addr, data);
			} else {
				L->writeCache(addr, data);
			}
			return 0; //Cache Miss (fetch -> write)
		}
	} else {
		if(debug1) printf("   W, L4\n");
		return 0;
	}
}

		
void report (ADDRS addr, int8_t*& data, int*& latency, memType rORw) {}

int cacheCtrl (memType rORw, ADDRS addr, BYTES memAxesSize, cache *_L1, cache *_L2, cache *_L3) //TODO is data declared correctly?
{
    int* latency = new int;
	L1 = _L1;
	L2 = _L2;
	L3 = _L3;
	// initialize random seed
	srand ( time(NULL) );

	//Generate Random Data (TODO: this should be removed)
	int8_t *data = (int8_t *) malloc(sizeof(int8_t)*8);
	int8_t *inData = (int8_t *) malloc(sizeof(int8_t)*8);
	//for (int i = 0; i < 8; i++) {inData[i] = 0;}
	
	//printf("cache address = %x\n", &L2);

	/*----Handle READS & WRITES----*/
	if (rORw == READ) { //Handle READ
  	    //printf("----READ: addr = %x\n", addr);
	    hitLevel = 1;
	    doRead(L1, addr, data, latency, 1, 'R', rORw);
	    //return getLatency(hitLevel);
	    //if (lineNum >= 71272839)
	    //    fprintf(outFile, "%d,%lx\n", getLatency(hitLevel), addr);
	} else if (rORw == WRITE) { //Handle WRITE
  	    //printf("----WRITE: addr = %x\n", addr);
	    hitLevel = 1;
	    doWrite(L1, addr, inData, latency, 1, rORw);
	    hitLevel = 1;//Hack to make write always a hit!
	    //return getLatency(hitLevel);
	    //if (lineNum >= 71272839)
	    //    fprintf(outFile, "%d,%lx\n", getLatency(hitLevel), addr);
	} else {
	    printf("WARNING: the memory access type is not specified\n");
	    return -1;
	}

	delete [] data;
	delete [] inData;
    delete latency;
	if(hitLevel == 0) printf("ERROR: hit level is 0\n");
	Assert (hitLevel != 0);
	if (debug1) printf("***********DONE MEM OP****************\n");
	return  getLatency(hitLevel, 0);
}












/* My tests for cache behavior:
		addr = rand() % 4094;
		addr1 = rand() % 4096;

  		    printf("----READ: addr = %x\n", addr);
		    hitLevel = 0;
		    result = doRead(L1, addr, data, latency, 1, 'R');
		    printf("   latency %d\n", getLatency(hitLevel));
		    printf("----READ: addr = %x\n", addr+256);
		    hitLevel = 0;
		    result = doRead(L1, addr+256, data, latency, 1, 'R');
		    printf("   latency %d\n", getLatency(hitLevel));
		    printf("----READ: addr = %x\n", addr1);
		    hitLevel = 0;
		    result = doRead(L1, addr1, data, latency, 1, 'R');
		    printf("   latency %d\n", getLatency(hitLevel));
		    printf("----WRITE: addr = %x, data = %d, %d, %d, %d, %d, %d, %d, %d\n", addr, inData[0], inData[1], inData[2], inData[3], inData[4], inData[5], inData[6], inData[7]);
		    hitLevel = 0;
		    result = doWrite(L1, addr, inData, latency, 1);
		    printf("   latency %d\n", getLatency(hitLevel));
		    printf("----READ: addr = %x\n", addr);
		    hitLevel = 0;
		    result = doRead(L1, addr, data, latency, 1, 'R');
		    for (int i = 0; i < 8; i++) {inData[i] = rand() % 10000000 + 1;}
		    printf("   latency %d\n", getLatency(hitLevel));
		    printf("----WRITE: addr = %x, data = %d, %d, %d, %d, %d, %d, %d, %d\n", addr, inData[0], inData[1], inData[2], inData[3], inData[4], inData[5], inData[6], inData[7]);
		    hitLevel = 0;
		    result = doWrite(L1, addr, inData, latency, 1);
		    printf("   latency %d\n", getLatency(hitLevel));
		    printf("----READ: addr = %x\n", addr);
		    hitLevel = 0;
		    result = doRead(L1, addr, data, latency, 1, 'R');
		    inData[7]+=10;
		    printf("   latency %d\n", getLatency(hitLevel));
		    printf("----WRITE: addr = %x, data = %d, %d, %d, %d, %d, %d, %d, %d\n", addr+256, inData[0], inData[1], inData[2], inData[3], inData[4], inData[5], inData[6], inData[7]);
		    hitLevel = 0;
		    result = doWrite(L1, addr+256, inData, latency, 1);
		    printf("   latency %d\n", getLatency(hitLevel));
		    printf("----READ: addr = %x\n", addr+256);
		    hitLevel = 0;
		    result = doRead(L1, addr+256, data, latency, 1, 'R');
		    printf("   latency %d\n", getLatency(hitLevel));
		    printf("----READ: addr = %x\n", addr);
		    hitLevel = 0;
		    result = doRead(L1, addr, data, latency, 1, 'R');
		    printf("   latency %d\n", getLatency(hitLevel));
*/
