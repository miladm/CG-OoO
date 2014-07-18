/*******************************************************************************
 *  instruction.h
 *  Instruction object. Each instruction is placed into the instruction window for
 *  fetch and then removed from the window when commited.
 ******************************************************************************/

#ifndef _INSTRUCTION_H
#define _INSTRUCTION_H
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/list.h"
#include "../global/global.h"
#include "latency.h"

#define NUM_REG 4

class dependencyTable;
class phrase;
class fragment;
class vliwScheduler;
class registerRename;
class lsq;

class instruction {
    public:
	instruction();
	~instruction();

	void setMemAddr(ADDRS memAddr);
	void setInsAddr(ADDRS insAddr);
	void setMemAccessSize(long int memAccessSize);
	void setRegister(long int *r, int *rt);
	long int getNthReg(int i);
	int getNthRegType(int i);
	void removeNthRegister(int i);
	int getNumReg();
	int getNumWrReg();
	int getNumLRFreg();
	void setType(type insType);
	void setStatus(status insStatus, long int cycle, int latency);
	void updateLatency(long int cycle, int latency);
	void setMemType(memType readORwrite);

	type getType();
	status getStatus();
	int getLatency();
	long int getCompleteCycle();
	long int getMemAddrCompCompleteCycle();
	long int getExecuteCycle();
	long int getMyReg(int i);
	ADDRS getMemAddr ();
	ADDRS getInsAddr ();
	long int getMemAccessSize ();
	int getMyRegType(int i);
	memType getMemType();
	char* getCmdStr();
	void setCmdStr(const char * cmd);
	void setInsID(INS_ID id);
	INS_ID getInsID();
	int getPipelineLat ();
	void setPiepelineLat (int pipeLineLat);
	void importCacheHitLevel(int hitLevel);
	void setCacheHitLevel(int hitLat);
	int  getCacheHitLevel();

	/* Dependency checking methods */
	List<instruction*>* getDependents ();
	void setAsDependent(instruction* ins);
	void notifyAllDepICompleted();
	void notifyAllDepICompleted_light();
	void notifyAllAncISquashed();
	void addDep();
	void releaseDep(instruction* ins);
	void releaseDep_light(instruction* ins);
	void squashDep(instruction* ins);
	bool isReady(long int cycle);
	void goToReadyList();
	void setReadyLists(List<instruction*>* rootALUins, List<instruction*>* rootMEMins);
	void delDepTableEntris(dependencyTable *depTables, int coreType, bool perfectRegRen);
	void delDepTableEntris_LRF(dependencyTable *depTables, int coreType, bool perfectRegRen);
	void br_dependencyTable(dependencyTable *depTables);
	void perfect_MemDependencyTable (dependencyTable *depTables, int coreType, int numSideBuffs); 
	void totalOrder_MemDependencyTable (lsq* totalOrderLSQ);
	void storeOrder_MemDependencyTable (instruction* ancestor);
	void noRRdependencyTable (dependencyTable *depTables, int coreType);
	void infRegdependencyTable (dependencyTable *depTables, int coreType);
	void updateDepTableEntris(dependencyTable *depTables, int coreType, instruction* replaceIns);

	/* Register Renaming */
	bool renameRegs(registerRename *GRF, int coreType);
	void completeRegs(registerRename *GRF);
	void squashRenameReg(registerRename *GRF);
	void commitRegs(registerRename *GRF);

	/* Side Buffer methods */
	void goToSideBuff();
	void getOutSideBuff();
	bool isGotoSideBuff();
	void notifyAllDepGoToSideBuff(int sb, INS_ID causeOfSBinsID, int numSideBuffs);
	long int notifyAllDepGetOutSideBuff(int sb, INS_ID causeOfSBinsID, int numSideBuffs);
	int getSideBuffNum();
	INS_ID getCauseOfSBinsID();
	void delMySB(int sb, INS_ID causeOfSBinsID,int numSideBuffs);
	void addAsMySB(int sb, INS_ID causeOfSBinsID, int numSideBuffs);

	//Critical Path
	int findLongestPath(long int cycle, bool UPLDhoisting, long int myPhraseID);
	int findLongestPathDynamicly(long int cycle, bool UPLDhoisting);
	int getLongestPath();
	void lookUpANDsetPathLen(); //ONLY used in FRAGMENT and PHRASE TRACE GEN
	int getMyPathLen();
	int getPathLen();
	bool isOnCritiPath(int candidatePhID);	

	//Register Renaming
	void renameWriteReg(long int reg);
	void renameReadReg(int indx, long int renReg);
	long int getRenamedReg(long int reg);
	bool isRepeated(instruction* temp, List<instruction*>*ancestors);


	//Rescheduling
	void printToFile(FILE *reScheduleFile, bool recordHitMiss);

	//Ancestors
	instruction* getNthAncestor(int i);
	int getNumAncestors();
	void setAsAncestor(instruction* ins);

