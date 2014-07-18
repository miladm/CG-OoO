/*******************************************************************************
 * lsq.cpp
 * Load-Store Queuee Unit Data Structures
 ******************************************************************************/

#include "lsq.h"
#include "instruction.h"

lsq::lsq() {
	_sqSize = 0;
	_sqTail = -1;
	_lqSize = 0;
	_lqTail = -1;
}

lsq::~lsq() {
	//empty for now
}

/***********************/
/* STORE QUEUE METHODS */ 
/***********************/
bool lsq::isSQfull() {
	Assert(_sqSize <= SQ_SIZE && "ERROR: Invalid Store Queue Size");
	Assert(_sqSize >= 0       && "ERROR: Invalid Store Queue Size");
	if (_sqSize == SQ_SIZE) return true;
	else return false;
}

bool lsq::isSQempty() {
	Assert(_sqSize <= SQ_SIZE && "ERROR: Invalid Store Queue Size");
	Assert(_sqSize >= 0       && "ERROR: Invalid Store Queue Size");
	if (_sqSize == 0) return true;
	else return false;
}

void lsq::pushBackSQ(instruction* ins) {
	Assert(_sqSize < SQ_SIZE && "ERROR: Store Queue is Full");

	// Contruct Store Object
	storeObj* sObj = new storeObj;
	sObj->_ins = ins;
	sObj->_id = ins->getInsID();
	sObj->_committed = false;
	sObj->_sentToCache = false;
	sObj->_validEntries = false; //functino called @fetch/decode
	sObj->_cacheAccessLatency = 0;
	sObj->_storeToCacheBeginCycle = 0;
	sObj->_storeToCacheEndCycle = 0;

	// Update Store Queue
	_sqTail = (_sqTail+1) % SQ_SIZE;
	_sq[_sqTail] = sObj;
	_sqSize++;
	//printf("INSERT %d\n", _sqSize);
}

void lsq::popFrontSQ() {
	Assert(_sqSize > 0 && "Store Queue is Empty.");
	Assert(_sqTail >= 0 && _sqTail < SQ_SIZE);
	unsigned int sqHead = (_sqTail - (_sqSize - 1)) % (-1*SQ_SIZE);
	delete _sq[sqHead];
	_sqSize--;
	//printf("REMOVE %d\n",_sqSize);
}

instruction* lsq::getSQtail() {
	Assert(_sqTail >= 0 && _sqTail < SQ_SIZE);
	return _sq[_sqTail]->_ins;
}

instruction* lsq::getSQhead() {
	Assert(_sqSize > 0 && "Store Queue is Empty.");
	Assert(_sqTail >= 0 && _sqTail < SQ_SIZE);
	unsigned int sqHead = (_sqTail - (_sqSize - 1)) % (-1*SQ_SIZE);
	return _sq[sqHead]->_ins;
}

//NOTE: here we only update the addr (data is not modeled in this simualtor)
void lsq::insertSQ_addrNdata(instruction* ins) {
	Assert(_sqSize > 0 && "Store Queue is Empty.");
	Assert(_sqTail >= 0 && _sqTail < SQ_SIZE);
	INS_ID insId = ins->getInsID();
	unsigned int sqHead = (_sqTail - (_sqSize - 1)) % (-1*SQ_SIZE);
	int counter = 0;
	//printf("Size: %d %d %d\n",_sqSize, _sqTail,sqHead);
	while (counter < _sqSize) {
		unsigned int i = (sqHead+counter)%SQ_SIZE;
		//printf("%d,%d: %llx, %llx\n",counter, i, insId,_sq[i]->_id);
		if (insId == _sq[i]->_id) {
			Assert(_sq[i]->_validEntries == false);
			_sq[i]->_memAddr = ins->getMemAddr();
			_sq[i]->_validEntries = true;
			//printf("\n");
			return;
		}
		counter++;
	}
	Assert(false == true && "Operation Entry not Found in SQ");
}

void lsq::updateSQcommitSet(instruction* ins) {
	Assert(_sqSize > 0 && "Store Queue is Empty.");
	Assert(_sqTail >= 0 && _sqTail < SQ_SIZE);
	INS_ID insId = ins->getInsID();
	unsigned int sqHead = (_sqTail - (_sqSize - 1)) % (-1*SQ_SIZE);
	int counter = 0;
	while (counter < _sqSize) {
		unsigned int i = (sqHead+counter)%SQ_SIZE;
		if (insId == _sq[i]->_id) {
			Assert(_sq[i]->_validEntries == true && _sq[i]->_committed == false);
			_sq[i]->_committed = true;
			return;
		} else {
			//All older store ops must be committed
			Assert(_sq[i]->_committed == true);
		}
		counter++;
	}
	Assert(false == true && "Operation Entry not Found in SQ");
}

