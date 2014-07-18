/*******************************************************************************
 *  stat.h
 ******************************************************************************/
#ifndef _STAT_H
#define _STAT_H
#include <stdint.h>
#include <stdlib.h>


class stat {
    public:
	stat();
	~stat(){/*nothing for now*/}

	void setMemAddr(uint64_t memAddr);
	void setRegister(int *r, int *rt);
	void setType(type insType);
	void setStatus(status insStatus, int cycle, int latency);
	void setMemType(memType readORwrite);

	type getType();
	status getStatus();
	int getLatency();
	int getCompleteCycle();
	int getIssueCycle();
	int getMyReg(int i);
	int getNumReg();
	uint64_t getMemAddr ();
	int getMyRegType(int i);
	memType getMemType();

    private:

	//convention:
	//_r* = -1: means reg is not assigned a value (fresh)
	//_r* = -2: means reg is invalid

	List<int*> *_r;  //Register List
	List<int*> *_rt; //Register Type List
	uint64_t _memAddr;
	int _issueCycle;
	int _completeCycle;
	int _latency;
	type _insType;
	status _insStatus;
	memType _readORwrite;
};

#endif