	//Dynamic Phrase
	instruction* getNthMemRdAncestor(int i);
	int getNthMemRdAncestorID(int i);
	int getNumMemRdAncestors();
	void genAllPhraseAncestorsList(INS_ID causeOfPhraseinsID, instruction* ins);
	void genPhraseAncestorsList(int causeOfPhraseinsID, instruction* ins);
	void findPhraseAncestors();
	void addAsPhraseAncestor(instruction* ins);

	//Phrasing
	phrase* getMyPhrase();
	void setMyPhrase(phrase* ph);
	void setMyPhraseID(int id);
	int getMyPhraseID();
	void addDepPhrase(phrase* ph);
	void notifyMyDepPhrasesICompleted();
	void notifyDepICommited();
	void releaseDepFromUPLD(instruction* ins);

	//Fragmenting
	void setCauseOfFragInsID(INS_ID causeOfFragInsID);
	INS_ID getCauseOfFragInsID();
	void addDepFrag(fragment* fr);
	void notifyMyDepFragsICompleted();
	void setMyFrag(fragment* ph);
	int getMyFragID();
	fragment* getMyFrag();
	

	//Unpredictable Memory Access
	void setMissRate(double missRate);
	double getMissrate();
	bool getDepOnUPLD();
	void setDepOnUPLD();

	//Branches
	void setBrTarget(long int brTarget);
	void setBrSide(long int brTaken);
	void setBrForward ();
	void setBrBias (float brBias);
	void setBrAccuracy (float brBias);
	bool getBrSide();
	void findMissPrediction(bool missPred);
	bool getMissPrediction();
	void setBrMode(brMode branchMode);
	brMode getBrMode();
	float getBrBias();
	float getBrAccuracy();
	//Branch ancestors
	instruction* getNthBrAncestor(int i);
	int getNumBrAncestors();
	void setAsBrAncestor (instruction* ins);
	void setAsBrDependent(instruction* ins);
	void releaseBrDep    (instruction* ins);
	void squashBrDep	 (instruction* ins);
	void delBrDependent(instruction* ins);
	void delNthBrAncestor(int i);
	void notifyAllBrAncestorsICompleted();
	void releaseBrAncestors(instruction* ins);
	

	//Instruction Map
	void removeFromInsMap();
	void insertToInsMap();
	void setVliwScheduler(vliwScheduler *scheduler);


	//Variables
	int _guardian;   //Those on which this depends on
	List<instruction*>* _dependents; //Those dependent on this
	List<instruction*>* _ancestors; //Those ancestors on this
	List<instruction*>* _phraseAncestors; //Those ancestors on this
	List<int>* _phraseAncestorsID;

	//Branch predictor
	void setPredHistObj(void* bp_hist);
	void* getPredHistObj();
	void setPrediction(bool brPred);
	bool getPrediction();

	//Phraseblock/basicblock scheduling
	bool isBBtail();
	bool isBBhead();
	void setBrHeaderAddr(INS_ADDR brAddr);
	void setBBtail();
	void setBBhead();
	INS_ADDR getBrHeaderAddr();

    private:
	/*
	 * Convention:
	 * _r* = -1: means reg is not assigned a value (fresh)
	 * _r* = -2: means reg is invalid
	 *
	 */

	List<long int> *_r, *_pr;  //Register List & physical write-register list
	List<int> *_rt; //Register Type List
	map<PR,AR> _pTOaRegMap;
	int _numWriteReg;
	ADDRS _memAddr;
	ADDRS _insAddr;
	long int _memAccessSize;
	long int _executeCycle;
	long int _completeCycle;
	long int _memAddrCompCompleteCycle;
	int _latency;
	type _insType;
	status _insStatus;
	long int _fetchEndCycle;
	memType _readORwrite;
	char _command[INS_STRING_SIZE];
	INS_ID _id;
	int _pipeLineLat;
	int _hitLevel;
	List<instruction*>* _rootList;
	vliwScheduler* _scheduler;

	//Data and Resource dependence

	//Side Buffer Params
	bool _inSideBuff;
	List<int> *_mySBnum;
	List<INS_ID> *_causeOfSBinsID;
	int _currentMySBnum;
	INS_ID _currentCauseOfSBinsID;
	INS_ID _delFlag, _addFlag, _findLPflag, _phraseAddFlag;
	map<long int,long int> writeRegRenMap;
	map<long int,long int> readRegRenMap;

	//Dependency Critical Path Length (longest branch)
	int _critPathLen;
	int _pathLen;

	//Phrasing
	int _myPhraseID;
	phrase* _myPhrase;
	List<phrase*> *_depPhrases;
	List<fragment*> *_depFrags;

	//Fragmenting
	INS_ID _causeOfFragInsID;
	int _myFragID;
	fragment* _myFrag;

	//Unpredictable Memory Access
	double _missRate;
	bool _hasUPLDancestor;

	//Branching
	bool _missPred;
	bool _brTaken;
	bool _brForward;
	bool _brPred;
	long int _brTarget;
	float _brBias;
	float _brAccuracy;
	List<instruction*>* _brAncestors;
	List<instruction*>* _brDependents;
	brMode _branchMode;
	void* _bp_hist;

	//Phraseblock/basicblock scheduling
	bool _bbTail;
	bool _bbHead;
	INS_ADDR _brHeaderAddr;
};

#endif