bool lsq::isFrontSQdoneWritingCache(int cycle) {
	Assert(_sqSize > 0 && "Store Queue is Empty.");
	Assert(_sqTail >= 0 && _sqTail < SQ_SIZE);
	unsigned int sqHead = (_sqTail - (_sqSize - 1)) % (-1*SQ_SIZE);
	//NOTE: can add this line to avoid bug - seems to be unnecessary for now
	//if (_sq[sqHead]->_committed == false || _sq[sqHead]->_sentToCache == false) return false;
	return (_sq[sqHead]->_storeToCacheEndCycle == cycle);
}

bool lsq::lookupSQ(instruction* ins) {
	if (_sqSize == 0) return false;
	Assert(_sqTail >= 0 && _sqTail < SQ_SIZE);
	Assert(ins->getMemType() == READ);
	ADDRS memAddr = ins->getMemAddr();
	unsigned int sqHead = (_sqTail - (_sqSize - 1)) % (-1*SQ_SIZE);
	int counter = 0;
	while (counter < _sqSize) {
		unsigned int i = (sqHead+counter)%SQ_SIZE;
		if (_sq[i]->_validEntries == true && memAddr == _sq[i]->_memAddr) {
			return true;
		}
		counter++;
	}
	return false; //Didn't find LD mem addr in SQ
}

bool lsq::hasCommittedUncachedSQentry() {
	if (_sqSize == 0) return false;
	Assert(_sqTail >= 0 && _sqTail < SQ_SIZE);
	unsigned int sqHead = (_sqTail - (_sqSize - 1)) % (-1*SQ_SIZE);
	int counter = 0;
	while (counter < _sqSize) {
		unsigned int i = (sqHead+counter)%SQ_SIZE;
		if (_sq[i]->_validEntries && _sq[i]->_committed) {
			if (!_sq[i]->_sentToCache) {
				return true; //found an uncached committed ST op
			}
		} else {
			return false;
		}
		counter++;
	}
	return false; //Didn't find an uncached committed ST op
}

ADDRS lsq::get_oldestCommittedUncachedSQentry_MemAddr(INS_ID id) {
	Assert(_sqSize > 0 && "Expected to find a committed, uncached ST op");
	Assert(_sqTail >= 0 && _sqTail < SQ_SIZE);
	unsigned int sqHead = (_sqTail - (_sqSize - 1)) % (-1*SQ_SIZE);
	int counter = 0;
	while (counter < _sqSize) {
		unsigned int i = (sqHead+counter)%SQ_SIZE;
		if (_sq[i]->_validEntries && _sq[i]->_committed && !_sq[i]->_sentToCache) {
			Assert(id == _sq[i]->_id);
			return _sq[i]->_memAddr;
		} else {
			Assert(_sq[i]->_validEntries && _sq[i]->_committed && _sq[i]->_sentToCache);
		}
		counter++;
	}
	Assert(true == false && "Expected to find a committed, uncached ST op");
	return 0; //does nothing - for compiler
}

INS_ID lsq::get_OldestCommittedUncachedSQentry_ID() {
	Assert(_sqSize > 0 && "Expected to find a committed, uncached ST op");
	Assert(_sqTail >= 0 && _sqTail < SQ_SIZE);
	unsigned int sqHead = (_sqTail - (_sqSize - 1)) % (-1*SQ_SIZE);
	int counter = 0;
	while (counter < _sqSize) {
		unsigned int i = (sqHead+counter)%SQ_SIZE;
		if (_sq[i]->_validEntries && _sq[i]->_committed && !_sq[i]->_sentToCache) {
			return _sq[i]->_id;
		} else {
			Assert(_sq[i]->_sentToCache == true);
		}
		counter++;
	}
	Assert(true == false && "Expected to find a committed, uncached ST op");
	return 0; //does nothing - for compiler
}

void lsq::set_OldestCommittedUncachedSQentry_setLatency(int latency, int cycle, INS_ID id) {
	Assert(_sqSize > 0 && "Expected to find a committed, uncached ST op");
	Assert(_sqTail >= 0 && _sqTail < SQ_SIZE);
	unsigned int sqHead = (_sqTail - (_sqSize - 1)) % (-1*SQ_SIZE);
	int counter = 0;
	while (counter < _sqSize) {
		unsigned int i = (sqHead+counter)%SQ_SIZE;
		if (_sq[i]->_validEntries && _sq[i]->_committed && !_sq[i]->_sentToCache) {
			Assert(id == _sq[i]->_id);
			_sq[i]->_cacheAccessLatency = latency;
			_sq[i]->_storeToCacheBeginCycle = cycle;
			_sq[i]->_storeToCacheEndCycle = cycle+latency;
			_sq[i]->_sentToCache = true;
			return;
		} else {
			Assert(_sq[i]->_sentToCache == true);
		}
		counter++;
	}
	Assert(true == false && "Expected to find a committed, uncached ST op");
}

