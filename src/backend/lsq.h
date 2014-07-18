/*******************************************************************************
 * lsq.h
 * Load-Store Queuee Unit Data Structures
 ******************************************************************************/

#ifndef _LSQ_H
#define _LSQ_H 

#include <map>
#include <vector>
#include "../global/global.h"

#define SQ_SIZE 36 //TODO what number is right?
#define LQ_SIZE 64 //TODO what number is right?

class instruction;

//Each Store entry is a struct by itself
struct storeObj {
	instruction* _ins; //TODO how to eliminate this?
	INS_ID _id;
	ADDRS _memAddr;
	bool _committed;
	bool _sentToCache;
	bool _validEntries;
	int _cacheAccessLatency;
	int _storeToCacheEndCycle;
	int _storeToCacheBeginCycle;
};

//Each Load entry is a struct by itself
struct loadObj {
	instruction* _ins; //TODO how to eliminate this?
	INS_ID _id;
	ADDRS _memAddr;
	bool _validDataEntry;
	bool _validMemAddrEntry;
	bool _sqDataFwd; //has SQ forwarded data already?
};

class lsq {
	public:
		lsq();
		~lsq();
		
		//SQ Operations
		bool isSQfull();
		bool isSQempty();
		void pushBackSQ(instruction* ins);
		void popFrontSQ();
		instruction* getSQhead();
		instruction* getSQtail();
		void insertSQ_addrNdata(instruction* ins);
		bool lookupSQ(instruction* ins);
		void updateSQcommitSet(instruction* ins);
		bool isFrontSQdoneWritingCache(int cycle);
		bool hasCommittedUncachedSQentry();
		ADDRS get_oldestCommittedUncachedSQentry_MemAddr(INS_ID id);
		INS_ID get_OldestCommittedUncachedSQentry_ID();
		void set_OldestCommittedUncachedSQentry_setLatency(int latency, int cycle, INS_ID id);
		bool SQhasIncompleteIns();
		int squashSQ(INS_ID insId);
		int getSQsize();
		
		//LQ Operations
		bool isLQfull();
		bool isLQempty();
		void pushBackLQ(instruction* ins);
		void popFrontLQ();
		INS_ID getLQheadId();
		void insertLQ_addr(instruction* ins);
		INS_ID lookupLQ(instruction* ins);
		void setForwardData(instruction* ins);
		int squashLQ(INS_ID insId);
		int getLQsize();

	private:
		//SQ
		storeObj* _sq[SQ_SIZE];
		unsigned int _sqTail;
		int _sqSize;
		//LQ
		loadObj* _lq[LQ_SIZE];
		unsigned int _lqTail;
		int _lqSize;
};

#endif