bool lsq::SQhasIncompleteIns() {
	if (_sqSize == 0) return false;
	Assert(_sqTail >= 0 && _sqTail < SQ_SIZE);
	unsigned int sqHead = (_sqTail - (_sqSize - 1)) % (-1*SQ_SIZE);
	int counter = 0;
	while (counter < _sqSize) {
		unsigned int i = (sqHead+counter)%SQ_SIZE;
		if (!_sq[i]->_validEntries || !_sq[i]->_committed) {
			return true;
		}
		counter++;
	}
	return false;
}


int lsq::squashSQ(INS_ADDR insId) {
	Assert(_sqSize > 0);
	Assert(_sqTail >= 0 && _sqTail < SQ_SIZE);
	unsigned int sqHead = (_sqTail - (_sqSize - 1)) % (-1*SQ_SIZE);
	unsigned int counter = 0;
	//printf("%d,%d,%d,%lu\n",_sqTail,_sqSize,sqHead,insId);
	unsigned int oldSqTail = _sqTail;
	int beforeSquashSize = (int) _sqSize;
	while (counter < (unsigned int)_sqSize) {
		unsigned int i = (sqHead+counter)%SQ_SIZE;
		//if (i == sqHead) printf("%d and %d\n",_sq[i]->_id, insId);
		if (_sq[i]->_id < insId) {
			_sqTail = i;
		} else {
			Assert(i != sqHead); //impossible case
			delete _sq[i];
		}
		counter++;
	}
	if (oldSqTail != _sqTail) {
		if (sqHead == _sqTail) _sqSize = 1;
		else                   _sqSize = (_sqTail - sqHead + 1) % (-1*SQ_SIZE);
	}
	//printf("%d,%d,%d\n",_sqTail,_sqSize,sqHead);
	int afterSquashSize = (int)_sqSize;
	Assert(_sqSize > 0);
	Assert(_sqTail >= 0 && _sqTail < SQ_SIZE);
	int squashInsCount = beforeSquashSize - afterSquashSize;
	Assert(squashInsCount >= 0);
	return squashInsCount;
}

int lsq::getSQsize() {
	return _sqSize;
}

/**********************/
/* LOAD QUEUE METHODS */ 
/**********************/
//Check if LQ is full
bool lsq::isLQfull() {
	Assert(_lqSize <= LQ_SIZE && "ERROR: Invalid Load Queue Size");
	Assert(_lqSize >= 0       && "ERROR: Invalid Load Queue Size");
	if (_lqSize == LQ_SIZE) return true;
	else return false;
}

//Check if LQ is empty
bool lsq::isLQempty() {
	Assert(_lqSize <= LQ_SIZE && "ERROR: Invalid Load Queue Size");
	Assert(_lqSize >= 0       && "ERROR: Invalid Load Queue Size");
	if (_lqSize == 0) return true;
	else return false;
}

//Push element to the end of LQ
void lsq::pushBackLQ(instruction* ins) {
	Assert(_lqSize < LQ_SIZE && "ERROR: Load Queue is Full");

	//printf("insert: %lu\n", ins->getInsID());
	// Contruct Load Object
	loadObj* lObj = new loadObj;
	lObj->_ins = ins;
	lObj->_id = ins->getInsID();
	lObj->_validMemAddrEntry = false; //functino called @fetch/decode
	lObj->_validDataEntry = false; //functino called @fetch/decode
	lObj->_sqDataFwd = false;

	// Update Load Queue
	_lqTail = (_lqTail+1) % LQ_SIZE;
	_lq[_lqTail] = lObj;
	_lqSize++;
}

//Dequeue the head of queue element
void lsq::popFrontLQ() {
	Assert(_lqSize > 0 && "Load Queue is Empty.");
	Assert(_lqTail >= 0 && _lqTail < LQ_SIZE);
	unsigned int lqHead = (_lqTail - (_lqSize - 1)) % (-1*LQ_SIZE);
	//printf("popping: %lu\n", _lq[lqHead]->_id);
	delete _lq[lqHead];
	_lqSize--;
}

//Get the head of LQ
INS_ID lsq::getLQheadId() {
	Assert(_lqSize > 0 && "Load Queue is Empty.");
	Assert(_lqTail >= 0 && _lqTail < LQ_SIZE);
	unsigned int lqHead = (_lqTail - (_lqSize - 1)) % (-1*LQ_SIZE);
	return _lq[lqHead]->_id;
}

//Insert the memory address into the corresponding LQ entry 
//(CAM lookup to find element)
//NOTE: here we only update the addr (data is technically updated in a later stage of pipeline)
void lsq::insertLQ_addr(instruction* ins) {
	//printf("insert addr: %lu\n", ins->getInsID());
	Assert(_lqSize > 0 && "Load Queue is Empty.");
	Assert(_lqTail >= 0 && _lqTail < LQ_SIZE);
	INS_ID insId = ins->getInsID();
	unsigned int lqHead = (_lqTail - (_lqSize - 1)) % (-1*LQ_SIZE);
	int counter = 0;
	while (counter < _lqSize) {
		unsigned int i = (lqHead+counter)%LQ_SIZE;
		if (insId == _lq[i]->_id) {
			Assert(_lq[i]->_validMemAddrEntry == false);
			_lq[i]->_memAddr = ins->getMemAddr();
			_lq[i]->_validMemAddrEntry = true;
			return;
		}
		counter++;
	}
	Assert(false == true && "Operation Entry not Found in LQ");
}

//Lookup/search the LQ 
//(CAM lookup to find element)
//True: found a mis-speculated memory operation 
//False: didn't find a mis-speculated memory operation 
INS_ID lsq::lookupLQ(instruction* ins) {
	if (_lqSize == 0) return false;
	Assert(_lqTail >= 0 && _lqTail < LQ_SIZE);
	Assert(ins->getMemType() == WRITE);
	ADDRS memAddr = ins->getMemAddr();
	unsigned int lqHead = (_lqTail - (_lqSize - 1)) % (-1*LQ_SIZE);
	int counter = 0;
	while (counter < _lqSize) {
		unsigned int i = (lqHead+counter)%LQ_SIZE;
		if (_lq[i]->_validMemAddrEntry == true && _lq[i]->_sqDataFwd == false && memAddr == _lq[i]->_memAddr) {
			Assert(_lq[i]->_id > 0);
			return _lq[i]->_id; //oops, need to squash
		}
		counter++;
	}
	return 0; //all speculations are find wrt this ST op
}

//Find the LQ entry and update its SQ data forwardig status 
//(CAM lookup to find element)
void lsq::setForwardData(instruction* ins) {
	Assert(_lqSize > 0 && "Load Queue is Empty.");
	Assert(_lqTail >= 0 && _lqTail < LQ_SIZE);
	Assert(ins->getMemType() == READ);
	INS_ID insId = ins->getInsID();
	unsigned int lqHead = (_lqTail - (_lqSize - 1)) % (-1*LQ_SIZE);
	int counter = 0;
	while (counter < _lqSize) {
		unsigned int i = (lqHead+counter)%LQ_SIZE;
		if (insId == _lq[i]->_id) {
			Assert(_lq[i]->_validMemAddrEntry == true && _lq[i]->_sqDataFwd == false);
			_lq[i]->_sqDataFwd = true; //Store Forwarding
			return;
		}
		counter++;
	}
	Assert(false == true && "Operation Entry not Found in LQ");
}

int lsq::squashLQ(INS_ADDR insId) {
	Assert(_lqSize > 0);
	Assert(_lqTail >= 0 && _lqTail < LQ_SIZE);
	unsigned int lqHead = (_lqTail - (_lqSize - 1)) % (-1*LQ_SIZE);
	int counter = 0;
	bool startSquash = false, oneElementLeft = false;
	int beforeSquashSize = (int) _lqSize;
	//printf("%d,%d,%d,%lu\n",_lqTail,_lqSize,lqHead,insId);
	while (counter < _lqSize) {
		unsigned int i = (lqHead+counter)%LQ_SIZE;
		//printf("%d\n",i);
		if (_lq[i]->_id == insId) {
			//printf("found: %d\n",i);
			Assert(startSquash == false);
			startSquash = true;
			if (lqHead == i) {
				_lqTail = i;
			} else {
				_lqTail = (i - 1) % (-1*LQ_SIZE);
				if (_lqTail == lqHead) oneElementLeft = true;
			}
		}
		if (startSquash) {
			Assert(_lq[i]->_id >= insId);
			delete _lq[i];
		}
		counter++;
	}
	if (lqHead == _lqTail && !oneElementLeft) _lqSize = 0;
	else if (lqHead == _lqTail && oneElementLeft) _lqSize = 1;
	else                   _lqSize = (_lqTail - lqHead + 1) % (-1*LQ_SIZE);
	//printf("%d,%d,%d\n",_lqTail,_lqSize,lqHead);
	int afterSquashSize = (int)_lqSize;
	Assert(_lqSize >= 0);
	Assert(startSquash == true && "We must have squashed at least one LD op");
	Assert(_lqTail >= 0 && _lqTail < LQ_SIZE);
	int squashInsCount = beforeSquashSize - afterSquashSize;
	Assert(squashInsCount >= 0);
	return squashInsCount;
}

int lsq::getLQsize() {
	return _lqSize;
}

