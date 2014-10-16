#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <algorithm>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string>
#include <list>
#include <set>
#include <iostream>
#include <sstream>

#include "dependencyTable.h"
#include "oooLD_lsq_ctrl.h"
#include "registerRename.h"
#include "vliwScheduler.h"
#include "instruction.h"
#include "../frontend/tournament.hh"
#include "cacheLine.h"
#include "cacheCtrl.h"
#include "quickSort.h"
#include "phraseGen.h"
#include "basicblock.h"
#include "sideBuff.h"
#include "fragment.h"
#include "latency.h"
#include "regFile.h"
#include "utility.h"
#include "parser.h"
#include "../global/global.h"
#include "cache.h"
#include "../lib/list.h"
#include "hist.h"
#include "dot.h"
#include "lsq.h"
#include "bkEnd.h"

time_t start; 
int64_t idleCount = 0;
int64_t crCount = 0;

/***************************************
 * Global Variables
 ***************************************/
//File

core coreType;
FILE* pinFile;
FILE* outFile;
FILE* outFile1;
FILE* outFile2;
FILE* reScheduleFile;
FILE* phraseFile;
FILE* branchProfile;
FILE* branchAccuracy;
FILE* upldProfile;
FILE* wbbSkipCountFile;
FILE* brSkipAccuracyFile;
string inFileName("../trace_regOnly_bzipSTM_default.out");
string reScheduleFileName("../trace_regOnly_bzipSTM_rescheduled_default.out");
string phrasingFileName("../trace_regOnly_bzipSTM_phrased_default.out");
string outFileName1("timingTrace_default_outFile_default.out");
string branchProfileFileName ("/home/milad/esc_project/svn/memTraceMilad/TraceSim/results/hmmer/branch_exe_count_map.csv");
string branchAccuracyFileName("/home/milad/esc_project/svn/memTraceMilad/TraceSim/results/bzip2/predhistory.csv");
string upldProfileFileName("/home/milad/esc_project/svn/memTraceMilad/TraceSim/results/hmmer/unpredictableLDs.csv");
string wbb_skip_count("wbb_skip_count.csv");
string br_skip_accuracy("br_skip_accuracy.csv");
char outFileName2[400];
long int oneLevDeepLatLevel;
int xLevDeepLatLevel;
int invalidInsCount;
bool eoc, eof;
long int corruptInsCount;
bool reportTrace = false;
bool reportTraceAndHitMiss = false; 
g_variable *__g_var;

//Instruction Queues, etc.
List<instruction*> *iResStation;
List<instruction*> *iResStations[NUM_FUNC_UNIT];
List<instruction*> *iMemBuf;
List<instruction*> *iWindow;
List<instruction*> *iROB;
List<basicblock*> *pbROB;
List<char*> *ICQ;
List<instruction*>** _pbLists;
List<int> *SBpriorityList;
sideBuff **iSideBuff;
map<long int,int> inFlightLDops;
map<long int,int> memRdTotCountTable;
map<long int,int> memRdMissCountTable;
map<long int,float> memRdMissRateTable;
map<long int,float> correlationMap;
map<long int,float> branchBiasProfileMap;
map<long int,float> branchAccuracyMap;
map<long int,float> upldMissRateProfileMap;
map<ADDRS,int> br_pred_update_distance;
set<long int> missingAccuracyBranches;
dependencyTable *depTables;
dependencyTable *LRFTables[NUM_PHRASEBLKS];
INS_ID insID;
int ROBsize;
int pbROBsize;
parser* parse;
long int insParseCap = -1;

//Branch Predictor
TournamentBP *predictor = NULL;

//ALU Units Parameters
int numFU = NUM_FUNC_UNIT;
bool aluAvail	[NUM_FUNC_UNIT];
int  aluFreeTime[NUM_FUNC_UNIT];
long int  aluStat    [NUM_FUNC_UNIT];
type aluKind[NUM_FUNC_UNIT];

//Caches & RF
cache *_L1;
cache *_L2;
cache *_L3;
regFile *RF;
int cacheLat[MEM_HIGHERARCHY];

//Register Renamers
registerRename *GRF;

//LSQ
lsq *loadStoreQue;
bool isCacheBusFree = true;

//Misc
extern bool debug;
long int cycle;
int phCycle = 0;
int frCycle = 0;
long int numSideBuffs = NUM_SIDE_BUFFERS;
long int SBlength = SB_SIZE_LIMIT;
bool reschedule = false;
bool makePhrase = false;
float unpredMemOpThreshold = UNPRED_MEM_THRESHOLD;
int phraseSizeBound = MAX_NUM_INS_VISITORS;
int parseHitMiss = 0;
int isSquashed = false;
char* comment;

//Fragment Scheduling
	

//STAT
float ipc;
long int insCount;
long int executeInsCount;
long int completeInsCount;
long int iROBSize = 0;
long int iWinSize = 0;
long int iResStnSize = 0;
long int iResStnsSize[NUM_FUNC_UNIT];
long int pbListsSize[NUM_PHRASEBLKS];
long int iSideBufSize = 0;
long int iMemBuffSize = 0;
long int inFlightLDopsSize = 0;
long int totNumSBactivations = 0;
long int *numSBactivations= new long int [numSideBuffs];
long int totNumSBreactivations = 0;
long int *numSBreactivations= new long int [numSideBuffs];
long int totInsVisitingSBcount = 0;
long int *insVisitingSBcount= new long int [numSideBuffs];
long unsigned totalSBsize = 0;
long unsigned *SBsize= new long unsigned [numSideBuffs];
long int totSBactiveCycles = 0;
long int *SBactiveCycles= new long int [numSideBuffs];
long int totInsCountWhenSBon = 0;
long int *InsCountWhenSBon= new long int [numSideBuffs];
long int totSBoffCycles = 0;
long int numOnSideBuffs = 0;
long int *SBoffCycles= new long int [numSideBuffs];
long int numMemOps = 0;
long int numALUOps = 0;
long int numFPUOps = 0;
long int numBROps = 0;
long int numAssignOps = 0;
long int missPredBROps = 0;
long int missPredBROps_fetch = 0;
long int missPredBROps_NT = 0; //NT: predicted not taken
long int missPredBROps_NT_fetch = 0; //NT: predicted not taken
long int numBrOps_predT = 0; //predicted taken
long int numBrOps_predT_fetch = 0; //predicted taken
long int numReadOps = 0;
long int numWriteOps = 0;
long int numDepInOtherSBs = 0;
long int emptyResStation = 0;
long int stFwdMemOp = 0;
long int nonBlockingMemOp = 0;
long int cacheAccessMemOp = 0;
long int maxSBsize = -1;
long int minSBsize = 100000000000; //A very large num
long int totFrameSize = 0;
long int totMainStreamBound = 0; //only used for static conting. scheduling
long int windowSatration = 0;
List<int> *SBsizeList = new List<int>;
List<void*>* bp_hist_list = new List<void*>;
long int longLatOpWhenSPisON = 0;
long int longLatOpWhenSPisDraining = 0;
long int longLatOpWhenSPisWaiting = 0;
long int unpredMemOpCnt = 0;
long int unpredMemInsCnt = 0;
long int totNumPhrase = 0;
long int totNumFrag   = 0;
long int totNumSoftBound = 0;
long int totNumPhUnpredMemOp = 0;
long int totNumRootIns = 0;
long int totNumPhraseAncestors = 0;
long int totNumPhGenResets = 0;
long int totPhStall = 0;
long int totFrStall = 0;
long int totNumRootPh = 0;
long int totNumCritPathViol = 0;
long int unexpectedMiss = 0;
long int unexpecteedLat = 0;
long int totNumRealFrag = 0;
long int totSizRealFrag = 0;
long int totNumOfSigleFragPhrases = 0;
long int numReadyFrags = 0;
long int interFragStallCycle = 0;
long int evaltMissRtCorrel = 0;
long int activeBuffCnt = 0;
long int bbCount = 0;
long int brInsCount = 0;
long int fetchStallCycle = 0;
long long int br_pred_update_dist = 0;
bool UPLDhoist = false;
bool branchProfileFlag = false;
bool upldProfileFlag = true;
bool perfectRegRen = false;
brMode branchMode = noBrMode; //1: No Br Run, 2: Run Br, 3: Stat Br Pred, 4: Scheduling (code inject) 5: no scheduling accross low bias branches
memModel memoryModel = PERFECT; //NAIVE_SPECUL; //PERFECT; //TOTAL_ORDER;
int inCompleteBBbuffIndx = -1;
long long int lqSize = 0;
long long int sqSize = 0;
long long int rrSize = 0;
long long int lrfCount = 0;
long long int grfCount = 0;
long long int lrfWrCountPerIns=0, grfWrCountPerIns=0;
long long int lrfRdCountPerIns=0, grfRdCountPerIns=0;

//HIST
hist* frSizeHist;
hist* frLatHist;
hist* phCritPathHist;
hist* phSizeHist;
hist* correlationHist;
hist* numChildrenHist;
hist* numParentsHist;

/***************************************
 * Functions
 ***************************************/

FILE *junk;
void bkEnd_init (int argc, char const * argv[], g_variable &g_var) {
	junk=fopen("junk_bzip.txt", "w");
	for (int i = 0; i < numSideBuffs; i++) {
		numSBreactivations[i] = 0;
		insVisitingSBcount[i] = 0;
		numSBactivations[i] = 0;
		InsCountWhenSBon[i] = 0;
		SBactiveCycles[i] = 0;
		SBoffCycles[i] = 0;
		SBsize[i] = 0;
	}
	//Initialize Branch Predictor
    predictor = new TournamentBP(2048, 2, 2048, 11, 8192, 13, 2, 8192, 2, 0);

	//Set cache level latencies
	cacheLat[0] = L1_LATENCY;
	cacheLat[1] = L2_LATENCY;
	cacheLat[2] = L3_LATENCY;
	cacheLat[3] = MEM_LATENCY;

	oneLevDeepLatLevel = cacheLat[0];
	xLevDeepLatLevel   = cacheLat[0];

	//Set core type
	coreType =IN_ORDER ; //;OUT_OF_ORDER;ONE_LEVEL_DEEP_DYN 
	ROBsize = ROB_SIZE;
	pbROBsize = PB_ROB_SIZE;
	
	//Comment
	comment = new char[500];

	//Parse inputs
	int c;
	int temp = -1;
	while ((c = getopt (argc, (char **)argv, "l:o:i:u:f:c:w:s:b:t:m:p:r:h:a:1:2:3:4:n:d:e:v:g:j:k:x:y:z:")) != -1) 
	{   
		switch(c)
		{   
		case 'l':
			xLevDeepLatLevel = atoi(optarg);
			break;
		case 'o':
			outFileName1.assign(optarg);
			break;
		case 'i':
			inFileName.assign(optarg);
			break;
		case 'u':
			reScheduleFileName.assign(optarg);
			break;
		case 'f':
			phrasingFileName.assign(optarg);
			break;
		case 'c':
			coreType = (core) atoi(optarg);
			break;
		case 'w':
			ROBsize = atoi(optarg);
			break;
		case 's':
			numSideBuffs = atoi(optarg);
			break;
		case 'b':
			SBlength = atoi(optarg);
			break;
		case 't':
			temp = atoi(optarg);
			if (temp == 0)	reportTrace = false;
			else		reportTrace = true;
			Assert(temp == 1 || temp == 0);
			break;
		case 'm':
			temp = atoi(optarg);
			if (temp == 0)	reportTraceAndHitMiss = false;
			else		reportTraceAndHitMiss = true;
			Assert(temp == 1 || temp == 0);
			break;
		case 'p':
			temp = atoi(optarg);
			if (temp == 0)	makePhrase = false;
			else		makePhrase = true;
			Assert(temp == 1 || temp == 0);
			break;
		case 'r':
			temp = atoi(optarg);
			if (temp == 0)  reschedule = false;
			else		reschedule = true;
			Assert(temp == 1 || temp == 0);
			break;
		case 'h':
			unpredMemOpThreshold = atof(optarg);
			break;
		case 'a':
			phraseSizeBound = atoi(optarg);
			break;
		case '1':
			cacheLat[0] = atoi(optarg);
			break;
		case '2':
			cacheLat[1] = atoi(optarg);
			break;
		case '3':
			cacheLat[2] = atoi(optarg);
			break;
		case '4':
			cacheLat[3] = atoi(optarg);
			break;
		case 'n': //NOTE: incomplete - only implemented for single issue INO and OOO
			numFU = atoi(optarg);
			break;
		case 'd':
			parseHitMiss = atoi(optarg);
			break;
		case 'e':
			comment = optarg;
			break;
		case 'v':
			evaltMissRtCorrel = atoi(optarg);
			break;
		case 'g':
			temp = atoi(optarg);
			if (temp == 0)  UPLDhoist = false;
			else		UPLDhoist = true;
			break;
		case 'j':
			branchMode = (brMode) atoi(optarg);
			break;
		case 'k':
			insParseCap = atoi(optarg);
			Assert(insParseCap > 0);
			break;
		case 'x':
			wbb_skip_count.assign(optarg);
			break;
		case 'y':
			temp = atoi(optarg);
			if (temp == 0)  branchProfileFlag = false;
			else			branchProfileFlag = true;
			break;
		case 'z':
			branchProfileFileName.assign(optarg);
			break;
		default:
			//Unrecognized input. Terminate the run
			exit(-1);
		};
	}
	//Assert(branchMode >= noBr && branchMode <= scheduleBr);

	//Build the cache
	_L1 = new cache(1, 64, 32768  ); //32KB
	_L2 = new cache(1, 64, 2097152); //2MB
	_L3 = new cache(1, 64, 8388608); //8MB

	//Build functional units status
	for (int i = 0; i < NUM_FUNC_UNIT; i++) {
		aluAvail   [i] = true;
		aluFreeTime[i] = 0;
		aluStat    [i] = 0;
		if (i == 0) {
			aluKind[i] = MEM;
		} else {
			aluKind[i] = ALU;
		}
	}
	
	//Build instruction window (empty right now)
	iROB           = new List<instruction*>;
	pbROB          = new List<basicblock*>;
	ICQ            = new List<char*>;
	iWindow        = new List<instruction*>;
	iMemBuf        = new List<instruction*>;
	iResStation    = new List<instruction*>;
	SBpriorityList = new List<int>;
	iSideBuff   = new sideBuff* [NUM_SIDE_BUFFERS];
	for (int i = 0; i < NUM_FUNC_UNIT; i++)
		iResStations[i] = new List<instruction*>;
	for (int i = 0; i < NUM_SIDE_BUFFERS; i++)
		iSideBuff[i] = new sideBuff;
	for (int i = 0; i < NUM_FUNC_UNIT; i++)
		iResStnsSize[i] = 0;
	for (int i = 0; i < NUM_PHRASEBLKS; i++)
		pbListsSize[i] = 0;

	//Open files
	sprintf(outFileName2,"%sSum",outFileName1.c_str());
	if((pinFile=fopen(inFileName.c_str(), "r")) == NULL || 
	   (outFile1=fopen(outFileName1.c_str(), "w+")) == NULL) {// ||
	   //(outFile2=fopen(outFileName2, "w+")) == NULL) {
	    printf("1-ERROR: Cannot open file(s).\n");
	    exit(1);
	}
	if(//reschedule == true &&
	   (reScheduleFile=fopen(reScheduleFileName.c_str(), "w")) == NULL) {
	    printf("2-ERROR: Cannot open file(s).\n");
	    exit(1);
	}
	if (makePhrase == true &&
	   (phraseFile=fopen(phrasingFileName.c_str(), "w")) == NULL) {
	    printf("3-ERROR: Cannot open file(s).\n");
	    exit(1);
	}

	if (branchProfileFlag == true &&
	    ((branchProfile=fopen(branchProfileFileName.c_str(), "r")) == NULL ||
	     (branchAccuracy=fopen(branchAccuracyFileName.c_str(), "r")) == NULL)) {
	    printf("4-ERROR: Cannot open file(s).\n");
	    exit(1);
	} else if (branchProfileFlag == true) {
		int total_count, taken_count, bbl_size;
		long int ins_addr, dst_addr;
		float taken_rate;
		while(fscanf(branchProfile, "%ld, %d, %d, %d, %f, %ld\n", &ins_addr, &bbl_size, &total_count, &taken_count, &taken_rate, &dst_addr) != EOF) {
		//while(fscanf(branchProfile, "%ld, %d, %d, %f\n", &ins_addr, &total_count, &taken_count, &taken_rate) != EOF) {
			branchBiasProfileMap[ins_addr] = taken_rate;
		}
		while(fscanf(branchAccuracy, "%ld, %f\n", &ins_addr, &taken_rate) != EOF) {
			branchAccuracyMap[ins_addr] = taken_rate;
		}
	}


	if (upldProfileFlag == true &&
	   (upldProfile=fopen(upldProfileFileName.c_str(), "r")) == NULL) {
	    printf("4-ERROR: Cannot open file(s).\n");
	    exit(1);
	} else if (upldProfileFlag == true) {
		long int ins_addr;
		float miss_rate;
		while(fscanf(upldProfile, "(%ld, %f)\n", &ins_addr, &miss_rate) != EOF) {
			upldMissRateProfileMap[ins_addr] = miss_rate;
		}
	}
	
	if ((wbbSkipCountFile=fopen(wbb_skip_count.c_str(), "w")) == NULL) {
	    printf("3-ERROR: Cannot open file(s).\n");
	    exit(1);
	}
	if ((brSkipAccuracyFile=fopen(br_skip_accuracy.c_str(), "w")) == NULL) {
	    printf("3-ERROR: Cannot open file(s).\n");
	    exit(1);
	}

	//Debug
	debug = false;

	//Invalid instruction counter
	invalidInsCount = 0;

	//EOC/EOF flags
	eoc = false;
	eof = false;

	corruptInsCount = 0;

	//generates the instruction Id
	insID = 0;
	cycle = 0;
	insCount = 0;
	completeInsCount = 0;
	executeInsCount = 0;

	//Histogram
	if (coreType == FRAGMENT || coreType == FRAGMENT2) {
		frSizeHist = new hist(phraseSizeBound+1, 0, phraseSizeBound+1);
		frLatHist = new hist(750, 0, 750);
	} else if (coreType == PHRASE) {
		//phCritPathHist = new hist(cacheLat[0]*phraseSizeBound, 0, cacheLat[0]*phraseSizeBound);
		//phSizeHist = new hist(phraseSizeBound+1, 0, phraseSizeBound+1);
	}
	numChildrenHist = new hist(ROBsize, 0, ROBsize);
	numParentsHist  = new hist(ROBsize, 0, ROBsize);

	//Parser
	parse = new parser;

	//Register Configuration
	if (perfectRegRen == true) {
		//Build x86 register file
		RF = new regFile;
		//Setup Dependency Table Obj for REG, MRM & BR
		depTables = new dependencyTable;
	} else { //Build an actual RF system
		switch (coreType) {
			case IN_ORDER:
				//Setup Dependency Table Obj for REG, MRM & BR
				depTables = new dependencyTable;
				break;
			case OUT_OF_ORDER:
				//Setup RR Obj for all reg
				GRF = new registerRename(LARF_LO,GARF_HI);
				//Setup Dependency Table Obj for MRM & BR
				depTables = new dependencyTable;
				break;
			case PHRASEBLOCK:
				//Setup RR Obj for global reg
				GRF = new registerRename;
				//GRF = new registerRename(LARF_LO,GARF_HI); /* for no LRF */
				//Setup Dependency Table Obj for local reg
				for (int i = 0; i < NUM_PHRASEBLKS; i++) {
					LRFTables[i] = new dependencyTable;
				}
				//Setup Dependency Table Obj for MRM & BR
				depTables = new dependencyTable;
				break;
			default: 
				Assert(true == false && "Register allocation regulaion is not defined for this core.");
		};
	}
	
	//LSQ Configuration
	if (memoryModel == PERFECT) {
		;//nothing to be done
	} else if (memoryModel == TOTAL_ORDER || memoryModel == NAIVE_SPECUL) {
		loadStoreQue = new lsq;
	}

	//INO is only valid when perfectRegRen is disabled
	if (coreType == IN_ORDER)
		Assert(perfectRegRen == false && "INO and perfectRegRen produce meaningless results");
	
	//Initialize the frontend variables
	__g_var = &g_var;

	//phraseblock initialization
	if (coreType == PHRASEBLOCK) {
		_pbLists = new List<instruction*>* [NUM_PHRASEBLKS];
		for (int i = 0; i < NUM_PHRASEBLKS; i++) {
			_pbLists[i] = new List<instruction*>;
		}
	}
}

/* -------------------------- */
/* Branch Predictor Functions */
void Update(uint64_t __pc, bool __taken, instruction* ins) {
    bool taken = __taken;
    uint64_t pc = __pc;
    void *bp_hist = ins->getPredHistObj();
	Assert(bp_hist != NULL);
    if(ins->getPrediction() != taken) {
        predictor->update(pc, taken, bp_hist, true);
    } else {
        predictor->update(pc, taken, bp_hist, false);
    }
}

bool PrePredict(uint64_t __pc, void* &bp_hist) {
    uint64_t pc = __pc;
    bool pred = predictor->lookup(pc, bp_hist);
	return pred;
}

bool isMisPrePredicted(bool bbPrediction, bool taken) {
    if(bbPrediction != taken) {
		return true; //mis-prediction
    } else {
		return false; //correct-prediction
    }
}

bool Predict(uint64_t __pc, void* &bp_hist) {
    uint64_t pc = __pc;
    return predictor->lookup(pc, bp_hist);
}

bool PredictAndUpdate(uint64_t __pc, int __taken) {
    bool taken = __taken;
    uint64_t pc = __pc;
    void *bp_hist = NULL;
    bool prediction = predictor->lookup(pc, bp_hist);
    //outfile << hex << "0x" << pc << ": ";
    if(prediction != taken) {
        //outfile << "wrong prediction\n";
		//if (brExeCount.find(pc) != brExeCount.end())	brExeCount[pc] += 1;
		//else											brExeCount[pc] = 1;
        predictor->update(pc, taken, bp_hist, true);
		return true; //mis-prediction
    } else {
        //outfile << "correct prediction\n";
		//if (correctPred.find(pc) != correctPred.end())	correctPred[pc] += 1;
		//else											correctPred[pc] = 1;
		//if (brExeCount.find(pc) != brExeCount.end())	brExeCount[pc] += 1;
		//else											brExeCount[pc] = 1;
        predictor->update(pc, taken, bp_hist, false);
		return false; //correct-prediction
    }
	//return prediction;
}
/* -------------------------- */

void freeALUs (int cycle) {
	for (int i = 0; i < NUM_FUNC_UNIT; i++) {
		if (aluAvail[i] == false) {
			aluStat[i]++;
		}
		if (aluAvail[i] == false && aluFreeTime[i] <= cycle) {
			aluAvail[i] = true; 
			//aluFreeTime[i] = 0;
		}
	}
}

void completeIns (int cycle,List<instruction*>* iROB) {
	//printf("\n");
	for (int i = 0; i < iROB->NumElements(); i++) {
		//long int x = ((iROB->Nth(i))->getStatus() == execute ? iROB->Nth(i)->getCompleteCycle() : -1);
		//printf("%llu, %llu, %d, %ld, %d (%s)\n", iROB->Nth(i)->getInsAddr(), iROB->Nth(i)->getInsID(), iROB->Nth(i)->getStatus(), x, cycle, iROB->Nth(i)->getCmdStr());
		if ((iROB->Nth(i))->getStatus() == execute && 
		    (iROB->Nth(i))->getCompleteCycle() <= cycle) {
			//if (debug) printf("just commited something\n");
			(iROB->Nth(i))->setStatus(complete, -1, -1);
			//Update the load buffer
			if (iROB->Nth(i)->getMemType() == READ) {
				long int insAddr = iROB->Nth(i)->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
				if (inFlightLDops.count(insAddr) > 0) {//Non-blocking Load Op
					inFlightLDops.erase(insAddr);
				}
			}
			//Release dependencies
			/*-----STAT-----*/
			//TODO put this back
			//numChildrenHist->addElem(iROB->Nth(i)->_dependents->NumElements());
			completeInsCount++;
			/*-----STAT-----*/
			iROB->Nth(i)->notifyAllBrAncestorsICompleted();
			iROB->Nth(i)->notifyAllDepICompleted();
			iROB->Nth(i)->delDepTableEntris(depTables, coreType, perfectRegRen);
			if (coreType == PHRASEBLOCK && perfectRegRen == false) {
				iROB->Nth(i)->completeRegs(GRF);
				for (int k = 0; k < NUM_PHRASEBLKS; k++) {
					iROB->Nth(i)->delDepTableEntris_LRF(LRFTables[k], coreType, perfectRegRen);
				}
			} else if (coreType == OUT_OF_ORDER && perfectRegRen == false) {
				iROB->Nth(i)->completeRegs(GRF);
			} else if (coreType == FRAGMENT || coreType == FRAGMENT2) {
				iROB->Nth(i)->notifyMyDepFragsICompleted();
			}
			//1LD support
			//if (iROB->Nth(i)->isGotoSideBuff() == true && 
			//    iROB->Nth(i)->getCauseOfSBinsID() == iROB->Nth(i)->getInsID()) { //TODO && oneLevelDeepEnable == true
			//	numDepInOtherSBs += iROB->Nth(i)->notifyAllDepGetOutSideBuff(iROB->Nth(i)->getSideBuffNum(), iROB->Nth(i)->getInsID(),numSideBuffs);
			//	/*-----STAT-----*/
			//	int currentSBsize = iSideBuff[iROB->Nth(i)->getSideBuffNum()]->NumElements();
			//	SBsizeList->Append(currentSBsize);
			//	Assert(currentSBsize >= 0);
			//	if (maxSBsize < currentSBsize) maxSBsize = currentSBsize;
			//	if (minSBsize > currentSBsize) minSBsize = currentSBsize;
			//	totalSBsize += currentSBsize;
			//	SBsize[iROB->Nth(i)->getSideBuffNum()] += iSideBuff[iROB->Nth(i)->getSideBuffNum()]->NumElements();
			//	/*-----STAT-----*/
			//}
		}
	}
}

void completePB (int cycle) {
	for (int j = 0; j < pbROB->NumElements(); j++) {
		basicblock* bb = pbROB->Nth(j);
		for (int i = 0; i < bb->getNumBBIns_o(); i++) {
			instruction * ins = bb->getNthBBIns_o(i);
			if (ins->getStatus() == execute && 
			    ins->getCompleteCycle() <= cycle) {
				ins->setStatus(complete, -1, -1);
				//Update the load buffer
				if (ins->getMemType() == READ) {
					long int insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
					if (inFlightLDops.count(insAddr) > 0) {//Non-blocking Load Op
						inFlightLDops.erase(insAddr);
					}
				}
				//Release dependencies
				/*-----STAT-----*/
				completeInsCount++;
				/*-----STAT-----*/
				ins->notifyAllBrAncestorsICompleted();
				ins->notifyAllDepICompleted();
				ins->delDepTableEntris(depTables, coreType, perfectRegRen);
				if (coreType == PHRASEBLOCK && perfectRegRen == false) {
					ins->completeRegs(GRF);
					for (int k = 0; k < NUM_PHRASEBLKS; k++) {
						ins->delDepTableEntris_LRF(LRFTables[k], coreType, perfectRegRen);
					}
				} else if (coreType == OUT_OF_ORDER && perfectRegRen == false) {
					ins->completeRegs(GRF);
				} else if (coreType == FRAGMENT || coreType == FRAGMENT2) {
					ins->notifyMyDepFragsICompleted();
				}
			}
		}
	}
}

void removePhrase(List<phrase*>* iPh, int phIndx) {
	//printf("%d\n", iPh->Nth(phIndx)->getPhraseID());
	delete iPh->Nth(phIndx);
	iPh->RemoveAt(phIndx);
}

void removeFrag(List<fragment*>* iFr, int frIndx) {
	delete iFr->Nth(frIndx);
	iFr->RemoveAt(frIndx);
}

void findReadyFrag(List<fragment*>* iFr_ready, List<fragment*>* iFr_wait) {
	List<int>* delInsList = new List<int>;
	for (int i = 0; i < iFr_wait->NumElements(); i++) {
		if (iFr_wait->Nth(i)->isReady() == true) {
			iFr_ready->Append(iFr_wait->Nth(i));
			delInsList->Append(i);
		}
	}
	for (int i = delInsList->NumElements()-1; i >= 0; i--) {
		//TODO confirm that delInsList is sorted
		iFr_wait->RemoveAt(delInsList->Nth(i));
	}
	delete delInsList;
}

void reportInsTiming (instruction* ins) {
	/*for fragment scheduling*/
	if (coreType == PHRASE || coreType == FRAGMENT || coreType == FRAGMENT2) {
		static int phID = -1;
		if (ins->getMyPhraseID() > phID) {
			phID = ins->getMyPhraseID();
			fprintf(outFile1, "------------------------------------PHRASE: %d\n", phID);
		}
		static int frID = -1;
		if (ins->getMyFragID() != frID) {
			frID = ins->getMyFragID();
			fprintf(outFile1, "----------------FR: %d\n", frID);
		}
		if (ins->getMissrate() > unpredMemOpThreshold) {
			fprintf(outFile1, "UPLD,  ");
		} else if (ins->getCacheHitLevel() > 1) {
			fprintf(outFile1, "UNEXP, ");
		}
		fprintf(outFile1, "%ld, %ld, %d,(%d, %d)\t\t%lld(%s)", 
			//cycle,
			ins->getExecuteCycle(),
			ins->getCompleteCycle(),
			ins->getLatency(),
			//ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET),
			//ins->getMemAddr(),
			ins->getMyPhraseID(), //only for FRAGMENT & PHRASE
			ins->getMyFragID(),   //only for FRAGMENT & PHRASE
			ins->getInsID(),
			ins->getCmdStr());
	}
	/*-----------------------*/
	else {
		fprintf(outFile1, "%ld, %ld,(%d, %d)\t\t%lld(%s)", 
			//cycle,
			ins->getExecuteCycle(),
			ins->getCompleteCycle(),
			ins->getLatency(),
			//ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET),
			//ins->getMemAddr(),
			ins->getMyFragID(),   //only for FRAGMENT & PHRASE
			ins->getInsID(),
			ins->getCmdStr());

	}
}

void printSTALL (int cycle) {
	fprintf(outFile1, "%d\n", cycle);
}

void createTraceAndHitMiss (int i) {
	iROB->Nth(i)->printToFile(reScheduleFile, true);
}

void addToPBCache() {
	if (__g_var->g_BBlist_indx >= 19) {eof = true; return;}
	__g_var->g_BBlist_indx++;
	pbROB->Append(__g_var->g_BBlist->Nth(__g_var->g_BBlist_indx));
}

void addToInsCache(int cycle) {
	//Read a number of lines from trace file
	int insCacheSiz = ROBsize*3;
	static int insList_indx = 0;
	Assert(__g_var->g_insList->NumElements() > insCacheSiz);
	while(ICQ->NumElements() < insCacheSiz) {
		if (insList_indx == __g_var->g_insList->NumElements() - 10) {
			eof = true; //EOF
			//if (debug) printf("breaking from addToInsCache ((%d,%d),%d)\n", insList_indx, insCacheSiz, __g_var->g_insList->NumElements());
			__g_var->g_insList_indx = insList_indx;
			insList_indx = 0;
			break;
		} else {
			char *c = new char[INS_STRING_SIZE];
			Assert(__g_var->g_insList->NumElements() > insList_indx);
			strcpy(c, (__g_var->g_insList->Nth(insList_indx))->c_str());
			insList_indx++;
			ICQ->Append(c);
			//if (debug) printf("\tadding instruction (%d, %d) (%s)\n", ICQ->NumElements(), iROB->NumElements(), (__g_var->g_insList->Nth(insList_indx))->c_str());
		}
	}
}

void updateBBboundaries(instruction *ins) {
	//if (ins->getInsAddr() == 245515768464) printf("is header: %d\n", ins->isBBhead());
	if (ins->isBBhead() && ins->isBBtail()) {
		INS_ADDR brAddr;
		brAddr = ins->getBrHeaderAddr();
		char *c1 = new char[INS_STRING_SIZE];
		char *c2 = new char[INS_STRING_SIZE];
		char *c3 = new char[INS_STRING_SIZE];
		char *c4 = new char[INS_STRING_SIZE];
		if (strcmp(ICQ->Nth(0),"{") != 0 && strcmp(ICQ->Nth(0),"{\n") != 0) {
			char *c = new char[INS_STRING_SIZE];
			strcpy(c, "{");
			ICQ->InsertAt(c,0);
			//printf("%s\n",ICQ->Nth(0));
		}
		strcpy(c4, "}");
		//printf("%s\n",c3);
		ICQ->InsertAt(c4,0);
		std::stringstream ss;
		strcpy(c3,ins->getCmdStr());
		//printf("%s\n",c3);
		ICQ->InsertAt(c3,0);
		//printf("%s",ICQ->Nth(0));
		ss << "H," << brAddr << "\n";
		string s = ss.str();
		strcpy(c2,s.c_str());
		//printf("%s",c2);
		ICQ->InsertAt(c2,0);
		//printf("%s\n",ICQ->Nth(0));
		strcpy(c1, "{");
		//printf("%s\n",c1);
		ICQ->InsertAt(c1,0);
		//printf("%s\n",ICQ->Nth(0));
	} else if (ins->isBBhead()) {
		//if (ins->getInsAddr() == 245515768464) printf("is header: %d, %llu\n", ins->isBBhead(),ins->getBrHeaderAddr());
		INS_ADDR brAddr;
		brAddr = ins->getBrHeaderAddr();
		char *c1 = new char[INS_STRING_SIZE];
		char *c2 = new char[INS_STRING_SIZE];
		char *c3 = new char[INS_STRING_SIZE];
		std::stringstream ss;
		strcpy(c3,ins->getCmdStr());
		//printf("%s\n",c3);
		ICQ->InsertAt(c3,0);
		//printf("%s",ICQ->Nth(0));
		ss << "H," << brAddr << "\n";
		string s = ss.str();
		strcpy(c2,s.c_str());
		//printf("%s",c2);
		ICQ->InsertAt(c2,0);
		//printf("%s\n",ICQ->Nth(0));
		strcpy(c1, "{");
		//printf("%s\n",c1);
		ICQ->InsertAt(c1,0);
		//printf("%s\n",ICQ->Nth(0));
	} else if (ins->isBBtail()) {
		char *c1 = new char[INS_STRING_SIZE];
		char *c2 = new char[INS_STRING_SIZE];
		if (strcmp(ICQ->Nth(0),"{") != 0 && strcmp(ICQ->Nth(0),"{\n") != 0) {
			char *c = new char[INS_STRING_SIZE];
			strcpy(c, "{");
			//printf("%s\n",c);
			ICQ->InsertAt(c,0);
			//printf("%s\n",ICQ->Nth(0));
		}
		strcpy(c2, "}");
		//printf("%s\n",c2);
		ICQ->InsertAt(c2,0);
		//printf("%s\n",ICQ->Nth(0));
		strcpy(c1,ins->getCmdStr());
		//printf("%s\n",c1);
		ICQ->InsertAt(c1,0);
		//printf("%s",ICQ->Nth(0));
	} else {
		char *c = new char[INS_STRING_SIZE];
		strcpy(c,ins->getCmdStr());
		//printf("%s\n",c);
		ICQ->InsertAt(c,0);
	}
}

void addLastBracket() {
	if (strcmp(ICQ->Nth(0), "{") == 0 || strcmp(ICQ->Nth(0),"{\n") == 0) return;
	char *c = new char[INS_STRING_SIZE];
	strcpy(c, "{");
	//printf("-%s\n",c);
	ICQ->InsertAt(c,0);
}

long long int squashInsCount=0;
void removeFromROB (List<instruction*> *iROB, INS_ID insId) {
	//printf("rob size: %d\n",iROB->NumElements());
	for (int i = iROB->NumElements()-1; i >= 0; i--) {
		squashInsCount++;
		if (coreType == PHRASEBLOCK) {
			updateBBboundaries(iROB->Nth(i));
		} else {
			char *c = new char[INS_STRING_SIZE];
			strcpy(c, iROB->Nth(i)->getCmdStr());
			ICQ->InsertAt(c,0);
		}
		if (insId == iROB->Nth(i)->getInsID()) {
			//insID = iROB->Nth(i)->getInsID()-1; //update global insID
			delete iROB->Nth(i);
			iROB->RemoveAt(i);
			return;
		}
		delete iROB->Nth(i);
		iROB->RemoveAt(i);
	}
	Assert(false == true && "Didn't find the instruction in ROB");
}

long long int squashWinInsCount=0;
long long int squashPhrInsCount=0;
void removeFromiWindow (INS_ID insID) {
	//printf("wind size: %d\n",iWindow->NumElements());
	for (int i = iWindow->NumElements()-1; i >= 0; i--) {
		if (insID <= iWindow->Nth(i)->getInsID()) {
			squashWinInsCount++;
			iWindow->RemoveAt(i);
		}
	}
	if (coreType == PHRASEBLOCK) {
		for (int j = 0; j < NUM_PHRASEBLKS; j++) {
			List<instruction*>* pbList = _pbLists[j];
			for (int i = pbList->NumElements()-1; i >= 0; i--) { 
				if (insID <= pbList->Nth(i)->getInsID()) {
					squashPhrInsCount++;
					pbList->RemoveAt(i);
				}
			}
		}
	}
	//printf("wind size: %d\n",iWindow->NumElements());
	//Assert(false == true && "Didn't find the instruction in iWindow");
}

long long int squashRSinsCount=0;
void removeFromiResStn (INS_ID insID) {
	//printf("resStn size: %d\n",iResStation->NumElements());
	for (int i = iResStation->NumElements()-1; i >= 0; i--) {
		if (insID <= iResStation->Nth(i)->getInsID()) {
			squashInsCount++;
			iResStation->RemoveAt(i);
		}
	}
	for (int i = 0; i < NUM_FUNC_UNIT; i++) {
		for (int j = iResStations[i]->NumElements()-1; j >= 0; j--) {
			if (insID <= iResStations[i]->Nth(j)->getInsID()) {
				squashInsCount++;
				iResStations[i]->RemoveAt(j);
			}
		}
	}
}

/*
void xfer_SqQ_to_ICQ(instruction *ins) {
	INS_ID insID = ins->getInsID();
	//Squash SqQ and Update ICQ
	for (int i = SqQ->NumElements()-1; i >= 0; i--) {
		ICQ->InsertAt(SqQ->Nth(i),0);
		if (insID == SqQ->Nth(i)->getInsID()) {
			SqQ->RemoveAt(i);
			return;
		}
		SqQ->RemoveAt(i);
	}
	Assert(false == true && "Didn't find the instruction in Squash Que");
}

bool popFromSquashQue() {
	if (SqQ->NumElements() == 0) return true;
	delete SqQ->Nth(0);
	SqQ->RemoveAt(0);
	return false;
}
*/
bool popFromInsCache() {
	if (ICQ->NumElements() == 0) return true;
	delete ICQ->Nth(0);
	ICQ->RemoveAt(0);
	return false;
}

long long int num_squashed_lrf_wr_fetch_reg=0;
long long int num_squashed_lrf_rd_fetch_reg=0;
long long int num_squashed_lrf_wr_ready_reg=0;
long long int num_squashed_lrf_rd_ready_reg=0;
long long int num_squashed_lrf_wr_execute_reg=0;
long long int num_squashed_lrf_rd_execute_reg=0;
long long int num_squashed_lrf_wr_complete_reg=0;
long long int num_squashed_lrf_rd_complete_reg=0;
long long int num_squashed_grf_wr_fetch_reg=0;
long long int num_squashed_grf_rd_fetch_reg=0;
long long int num_squashed_grf_wr_ready_reg=0;
long long int num_squashed_grf_rd_ready_reg=0;
long long int num_squashed_grf_wr_execute_reg=0;
long long int num_squashed_grf_rd_execute_reg=0;
long long int num_squashed_grf_wr_complete_reg=0;
long long int num_squashed_grf_rd_complete_reg=0;
void getSquashRFstatForStatus(instruction *ins, status st) {
	if (coreType == PHRASEBLOCK) {
		for (int i = 0; i < ins->getNumReg(); i++) {
			if (ins->getNthReg(i) >= LARF_LO && ins->getNthReg(i) <= LARF_HI) {
				switch(st) {
					case FETCH:
						if (ins->getNthRegType(i) == WRITE) num_squashed_lrf_wr_fetch_reg++;
						else num_squashed_lrf_rd_fetch_reg++;
						break;
					case ready:
						if (ins->getNthRegType(i) == WRITE) num_squashed_lrf_wr_ready_reg++;
						else num_squashed_lrf_rd_ready_reg++;
						break;
					case execute:
						if (ins->getNthRegType(i) == WRITE) num_squashed_lrf_wr_execute_reg++;
						else num_squashed_lrf_rd_execute_reg++;
						break;
					case complete:
						if (ins->getNthRegType(i) == WRITE) num_squashed_lrf_wr_complete_reg++;
						else num_squashed_lrf_rd_complete_reg++;
						break;
					default:
						Assert(true == false && "Ins Status Not Recognized");
				}
			} else if (ins->getNthReg(i) >= GARF_LO && ins->getNthReg(i) <= GARF_HI) {
				switch(st) {
					case FETCH:
						if (ins->getNthRegType(i) == WRITE) num_squashed_grf_wr_fetch_reg++;
						else num_squashed_grf_rd_fetch_reg++;
						break;
					case ready:
						if (ins->getNthRegType(i) == WRITE) num_squashed_grf_wr_ready_reg++;
						else num_squashed_grf_rd_ready_reg++;
						break;
					case execute:
						if (ins->getNthRegType(i) == WRITE) num_squashed_grf_wr_execute_reg++;
						else num_squashed_grf_rd_execute_reg++;
						break;
					case complete:
						if (ins->getNthRegType(i) == WRITE) num_squashed_grf_wr_complete_reg++;
						else num_squashed_grf_rd_complete_reg++;
						break;
					default:
						Assert(true == false && "Ins Status Not Recognized");
				}
			} else {
				Assert(true == false && "invalid register value");
			}
		}
	} else if (coreType == OUT_OF_ORDER) {
		for (int i = 0; i < ins->getNumReg(); i++) {
			switch(st) {
				case FETCH:
					if (ins->getNthRegType(i) == WRITE) num_squashed_grf_wr_fetch_reg++;
					else num_squashed_grf_rd_fetch_reg++;
					break;
				case ready:
					if (ins->getNthRegType(i) == WRITE) num_squashed_grf_wr_ready_reg++;
					else num_squashed_grf_rd_ready_reg++;
					break;
				case execute:
					if (ins->getNthRegType(i) == WRITE) num_squashed_grf_wr_execute_reg++;
					else num_squashed_grf_rd_execute_reg++;
					break;
				case complete:
					if (ins->getNthRegType(i) == WRITE) num_squashed_grf_wr_complete_reg++;
					else num_squashed_grf_rd_complete_reg++;
					break;
				default:
					Assert(true == false && "Ins Status Not Recognized");
			}
		}
	} else if (coreType == IN_ORDER) {
		for (int i = 0; i < ins->getNumReg(); i++) {
			switch(st) {
				case FETCH:
					if (ins->getNthRegType(i) == WRITE) num_squashed_lrf_wr_fetch_reg++;
					else num_squashed_lrf_rd_fetch_reg++;
					break;
				case ready:
					if (ins->getNthRegType(i) == WRITE) num_squashed_lrf_wr_ready_reg++;
					else num_squashed_lrf_rd_ready_reg++;
					break;
				case execute:
					if (ins->getNthRegType(i) == WRITE) num_squashed_lrf_wr_execute_reg++;
					else num_squashed_lrf_rd_execute_reg++;
					break;
				case complete:
					if (ins->getNthRegType(i) == WRITE) num_squashed_lrf_wr_complete_reg++;
					else num_squashed_lrf_rd_complete_reg++;
					break;
				default:
					Assert(true == false && "Ins Status Not Recognized");
			}
		}
	} else {
		Assert(true == false && "unsupported core type");
	}
}

long long int squashRegRenCount=0;
long long int squashLRFcount=0;
long long int num_squashed_grf_rd_reg = 0;
long long int num_squashed_grf_wr_reg = 0;
long long int num_squashed_lrf_rd_reg = 0;
long long int num_squashed_lrf_wr_reg = 0;
void getSquashRFstat(instruction* ins) {
	//int lrfCountPerIns=0, grfCountPerIns=0;
	for (int i = 0; i < ins->getNumReg(); i++) {
		if (coreType == PHRASEBLOCK) {
			if (ins->getNthReg(i) >= LARF_LO && ins->getNthReg(i) <= LARF_HI) {
				lrfCount++;
				if (ins->getNthRegType(i) == WRITE) {
					num_squashed_lrf_wr_reg++;
					squashLRFcount++;
				} else num_squashed_lrf_rd_reg++;
			} else if (ins->getNthReg(i) >= GARF_LO && ins->getNthReg(i) <= GARF_HI) {
				grfCount++;
				if (ins->getNthRegType(i) == WRITE) {
					num_squashed_grf_wr_reg++;
					squashRegRenCount++;
				} else num_squashed_grf_rd_reg++;
			} else {
				Assert(true == false && "invalid register value");
			}
		} else if (coreType == OUT_OF_ORDER) {
			Assert (ins->getNthReg(i) >= LARF_LO && ins->getNthReg(i) <= GARF_HI);
			if (ins->getNthRegType(i) == WRITE) {
				num_squashed_grf_wr_reg++;
				squashRegRenCount++;
			} else num_squashed_grf_rd_reg++;
		} else if (coreType == IN_ORDER) {
			Assert (ins->getNthReg(i) >= LARF_LO && ins->getNthReg(i) <= GARF_HI);
			if (ins->getNthRegType(i) == WRITE) {
				num_squashed_lrf_wr_reg++;
				squashLRFcount++;
			} else num_squashed_lrf_rd_reg++;
		} else {
			Assert(true == false && "unsupported core type");
		}
	}
}


long long int squashSQinsCount=0;
long long int squashLQinsCount=0;
long long int squashBRinsCount=0;
void squash (List<instruction*> *iROB, INS_ID insID, lsq *oooLd_inoSt_LSQ) {
	for (int i = iROB->NumElements()-1; i >= 0; i--) {
		//Remove Ancesotor Dependencies
		iROB->Nth(i)->notifyAllAncISquashed();
		//Remove Dependency Table Elements (BR,MEM,REG)
		iROB->Nth(i)->delDepTableEntris(depTables, coreType, perfectRegRen);
		//Reset Register Renaming Table
		if (!perfectRegRen && coreType != IN_ORDER) {
			iROB->Nth(i)->squashRenameReg(GRF);
		}
		if (coreType == PHRASEBLOCK) {
			for (int k = 0; k < NUM_PHRASEBLKS; k++) {
				iROB->Nth(i)->delDepTableEntris_LRF(LRFTables[k], coreType, perfectRegRen);
			}
		}
		getSquashRFstatForStatus(iROB->Nth(i), iROB->Nth(i)->getStatus());
		getSquashRFstat(iROB->Nth(i));
		if (iROB->Nth(i)->getType() == BR) squashBRinsCount++;
		if (insID == iROB->Nth(i)->getInsID()) break;
		//ATTENTION: Don't add code here
	}
	//Flush LSQ
	if (memoryModel == NAIVE_SPECUL) {
		squashLQinsCount += oooLD_squashLQ (loadStoreQue, insID);
		squashSQinsCount += oooLD_squashSQ (loadStoreQue, insID);
	}
	//Reset the ICQ Ins Pointer &
	//Remove Instruction from ROB
	removeFromiWindow(insID);
	removeFromiResStn (insID);
	removeFromROB (iROB, insID);
}

void getRFstat(instruction* ins) {
	for (int i = 0; i < ins->getNumReg(); i++) {
		if (coreType == PHRASEBLOCK) {
			if (ins->getNthReg(i) >= LARF_LO && ins->getNthReg(i) <= LARF_HI) {
				lrfCount++;
				if (ins->getNthRegType(i) == WRITE) lrfWrCountPerIns++;
				else lrfRdCountPerIns++;
			} else if (ins->getNthReg(i) >= GARF_LO && ins->getNthReg(i) <= GARF_HI) {
				grfCount++;
				if (ins->getNthRegType(i) == WRITE) grfWrCountPerIns++;
				else grfRdCountPerIns++;
			} else {
				Assert(true == false && "invalid register value");
			}
		} else if (coreType == OUT_OF_ORDER) {
			Assert (ins->getNthReg(i) >= LARF_LO && ins->getNthReg(i) <= GARF_HI);
			grfCount++;
			if (ins->getNthRegType(i) == WRITE) grfWrCountPerIns++;
			else grfRdCountPerIns++;
		} else if (coreType == IN_ORDER) {
			Assert (ins->getNthReg(i) >= LARF_LO && ins->getNthReg(i) <= GARF_HI);
			lrfCount++;
			if (ins->getNthRegType(i) == WRITE) lrfWrCountPerIns++;
			else lrfRdCountPerIns++;
		} else {
			Assert(true == false && "unsupported core type");
		}
	}
}

void insTypeCountStat(instruction* ins) {
	type insType = ins->getType();
	switch (insType) {
		case MEM:
			numMemOps++;
			if (ins->getMemType() == READ) numReadOps++;
			else if (ins->getMemType() == WRITE) numWriteOps++;
			else Assert(true == false && "invalid memory type");
			break;
		case ALU:
			numALUOps++;
			break;
		case FPU:
			numFPUOps++;
			break;
		case BR:
			numBROps++;
			if (ins->getPrediction() == true) numBrOps_predT++;
			if (ins->getMissPrediction()==true) {
				missPredBROps++;
				if (ins->getPrediction() == false) missPredBROps_NT++;
			}
			break;
		case ASSIGN:
			numAssignOps++;
			break;
		default:
			Assert(true == false && "invalid instruction type");
	};
}

int squashCount=0;
int commitIns (int cycle, List<instruction*> *iROB) {
	if (iROB->NumElements() > 0) {
		//printf("in commit %s, %d, %d\n",iROB->Nth(0)->getCmdStr(), iROB->Nth(0)->getStatus(), iROB->Nth(0)->getCompleteCycle());
		while(iROB->NumElements() > 0) {
			if ((iROB->Nth(0))->getStatus() == complete) {
				//Report execution timing
				//if (reportTrace == true) reportInsTiming(iROB->Nth(0));
				//if (reportTraceAndHitMiss == true) createTraceAndHitMiss(0); //TODO remove it... moved somewhere else in main.cpp
				
				/* Update BP */
				if (branchMode == dynPredBr && iROB->Nth(0)->getType() == BR) {
					Update((uint64_t)iROB->Nth(0)->getInsAddr(), iROB->Nth(0)->getBrSide(), iROB->Nth(0));
				}
				if (perfectRegRen == false) {
					//Update register rename states
					iROB->Nth(0)->commitRegs(GRF);
				}

				/* Manage LSQ */ 
				if (memoryModel == PERFECT) {
					;//nothing to be done
				} else if (memoryModel == TOTAL_ORDER) {
					if (iROB->Nth(0)->getMemType() == WRITE && !loadStoreQue->isSQempty()) {
						instruction *sqHead = loadStoreQue->getSQhead();
						if (sqHead->getInsID() == iROB->Nth(0)->getInsID()) {
							loadStoreQue->popFrontSQ();
							//TODO this is NOT the place to pop from SQ (fix it)
						}
					}
				} else if (memoryModel == NAIVE_SPECUL) {
					if (iROB->Nth(0)->getMemType() == WRITE) {
						oooLD_updateSQcommitSet(loadStoreQue, iROB->Nth(0));
						INS_ID LDinsID = oooLD_findLQviolation(loadStoreQue, iROB->Nth(0));
						if (LDinsID != 0) {
							//printf("SQUASH\n");
							squashCount++;
							//squash(iROB,LDinsID,loadStoreQue);
							isSquashed = true;
							if (iROB->NumElements() == 0) return 0;
						}
					} else if (iROB->Nth(0)->getMemType() == READ) {
						oooLD_lqDequ (loadStoreQue, iROB->Nth(0));
					}
				}
				/*-----STAT-----*/
				if (!perfectRegRen) getRFstat(iROB->Nth(0));
				insCount++; /*non-speculative ins conut */
				insTypeCountStat(iROB->Nth(0));
				/*-----STAT-----*/
				/* Remove item */
				delete iROB->Nth(0);
				iROB->RemoveAt(0); //Commit
				//if (debug) printf("completed something - Window Size = %d\n", iROB->NumElements());
			} else {
				break;
			}
		}
		return 0;
	} else {
		//printf("The iROB is empty\n");
		return -1;
	}
}

int commitPB(int cycle) {
	if (pbROB->NumElements() > 0) {
		//printf("in commit %s, %d, %d\n",iROB->Nth(0)->getCmdStr(), iROB->Nth(0)->getStatus(), iROB->Nth(0)->getCompleteCycle());
		int numPB = pbROB->NumElements();
		for (int j = 0; j < numPB; j++) {
			basicblock* bb = pbROB->Nth(0);
			if (bb->isBBcomplete()) {
				int numIns = bb->getNumBBIns_o();
				for (int k = 0; k < numIns; k++) {
					instruction* ins = bb->getNthBBIns_o(0);
					//Report execution timing
					//if (reportTrace == true) reportInsTiming(iROB->Nth(0));
					//if (reportTraceAndHitMiss == true) createTraceAndHitMiss(0); //TODO remove it... moved somewhere else in main.cpp
					/* Update BP */
					if (branchMode == dynPredBr && ins->getType() == BR) {
						Update((uint64_t)ins->getInsAddr(), ins->getBrSide(), ins);
					}
					if (perfectRegRen == false) {
						//Update register rename states
						ins->commitRegs(GRF);
					}

					/* Manage LSQ */ 
					if (memoryModel == PERFECT) {
						;//nothing to be done
					//} else if (memoryModel == TOTAL_ORDER) {
					//	if (ins->getMemType() == WRITE && !loadStoreQue->isSQempty()) {
					//		instruction *sqHead = loadStoreQue->getSQhead();
					//		if (sqHead->getInsID() == ins->getInsID()) {
					//			loadStoreQue->popFrontSQ();
					//			//TODO this is NOT the place to pop from SQ (fix it)
					//		}
					//	}
					//} else if (memoryModel == NAIVE_SPECUL) {
					//	if (ins->getMemType() == WRITE) {
					//		oooLD_updateSQcommitSet(loadStoreQue, ins);
					//		INS_ID LDinsID = oooLD_findLQviolation(loadStoreQue, ins);
					//		if (LDinsID != 0) {
					//			//printf("SQUASH\n");
					//			squashCount++;
					//			//squash(iROB,LDinsID,loadStoreQue);
					//			isSquashed = true;
					//			if (pbROB->NumElements() == 0) return 0;
					//		}
					//	} else if (ins->getMemType() == READ) {
					//		oooLD_lqDequ (loadStoreQue, ins);
					//	}
					}
					/*-----STAT-----*/
					if (!perfectRegRen) getRFstat(ins);
					insCount++; /*non-speculative ins conut */
					//insTypeCountStat(ins); TODO put it back after fixing braanches - milad
					/*-----STAT-----*/
					//if (debug) printf("completed something - Window Size = %d\n", pbROB->NumElements());
				}
				delete pbROB->Nth(0);
				pbROB->RemoveAt(0);
			} else {
				break;
			}
		}
		return 0;
	} else {
		printf("The pbROB is empty\n");
		return -1;
	}
}

void resetInput (char *c, int i) {
	//this letter is never used in trace
	c[i] = 'z';
}
/*
bool checkInvalid (char *reg, int size) {
	if (reg[0] == '*' &&
	    reg[1] == 'i' &&
	    reg[2] == 'n' &&
	    size == 9) {
	    invalidInsCount++; //TODO report this number later
	    return false; //register is invalid
	} else if (size == 1) {
		return false; //no register
	}
	return true;
}
*/
long int getReg(char *c) {
	int i = 0, start = 0, end = 0;
	char temp[INS_STRING_SIZE];

	//Parse a x86 resgiter name
	while (c[i]=='z') {i++;}
	if (c[i] == ',') {
		resetInput(c,i);
		i++;
		start = i;
		while (c[i] != '#' && c[i] != '\0' && c[i] != '\n') {
			temp[i] = c[i];
			resetInput(c,i);
			if (debug) printf("temp[i] = %c\n", temp[i]);
			if (c[i] == ',') return -1; //corrupt ins
			i++;
		}
		end = i;
		if (c[i] == '\n' || c[i] == '\0') return -3; //End of line
	}
	int size = end - start;
	char * reg = new char[size+1];
	for (i=0; i < size; i++) {
		reg[i] = temp[i+start];
		if (debug) printf("reg [i] = %c, size = %d", reg[i], size);
	}
	reg[size] = '\0';
	//for (i=0; i < (end-start); i++) {
	//	if (debug) printf("%c", reg[i], size);
	//}
	long int result = atoi(reg); //RF->getRegNum(reg); 
	delete [] reg;
	return result;
}

int getRegType(char *c) {
	int i = 0, start = 0, end = 0;
	char temp[INS_STRING_SIZE];

	//Parse a x86 resgiter name
	while (c[i]=='z') {i++;}
	if (c[i] == '#') {
		resetInput(c,i);
		i++;
		start = i;
		while (c[i] != ',' && c[i] != '\0' && c[i] != '\n') {
			temp[i] = c[i];
			resetInput(c,i);
			//if (debug) printf("temp[i] = %c\n", temp[i]);
			i++;
		}
		if (c[i] == '\n' || c[i] == '\0') return -3; //End of line
		end = i;
	}
	int size = end - start;
	char * reg = new char[size+1];
	for (i=0; i < size; i++) {
		reg[i] = temp[i+start];
		if (debug) printf("reg [i] = %c, size = %d\n", reg[i], size);
	}
	reg[size] = '\0';
	int result = (int) strtol(reg, NULL, 10);
	//if (debug) printf("priority: %c,%c,%c\n", reg[0],reg[1],reg[2]);
	delete [] reg;
	return result;
}

long int getANumber(char *c) {
	int i = 0, start = 0, end = 0;
	char temp[INS_STRING_SIZE];

	//Parse a x86 resgiter name
	while (c[i]=='z') {i++;}
	if (c[i] == ',' || c[i] == ' ') {
		resetInput(c,i);
		i++;
		start = i;
		while (c[i] != ',' && c[i] != '\0' && c[i] != '\n' && c[i] != ')') {
			temp[i] = c[i];
			resetInput(c,i);
			if (debug) printf("temp[i] = %c\n", temp[i]);
			i++;
		}
		//if (c[i] == '\n' || c[i] == '\0') return -3; //End of line (unsigned return)
		end = i;
	}
	int size = end - start;
	char * addr = new char[size+1];
	for (i=0; i < (end-start); i++) {
		addr[i] = temp[i+start];
		if (addr[i] < 48 || addr[i] > 57) {
			delete [] addr;
			return 0; //corrupt number
		}
		if (debug) printf("addr [i] = %c, size = %d\n", addr[i], size);
	}
	addr[size] = '\0';
	long int result = 0;
	for (i = size-1; i >= 0; i--) {
		long int tenPow = 1;
		for (int j = 0; j < size-1-i; j++) {
			tenPow *= 10;}
		result += ((long int)addr[i]-48)*tenPow;
	}
	if (debug) printf("ins addr = %s, result = %ld\n", addr, result);
	//No negative or zero values allowed
	if (result <= 0) {
		delete [] addr;
		return 0; //corrupt number
	}
	delete [] addr;
	return result;
}

ADDRS getAddr(char *c) {
	int i = 0, start = 0, end = 0;
	char temp[INS_STRING_SIZE];

	//Parse a x86 resgiter name
	while (c[i]=='z') {i++;}
	if (c[i] == ',') {
		resetInput(c,i);
		i++;
		start = i;
		while (c[i] != ',' && c[i] != '\0' && c[i] != '\n') {
			temp[i] = c[i];
			resetInput(c,i);
			if (debug) printf("temp[i] = %c\n", temp[i]);
			i++;
		}
		//if (c[i] == '\n' || c[i] == '\0') return -3; //End of line (unsigned return)
		end = i;
	}
	int size = end - start;
	char * addr = new char[size+1];
	for (i=0; i < (end-start); i++) {
		addr[i] = temp[i+start];
		if (debug) printf("addr [i] = %c, size = %d\n", addr[i], size);
	}
	addr[size] = '\0';
	ADDRS result = 0;
	result = strtoull(addr, NULL, 10);
	if (debug) printf("addr = %s, result = %llu\n", addr, result);
	if (result == 0) {
		delete [] addr;
		return 0; //corrupt number
	}
	delete [] addr;
	return result;
}

int setupNewIns (instruction* ins) {
	int rt=-1;
	ADDRS addr, insAddr;
	long int memAccessSize;
	long int r=-1;
	long int brTaken = -1;
	double missRate;
	char cTemp[INS_STRING_SIZE], c[INS_STRING_SIZE];
	//if (fgets (c, INS_STRING_SIZE, pinFile) == NULL) return -1; //EOF
	if (ICQ->NumElements() == 0) return -1; //EOC
	strcpy(c, ICQ->Nth(0));
	Assert(c[0] != 'z');
	strcpy (cTemp, c);
	ins->setBrMode(branchMode);
	//if (debug) printf ("%s", c);
	switch (c[0]) {
		case 'A':  
			//printf ("%s", c);
			ins->setCmdStr(cTemp);
			resetInput (c, 0);
			ins->setType(ALU);
        		insAddr = getAddr(c); //getANumber(c);
        		if (insAddr == 0) {
				printf("WARNING: Instruction address corrupttion. Skipping %s\n", cTemp);
				corruptInsCount++;
        			return -2;
        		} else {
				ins->setInsAddr(insAddr);
			}
			if (makePhrase == true || parseHitMiss == 1) {
				missRate = (double)getANumber(c)/(double)COEFF;
				//Do nothing productive!
			}
			while (rt != 0) {
			    r = getReg(c);
			    rt = getRegType(c);
			    //if (debug) printf("r = %ld, rt = %d\n", r,rt);
			    //if (rt > 0 && rt < 3 && r > 0 && r < RF->getNumberOfRegs()+1) {ins->setRegister(&r, &rt);}
			    //if (rt > 0 && rt < 3 && r > 0 && r < NUM_REGISTERS) {ins->setRegister(&r, &rt);}
			    if (rt > 0 && rt < 3 && (r > 0  || r < -99)) {ins->setRegister(&r, &rt);}	
			    else if (rt == -3 || r == -3) {break;} //reached the end of line
			    else {
					if (debug) printf("WARNING: Instruction Corruption. Skipping %s\n",cTemp);
					corruptInsCount++;
			    	return -2;
			    }
			}
			ins->setPiepelineLat(ALU_LATENCY);
			break;
		case 'F':
			//printf ("%s", c);
			ins->setCmdStr(cTemp);
			resetInput (c, 0);
			ins->setType(FPU);
        		insAddr = getAddr(c); //getANumber(c);
        		if (insAddr == 0) {
				printf("WARNING: Instruction address corrupttion. Skipping %s\n", cTemp);
				corruptInsCount++;
        			return -2;
        		} else {
				ins->setInsAddr(insAddr);
			}
			if (makePhrase == true || parseHitMiss == 1) {
				missRate = (double)getANumber(c)/(double)COEFF;
				//Do nothing productive!
			}
			while (rt != 0) {
			    r = getReg(c);
			    rt = getRegType(c);
			    //if (debug) printf("r = %d, rt = %d\n", r,rt);
			    //if (rt > 0 && rt < 3 && r > 0 && r < RF->getNumberOfRegs()+1) {ins->setRegister(&r, &rt);}
			    //if (rt > 0 && rt < 3 && r > 0 && r < NUM_REGISTERS) {ins->setRegister(&r, &rt);}
			    if (rt > 0 && rt < 3 && (r > 0 || r < -99)) {ins->setRegister(&r, &rt);}
			    else if (rt == -3 || r == -3) {break;} //reached the end of line
			    else {
			    if (debug) printf("WARNING: Instruction Corruption. Skipping %s\n",cTemp);
					corruptInsCount++;
			    	return -2;
			    }
			}
			ins->setPiepelineLat(FPU_LATENCY);
			break;
		case 'B':  
			//printf ("%s", c);
			ins->setCmdStr(cTemp);
			resetInput (c, 0);
			ins->setType(BR);
        	insAddr = getAddr(c); //getANumber(c);
        	if (insAddr == 0) {
				printf("WARNING: Instruction address corrupttion. Skipping %s\n", cTemp);
				corruptInsCount++;
        		return -2;
        	} else {
				ins->setInsAddr(insAddr);
			}
			if (branchBiasProfileMap.find(insAddr) != branchBiasProfileMap.end()) {
				ins->setBrBias(branchBiasProfileMap[insAddr]);
			}
			if (branchAccuracyMap.find(insAddr) != branchAccuracyMap.end()) {
				ins->setBrAccuracy(branchAccuracyMap[insAddr]);
			} else {
				missingAccuracyBranches.insert(ins->getInsAddr());
			}
        	ins->setBrSide(getANumber(c));
			brTaken = getANumber(c);
			if (brTaken == 0) {
				printf("WARNING: Instruction Corruption. Skipping %s\n",cTemp);
				corruptInsCount++;
			    	return -2;
        		} else {
				ins->setBrTarget(brTaken);
			}
			ins->setBrForward();
			bool missPred;
			void *bp_hist;
			bp_hist = NULL;
			bool prediction;
			prediction = Predict((uint64_t)ins->getInsAddr(), bp_hist);
			if(prediction != ins->getBrSide())
				missPred = true; //mis-prediction
			else
				missPred = false; //correct-prediction
			Assert(bp_hist != NULL);
			ins->setPredHistObj(bp_hist);
			ins->setPrediction(prediction);
			ins->findMissPrediction(missPred);
			if (makePhrase == true || parseHitMiss == 1) {
				missRate = (double)getANumber(c)/(double)COEFF;
				//Do nothing productive!
			}
			while (rt != 0) {
			    r = getReg(c);
			    rt = getRegType(c);
			    //if (debug) printf("r = %d, rt = %d\n", r,rt);
			    //if (rt > 0 && rt < 3 && r > 0 && r < RF->getNumberOfRegs()+1) {ins->setRegister(&r, &rt);}
			    //if (rt > 0 && rt < 3 && r > 0 && r < NUM_REGISTERS) {ins->setRegister(&r, &rt);}
			    if (rt > 0 && rt < 3 && r > 0) {ins->setRegister(&r, &rt);}
			    else if (rt == -3 || r == -3) {break;} //reached the end of line
			    else {
			    	if (debug) printf("WARNING: Instruction Corruption. Skipping %s\n",cTemp);
					corruptInsCount++;
			    	return -2;
			    }
			}
			if (branchMode == statPredBr) {
				if (ins->getPrediction() == true) numBrOps_predT_fetch++;
				if (ins->getMissPrediction()==true)  {//Static branch prediction
					ins->setPiepelineLat(BR_LATENCY);
					missPredBROps_fetch++;
					if (prediction == false) missPredBROps_NT_fetch++;
				} else {
					ins->setPiepelineLat(ALU_LATENCY);
				}
			} else if (branchMode == dynPredBr) {
				if (ins->getPrediction() == true) numBrOps_predT_fetch++;
				if (ins->getMissPrediction()==true)  {//Dynamic branch prediction
					ins->setPiepelineLat(BR_LATENCY); //latency from zsim
					missPredBROps_fetch++;
					if (prediction == false) missPredBROps_NT_fetch++;
				} else {
					ins->setPiepelineLat(ALU_LATENCY);
				}
			} else if (branchMode == scheduleBr || branchMode == noBr) {
				ins->setPiepelineLat(ALU_LATENCY);
			} else {
				ins->setPiepelineLat(BR_LATENCY);
			}
			break;
        case 'R':  
        		//printf ("%s", c);
			ins->setCmdStr(cTemp);
        	resetInput (c, 0);
        	ins->setType(MEM);
        	ins->setMemType(READ);
        	addr = getAddr(c);
        	insAddr = getAddr(c); //getANumber(c);
			memAccessSize= getANumber(c);
        	if (addr == 0 || insAddr == 0 || memAccessSize == 0) {
				printf("WARNING: Instruction address corrupttion. Skipping %s\n", cTemp);
				corruptInsCount++;
        		return -2;
        	} else {
				ins->setMemAddr(addr);
				ins->setInsAddr(insAddr);
				ins->setMemAccessSize(memAccessSize);
			}
			if (makePhrase == true || parseHitMiss == 1) {
				missRate = (double)getANumber(c)/(double)COEFF;
				ins->setMissRate(missRate);
			}
        	while (rt != 0) {
        	    r  = getReg(c);
        	    rt = getRegType(c);
        	    //if (debug) printf("r = %ld, rt = %d\n", r,rt);
        	    //if (rt > 0 && rt < 3 && r > 0 && r < RF->getNumberOfRegs()+1) {ins->setRegister(&r, &rt);}
        	    //if (rt > 0 && rt < 3 && r > 0 && r < NUM_REGISTERS) {ins->setRegister(&r, &rt);}
        	    if ((rt > 0 && rt < 3) && (r > 0 || r < -99)) {ins->setRegister(&r, &rt);}
        	    else if (rt == -3 || r == -3) {break;} //reached the end of line
        	    else { //when r = -2  or -1 or ...
        	    	if (debug) printf("WARNING: Instruction reg MAY BE corrupt. Skipping %s\n", cTemp);
					corruptInsCount++;
        			return -2;
        	    }
        	    if (rt == -1) break; //nothing was set!
        	}
			ins->setPiepelineLat(cacheLat[0]);
        	break;
        case 'W':  
        	//printf ("%s", c);
			ins->setCmdStr(cTemp);
        	resetInput (c, 0);
        	ins->setType(MEM);
        	ins->setMemType(WRITE);
        	addr = getAddr(c);
        	insAddr = getAddr(c); //getANumber(c);
			memAccessSize = getANumber(c);
        	if (addr == 0 || insAddr == 0 || memAccessSize == 0) {
				printf("WARNING: Instruction address corrupttion. Skipping %s\n", cTemp);
				corruptInsCount++;
        		return -2;
        	} else {
				ins->setMemAddr(addr);
				ins->setInsAddr(insAddr);
				ins->setMemAccessSize(memAccessSize);
			}
			if (makePhrase == true || parseHitMiss == 1) {
				missRate = (double)getANumber(c)/(double)COEFF;
				ins->setMissRate(missRate);
			}
        	while (rt != 0) {
        	    r  = getReg(c);
        	    rt = getRegType(c);
        	    //if (debug) printf("r = %d, rt = %d\n", r,rt);
        	    //if (rt > 0 && rt < 3 && r > 0 && r < RF->getNumberOfRegs()+1) {ins->setRegister(&r, &rt);}
        	    //if (rt > 0 && rt < 3 && r > 0 && r < NUM_REGISTERS) {ins->setRegister(&r, &rt);}
        	    if (rt > 0 && rt < 3 && r > 0) {ins->setRegister(&r, &rt);}
        	    else if (rt == -3 || r == -3) {break;} //reached the end of line
        	    else { //when r = -2  or -1 or ...
        	    	if (debug) printf("WARNING: Instruction reg MAY BE corrupt. Skipping %s\n", cTemp);
					corruptInsCount++;
					return -2;
        	    }
        	    if (rt == -1) break; //nothing was set!
        	}
			ins->setPiepelineLat(cacheLat[0]);
        	break;
		case 'T':  
			//printf ("%s", c);
			ins->setCmdStr(cTemp);
			resetInput (c, 0);
			ins->setType(ASSIGN);
        		insAddr = getAddr(c); //getANumber(c);
        		if (insAddr == 0) {
				printf("WARNING: Instruction address corrupttion. Skipping %s\n", cTemp);
				corruptInsCount++;
        			return -2;
        		} else {
				ins->setInsAddr(insAddr);
			}
			if (makePhrase == true || parseHitMiss == 1) {
				missRate = (double)getANumber(c)/(double)COEFF;
				//Do nothing productive!
			}
			while (rt != 0) {
			    r = getReg(c);
			    rt = getRegType(c);
			    //if (debug) printf("r = %d, rt = %d\n", r,rt);
			    //if (rt > 0 && rt < 3 && r > 0 && r < RF->getNumberOfRegs()+1) {ins->setRegister(&r, &rt);}
			    //if (rt > 0 && rt < 3 && r > 0 && r < NUM_REGISTERS) {ins->setRegister(&r, &rt);}
			    if (rt > 0 && rt < 3 && (r > 0 || r < -99)) {ins->setRegister(&r, &rt);}
			    else if (rt == -3 || r == -3) {break;} //reached the end of line
			    else {
			    	if (debug) printf("WARNING: Instruction Corruption. Skipping %s\n",cTemp);
					corruptInsCount++;
			    	return -2;
			    }
			}
			ins->setPiepelineLat(ASSIGN_LATENCY);
			break;
		case '{':
			if (debug) printf("WARNING: Unrecognized Instruction. Skipping (%s)\n", cTemp);
			return -2;
		case '}':
			if (debug) printf("WARNING: Unrecognized Instruction. Skipping (%s)\n", cTemp);
			return -2;
		case 'H':
			if (debug) printf("WARNING: Unrecognized Instruction. Skipping (%s)\n", cTemp);
			return -2;
		default:
			if (debug) printf("WARNING: Unrecognized Instruction. Skipping (%s)\n", cTemp);
			corruptInsCount++;
			return -2;
	};
	return 0; //Successful completion (TODO: fix return values)
}

int addIns (int cycle) {
	//At SQUASH, wait for ROB to drain
	if (isSquashed && iROB->NumElements() > 0) {return 0;}
	else if (isSquashed && iROB->NumElements() == 0) {
		Assert(memoryModel == NAIVE_SPECUL);
		isSquashed = false;
		//printf("SQUASH END\n");
		if (!perfectRegRen && coreType != IN_ORDER) GRF->flush_fRAT();
		depTables->flush_depTables();
	}
	//Fill ICQ
	if (!eof) addToInsCache(cycle);
	//int activeWinRange = iROB->NumElements() - iWinPointer;
	if (iROB->NumElements() < ROBsize) {
		int diff = ROBsize - iROB->NumElements();
		for (int i = 0; i < diff; i++) {
			instruction *newIns = new instruction;
			newIns->setStatus(FETCH, cycle, -1);
			int result = setupNewIns(newIns);
			if (debug) printf ("result = %d\n", result);
			if (result == -1) {
				//eoc = true; //End of program
				delete newIns;
				break;
			} else if (result == -2) {
				popFromInsCache();
				delete newIns;
				i--;
				continue; //Skip the line and read next line
			} else {
				insID++;
				newIns->setInsID(insID);
				bool isRRstall = false, isLSQstall = false;
				if (perfectRegRen) {
					/* Manage LSQ */ 
					if (memoryModel == PERFECT) {
						newIns->perfect_MemDependencyTable(depTables, coreType, numSideBuffs);
					} else if (memoryModel == TOTAL_ORDER) {
						newIns->totalOrder_MemDependencyTable(loadStoreQue);
					} else if (memoryModel == NAIVE_SPECUL) {
						instruction* ancestor = NULL;
						bool sqHasIncomplIns = loadStoreQue->SQhasIncompleteIns();
						if (sqHasIncomplIns) ancestor = loadStoreQue->getSQtail();
						isLSQstall = oooLD_lsqHazard (loadStoreQue, newIns);
						if (!isLSQstall) oooLD_lsqEnque (loadStoreQue, newIns);
						if (sqHasIncomplIns && !isLSQstall) newIns->storeOrder_MemDependencyTable(ancestor);
					}
					if (isLSQstall) {
						/*-----STAT-----*/
						fetchStallCycle++;
						/*-----STAT-----*/
						delete newIns;
						insID--;
						break;
					} else {
						popFromInsCache();
					}
					newIns->infRegdependencyTable(depTables, coreType);
					newIns->br_dependencyTable(depTables);
				} else if (coreType == IN_ORDER) {
					/* Manage LSQ */ 
					if (memoryModel == PERFECT) {
						newIns->perfect_MemDependencyTable(depTables, coreType, numSideBuffs);
					} else if (memoryModel == TOTAL_ORDER) {
						newIns->totalOrder_MemDependencyTable(loadStoreQue);
					} else if (memoryModel == NAIVE_SPECUL) {
						instruction* ancestor = NULL;
						bool sqHasIncomplIns = loadStoreQue->SQhasIncompleteIns();
						if (sqHasIncomplIns) ancestor = loadStoreQue->getSQtail();
						isLSQstall = oooLD_lsqHazard (loadStoreQue, newIns);
						if (!isLSQstall) oooLD_lsqEnque (loadStoreQue, newIns);
						if (sqHasIncomplIns && !isLSQstall) newIns->storeOrder_MemDependencyTable(ancestor);
					}
					if (isLSQstall) {
						/*-----STAT-----*/
						fetchStallCycle++;
						/*-----STAT-----*/
						delete newIns;
						insID--;
						break;
					} else {
						popFromInsCache();
					}
					newIns->noRRdependencyTable(depTables, coreType);
					newIns->br_dependencyTable(depTables);
				} else if (coreType == OUT_OF_ORDER) {
					/* Manage LSQ */ 
					if (memoryModel == PERFECT) {
						isRRstall = newIns->renameRegs(GRF, coreType);
						if (isRRstall) { //if regrename not available, stall
							/*-----STAT-----*/
							fetchStallCycle++;
							/*-----STAT-----*/
							delete newIns;
							insID--;
							break;
						}
						newIns->perfect_MemDependencyTable(depTables, coreType, numSideBuffs);
					} else if (memoryModel == TOTAL_ORDER) {
						isRRstall = newIns->renameRegs(GRF, coreType);
						if (isRRstall) { //if regrename not available, stall
							/*-----STAT-----*/
							fetchStallCycle++;
							/*-----STAT-----*/
							delete newIns;
							insID--;
							break;
						}
						newIns->totalOrder_MemDependencyTable(loadStoreQue);
					} else if (memoryModel == NAIVE_SPECUL) {
						instruction* ancestor = NULL;
						bool sqHasIncomplIns = loadStoreQue->SQhasIncompleteIns();
						if (sqHasIncomplIns) ancestor = loadStoreQue->getSQtail();
						isLSQstall = oooLD_lsqHazard (loadStoreQue, newIns);
						if (isLSQstall) {
							/*-----STAT-----*/
							fetchStallCycle++;
							/*-----STAT-----*/
							delete newIns;
							insID--;
							break;
						}
						isRRstall = newIns->renameRegs(GRF, coreType);
						if (isRRstall) { //if regrename not available, stall
							/*-----STAT-----*/
							fetchStallCycle++;
							/*-----STAT-----*/
							delete newIns;
							insID--;
							break;
						}
						oooLD_lsqEnque (loadStoreQue, newIns);
						if (sqHasIncomplIns) newIns->storeOrder_MemDependencyTable(ancestor);
					}
					popFromInsCache();
					newIns->br_dependencyTable(depTables);
				} else {
					Assert(true == false && "Depeency provisions for this core type is not supported.");
				}
				/*-----STAT-----*/
				numParentsHist->addElem(newIns->getNumAncestors());
				/*-----STAT-----*/
				if (reschedule == false && makePhrase == false)
					iWindow->Append(newIns);
				iROB->Append(newIns);
				newIns->setBrMode(branchMode);
				newIns->findPhraseAncestors(); //TODO this should go somewhere else
				newIns = NULL;
				//printf("TEST: newIndx = %d, ROB size = %d\n", *iWindow->Nth(iWindow->NumElements()-1), iROB->NumElements());
			}
		}
		if (debug) printf ("Added %d instruction(s)\n", diff);
	} else {
		if (debug) printf ("WARNING: The iROB is FULL\n");
	}
	return 0;
}


bool bbPrediction;
int setupNewInsV2 (instruction* ins, char *c) {
	int rt=-1;
	ADDRS addr; ADDRS insAddr;
	long int memAccessSize;
	long int r=-1;
	long int brTaken = -1;
	char cTemp[INS_STRING_SIZE];
	strcpy (cTemp, c);
	// if (debug) printf ("%s", c);
	float missRate;
	ins->setBrMode(branchMode);
	switch (c[0]) {
		case 'A':  
			//printf ("%s", c);
			ins->setCmdStr(cTemp);
			resetInput (c, 0);
			ins->setType(ALU);
        		insAddr = getAddr(c); //getANumber(c);
        		if (insAddr == 0) {
				printf("WARNING: Instruction address corrupttion. Skipping %s\n", cTemp);
				corruptInsCount++;
        			return -2;
        		} else {
				ins->setInsAddr(insAddr);
			}
			if (makePhrase == true || parseHitMiss == 1) {
				missRate = (double)getANumber(c)/(double)COEFF;
				//Do nothing productive!
			}
			while (rt != 0) {
			    r = getReg(c);
			    rt = getRegType(c);
			    //if (debug) printf("r = %d, rt = %d\n", r,rt);
			    //if (rt > 0 && rt < 3 && r > 0 && r < RF->getNumberOfRegs()+1) {ins->setRegister(&r, &rt);}
			    //if (rt > 0 && rt < 3 && r > 0 && r < NUM_REGISTERS) {ins->setRegister(&r, &rt);}
			    if (rt > 0 && rt < 3 && (r > 0 || r < -99)) {ins->setRegister(&r, &rt);}
			    else if (rt == -3 || r == -3) {break;} //reached the end of line
			    else {
			    if (debug) printf("WARNING: Instruction Corruption. Skipping %s\n",cTemp);
					corruptInsCount++;
			    	return -2;
			    }
			}
			ins->setPiepelineLat(ALU_LATENCY);
			break;
		case 'F':
			//printf ("%s", c);
			ins->setCmdStr(cTemp);
			resetInput (c, 0);
			ins->setType(FPU);
        		insAddr = getAddr(c); //getANumber(c);
        		if (insAddr == 0) {
				printf("WARNING: Instruction address corrupttion. Skipping %s\n", cTemp);
				corruptInsCount++;
        			return -2;
        		} else {
				ins->setInsAddr(insAddr);
			}
			if (makePhrase == true || parseHitMiss == 1) {
				missRate = (double)getANumber(c)/(double)COEFF;
				//Do nothing productive!
			}
			while (rt != 0) {
			    r = getReg(c);
			    rt = getRegType(c);
			    //if (debug) printf("r = %d, rt = %d\n", r,rt);
			    //if (rt > 0 && rt < 3 && r > 0 && r < RF->getNumberOfRegs()+1) {ins->setRegister(&r, &rt);}
			    //if (rt > 0 && rt < 3 && r > 0 && r < NUM_REGISTERS) {ins->setRegister(&r, &rt);}
			    if (rt > 0 && rt < 3 && (r > 0 || r < -99)) {ins->setRegister(&r, &rt);}
			    else if (rt == -3 || r == -3) {break;} //reached the end of line
			    else {
			    if (debug) printf("WARNING: Instruction Corruption. Skipping %s\n",cTemp);
					corruptInsCount++;
			    	return -2;
			    }
			}
			ins->setPiepelineLat(FPU_LATENCY);
			break;
		case 'B':  
			//printf ("%s", c);
			ins->setCmdStr(cTemp);
			resetInput (c, 0);
			ins->setType(BR);
        	insAddr = getAddr(c); //getANumber(c);
        	if (insAddr == 0) {
				printf("WARNING: Instruction address corrupttion. Skipping %s\n", cTemp);
				corruptInsCount++;
        		return -2;
        	} else {
				ins->setInsAddr(insAddr);
			}
			if (branchBiasProfileMap.find(insAddr) != branchBiasProfileMap.end()) {
				ins->setBrBias(branchBiasProfileMap[insAddr]);
			}
			if (branchAccuracyMap.find(insAddr) != branchAccuracyMap.end()) {
				ins->setBrAccuracy(branchAccuracyMap[insAddr]);
			} else {
				missingAccuracyBranches.insert(ins->getInsAddr());
			}
        	ins->setBrSide(getANumber(c));
			brTaken = getANumber(c);
			if (brTaken == 0) {
				printf("WARNING: Instruction Corruption. Skipping %s\n",cTemp);
				corruptInsCount++;
			    	return -2;
        		} else {
				ins->setBrTarget(brTaken);
			}
			ins->setBrForward();
			bool missPred;
			missPred = isMisPrePredicted(bbPrediction, ins->getBrSide());
			//if (ins->getBrSide() == true)
			//	missPred = Predict((uint64_t)ins->getInsAddr(), 1);
			//	//missPred = PredictAndUpdate((uint64_t)ins->getInsAddr(), 1);
			//else
			//	missPred = Predict((uint64_t)ins->getInsAddr(), 0);
			//	//missPred = PredictAndUpdate((uint64_t)ins->getInsAddr(), 0);
			//ins->setPredHistObj(bp_hist_list->Nth(0));
			//bp_hist_list->RemoveAt(0);
			//ins->setPrediction(bbPrediction);
			ins->findMissPrediction(missPred);
			if (makePhrase == true || parseHitMiss == 1) {
				missRate = (double)getANumber(c)/(double)COEFF;
				//Do nothing productive!
			}
			while (rt != 0) {
			    r = getReg(c);
			    rt = getRegType(c);
			    //if (debug) printf("r = %d, rt = %d\n", r,rt);
			    //if (rt > 0 && rt < 3 && r > 0 && r < RF->getNumberOfRegs()+1) {ins->setRegister(&r, &rt);}
			    //if (rt > 0 && rt < 3 && r > 0 && r < NUM_REGISTERS) {ins->setRegister(&r, &rt);}
			    if (rt > 0 && rt < 3 && r > 0) {ins->setRegister(&r, &rt);}
			    else if (rt == -3 || r == -3) {break;} //reached the end of line
			    else {
			    	if (debug) printf("WARNING: Instruction Corruption. Skipping %s\n",cTemp);
					corruptInsCount++;
			    	return -2;
			    }
			}
			if (branchMode == statPredBr) {
				if (ins->getPrediction() == true) numBrOps_predT_fetch++;
				if (ins->getMissPrediction()==true)  {//Static branch prediction
					ins->setPiepelineLat(BR_LATENCY);
					missPredBROps_fetch++;
					if (ins->getPrediction() == false) missPredBROps_NT_fetch++;
				} else {
					ins->setPiepelineLat(ALU_LATENCY);
				}
			} else if (branchMode == dynPredBr) {
				if (ins->getPrediction() == true) numBrOps_predT_fetch++;
				if (ins->getMissPrediction()==true)  {//Static branch prediction
					ins->setPiepelineLat(BR_LATENCY); //latency from zsim
					missPredBROps_fetch++;
					if (ins->getPrediction() == false) missPredBROps_NT_fetch++;
				} else {
					ins->setPiepelineLat(ALU_LATENCY);
				}
			} else if (branchMode == scheduleBr || branchMode == noBr) {
				ins->setPiepelineLat(ALU_LATENCY);
			} else {
				ins->setPiepelineLat(BR_LATENCY);
			}
			break;
        case 'R':  
        	//printf ("%s", c);
			ins->setCmdStr(cTemp);
        	resetInput (c, 0);
        	ins->setType(MEM);
        	ins->setMemType(READ);
        	addr = getAddr(c);
        	insAddr = getAddr(c); //getANumber(c);
			memAccessSize = getANumber(c);
        	if (addr == 0 || insAddr == 0) {
				printf("WARNING: Instruction address corrupttion. Skipping %s\n", cTemp);
				corruptInsCount++;
        		return -2;
        	} else {
				ins->setMemAddr(addr);
				ins->setInsAddr(insAddr);
			}
			if (makePhrase == true || parseHitMiss == 1) {
				missRate = (double)getANumber(c)/(double)COEFF;
				ins->setMissRate(missRate);
			}
        	while (rt != 0) {
        	    r  = getReg(c);
        	    rt = getRegType(c);
        	    //if (debug) printf("r = %d, rt = %d\n", r,rt);
        	    //if (rt > 0 && rt < 3 && r > 0 && r < RF->getNumberOfRegs()+1) {ins->setRegister(&r, &rt);}
        	    //if (rt > 0 && rt < 3 && r > 0 && r < NUM_REGISTERS) {ins->setRegister(&r, &rt);}
        	    if (rt > 0 && rt < 3 && (r > 0 || r < -99)) {ins->setRegister(&r, &rt);}
        	    else if (rt == -3 || r == -3) {break;} //reached the end of line
        	    else { //when r = -2  or -1 or ...
        	    	//if (debug) 
					printf("WARNING: Instruction reg MAY BE corrupt. Skipping %s\n", cTemp);
					corruptInsCount++;
        			return -2;
        	    }
        	    if (rt == -1) break; //nothing was set!
        	}
			ins->setPiepelineLat(cacheLat[0]);
        	break;
        case 'W':  
        	//printf ("%s", c);
			ins->setCmdStr(cTemp);
        	resetInput (c, 0);
        	ins->setType(MEM);
        	ins->setMemType(WRITE);
        	addr = getAddr(c);
        	insAddr = getAddr(c); //getANumber(c);
			memAccessSize = getANumber(c);
        	if (addr == 0 || insAddr == 0) {
				printf("WARNING: Instruction address corrupttion. Skipping %s\n", cTemp);
				corruptInsCount++;
        		return -2;
        	} else {
				ins->setMemAddr(addr);
				ins->setInsAddr(insAddr);
			}
			if (makePhrase == true || parseHitMiss == 1) {
				missRate = (double)getANumber(c)/(double)COEFF;
				ins->setMissRate(missRate);
			}
        	while (rt != 0) {
        	    r  = getReg(c);
        	    rt = getRegType(c);
        	    //if (debug) printf("r = %d, rt = %d\n", r,rt);
        	    //if (rt > 0 && rt < 3 && r > 0 && r < RF->getNumberOfRegs()+1) {ins->setRegister(&r, &rt);}
        	    //if (rt > 0 && rt < 3 && r > 0 && r < NUM_REGISTERS) {ins->setRegister(&r, &rt);}
        	    if (rt > 0 && rt < 3 && r > 0) {ins->setRegister(&r, &rt);}
        	    else if (rt == -3 || r == -3) {break;} //reached the end of line
        	    else { //when r = -2  or -1 or ...
        	    	//if (debug) 
					printf("WARNING: Instruction reg MAY BE corrupt. Skipping %s\n", cTemp);
					corruptInsCount++;
        			return -2;
        	    }
        	    if (rt == -1) break; //nothing was set!
        	}
			ins->setPiepelineLat(cacheLat[0]);
        	break;
		case 'T':  
			//printf ("%s", c);
			ins->setCmdStr(cTemp);
			resetInput (c, 0);
			ins->setType(ASSIGN);
        	insAddr = getAddr(c);//getANumber(c);
        	if (insAddr == 0) {
				printf("WARNING: Instruction address corrupttion. Skipping %s\n", cTemp);
				corruptInsCount++;
        		return -2;
        	} else {
				ins->setInsAddr(insAddr);
			}
			if (makePhrase == true || parseHitMiss == 1) {
				missRate = (double)getANumber(c)/(double)COEFF;
				//Do nothing productive!
			}
			while (rt != 0) {
			    r = getReg(c);
			    rt = getRegType(c);
			    //if (debug) printf("r = %d, rt = %d\n", r,rt);
			    //if (rt > 0 && rt < 3 && r > 0 && r < RF->getNumberOfRegs()+1) {ins->setRegister(&r, &rt);}
			    //if (rt > 0 && rt < 3 && r > 0 && r < NUM_REGISTERS) {ins->setRegister(&r, &rt);}
			    if (rt > 0 && rt < 3 && (r > 0 || r < -99)) {ins->setRegister(&r, &rt);}
			    else if (rt == -3 || r == -3) {break;} //reached the end of line
			    else {
			    	if (debug) printf("WARNING: Instruction Corruption. Skipping %s\n",cTemp);
					corruptInsCount++;
			    	return -2;
			    }
			}
			ins->setPiepelineLat(ASSIGN_LATENCY);
			break;
		default:
			if (debug) printf("WARNING: Unrecognized Instruction. Skipping (%s)\n", cTemp);
			corruptInsCount++;
			return -2;
	};
	/// if (debug) printf("ins addr: %llu\n", insAddr);
	return 0; //Successful completion (TODO: fix return values)
    return memAccessSize;
}

int PhListSize = 100; //TODO put it in the right place
int phraseID = 0;
void addPhrase (List<phrase*> *iPh, int cycle, dot* d) {
	//int activeWinRange = iROB->NumElements() - iWinPointer;
	if (iPh->NumElements() < PhListSize) {
		int diff = PhListSize - iPh->NumElements();
		char c[INS_STRING_SIZE];
		for (int i = 0; i < diff; i++) {
			while(true) {
				if (fgets (c, INS_STRING_SIZE, pinFile) == NULL) {
					eoc = true;
					delete d;
					return;} //EOC
				//printf("%c\n",c[0]);
				if (c[0] == '{') {
					phrase* ph = new phrase(phraseID);
					phraseID++;
					while(true) {
						if (fgets (c, INS_STRING_SIZE, pinFile) == NULL) {
							eoc = true;
							printf("An incomplete phrase has been detected!\n");
							for (int j = 0; j < iPh->NumElements(); j++) {
								printf ("%d, ", iPh->Nth(j)->getPhraseSize());
							}
							exit(-1);} //EOC
						//printf("%c\n",c[0]);
						if (c[0] != '}') {
							instruction *newIns = new instruction;
							newIns->setStatus(FETCH, cycle, -1);
							int result = setupNewInsV2(newIns, c);
							if (result == -1) {
								eoc = true;
								delete newIns;
								break;
							} //EOC
							else if (result == -2) {
								delete newIns;
								i--;
								continue; //Skip the line and read next line
							} else {
								insID++;
								newIns->setInsID(insID);
								newIns->br_dependencyTable(depTables);
								/* Manage LSQ */ 
								if (memoryModel == PERFECT) {
									newIns->perfect_MemDependencyTable(depTables, coreType, numSideBuffs);
								} else if (memoryModel == TOTAL_ORDER) {
									newIns->totalOrder_MemDependencyTable(loadStoreQue);
								} else if (memoryModel == NAIVE_SPECUL) {
									oooLD_lsqEnque (loadStoreQue, newIns);
								}
								newIns->setMyPhrase(ph);
								//if (reschedule == false)
								//	iWindow->Append(newIns);
								iROB->Append(newIns);
								ph->addToPhrase(newIns);
								newIns = NULL;
							}
						} else {break;}
					}
					Assert(ph->getPhraseSize() > 0);
					//phSizeHist->addElem(ph->getPhraseSize_unsort());
					printf("%d\n", phraseID);
					//if (phraseID > 500 && phraseID < 520) {
						printf("phraseID: %d\n",phraseID);
						d->runDot(ph->getInsList_unsort(),ph->getPhraseID());
					//}
					iPh->Append(ph);
					//printf("number of phrases = %d\n", iPh->Nth(iPh->NumElements()-1)->getPhraseID());
					break;
				}
			}
		}
		if (debug) printf ("Added %d instruction(s)\n", diff);
	} else {
		if (debug) printf ("WARNING: The iROB is FULL\n");
	}
}

void addPhraseblock (List<instruction*>** pbLists, int cycle) {
	//static bool firstBBins = true;
	//At Squash, wait for ROB to drain
	//if (isSquashed && pbROB->NumElements() > 0) {return;}
	//else if (isSquashed && iROB->NumElements() == 0) {
	//	Assert(memoryModel == NAIVE_SPECUL);
	//	isSquashed = false;
	//	//printf("SQUASH END\n");
	//	if (!perfectRegRen) GRF->flush_fRAT();
	//	depTables->flush_depTables();
	//	if (!perfectRegRen) {
	//		for (int i = 0; i < NUM_PHRASEBLKS; i++) {
	//			LRFTables[i]->flush_depTables();
	//		}
	//	}
	//	if (coreType == PHRASEBLOCK) {
	//		addLastBracket();
	//		inCompleteBBbuffIndx = -1;
	//	}
	//}
	if (pbROB->NumElements() < pbROBsize) {
		for (int k = 0; k < NUM_PHRASEBLKS; k++) {
			List<instruction*>* pbList = pbLists[k];
			if (pbList->NumElements() == 0) {
				addToPBCache();
				if (eof) return;
				char c[INS_STRING_SIZE];
				for (int j = 0; j < pbROB->Last()->getNumBBIns_s(); j++) {
					strcpy(c, (pbROB->Last()->getNthBBIns_s(j))->c_str());
					instruction *newIns = new instruction;
					newIns->setStatus(FETCH, cycle, -1);
					//bool isRRstall = false, isLSQstall = false;
					int result = setupNewInsV2(newIns, c);
					if (result == -1) { //TODO redundant block I think - eliminate
						delete newIns;
						break;
					} //EOC
					else if (result == -2) {
						delete newIns;
						continue; //Skip the line and read next line
					} else {
						insID++;
						newIns->setInsID(insID);
						if (perfectRegRen) {
							/* Manage LSQ */ 
							if (memoryModel == PERFECT) {
								newIns->perfect_MemDependencyTable(depTables, coreType, numSideBuffs);
							//} else if (memoryModel == TOTAL_ORDER) {
							//	newIns->totalOrder_MemDependencyTable(loadStoreQue);
							//} else if (memoryModel == NAIVE_SPECUL) {
							//	instruction* ancestor = NULL;
							//	bool sqHasIncomplIns = loadStoreQue->SQhasIncompleteIns();
							//	if (sqHasIncomplIns) ancestor = loadStoreQue->getSQtail();
							//	isLSQstall = oooLD_lsqHazard (loadStoreQue, newIns);
							//	if (!isLSQstall) oooLD_lsqEnque (loadStoreQue, newIns);
							//	if (sqHasIncomplIns && !isLSQstall) newIns->storeOrder_MemDependencyTable(ancestor);
							//}
							//if (isLSQstall) {
							//	/*-----STAT-----*/
							//	fetchStallCycle++;
							//	/*-----STAT-----*/
							//	delete newIns;
							//	insID--;
							//	inCompleteBBbuffIndx = k;
							//	return;
							//} else {
							//	popFromInsCache();
							}
							newIns->infRegdependencyTable(depTables, coreType);
							newIns->br_dependencyTable(depTables);
						//} else {
						//	/* Manage LSQ */ 
						//	if (memoryModel == PERFECT) {
						//		isRRstall = newIns->renameRegs(GRF, coreType);
						//		if (isRRstall) { //if regrename not available, stall
						//			/*-----STAT-----*/
						//			fetchStallCycle++;
						//			/*-----STAT-----*/
						//			delete newIns;
						//			insID--;
						//			inCompleteBBbuffIndx = k;
						//			return;
						//		}
						//		newIns->perfect_MemDependencyTable(depTables, coreType, numSideBuffs);
						//	} else if (memoryModel == TOTAL_ORDER) {
						//		isRRstall = newIns->renameRegs(GRF, coreType);
						//		if (isRRstall) { //if regrename not available, stall
						//			/*-----STAT-----*/
						//			fetchStallCycle++;
						//			/*-----STAT-----*/
						//			delete newIns;
						//			insID--;
						//			inCompleteBBbuffIndx = k;
						//			return;
						//		}
						//		newIns->totalOrder_MemDependencyTable(loadStoreQue);
						//	} else if (memoryModel == NAIVE_SPECUL) {
						//		instruction* ancestor = NULL;
						//		bool sqHasIncomplIns = loadStoreQue->SQhasIncompleteIns();
						//		if (sqHasIncomplIns) ancestor = loadStoreQue->getSQtail();
						//		isLSQstall = oooLD_lsqHazard (loadStoreQue, newIns);
						//		if (isLSQstall) {
						//			/*-----STAT-----*/
						//			fetchStallCycle++;
						//			/*-----STAT-----*/
						//			delete newIns;
						//			insID--;
						//			inCompleteBBbuffIndx = k;
						//			return;
						//		}
						//		isRRstall = newIns->renameRegs(GRF, coreType);
						//		if (isRRstall) { //if regrename not available, stall
						//			/*-----STAT-----*/
						//			fetchStallCycle++;
						//			/*-----STAT-----*/
						//			delete newIns;
						//			insID--;
						//			inCompleteBBbuffIndx = k;
						//			return;
						//		}
						//		oooLD_lsqEnque (loadStoreQue, newIns);
						//		if (sqHasIncomplIns) newIns->storeOrder_MemDependencyTable(ancestor);
						//	}
						//	popFromInsCache();
						//	newIns->noRRdependencyTable(LRFTables[k], coreType); //LRF depdency handling
						//	newIns->br_dependencyTable(depTables);
						//}
						//iROB->Append(newIns);
						//if (firstBBins) {
						//	newIns->setBBhead();
						//	newIns->setBrHeaderAddr(header);
						//	firstBBins = false;
						}
						pbROB->Last()->addToBB(newIns);
						pbList->Append(newIns);
						newIns = NULL;
					}
					//inCompleteBBbuffIndx = -1;
					//} else if (c[0] == 'H') {
					//	c[0] = 'z';
					//	header = getAddr(c);
					//	void *bp_hist = NULL;
					//	bbPrediction = PrePredict(header, bp_hist);
					//	Assert(bp_hist != NULL);
					//	popFromInsCache();
					//	bp_hist_list->Append(bp_hist);
					//	br_pred_update_distance.insert(pair<ADDRS,int> (header,cycle));
					//	inCompleteBBbuffIndx = -1;
					//} else {
					//	if (iROB->NumElements() > 0) iROB->Nth(iROB->NumElements()-1)->setBBtail();
					//	popFromInsCache();
					//	inCompleteBBbuffIndx = -1;
					//	break;
					//}
				}
			}
		}
	} else {
		if (debug) printf ("WARNING: The iROB is FULL\n");
	}
}

//void addPhraseblock (List<instruction*>** pbLists, int cycle) {
//   static bool firstBBins = true;
//   //At Squash, wait for ROB to drain
//   if (isSquashed && iROB->NumElements() > 0) return;
//   else if (isSquashed && iROB->NumElements() == 0) {
//   	Assert(memoryModel == NAIVE_SPECUL);
//   	isSquashed = false;
//   	//printf("SQUASH END\n");
//   	if (!perfectRegRen) GRF->flush_fRAT();
//   	depTables->flush_depTables();
//   	if (!perfectRegRen) {
//   		for (int i = 0; i < NUM_PHRASEBLKS; i++) {
//   			LRFTables[i]->flush_depTables();
//   		}
//   	}
//   	if (coreType == PHRASEBLOCK) {
//   		addLastBracket();
//   		inCompleteBBbuffIndx = -1;
//   	}
//   }
//   if (iROB->NumElements() < ROBsize) {
//   	if (inCompleteBBbuffIndx >= 0 && inCompleteBBbuffIndx <= NUM_PHRASEBLKS) {
//   		List<instruction*>* pbList = pbLists[inCompleteBBbuffIndx];
//   		//printf("filling incomplete phrase\n");
//   		char c[INS_STRING_SIZE];
//   		while(true) {
//   			if (!eof) addToInsCache(cycle);
//   			if (ICQ->NumElements() == 0) { //EOC
//   				//eoc = true;
//   				printf("An incomplete phrase has been detected!\n");
//   				return;
//   			}
//   			strcpy(c, ICQ->Nth(0));
//   			Assert(c[0] != '{' && c[0] != 'H' && c[0] != 'z');
//   			if (c[0] != '}') {
//   				instruction *newIns = new instruction;
//   				newIns->setStatus(fetch, cycle, -1);
//   				bool isRRstall = false, isLSQstall = false;
//   				int result = setupNewInsV2(newIns, c);
//   				if (result == -1) {
//   					//eoc = true;
//   					delete newIns;
//   					return;
//   				} //EOC
//   				else if (result == -2) {
//   					popFromInsCache();
//   					delete newIns;
//   					//i--;
//   					continue; //Skip the line and read next line
//   				} else {
//   					insID++;
//   					newIns->setInsID(insID);
//   					if (perfectRegRen) {
//   						/* Manage LSQ */ 
//   						if (memoryModel == PERFECT) {
//   							newIns->perfect_MemDependencyTable(depTables, coreType, numSideBuffs);
//   						} else if (memoryModel == TOTAL_ORDER) {
//   							newIns->totalOrder_MemDependencyTable(loadStoreQue);
//   						} else if (memoryModel == NAIVE_SPECUL) {
//   							instruction* ancestor = NULL;
//   							bool sqHasIncomplIns = loadStoreQue->SQhasIncompleteIns();
//   							if (sqHasIncomplIns) ancestor = loadStoreQue->getSQtail();
//   							isLSQstall = oooLD_lsqHazard (loadStoreQue, newIns);
//   							if (!isLSQstall) oooLD_lsqEnque (loadStoreQue, newIns);
//   							if (sqHasIncomplIns && !isLSQstall) newIns->storeOrder_MemDependencyTable(ancestor);
//   						}
//   						if (isLSQstall) {
//   							/*-----STAT-----*/
//   							fetchStallCycle++;
//   							/*-----STAT-----*/
//   							delete newIns;
//   							insID--;
//   							return;
//   						} else {
//   							popFromInsCache();
//   						}
//   						newIns->infRegdependencyTable(depTables, coreType);
//   						newIns->br_dependencyTable(depTables);
//   					} else {
//   						/* Manage LSQ */ 
//   						if (memoryModel == PERFECT) {
//   							isRRstall = newIns->renameRegs(GRF, coreType);
//   							if (isRRstall) { //if regrename not available, stall
//   								/*-----STAT-----*/
//   								fetchStallCycle++;
//   								/*-----STAT-----*/
//   								delete newIns;
//   								insID--;
//   								return;
//   							}
//   							newIns->perfect_MemDependencyTable(depTables, coreType, numSideBuffs);
//   						} else if (memoryModel == TOTAL_ORDER) {
//   							isRRstall = newIns->renameRegs(GRF, coreType);
//   							if (isRRstall) { //if regrename not available, stall
//   								/*-----STAT-----*/
//   								fetchStallCycle++;
//   								/*-----STAT-----*/
//   								delete newIns;
//   								insID--;
//   								return;
//   							}
//   							newIns->totalOrder_MemDependencyTable(loadStoreQue);
//   						} else if (memoryModel == NAIVE_SPECUL) {
//   							instruction* ancestor = NULL;
//   							bool sqHasIncomplIns = loadStoreQue->SQhasIncompleteIns();
//   							if (sqHasIncomplIns) ancestor = loadStoreQue->getSQtail();
//   							isLSQstall = oooLD_lsqHazard (loadStoreQue, newIns);
//   							if (isLSQstall) {
//   								/*-----STAT-----*/
//   								fetchStallCycle++;
//   								/*-----STAT-----*/
//   								delete newIns;
//   								insID--;
//   								return;
//   							}
//   							isRRstall = newIns->renameRegs(GRF, coreType);
//   							if (isRRstall) { //if regrename not available, stall
//   								/*-----STAT-----*/
//   								fetchStallCycle++;
//   								/*-----STAT-----*/
//   								delete newIns;
//   								insID--;
//   								return;
//   							}
//   							oooLD_lsqEnque (loadStoreQue, newIns);
//   							if (sqHasIncomplIns) newIns->storeOrder_MemDependencyTable(ancestor);
//   						}
//   						popFromInsCache();
//   						newIns->noRRdependencyTable(LRFTables[inCompleteBBbuffIndx], coreType); //LRF depdency handling
//   						newIns->br_dependencyTable(depTables);
//   					}
//   					iROB->Append(newIns);
//   					if (firstBBins) {
//   						newIns->setBBhead();
//   						firstBBins = false;
//   					}
//   					pbList->Append(newIns);
//   					newIns = NULL;
//   				}
//   			} else {
//   				popFromInsCache();
//   				inCompleteBBbuffIndx = -1;
//   				if (iROB->NumElements() > 0) 
//   					iROB->Nth(iROB->NumElements()-1)->setBBtail();
//   				break;
//   			}
//   		}
//   	}
//   	if (inCompleteBBbuffIndx == -1) {
//   		//printf("filling complete phrase\n");
//   		for (int k = 0; k < NUM_PHRASEBLKS; k++) {
//   			List<instruction*>* pbList = pbLists[k];
//   			if (pbList->NumElements() == 0) {
//   				char c[INS_STRING_SIZE];
//   				while(true) {
//   					if (!eof) addToInsCache(cycle);
//   					if (ICQ->NumElements() == 0) { //EOC
//   						//eoc = true;
//   						return;
//   					}
//   					strcpy(c, ICQ->Nth(0));
//   					int counter = 0;
//   					Assert(c[0] != 'z');
//   					if (c[0] == '{') {
//   						firstBBins = true;
//   						popFromInsCache();
//   						/*-----STAT-----*/
//   						bbCount++;
//   						/*-----STAT-----*/
//   						//printf("===%c\n",c[0]);
//   						while(true) {
//   							INS_ADDR header = 0;
//   							if (!eof) addToInsCache(cycle);
//   							Assert(ICQ->NumElements() != 0 && "ICQ must not have been empty");
//   							strcpy(c, ICQ->Nth(0));
//   							//printf("***%s",c);
//   							Assert(c[0] != '{' && c[0] != 'z');
//   							if (c[0] != '}' && c[0] != 'H') {
//   								counter++;
//   								instruction *newIns = new instruction;
//   								newIns->setStatus(fetch, cycle, -1);
//   								bool isRRstall = false, isLSQstall = false;
//   								int result = setupNewInsV2(newIns, c);
//   								if (result == -1) { //TODO redundant block I think - eliminate
//   									//eoc = true;
//   									delete newIns;
//   									break;
//   								} //EOC
//   								else if (result == -2) {
//   									popFromInsCache();
//   									delete newIns;
//   									//i--;
//   									continue; //Skip the line and read next line
//   								} else {
//   									insID++;
//   									newIns->setInsID(insID);
//   									if (perfectRegRen) {
//   										/* Manage LSQ */ 
//   										if (memoryModel == PERFECT) {
//   											newIns->perfect_MemDependencyTable(depTables, coreType, numSideBuffs);
//   										} else if (memoryModel == TOTAL_ORDER) {
//   											newIns->totalOrder_MemDependencyTable(loadStoreQue);
//   										} else if (memoryModel == NAIVE_SPECUL) {
//   											instruction* ancestor = NULL;
//   											bool sqHasIncomplIns = loadStoreQue->SQhasIncompleteIns();
//   											if (sqHasIncomplIns) ancestor = loadStoreQue->getSQtail();
//   											isLSQstall = oooLD_lsqHazard (loadStoreQue, newIns);
//   											if (!isLSQstall) oooLD_lsqEnque (loadStoreQue, newIns);
//   											if (sqHasIncomplIns && !isLSQstall) newIns->storeOrder_MemDependencyTable(ancestor);
//   										}
//   										if (isLSQstall) {
//   											/*-----STAT-----*/
//   											fetchStallCycle++;
//   											/*-----STAT-----*/
//   											delete newIns;
//   											insID--;
//   											inCompleteBBbuffIndx = k;
//   											return;
//   										} else {
//   											popFromInsCache();
//   										}
//   										newIns->infRegdependencyTable(depTables, coreType);
//   										newIns->br_dependencyTable(depTables);
//   									} else {
//   										/* Manage LSQ */ 
//   										if (memoryModel == PERFECT) {
//   											isRRstall = newIns->renameRegs(GRF, coreType);
//   											if (isRRstall) { //if regrename not available, stall
//   												/*-----STAT-----*/
//   												fetchStallCycle++;
//   												/*-----STAT-----*/
//   												delete newIns;
//   												insID--;
//   												inCompleteBBbuffIndx = k;
//   												return;
//   											}
//   											newIns->perfect_MemDependencyTable(depTables, coreType, numSideBuffs);
//   										} else if (memoryModel == TOTAL_ORDER) {
//   											isRRstall = newIns->renameRegs(GRF, coreType);
//   											if (isRRstall) { //if regrename not available, stall
//   												/*-----STAT-----*/
//   												fetchStallCycle++;
//   												/*-----STAT-----*/
//   												delete newIns;
//   												insID--;
//   												inCompleteBBbuffIndx = k;
//   												return;
//   											}
//   											newIns->totalOrder_MemDependencyTable(loadStoreQue);
//   										} else if (memoryModel == NAIVE_SPECUL) {
//   											instruction* ancestor = NULL;
//   											bool sqHasIncomplIns = loadStoreQue->SQhasIncompleteIns();
//   											if (sqHasIncomplIns) ancestor = loadStoreQue->getSQtail();
//   											isLSQstall = oooLD_lsqHazard (loadStoreQue, newIns);
//   											if (isLSQstall) {
//   												/*-----STAT-----*/
//   												fetchStallCycle++;
//   												/*-----STAT-----*/
//   												delete newIns;
//   												insID--;
//   												inCompleteBBbuffIndx = k;
//   												return;
//   											}
//   											isRRstall = newIns->renameRegs(GRF, coreType);
//   											if (isRRstall) { //if regrename not available, stall
//   												/*-----STAT-----*/
//   												fetchStallCycle++;
//   												/*-----STAT-----*/
//   												delete newIns;
//   												insID--;
//   												inCompleteBBbuffIndx = k;
//   												return;
//   											}
//   											oooLD_lsqEnque (loadStoreQue, newIns);
//   											if (sqHasIncomplIns) newIns->storeOrder_MemDependencyTable(ancestor);
//   										}
//   										popFromInsCache();
//   										newIns->noRRdependencyTable(LRFTables[k], coreType); //LRF depdency handling
//   										newIns->br_dependencyTable(depTables);
//   									}
//   									iROB->Append(newIns);
//   									if (firstBBins) {
//   										newIns->setBBhead();
//   										newIns->setBrHeaderAddr(header);
//   										firstBBins = false;
//   									}
//   									pbList->Append(newIns);
//   									newIns = NULL;
//   								}
//   								inCompleteBBbuffIndx = -1;
//   							} else if (c[0] == 'H') {
//   								c[0] = 'z';
//   								header = getAddr(c);
//   								void *bp_hist = NULL;
//   								bbPrediction = PrePredict(header, bp_hist);
//   								Assert(bp_hist != NULL);
//   								popFromInsCache();
//   								bp_hist_list->Append(bp_hist);
//   								br_pred_update_distance.insert(pair<ADDRS,int> (header,cycle));
//   								inCompleteBBbuffIndx = -1;
//   							} else {
//   								if (iROB->NumElements() > 0) iROB->Nth(iROB->NumElements()-1)->setBBtail();
//   								popFromInsCache();
//   								inCompleteBBbuffIndx = -1;
//   								break;
//   							}
//   						}
//   						//if (counter > 200) printf("LARGE: %llu\n", iROB->Nth(iROB->NumElements()-1)->getInsAddr());
//   						//printf("number of phrases = %d\n", iPh->Nth(iPh->NumElements()-1)->getPhraseID());
//   						break;
//   					} else {
//   						//printf("ERRPR: leaking instructions: %s\n", c);
//   						popFromInsCache();
//   					}
//   				}
//   			}
//   			if (iROB->NumElements() >= ROBsize) break;
//   		}
//   	}
//   } else {
//   	if (debug) printf ("WARNING: The iROB is FULL\n");
//   }
//}

//int FrListSize = 200; //TODO put it in the right place
//int FragID = 0;
//int FragNum = 0;
//int wavefrontID = 0;
void addFrag (List<fragment*> *iFr, int cycle) {}

/* Check if an instruction has any of its ancestors 
 * within the range by which it is to be shifted */
bool checkAncestor(instruction* ins, int bottom, int top, List<instruction*> *iWindow) {
	if (ins->isGotoSideBuff() == true) return false;
	for (int i = 0; i < ins->getNumAncestors(); i++) {
		instruction* ancestor = ins->getNthAncestor(i);
		for (int j = bottom; j < top; j++) {
			if (iWindow->Nth(j)->getType() == ins->getType() &&
			    iWindow->Nth(j)->isGotoSideBuff() == false &&
			    iWindow->Nth(j)->getInsID() == ancestor->getInsID()) {
				return false; //ins cannot be moved due to dependency
			}
		}
	}
	return true;
}

void scheduleInsInFlight(List<int> *SBlist, List<int> *delList, List<instruction*> *iWindow, int top, int cycle) {
	while (SBlist->NumElements() > 0) {
		int bottom = SBlist->Nth(0); //findBottom(iWindow, SBlist); //TODO this is inaccurate
		bool swapped = false;
		//if (bottom >= top) break; //TODO do I need it... is it correct?
		//printf("SB: %d\n",SBlist->Nth(0));
		//printf("bottom %d, top %d\n", bottom, top);
		for (int j =  bottom+1; j <= top; j++) {
			//Assert (j > SBlist->Nth(0));
			bool result = checkAncestor(iWindow->Nth(j),bottom,j,iWindow);
			if (result == true) {
				//printf("hi %d\n",j);
				swap(iWindow,j,SBlist->Nth(0));
				swapped = true;
				SBlist->Append(j);
				break;
			}
		}
		//printf("hiii %d\n",SBlist->NumElements());
		if (swapped == false) {
			delList->Append(SBlist->Nth(0));
			SBlist->RemoveAt(0);
		} else {
			SBlist->RemoveAt(0);
			quicksort(SBlist,0,SBlist->NumElements()-1,cycle);
		}
	}
	quicksort(delList,0,delList->NumElements()-1,cycle);
	for (int i = delList->NumElements()-1; i >= 0; i--) {
		//printf("%d\n",delList->Nth(i));
		iWindow->RemoveAt(delList->Nth(i));
	}
	while (delList->NumElements() > 0) {
		delList->RemoveAt(0);
	}
}

bool isALUfree () {
	for (int i = 0; i < NUM_FUNC_UNIT; i++) {
		if (aluAvail[i]) {return true;}
	}
	return false; //No available ALU
}

bool isWinNotEmpty () {
	if (iROB->NumElements() > 0) return true;
	else return false;
}

/* REGISTER DEPENDENCY CHECK */
bool checkRegDependency (int indx) {
	//for (int j = 0; j < iROB->Nth(indx)->getNumReg(); j++) {
	//	if (iROB->Nth(indx)->getMyRegType(j) == READ) {
	//		//TODO I am not sure about this condition below
	//		//if (RF->getRegStatus(iROB->Nth(indx)->getMyReg(j)) < iROB->Nth(indx)->getInsID()) {
	//		int regStat = RF->getRegStat(iROB->Nth(indx)->getMyReg(j));
	//		if (regStat != -1 && regStat < iROB->Nth(indx)->getInsID()) {
	//			//register is reserved for write
	//			return true;
	//		}
	//	}
	//}
	for (int i = indx-1; i >= 0; i--) {
		if ((iROB->Nth(i))->getStatus() != complete) {
			for (int j = 0; j < iROB->Nth(indx)->getNumReg(); j++) {
				if (iROB->Nth(indx)->getMyRegType(j) == READ) {
					for (int ii = 0; ii < iROB->Nth(i)->getNumReg(); ii++) {
						if (iROB->Nth(i)->getMyReg(ii) == iROB->Nth(indx)->getMyReg(j)) {
							if (iROB->Nth(i)->getMyRegType(ii) == WRITE) {
								return true; //found true dependency
							}
						}
					}
				} else if (coreType == IN_ORDER && iROB->Nth(indx)->getMyRegType(j) == WRITE) {
					for (int ii = 0; ii < iROB->Nth(i)->getNumReg(); ii++) {
						if (iROB->Nth(i)->getMyReg(ii) == iROB->Nth(indx)->getMyReg(j)) {
							return true; //found false dependency (in order superscalar only)
						}
					}
				}
			}
		}
	}
	return false;
}


/* MEMORY ADDRESS DEPENDENCY CHECK */
bool checkMemDependency (int indx) {
	if (iROB->Nth(indx)->getMemType() == READ) { //RAW Hazard Check (word level address check)
		for (int i = indx-1; i >= 0; i--) {
			if (iROB->Nth(i)->getMemType() == WRITE && iROB->Nth(i)->getStatus() != complete) {
				long int temp1 = iROB->Nth(i)->getMemAddr() >> WORD_OFFSET;
				long int temp2 = iROB->Nth(indx)->getMemAddr() >> WORD_OFFSET;
				if (temp1 == temp2) {
					return true; //found true dependency (RAW)
				}
			}
		}
	} else if (iROB->Nth(indx)->getMemType() == WRITE) { //WAR Hazard Check (cache line level addr check)
		for (int i = indx-1; i >= 0; i--) {
			if ((iROB->Nth(i))->getMemType() == WRITE && (iROB->Nth(i))->getStatus() != complete) {
				return true; //Memory store total ordering (WAW)
			} else if (iROB->Nth(i)->getMemType() == READ && iROB->Nth(i)->getStatus() != complete) {
				//WAR hazard check
				long int temp1 = iROB->Nth(i)->getMemAddr()    >> (WORD_OFFSET+BLOCK_OFFSET);
				long int temp2 = iROB->Nth(indx)->getMemAddr() >> (WORD_OFFSET+BLOCK_OFFSET);
				if (temp1 == temp2) {
					return true; //found false dependency (WAR)
				}
			}
		}
	}
	return false;
}

void recordMemAccess(instruction *_ins, int lat) {
	int totCount = 0;
	int missCount = 0;
	long int insAddr = _ins->getInsAddr();
	Assert(insAddr > 0);

	//Keep track of total number of accesses
	if (memRdTotCountTable.count(insAddr) > 0) {
		totCount = memRdTotCountTable.find(insAddr)->second;
		memRdTotCountTable.erase(insAddr);
	}
	totCount++;
	memRdTotCountTable.insert(pair<long int,int>(insAddr,totCount));


	//Keep track of total number of misses
	//If an ins never misses, it will not be in this list
	if (lat > cacheLat[0]) {
		if (memRdMissCountTable.count(insAddr) > 0) {
			missCount = memRdMissCountTable.find(insAddr)->second;
			memRdMissCountTable.erase(insAddr);
			missCount++;
			memRdMissCountTable.insert(pair<long int,int>(insAddr,missCount));
		} else {
			missCount = 1;
			memRdMissCountTable.insert(pair<long int,int>(insAddr,missCount));
		}
	}
	Assert(totCount > 0 && missCount >= 0);
}

void printMissRatetoFile () {
	FILE* missRateFile;
	if((missRateFile=fopen("/home/milad/esc_project/svn/memTraceMilad/missRate.txt", "w")) == NULL) {
	    printf("5-ERROR: Cannot open file(s).\n");
	    exit(1);
	}
	map<long int,float>::iterator it;
	for (it=memRdMissRateTable.begin(); it != memRdMissRateTable.end(); it++) {
		fprintf(missRateFile, "%ld, %f\n", it->first, it->second);
	}
	fclose(missRateFile);
}

void computeMissRates() {
	int missCount, totalCount;
	float missRate;
	long int insAddr;
	map<long int,int>::iterator it;
	for (it=memRdTotCountTable.begin(); it != memRdTotCountTable.end(); it++) {
		insAddr    = (*it).first;
		totalCount = (*it).second;
		if (memRdMissCountTable.count(insAddr) > 0) {
			missCount = memRdMissCountTable.find(insAddr)->second; 
			missRate = (float)missCount/(float)totalCount;
			memRdMissRateTable.insert(pair<long int, float>(insAddr,missRate));
			memRdMissCountTable.erase(insAddr);
			/*-----STAT-----*/
			if (missRate > unpredMemOpThreshold) 
				unpredMemInsCnt++;
			/*-----STAT-----*/
		}
	}
	//Assert(memRdTotCountTable.empty() == true);
	Assert(memRdMissCountTable.empty() == true);
}

/* COMPUTE INSTRUCTION LATENCY */
int findLatency (instruction *_ins) {
	if (_ins->getType() == MEM) {
		int *lat = new int; //TODO stupid setup... get rid of lat
		int lat2;
		lat2 = cacheCtrl(_ins->getMemType(), _ins->getMemAddr(), 4, _L1, _L2, _L3);
		if (reportTraceAndHitMiss == true &&
		    _ins->getMemType() == READ) {recordMemAccess(_ins,lat2);}
		//Cache Miss Transient Measurement (disabled by default)
		//if (lat2 > cacheLat[0]) fprintf(outFile, "%d,", _ins->getInsID());
		/*-----STAT-----*/
		if ((coreType == PHRASE || coreType == FRAGMENT || coreType == FRAGMENT2) 
		    && lat2 > cacheLat[0]
		    && _ins->getMissrate() <= unpredMemOpThreshold) {
			unexpectedMiss++;
			unexpecteedLat+=lat2;
			//printf("%d, %f\n", lat2,_ins->getMissrate());
			//lat2 = cacheLat[0]; //TODO remove this - for single experiment purposes
			//printf("WF: %d, FR: %d, lat: %d\n", _ins->getMyPhraseID(), _ins->getMyFragID(), lat2);
		}
		/*-----STAT-----*/
		delete lat;
		if (_ins->getMemType() == READ) {
			_ins->setCacheHitLevel(lat2);
			return lat2;
		} else {
			_ins->setCacheHitLevel(ST_LATENCY);
			return ST_LATENCY;
		}
	} else if (_ins->getType() == ALU) {
		return ALU_LATENCY;
	} else if (_ins->getType() == FPU) {
		return FPU_LATENCY;
	} else if (_ins->getType() == ASSIGN) {
		return ASSIGN_LATENCY;
	} else if (_ins->getType() == BR && branchMode == statPredBr) {
		if (_ins->getMissPrediction() == true) //Static branch prediction
			return BR_LATENCY;
		else
			return ALU_LATENCY;
	} else if (_ins->getType() == BR && branchMode == noBr) {//No branches
			return ALU_LATENCY;
	} else if (_ins->getType() == BR && branchMode == dynPredBr) {
			/* Set Br Latency */
			if (_ins->getMissPrediction())
				return ALU_LATENCY;
			else
				return BR_LATENCY; //From Zsim
	} else if (_ins->getType() == BR) {
			return BR_LATENCY;
	} else {
		printf("Invalid Functional Unit Type!\nTerminating the program...\n");
		exit(-1);
	}
}

void executeIns (instruction *_ins, int _cycle, int _latency) {
	executeInsCount++; /* STAT */
	_ins->setStatus(execute, _cycle, _latency);
	if (reportTrace == true) reportInsTiming(_ins);
}

void updateInsLatency (instruction *_ins, int _cycle, int _latency) {
	Assert(_ins->getType() == MEM && "Only memory ops have unpredictable latency");
	Assert(_ins->getStatus() == execute);
	Assert(_latency >= 0);
	_ins->updateLatency(_cycle, _latency);
	//if (reportTrace == true) reportInsTiming(_ins);
}

void exeMemPipeStage(List<instruction*> *iROB, int cycle, lsq *loadStoreQue) {
	isCacheBusFree = true;
	for (int i = 0; i < iROB->NumElements(); i++) {
		instruction* ins = iROB->Nth(i);
		if (ins->getMemType() == READ && 
		    ins->getStatus() == execute) {
			if (ins->getMemAddrCompCompleteCycle() == cycle) {
				//Assuming MSHR && SQ are looked up in parallel
				long int insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
				if (oooLD_insertLQ_addr(loadStoreQue,ins)) { //Store FWD
					/*-----STAT-----*/
					stFwdMemOp++;
					/*-----STAT-----*/
					updateInsLatency(ins, cycle, ADDR_COMPUTE_LATENCY+CAM_ACCESS_LATENCY); //Complete next cycle
				} else if (inFlightLDops.find(insAddr)->second >= cycle) {//Non-blocking Load Op (MSHR)
					/*-----STAT-----*/
					nonBlockingMemOp++;
					/*-----STAT-----*/
					int tempLat = inFlightLDops.find(insAddr)->second - cycle;
					updateInsLatency(ins, cycle, tempLat);
					getLatency(0,tempLat); //Generate hit latency stat
				} else { //Cache Access (TODO can access cache more than once in a cycle (fix it)
					/*-----STAT-----*/
					cacheAccessMemOp++;
					/*-----STAT-----*/
					isCacheBusFree = false;
					int latency = findLatency(ins);
					updateInsLatency(ins, cycle, latency);
					int completeCycle = cycle+latency;
					insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
					inFlightLDops.insert(pair<long int,int>(insAddr,completeCycle));
				}
			}
		} else if (ins->getMemType() == WRITE &&
				   ins->getStatus() == execute &&
				   ins->getMemAddrCompCompleteCycle() == cycle) {
			oooLD_insertSQ_addrNdata(loadStoreQue,ins);
			updateInsLatency(ins, cycle, ADDR_COMPUTE_LATENCY+CAM_ACCESS_LATENCY); //Complete next cycle (this should be unnecessary)
			//NOTE: do not access cache for WRITE here
		}
	}
}

//Run the SQ dequeue functionality (if no LD is issued in this cycle)
void runStoreQueue(lsq *loadStoreQue, int cycle) {
	/* Pop SQ */
	oooLD_sqDequ (loadStoreQue, cycle);
	/* Write To Cache */
	if (isCacheBusFree) { //Support at most ONE store to cache per cycle
		if (loadStoreQue->hasCommittedUncachedSQentry()) {
			isCacheBusFree = false;
			int *lat = new int; //TODO stupid setup... get rid of lat
			INS_ID id = loadStoreQue->get_OldestCommittedUncachedSQentry_ID();
			ADDRS memAddr = loadStoreQue->get_oldestCommittedUncachedSQentry_MemAddr(id);
			int latency = cacheCtrl(WRITE, memAddr, 4, _L1, _L2, _L3);
			loadStoreQue->set_OldestCommittedUncachedSQentry_setLatency(latency, cycle, id);
			delete lat;
		}
	}
}

void getLSQsize(lsq *loadStoreQue) {
	lqSize += loadStoreQue->getLQsize();
	sqSize += loadStoreQue->getSQsize();
}

void getRRsize(registerRename *GRF) {
	//Assert(GRRF_SIZE <= GRF->getNumAvailablePR()); //TODO put this back?
	rrSize += (GRRF_SIZE - GRF->getNumAvailablePR());
}

/***************************************
 * OUT OF ORDER Core implementation (single issue)
 ***************************************/
map<long int, int> num_bypassed_wbb;
map<long int, int> num_ins_exe_cnt;
map<long int, float> ins_exe_hoist_accuracy;
bool collect_stat = false;
int print_count = 0, laten = -1;
bool start_print = false;
void runOOOcoreSingleIssue(int cycle) {
	//Cosntruct the ready list
	int iWinWalkIndx = -1;
	int wbb_bypass_count = 0, br_bypass_count = 0;
	float wbb_chain_accuracy = 1.0, br_chain_accuracy = 1.0;
	set<long int> list_of_wbb;
	int scheduled_mem = 0, scheduled_alu = 0;
	for (int i = 0; i < iWindow->NumElements(); i++) {
		/*-----STAT-----*/
		if (iWindow->Nth(i)->getType() == BR
		    && iWindow->Nth(i)->isReady(cycle) == false
			&& iWindow->Nth(i)->getBrBias() >= 0.05 
			&& iWindow->Nth(i)->getBrBias() <= 0.95) {
			list_of_wbb.insert(iWindow->Nth(i)->getInsAddr());
			wbb_bypass_count++;
			wbb_chain_accuracy *= iWindow->Nth(i)->getBrAccuracy();
		}
		if (iWindow->Nth(i)->getType() == BR) {
			br_bypass_count++;
			br_chain_accuracy *= iWindow->Nth(i)->getBrAccuracy();
		}
		/*-----STAT-----*/
		if (iWindow->Nth(i)->isReady(cycle) == true) {
			/*-----STAT-----*/
			//if (upldMissRateProfileMap.find(iWindow->Nth(i)->getInsAddr()) != upldMissRateProfileMap.end()) {
			////if (iWindow->Nth(i)->getInsAddr() == 4214686) { //bzip
			//if (iWindow->Nth(i)->getInsAddr() == 4248075) { //hmmer
			//	fprintf(junk,"\n\n");
			//	collect_stat = true;
			////} else if (iWindow->Nth(i)->getInsAddr() == 4214340 ||
			////           iWindow->Nth(i)->getInsAddr() == 4214683) { //bzip2
			//} else if ((iWindow->Nth(i)->getInsAddr() == 4248005 && 
			//			i+1 < iWindow->NumElements() &&
			//            iWindow->Nth(i+1)->getInsAddr() != 4248075)||
			//           iWindow->Nth(i)->getInsAddr() == 4248071) { //hmmer
			//	collect_stat = false;
			//}
			//if (collect_stat == true) {
			//	fprintf(junk,"%ld, ", iWindow->Nth(i)->getInsAddr());
			//}
			if (num_ins_exe_cnt.find(iWindow->Nth(i)->getInsAddr()) != num_ins_exe_cnt.end())
				num_ins_exe_cnt[iWindow->Nth(i)->getInsAddr()] += 1;
			else
				num_ins_exe_cnt[iWindow->Nth(i)->getInsAddr()] = 1;
			if (num_bypassed_wbb.find(iWindow->Nth(i)->getInsAddr()) != num_bypassed_wbb.end())
				//num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] += list_of_wbb.size(); //static ins
				num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] += wbb_bypass_count; //dynamic ins
			else
				//num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] = list_of_wbb.size(); //static ins
				num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] = wbb_bypass_count; //dynamic ins
			if (ins_exe_hoist_accuracy.find(iWindow->Nth(i)->getInsAddr()) != ins_exe_hoist_accuracy.end())
				ins_exe_hoist_accuracy[iWindow->Nth(i)->getInsAddr()] += br_chain_accuracy;
			else
				ins_exe_hoist_accuracy[iWindow->Nth(i)->getInsAddr()] = br_chain_accuracy;
			/*-----STAT-----*/
			iWindow->Nth(i)->setStatus(ready,-1,-1);
			iResStation->Append(iWindow->Nth(i));
			iWinWalkIndx = i;
			//Dont's go too far down the IQ if enough ready instructions are found (HW constraint modeling)
			if (scheduled_alu >= NUM_ALU_UNIT && 
			    scheduled_mem >= NUM_MEM_UNIT)
				break;
		}
	}
	quicksortInsList(iResStation,0,iResStation->NumElements()-1);
	//Updaate the fetch list
	for (int i = iWinWalkIndx; i >= 0; i--) {
		if (iWindow->Nth(i)->getStatus() == ready)
			iWindow->RemoveAt(i);
	}
	/*-----STAT-----*/
	iWinSize      += iWindow->NumElements();
	iROBSize      += iROB->NumElements();
	iResStnSize   += iResStation->NumElements();
	/*-----STAT-----*/
	//Check resource availability & execute ins
	if (iResStation->NumElements() == 0 && print_count < laten) {
		;//printf("STALL, ");
	}
	//if (start_print && iResStation->NumElements() == 0 && print_count >= laten) {
	if (start_print && iResStation->NumElements() == 0 && print_count >= laten) {
		start_print = false;
		printf("\n");
	}
	print_count++;
	/*-----STAT-----*/
	for (int j = 0; j < 1; j++) {
		if (aluAvail[j]==true) {
			if (iResStation->NumElements() > 0 && 
			    iResStation->Nth(0)->getType() == MEM) {
				aluAvail[j] = false;
				instruction* ins = iResStation->Nth(0);
				if (ins->getMemType() == READ) {
					long int insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
					if (inFlightLDops.count(insAddr) > 0) {//Non-blocking Load Op
						int tempLat = inFlightLDops.find(insAddr)->second - cycle;
						executeIns(ins, cycle, tempLat);
						getLatency(0,tempLat); //Generate hit latency stat
					} else {
						int latency = findLatency(ins);
						executeIns(ins, cycle, latency);
						int completeCycle = cycle+latency;
						insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
						inFlightLDops.insert(pair<long int,int>(insAddr,completeCycle));
					}
				} else if (ins->getMemType() == WRITE) {
					int latency = findLatency(ins);
					executeIns(ins, cycle, latency);
				} else {
					Assert(ins->getMemType() == WRITE || ins->getMemType() == READ);
				}
				/*-----STAT-----*/
				aluFreeTime[j] = cycle+1;//non-blocking LD/ST
				if (iResStation->Nth(0)->getInsAddr() == 4356400 || 
					iResStation->Nth(0)->getInsAddr() == 4248208 || 
				    iResStation->Nth(0)->getInsAddr() == 4397864) {
					//if (start_print) printf("\n");
					if (!start_print) printf("%d, , ", laten);
					start_print = true;
					print_count = 0;
					laten = iResStation->Nth(0)->getLatency();
				}
				if (start_print && print_count < laten) {
					printf("%lld, ", iResStation->Nth(0)->getInsID());
				} else if (start_print) {
					start_print = false;
					printf("\n");
				}
				/*-----STAT-----*/
				iResStation->RemoveAt(0);
			} else if (iResStation->NumElements() > 0 && 
			          (iResStation->Nth(0)->getType() == ALU ||
			           iResStation->Nth(0)->getType() == BR  ||
				       iResStation->Nth(0)->getType() == FPU)) {
				aluAvail[j] = false;
				instruction* ins = iResStation->Nth(0);
				int latency = findLatency(ins);
				executeIns(ins, cycle, latency);
				aluFreeTime[j] = cycle+latency;
				iResStation->RemoveAt(0);
				if (start_print && print_count < laten) {
					printf("%lld, ", iResStation->Nth(0)->getInsID());
				} else if (start_print) {
					start_print = false;
					printf("\n");
				}
				/*-----STAT-----*/
			}
		}
	}
	/*-----STAT-----*/
	inFlightLDopsSize += inFlightLDops.size();
	/*-----STAT-----*/
}


/***************************************
 * OUT OF ORDER Core implementation
 ***************************************/
void runOOOcore(int cycle) {
	//Cosntruct the ready list
	int iWinWalkIndx = -1;
	int wbb_bypass_count = 0, br_bypass_count = 0;
	float wbb_chain_accuracy = 1.0, br_chain_accuracy = 1.0;
	set<long int> list_of_wbb;
	int scheduled_mem = 0, scheduled_alu = 0;
	for (int i = 0; i < iWindow->NumElements(); i++) {
		/*-----STAT-----*/
		if (iWindow->Nth(i)->getType() == BR
		    && iWindow->Nth(i)->isReady(cycle) == false
			&& iWindow->Nth(i)->getBrBias() >= 0.05 
			&& iWindow->Nth(i)->getBrBias() <= 0.95) {
			list_of_wbb.insert(iWindow->Nth(i)->getInsAddr());
			wbb_bypass_count++;
			wbb_chain_accuracy *= iWindow->Nth(i)->getBrAccuracy();
		}
		if (iWindow->Nth(i)->getType() == BR) {
			br_bypass_count++;
			br_chain_accuracy *= iWindow->Nth(i)->getBrAccuracy();
		}
		/*-----STAT-----*/
		if (iWindow->Nth(i)->isReady(cycle) == true) {
			/*-----STAT-----*/
			//if (iWindow->Nth(i)->getInsAddr() == 4214686) { //bzip
			if (iWindow->Nth(i)->getInsAddr() == 4248075) { //hmmer
				fprintf(junk,"\n\n");
				collect_stat = true;
			//} else if (iWindow->Nth(i)->getInsAddr() == 4214340 ||
			//           iWindow->Nth(i)->getInsAddr() == 4214683) { //bzip2
			} else if ((iWindow->Nth(i)->getInsAddr() == 4248005 && 
						i+1 < iWindow->NumElements() &&
			            iWindow->Nth(i+1)->getInsAddr() != 4248075)||
			           iWindow->Nth(i)->getInsAddr() == 4248071) { //hmmer
				collect_stat = false;
			}
			if (collect_stat == true) {
				fprintf(junk,"%llu, ", iWindow->Nth(i)->getInsAddr());
			}
			if (num_ins_exe_cnt.find(iWindow->Nth(i)->getInsAddr()) != num_ins_exe_cnt.end())
				num_ins_exe_cnt[iWindow->Nth(i)->getInsAddr()] += 1;
			else
				num_ins_exe_cnt[iWindow->Nth(i)->getInsAddr()] = 1;
			if (num_bypassed_wbb.find(iWindow->Nth(i)->getInsAddr()) != num_bypassed_wbb.end())
				//num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] += list_of_wbb.size(); //static ins
				num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] += wbb_bypass_count; //dynamic ins
			else
				//num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] = list_of_wbb.size(); //static ins
				num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] = wbb_bypass_count; //dynamic ins
			if (ins_exe_hoist_accuracy.find(iWindow->Nth(i)->getInsAddr()) != ins_exe_hoist_accuracy.end())
				ins_exe_hoist_accuracy[iWindow->Nth(i)->getInsAddr()] += br_chain_accuracy;
			else
				ins_exe_hoist_accuracy[iWindow->Nth(i)->getInsAddr()] = br_chain_accuracy;
			/*-----STAT-----*/
			iWindow->Nth(i)->setStatus(ready,-1,-1);
			if      (iWindow->Nth(i)->getType() == ALU ||
			         iWindow->Nth(i)->getType() == FPU ||
			         iWindow->Nth(i)->getType() == BR) {
				iResStation->Append(iWindow->Nth(i));
				scheduled_alu++;
			} else if  (iWindow->Nth(i)->getType() == MEM) {
				iMemBuf->Append(iWindow->Nth(i));
				scheduled_mem++;
			} else {
				Assert (iWindow->Nth(i)->getType() == MEM || 
				        iWindow->Nth(i)->getType() == BR  ||
				        iWindow->Nth(i)->getType() == ALU ||
				        iWindow->Nth(i)->getType() == FPU);
			}
			iWinWalkIndx = i;
			//Dont's go too far down the IQ if enough ready instructions are found (HW constraint modeling)
			if (scheduled_alu >= NUM_ALU_UNIT && 
			    scheduled_mem >= NUM_MEM_UNIT)
				break;
		}
	}
	quicksortInsList(iResStation,0,iResStation->NumElements()-1);
	quicksortInsList(iMemBuf,0,iMemBuf->NumElements()-1);
	//printf("size of iROB        = %d\n", iROB->NumElements());
	//printf("size of iWindow     = %d\n", iWindow->NumElements());
	//printf("size of iResStation = %d\n", iResStation->NumElements());
	//printf("size of iMemBuf     = %d\n", iMemBuf->NumElements());
	//Updaate the fetch list
	for (int i = iWinWalkIndx; i >= 0; i--) {
		if (iWindow->Nth(i)->getStatus() == ready)
			iWindow->RemoveAt(i);
	}
	/*-----STAT-----*/
	iWinSize      += iWindow->NumElements();
	iROBSize      += iROB->NumElements();
	iResStnSize   += iResStation->NumElements();
	iMemBuffSize  += iMemBuf->NumElements();
	/*-----STAT-----*/
	//Check resource availability & execute ins
	for (int j = 0; j < NUM_FUNC_UNIT; j++) {
		if (aluAvail[j]==true) {
			if (aluKind[j] == MEM && iMemBuf->NumElements() > 0) {
				aluAvail[j] = false;
				instruction* ins = iMemBuf->Nth(0);
				if (ins->getMemType() == READ) {
					long int insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
					if (inFlightLDops.count(insAddr) > 0) {//Non-blocking Load Op
						int tempLat = inFlightLDops.find(insAddr)->second - cycle;
						executeIns(ins, cycle, tempLat);
						getLatency(0,tempLat); //Generate hit latency stat
					} else {
						int latency = findLatency(ins);
						executeIns(ins, cycle, latency);
						int completeCycle = cycle+latency;
						insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
						inFlightLDops.insert(pair<long int,int>(insAddr,completeCycle));
					}
				} else if (ins->getMemType() == WRITE) {
					int latency = findLatency(ins);
					executeIns(ins, cycle, latency);
				} else {
					Assert(ins->getMemType() == WRITE || ins->getMemType() == READ);
				}
				aluFreeTime[j] = cycle+1;//non-blocking LD/ST
				iMemBuf->RemoveAt(0);
			} else if (aluKind[j] == ALU && iResStation->NumElements() > 0) {
				aluAvail[j] = false;
				instruction* ins = iResStation->Nth(0);
				int latency = findLatency(ins);
				executeIns(ins, cycle, latency);
				aluFreeTime[j] = cycle+latency;
				iResStation->RemoveAt(0);
			}
		}
	}
	/*-----STAT-----*/
	inFlightLDopsSize += inFlightLDops.size();
	/*-----STAT-----*/
}

/***************************************
 * OUT OF ORDER Core with N EU's  (default: N=4)
 ***************************************/
int findBestResStation() {
	int indx = 0, shortest_res_station = 10000;
	for (int i = 0; i < NUM_FUNC_UNIT; i++) {
		if (shortest_res_station > iResStations[i]->NumElements()) {
			shortest_res_station = iResStations[i]->NumElements();
			indx = i;
		}
	}
	return indx;
}
void runOOOcore2(int cycle) {
	//Cosntruct the ready list
	//int iWinWalkIndx = -1;
	//int wbb_bypass_count = 0, br_bypass_count = 0;
	//float wbb_chain_accuracy = 1.0, br_chain_accuracy = 1.0;
	set<long int> list_of_wbb;
	for (int i = 0; i < iWindow->NumElements(); i++) {
		/*-----STAT-----*/
		//if (iWindow->Nth(i)->getType() == BR
		//    && iWindow->Nth(i)->isReady(cycle) == false
		//	&& iWindow->Nth(i)->getBrBias() >= 0.05 
		//	&& iWindow->Nth(i)->getBrBias() <= 0.95) {
		//	list_of_wbb.insert(iWindow->Nth(i)->getInsAddr());
		//	wbb_bypass_count++;
		//	wbb_chain_accuracy *= iWindow->Nth(i)->getBrAccuracy();
		//}
		//if (iWindow->Nth(i)->getType() == BR) {
		//	br_bypass_count++;
		//	br_chain_accuracy *= iWindow->Nth(i)->getBrAccuracy();
		//}
		/*-----STAT-----*/
		if (iWindow->Nth(i)->isReady(cycle) == true) {
			/*-----STAT-----*/
			////if (iWindow->Nth(i)->getInsAddr() == 4214686) { //bzip
			//if (iWindow->Nth(i)->getInsAddr() == 4248075) { //hmmer
			//	fprintf(junk,"\n\n");
			//	collect_stat = true;
			////} else if (iWindow->Nth(i)->getInsAddr() == 4214340 ||
			////           iWindow->Nth(i)->getInsAddr() == 4214683) { //bzip2
			//} else if ((iWindow->Nth(i)->getInsAddr() == 4248005 && 
			//			i+1 < iWindow->NumElements() &&
			//            iWindow->Nth(i+1)->getInsAddr() != 4248075)||
			//           iWindow->Nth(i)->getInsAddr() == 4248071) { //hmmer
			//	collect_stat = false;
			//}
			//if (collect_stat == true) {
			//	fprintf(junk,"%ld, ", iWindow->Nth(i)->getInsAddr());
			//}
			//if (num_ins_exe_cnt.find(iWindow->Nth(i)->getInsAddr()) != num_ins_exe_cnt.end())
			//	num_ins_exe_cnt[iWindow->Nth(i)->getInsAddr()] += 1;
			//else
			//	num_ins_exe_cnt[iWindow->Nth(i)->getInsAddr()] = 1;
			//if (num_bypassed_wbb.find(iWindow->Nth(i)->getInsAddr()) != num_bypassed_wbb.end())
			//	//num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] += list_of_wbb.size(); //static ins
			//	num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] += wbb_bypass_count; //dynamic ins
			//else
			//	//num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] = list_of_wbb.size(); //static ins
			//	num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] = wbb_bypass_count; //dynamic ins
			//if (ins_exe_hoist_accuracy.find(iWindow->Nth(i)->getInsAddr()) != ins_exe_hoist_accuracy.end())
			//	ins_exe_hoist_accuracy[iWindow->Nth(i)->getInsAddr()] += br_chain_accuracy;
			//else
			//	ins_exe_hoist_accuracy[iWindow->Nth(i)->getInsAddr()] = br_chain_accuracy;
			/*-----STAT-----*/
			iWindow->Nth(i)->setStatus(ready,-1,-1);
			int indx = findBestResStation();
			iResStations[indx]->Append(iWindow->Nth(i));
			//iWinWalkIndx = i;
		}
	}
	//quicksortInsList(iResStation,0,iResStation->NumElements()-1);
	//quicksortInsList(iMemBuf,0,iMemBuf->NumElements()-1);
	//printf("size of iROB        = %d\n", iROB->NumElements());
	//printf("size of iWindow     = %d\n", iWindow->NumElements());
	//printf("size of iResStation = %d\n", iResStation->NumElements());
	//printf("size of iMemBuf     = %d\n", iMemBuf->NumElements());
	//Updaate the fetch list
	for (int i = iWindow->NumElements()-1; i >= 0; i--) {
		if (iWindow->Nth(i)->getStatus() == ready)
			iWindow->RemoveAt(i);
	}
	/*-----STAT-----*/
	iWinSize      += iWindow->NumElements();
	iROBSize      += iROB->NumElements();
	for (int i = 0; i < NUM_FUNC_UNIT; i++)
		iResStnsSize[i] += iResStations[i]->NumElements();
	/*-----STAT-----*/
	//Check resource availability & execute ins
	for (int j = 0; j < NUM_FUNC_UNIT; j++) {
		if (aluAvail[j]==true) {
			if (iResStations[j]->NumElements() > 0 &&
			    iResStations[j]->Nth(0)->getStatus() == ready) {
				aluAvail[j] = false;
				instruction* ins = iResStations[j]->Nth(0);
				if (ins->getType() == MEM && ins->getMemType() == READ) {
					if (memoryModel == NAIVE_SPECUL) {
							executeIns(ins, cycle, LONG_LATENCY);
					} else {
						long int insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
						if (inFlightLDops.count(insAddr) > 0) {//Non-blocking Load Op
							int tempLat = inFlightLDops.find(insAddr)->second - cycle;
							executeIns(ins, cycle, tempLat);
							getLatency(0,tempLat); //Generate hit latency stat
						} else {
							int latency = findLatency(ins);
							executeIns(ins, cycle, latency);
							int completeCycle = cycle+latency;
							insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
							inFlightLDops.insert(pair<long int,int>(insAddr,completeCycle));
						}
					}
					aluFreeTime[j] = cycle+1;//non-blocking LD/ST
				} else if (ins->getType() == MEM && ins->getMemType() == WRITE) {
					if (memoryModel == NAIVE_SPECUL) {
						executeIns(ins, cycle, ST_LATENCY);
					} else {
						int latency = findLatency(ins);
						executeIns(ins, cycle, latency);
					}
					aluFreeTime[j] = cycle+1;//non-blocking LD/ST
				} else if (ins->getType() == ALU || 
						   ins->getType() == BR  || 
						   ins->getType() == ASSIGN || 
						   ins->getType() == FPU) {
					int latency = findLatency(ins);
					executeIns(ins, cycle, latency);
					aluFreeTime[j] = cycle+latency;
				} else {
					Assert(ins->getType() == MEM || ins->getType() == ALU ||
					       ins->getType() == BR  || ins->getType() == FPU ||
					       ins->getType() == BR  || ins->getType() == FPU);
				}
				iResStations[j]->RemoveAt(0);
			}
		}
	}
	/*-----STAT-----*/
	inFlightLDopsSize += inFlightLDops.size();
	/*-----STAT-----*/
}


/***************************************
 * STRAND Core implementation
 ***************************************/
long long int total_num_strand_ins = 0;
long long int dyn_num_strands = 0;
void createStrand(instruction *ins, int indx, int iWinIndx, List<instruction*> *iWindow) {
	bool foundIns = false;
	bool strand_formed = false;
	instruction *prevIns = ins;
	if (ins->getDependents()->NumElements() != 1)
		return;
	do {
		foundIns = false;
		prevIns = ins;
		ins = ins->getDependents()->Nth(0);
		if (!(ins->getDependents()->NumElements() == 1 && 
		      ins->getNumAncestors() == 1 && 
			  ins->getNthAncestor(0)->getInsID() == prevIns->getInsID()))// &&
			  //upldMissRateProfileMap.find(ins->getInsAddr()) ==  upldMissRateProfileMap.end()))
			  //ins->getType() != MEM))
			break;
		for (int i = iWinIndx; i < iWindow->NumElements(); i++) {
			if (ins->getInsID() == iWindow->Nth(i)->getInsID()) {
				foundIns = true;
				break;
			}
		}
		if (foundIns) {
			//printf("YAAY %d, ", ins->getInsID());
			ins->setStatus(chain,-1,-1);
			iResStations[indx]->Append(ins);
			strand_formed = true;
			total_num_strand_ins++;
		} else {
			break;
		}
	} while (1);
	if (strand_formed) /* strand has a min length of 2 */
		dyn_num_strands++;
}

void runStrandcore(int cycle) {
	//Cosntruct the ready list
	//int iWinWalkIndx = -1;
	//int wbb_bypass_count = 0, br_bypass_count = 0;
	//float wbb_chain_accuracy = 1.0, br_chain_accuracy = 1.0;
	set<long int> list_of_wbb;
	for (int i = 0; i < iWindow->NumElements(); i++) {
		/*-----STAT-----*/
		//if (iWindow->Nth(i)->getType() == BR
		//    && iWindow->Nth(i)->isReady(cycle) == false
		//	&& iWindow->Nth(i)->getBrBias() >= 0.05 
		//	&& iWindow->Nth(i)->getBrBias() <= 0.95) {
		//	list_of_wbb.insert(iWindow->Nth(i)->getInsAddr());
		//	wbb_bypass_count++;
		//	wbb_chain_accuracy *= iWindow->Nth(i)->getBrAccuracy();
		//}
		//if (iWindow->Nth(i)->getType() == BR) {
		//	br_bypass_count++;
		//	br_chain_accuracy *= iWindow->Nth(i)->getBrAccuracy();
		//}
		/*-----STAT-----*/
		if (iWindow->Nth(i)->isReady(cycle) == true) {
			/*-----STAT-----*/
			////if (iWindow->Nth(i)->getInsAddr() == 4214686) { //bzip
			//if (iWindow->Nth(i)->getInsAddr() == 4248075) { //hmmer
			//	fprintf(junk,"\n\n");
			//	collect_stat = true;
			////} else if (iWindow->Nth(i)->getInsAddr() == 4214340 ||
			////           iWindow->Nth(i)->getInsAddr() == 4214683) { //bzip2
			//} else if ((iWindow->Nth(i)->getInsAddr() == 4248005 && 
			//			i+1 < iWindow->NumElements() &&
			//            iWindow->Nth(i+1)->getInsAddr() != 4248075)||
			//           iWindow->Nth(i)->getInsAddr() == 4248071) { //hmmer
			//	collect_stat = false;
			//}
			//if (collect_stat == true) {
			//	fprintf(junk,"%ld, ", iWindow->Nth(i)->getInsAddr());
			//}
			//if (num_ins_exe_cnt.find(iWindow->Nth(i)->getInsAddr()) != num_ins_exe_cnt.end())
			//	num_ins_exe_cnt[iWindow->Nth(i)->getInsAddr()] += 1;
			//else
			//	num_ins_exe_cnt[iWindow->Nth(i)->getInsAddr()] = 1;
			//if (num_bypassed_wbb.find(iWindow->Nth(i)->getInsAddr()) != num_bypassed_wbb.end())
			//	//num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] += list_of_wbb.size(); //static ins
			//	num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] += wbb_bypass_count; //dynamic ins
			//else
			//	//num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] = list_of_wbb.size(); //static ins
			//	num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] = wbb_bypass_count; //dynamic ins
			//if (ins_exe_hoist_accuracy.find(iWindow->Nth(i)->getInsAddr()) != ins_exe_hoist_accuracy.end())
			//	ins_exe_hoist_accuracy[iWindow->Nth(i)->getInsAddr()] += br_chain_accuracy;
			//else
			//	ins_exe_hoist_accuracy[iWindow->Nth(i)->getInsAddr()] = br_chain_accuracy;
			/*-----STAT-----*/
			//iWindow->Nth(i)->setStatus(ready,-1,-1);
			//if      (iWindow->Nth(i)->getType() == ALU ||
			//         iWindow->Nth(i)->getType() == FPU ||
			//         iWindow->Nth(i)->getType() == BR) {
			//	iResStation->Append(iWindow->Nth(i));
			//} else if  (iWindow->Nth(i)->getType() == MEM) {
			//	iMemBuf->Append(iWindow->Nth(i));
			//} else {
			//	Assert (iWindow->Nth(i)->getType() == MEM || 
			//	        iWindow->Nth(i)->getType() == BR  ||
			//	        iWindow->Nth(i)->getType() == ALU ||
			//	        iWindow->Nth(i)->getType() == FPU);
			//}
			//iWinWalkIndx = i;
			iWindow->Nth(i)->setStatus(ready,-1,-1);
			int indx = findBestResStation();
			iResStations[indx]->Append(iWindow->Nth(i));
			createStrand(iWindow->Nth(i), indx, i+1, iWindow);
			//printf("\n>> ");
			//iWinWalkIndx = i;
		}
	}
	//quicksortInsList(iResStation,0,iResStation->NumElements()-1);
	//quicksortInsList(iMemBuf,0,iMemBuf->NumElements()-1);
	//printf("size of iROB        = %d\n", iROB->NumElements());
	//printf("size of iWindow     = %d\n", iWindow->NumElements());
	//printf("size of iResStation = %d\n", iResStation->NumElements());
	//printf("size of iMemBuf     = %d\n", iMemBuf->NumElements());
	//Updaate the fetch list
	for (int i = iWindow->NumElements()-1; i >= 0; i--) {
		if (iWindow->Nth(i)->getStatus() == ready) {
			iWindow->RemoveAt(i);
		} else if (iWindow->Nth(i)->getStatus() == chain) {
			iWindow->RemoveAt(i);
		}
	}
	for (int i = 0; i < NUM_FUNC_UNIT; i++) {
		//printf("(%d, %d), ", iResStations[i]->Nth(0)->getInsID(), iResStations[i]->Nth(0)->getNumAncestors());
		for (int j = 0; j < iResStations[i]->NumElements(); j++) {
			if (iResStations[i]->Nth(j)->isReady(cycle) &&
			    iResStations[i]->Nth(j)->getStatus() == chain) {
				iResStations[i]->Nth(j)->setStatus(ready, -1, -1);
			}
		}
	}
	//printf("\n");
	/*-----STAT-----*/
	iWinSize      += iWindow->NumElements();
	iROBSize      += iROB->NumElements();
	for (int i = 0; i < NUM_FUNC_UNIT; i++)
		iResStnsSize[i] += iResStations[i]->NumElements();
	/*-----STAT-----*/
	//Check resource availability & execute ins
	for (int j = 0; j < NUM_FUNC_UNIT; j++) {
		if (aluAvail[j]==true) {
			if (iResStations[j]->NumElements() > 0 &&
			    iResStations[j]->Nth(0)->getStatus() == ready) {
				aluAvail[j] = false;
				instruction* ins = iResStations[j]->Nth(0);
				if (ins->getType() == MEM && ins->getMemType() == READ) {
					long int insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
					if (inFlightLDops.count(insAddr) > 0) {//Non-blocking Load Op
						int tempLat = inFlightLDops.find(insAddr)->second - cycle;
						executeIns(ins, cycle, tempLat);
						getLatency(0,tempLat); //Generate hit latency stat
					} else {
						int latency = findLatency(ins);
						executeIns(ins, cycle, latency);
						int completeCycle = cycle+latency;
						insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
						inFlightLDops.insert(pair<long int,int>(insAddr,completeCycle));
					}
					aluFreeTime[j] = cycle+1;//non-blocking LD/ST
				} else if (ins->getType() == MEM && ins->getMemType() == WRITE) {
					int latency = findLatency(ins);
					executeIns(ins, cycle, latency);
					aluFreeTime[j] = cycle+1;//non-blocking LD/ST
				} else if (ins->getType() == ALU || 
						   ins->getType() == BR  || 
						   ins->getType() == ASSIGN || 
						   ins->getType() == FPU) {
					int latency = findLatency(ins);
					executeIns(ins, cycle, latency);
					aluFreeTime[j] = cycle+latency;
				} else {
					Assert(ins->getType() == MEM || ins->getType() == ALU ||
					       ins->getType() == BR  || ins->getType() == FPU ||
					       ins->getType() == BR  || ins->getType() == FPU);
				}
				iResStations[j]->RemoveAt(0);
			}
		}
	}
	/*-----STAT-----*/
	inFlightLDopsSize += inFlightLDops.size();
	/*-----STAT-----*/
}

/***************************************
 * PHRASEBLOCK Core implementation
 ***************************************/
void runPhraseblockCore(int cycle, List<instruction*>** pbLists) {
	//Cosntruct the ready list
	int iWinWalkIndx[NUM_PHRASEBLKS];
	for (int j = 0; j < NUM_PHRASEBLKS; j++)
		iWinWalkIndx[j] = -1;
	for (int j = 0; j < NUM_PHRASEBLKS; j++) {
		List<instruction*>* pbList = pbLists[j];
		for (int i = 0; i < pbList->NumElements(); i++) { 
			instruction* ins = pbList->Nth(i);
			if (ins->isReady(cycle) == true) {
				ins->setStatus(ready,-1,-1);
				//int indx = j/2;
				int indx = findBestResStation();
				iResStations[indx]->Append(ins);
				//createStrand(ins, indx, i+1, pbList);
				iWinWalkIndx[j] = i;
			} else {
				break; //in-order issue dependency
			}
		}
	}
	//Updaate the fetch list
	for (int j = 0; j < NUM_PHRASEBLKS; j++) {
		List<instruction*>* pbList = pbLists[j];
		//for (int i = iWinWalkIndx[j]; i >= 0; i--) {
		for (int i = pbLists[j]->NumElements()-1; i >= 0; i--) {
			if (pbList->Nth(i)->getStatus() == ready) {
				pbList->RemoveAt(i);
			} else if (pbList->Nth(i)->getStatus() == chain) {
				pbList->RemoveAt(i);
			}
		}
		/*-----STAT-----*/
		if (iWinWalkIndx[j] != -1) activeBuffCnt++;
		/*-----STAT-----*/
	}
	for (int i = 0; i < NUM_FUNC_UNIT; i++) {
		for (int j = 0; j < iResStations[i]->NumElements(); j++) {
			if (iResStations[i]->Nth(j)->isReady(cycle) &&
			    iResStations[i]->Nth(j)->getStatus() == chain) {
				iResStations[i]->Nth(j)->setStatus(ready, -1, -1);
			}
		}
	}
	/*-----STAT-----*/
	iROBSize      += iROB->NumElements();
	for (int i = 0; i < NUM_FUNC_UNIT; i++)
		iResStnsSize[i] += iResStations[i]->NumElements();
	for (int i = 0; i < NUM_PHRASEBLKS; i++)
		pbListsSize[i] += pbLists[i]->NumElements();
	/*-----STAT-----*/
	//Check resource availability & execute ins
	for (int j = 0; j < NUM_FUNC_UNIT; j++) {
		if (aluAvail[j]==true) {
			if (iResStations[j]->NumElements() > 0 &&
			    iResStations[j]->Nth(0)->getStatus() == ready) {
				aluAvail[j] = false;
				instruction* ins = iResStations[j]->Nth(0);
				if (ins->getType() == MEM && ins->getMemType() == READ) {
					long int insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
					if (memoryModel == NAIVE_SPECUL) {
							executeIns(ins, cycle, LONG_LATENCY);
					} else {
						//if (inFlightLDops.count(insAddr) > 0) {//Non-blocking Load Op
						if (inFlightLDops.find(insAddr)->second >= cycle) {//Non-blocking Load Op
							int tempLat = inFlightLDops.find(insAddr)->second - cycle;
							executeIns(ins, cycle, tempLat);
							getLatency(0,tempLat); //Generate hit latency stat
						} else {
							int latency = findLatency(ins);
							executeIns(ins, cycle, latency);
							int completeCycle = cycle+latency;
							insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
							inFlightLDops.insert(pair<long int,int>(insAddr,completeCycle));
						}
					}
					aluFreeTime[j] = cycle+1;//non-blocking LD/ST
				} else if (ins->getType() == MEM && ins->getMemType() == WRITE) {
					if (memoryModel == NAIVE_SPECUL) {
						executeIns(ins, cycle, ST_LATENCY);
					} else {
						int latency = findLatency(ins);
						executeIns(ins, cycle, latency);
					}
					aluFreeTime[j] = cycle+1;//non-blocking LD/ST
				} else if (ins->getType() == ALU || 
						   ins->getType() == BR  || 
						   ins->getType() == ASSIGN || 
						   ins->getType() == FPU) {
					int latency = findLatency(ins);
					executeIns(ins, cycle, latency);
					aluFreeTime[j] = cycle+latency;
					if(ins->getType() == BR && branchMode == dynPredBr) {
						brInsCount++;
						br_pred_update_dist += cycle-br_pred_update_distance[ins->getInsAddr()];
						br_pred_update_distance.erase(ins->getInsAddr());
					}
				} else {
					Assert(ins->getType() == MEM || ins->getType() == ALU ||
					       ins->getType() == BR  || ins->getType() == FPU ||
					       ins->getType() == BR  || ins->getType() == FPU);
				}
				iResStations[j]->RemoveAt(0);
			}
		}
	}
	/*-----STAT-----*/
	inFlightLDopsSize += inFlightLDops.size();
	/*-----STAT-----*/
}



/***************************************
 * X LEVEL DEEP Core Implementation
 ***************************************/
bool xLevelDeepEnable = false;
void runxLDcore_DYN(int cycle) {
	//Cosntruct the ready list
	int iWinWalkIndx = -1;
	int *iSideBfWalkIndx = new int [numSideBuffs];


	//if(cycle > 10) printf("0)iWin/iSB/RS size = %d,%d,%d,|%s|,%d,%d|%d\n", iWindow->NumElements(), iSideBuff[0]->NumElements(),iResStation->NumElements(),iResStation->Nth(0)->getCmdStr(),iResStation->Nth(0)->getInsID(),iResStation->Nth(0)->getType(),cycle);

	//bool aSBdraining = false;
	/*-----STAT-----*/
	for (int i = 0; i < numSideBuffs; i++) {
		if (iSideBuff[i]->isFree()==true) SBoffCycles[i]++;
	}
	for (int i = 0; i < numSideBuffs; i++) {
		if (iSideBuff[i]->isFree()==false) {
			if (iSideBuff[i]->getExpiration() <= cycle) {
				//aSBdraining = true;
				break;
			} else {
				SBactiveCycles[i]++;
				totSBactiveCycles++;
			}
		}
	}
	/*-----STAT-----*/

	//Assign instructions to the right buffer
	//if (aSBdraining == false) {
		for (int i = 0; i < iWindow->NumElements(); i++) {
			if (iWindow->Nth(i)->isGotoSideBuff() == true &&
			    iSideBuff[iWindow->Nth(i)->getSideBuffNum()]->isFree() == false &&
			    iSideBuff[iWindow->Nth(i)->getSideBuffNum()]->getExpiration() > cycle) {
				//printf("BS: %d,  ", iWindow->Nth(i)->getType());
				iWindow->Nth(i)->setStatus(sideBuffer,-1,-1);
				int indx = iWindow->Nth(i)->getSideBuffNum();
				//if (iWindow->Nth(i)->getMemAddr() == 140735928907336) printf("GOT in SB %s, cycle=%d\n",iWindow->Nth(i)->getCmdStr(),cycle);
				iSideBuff[indx]->Append(iWindow->Nth(i));
				iWinWalkIndx = i;
				/*-----STAT-----*/
				totInsVisitingSBcount++;
				insVisitingSBcount[indx]++;
				/*-----STAT-----*/
			} else {
		       		//Transfer to Reservation Station
		       		if (iWindow->Nth(i)->isReady(cycle) == true) {
					//printf("%d,  ", iWindow->Nth(i)->getType());
		       			iWindow->Nth(i)->setStatus(ready,-1,-1);
				//if (iWindow->Nth(i)->getMemAddr() == 140735928907336) printf("GOT in RS %s, cycle=%d\n",iWindow->Nth(i)->getCmdStr(),cycle);
		       			iResStation->Append(iWindow->Nth(i));
		       			iWinWalkIndx = i;
		       		} else {
		       			//depFailiniWin++;
		       			break; //in-order issue dependency
		       		}
			}
		}
	//}
	//Clear the iWindow

	for (int indx = 0; indx < SBpriorityList->NumElements(); indx++) {
		int sb = SBpriorityList->Nth(indx);
		iSideBfWalkIndx[sb]=-1; //TODO verify correctness of this line
		if (iSideBuff[sb]->isFree()==false && iSideBuff[sb]->getExpiration() <= cycle) {//Drain SB
			if (iSideBuff[sb]->getExpiration() == cycle) numOnSideBuffs--;
			if (iSideBuff[sb]->NumElements() > 0) {
				for (int i = 0; i < iSideBuff[sb]->NumElements(); i++) {
					if (iSideBuff[sb]->Nth(i)->isReady(cycle) == true) {
						//TODO how many dependencies instructions 
						//have at this point? 1 hopefully?
						iSideBuff[sb]->Nth(i)->setStatus(sideReady,-1,-1);
						//if (iSideBuff[sb]->Nth(i)->getMemAddr() == 140735928907336) printf("GOT in SB1%s, cycle=%d, size=%d\n",iSideBuff[sb]->Nth(i)->getCmdStr(),cycle,iSideBuff[sb]->NumElements());
						iResStation->Append(iSideBuff[sb]->Nth(i));
						//iWinWalkIndx = -1;
						iSideBfWalkIndx[sb] = i;
					} else {
						//depFailinSideBuf++;
						break; //in-order side buff
					}
				}
			} else {
				iSideBuff[sb]->setFree();
				iSideBuff[sb]->setExpiration(-1);
				SBpriorityList->RemoveAt(indx);
			}
		}
	}

	//Transfer to Reservation Station
	/*-----STAT-----*/
	if (numOnSideBuffs == 0) {totSBoffCycles++;}
	/*-----STAT-----*/
	//Updaate the fetch list
	//printf("1)iWin/iSB/RS size = %d,%d,%d,|%s|,%d,%d|%d\n", iWindow->NumElements(), iSideBuff[0]->NumElements(),iResStation->NumElements(),iResStation->Nth(0)->getCmdStr(),iResStation->Nth(0)->getInsID(),iResStation->Nth(0)->getType(),cycle);
	if (iWinWalkIndx >=0 ) {
	       for (int i = iWinWalkIndx; i >= 0; i--) {
	       	if (iWindow->Nth(i)->getStatus() == ready ||
	       	    iWindow->Nth(i)->getStatus() == sideBuffer)
	       		iWindow->RemoveAt(i);
	       }
	}
	//printf("here\n");
	for (int indx = 0; indx < SBpriorityList->NumElements(); indx++) {
		int sb = SBpriorityList->Nth(indx);
		//printf("sb = %d,%d\n",sb,iSideBfWalkIndx[sb]);
		if (iSideBfWalkIndx[sb] >= 0 && iSideBuff[sb]->NumElements()!=0) {
		       for (int i = iSideBuff[sb]->NumElements()-1; i >= 0; i--) {
				if (iSideBuff[sb]->Nth(i)->getStatus() == sideReady)
					iSideBuff[sb]->RemoveAt(i);
		       }
		}
		/*-----STAT-----*/
		iSideBufSize  += iSideBuff[sb]->NumElements();
		/*-----STAT-----*/
	}
	//printf("2)iWin/iSB/RS size = %d,%d,%d,|%s|,%d,%d|%d\n", iWindow->NumElements(), iSideBuff[0]->NumElements(),iResStation->NumElements(),iResStation->Nth(0)->getCmdStr(),iResStation->Nth(0)->getInsID(),iResStation->Nth(0)->getType(),cycle);
	
	//printf("iWin/iSB/RS size = %d,%d,%d,%s|%s|%s,%d\n", iWindow->NumElements(), iSideBuff->NumElements(),iResStation->NumElements(),iWindow->Nth(0)->getCmdStr(),iSideBuff[0].Nth(0)->getCmdStr(),iSideBuff[1].Nth(0)->getCmdStr(),  iResStation->Nth(0)->getStatus());
	//printf("iWin/iSB/RS size = %d,%d,%d,|%s|,%d\n", iWindow->NumElements(), iSideBuff->NumElements(),iResStation->NumElements(),iResStation->Nth(0)->getCmdStr(),iResStation->Nth(0)->getType());
	//printf("%d, %d, %d, %d\n", iWindow->NumElements(), iSideBuff[0]->NumElements(),iSideBuff[1]->NumElements(),iResStation->NumElements());
	/*-----STAT-----*/
	if (iResStation->NumElements() == 0) emptyResStation++;
	iWinSize      += iWindow->NumElements();
	iROBSize      += iROB->NumElements();
	iResStnSize   += iResStation->NumElements();
	Assert(iSideBuff[0]->NumElements() >= 0 && 
	      iWindow->NumElements() >= 0 &&
	      iROB->NumElements() >= 0 &&
	      iResStation->NumElements() >= 0);
	/*-----STAT-----*/
	//Check resource availability & execute ins
	while (iResStation->NumElements() > 0) {
	//printf("3)iWin/iSB/RS size = %d,%d,%d,|%s|,%d,%d|%d\n", iWindow->NumElements(), iSideBuff[0]->NumElements(),iResStation->NumElements(),iResStation->Nth(0)->getCmdStr(),iResStation->Nth(0)->getInsID(),iResStation->Nth(0)->getType(),cycle);
	       int prevInsCount = insCount;
	       for (int i = 0; i < NUM_FUNC_UNIT; i++) {
			if (aluAvail[i]==true) {
				//printf("size of iResStation = %d\n", iResStation->NumElements());
				if (aluKind[i] == MEM && iResStation->Nth(0)->getType() == MEM) {
					//Implementing two MEM issue per cycle (two mem units)
					aluAvail[i] = false;
					instruction* ins = iResStation->Nth(0);
					if (ins->getMemType() == READ) {
						long int insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
						int latency = 0;
						int completeCycle = 0;
						//Non-blocking Load Op
						if (inFlightLDops.count(insAddr) > 0) {
							latency = inFlightLDops.find(insAddr)->second - cycle;
							executeIns(ins, cycle, latency);
							getLatency(0,latency); //Gen hit latency stat
							completeCycle = cycle+latency;
						} else {
							latency = findLatency(ins);
							executeIns(ins, cycle, latency);
							completeCycle = cycle+latency;
							insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
							inFlightLDops.insert(pair<long int,int>(insAddr,completeCycle));
						}
						//X Level Deep Activation
						Assert (latency > 0 && completeCycle > 0);
						if (latency > xLevDeepLatLevel) {
							//Find ONE free side-buffer
							if (ins->getStatus() == sideReady) {
								int sb = ins->getSideBuffNum();
								iSideBuff[sb]->setBusy();
								numOnSideBuffs++;
								/*-----STAT-----*/
								totNumSBactivations++;
								numSBactivations[sb]++;
								/*-----STAT-----*/
								for (int i = 0; i < SBpriorityList->NumElements(); i++) {
									if (SBpriorityList->Nth(i) == sb) {
										SBpriorityList->RemoveAt(i);
										/*-----STAT-----*/
										totNumSBreactivations++;
										numSBreactivations[sb]++;
										/*-----STAT-----*/
										break;
									}
								}
								ins->notifyAllDepGoToSideBuff(sb,ins->getInsID(),numSideBuffs);
								iSideBuff[sb]->setExpiration(completeCycle);
								SBpriorityList->Append(sb);//SB Access Prioriyt list
								Assert(sb >= 0 && sb < numSideBuffs);
								Assert(SBpriorityList->NumElements() >= 0 &&
								       SBpriorityList->NumElements() <= numSideBuffs);
							} else {
								for (int sb = 0; sb < numSideBuffs; sb++) {
									if (iSideBuff[sb]->isFree() == true) {
										iSideBuff[sb]->setBusy();
										numOnSideBuffs++;
										/*-----STAT-----*/
										totNumSBactivations++;
										numSBactivations[sb]++;
										/*-----STAT-----*/
										ins->notifyAllDepGoToSideBuff(sb,ins->getInsID(),numSideBuffs);
										iSideBuff[sb]->setExpiration(completeCycle);
										SBpriorityList->Append(sb);//SB Access Prioriyt list
										Assert(sb >= 0 && sb < numSideBuffs);
										Assert(SBpriorityList->NumElements() >= 0 &&
										       SBpriorityList->NumElements() <= numSideBuffs);
										break;
									}
								}
							}
						}
					} else if (ins->getMemType() == WRITE) {
						//printf("mem write %d\n", ins->getInsID());
						int latency = findLatency(ins);
						executeIns(ins, cycle, latency);
					} else {
						Assert(ins->getMemType() == WRITE || ins->getMemType() == READ);
					}
					aluFreeTime[i] = cycle+1;//non-blocking LD/ST
					//printf("mem removed %d\n", ins->getInsID());
					iResStation->RemoveAt(0);
					/*-----STAT-----*/
					for (int i = 0; i < numSideBuffs; i++) {
						if (iSideBuff[i]->isFree()==false && iSideBuff[i]->getExpiration() > cycle) {
							InsCountWhenSBon[i]++;
							totInsCountWhenSBon++;
						}
					}
					/*-----STAT-----*/
					break;
				} else if (aluKind[i] == ALU && 
					   (iResStation->Nth(0)->getType() == ALU ||
					   iResStation->Nth(0)->getType() == FPU)) {
					aluAvail[i] = false;
					instruction* ins = iResStation->Nth(0);
					int latency = findLatency(ins);
					executeIns(ins, cycle, latency);
					aluFreeTime[i] = cycle+latency;
					//printf("removed %d\n", ins->getInsID());
					iResStation->RemoveAt(0);
					/*-----STAT-----*/
					for (int i = 0; i < numSideBuffs; i++) {
						if (iSideBuff[i]->isFree()==false && iSideBuff[i]->getExpiration() > cycle) {
							InsCountWhenSBon[i]++;
							totInsCountWhenSBon++;
						}
					}
					/*-----STAT-----*/
					break;
				}
			}
	       }
	       if (insCount-prevInsCount == 0) {break;} //in-order issue struct hazard
	}
	/*-----STAT-----*/
	inFlightLDopsSize += inFlightLDops.size();
	/*-----STAT-----*/
	//printf("5)iWin/iSB/RS size = %d,%d,%d,|%s|,%d,%d|%d\n", iWindow->NumElements(), iSideBuff[0]->NumElements(),iResStation->NumElements(),iResStation->Nth(0)->getCmdStr(),iResStation->Nth(0)->getInsID(),iResStation->Nth(0)->getType(),cycle);
	delete [] iSideBfWalkIndx;
}


/***************************************
 * X LEVEL DEEP STATIC Core Implementation
 ***************************************/
int mainStreamInsCount = 0;
int mainStreamBound = -1;
List<int> *delList = new List<int>;
List<int> *SBlist = new List<int>;
int numActiveSideBuffs = 1;
int drainedSBcount = 0;

void runxLD_STAT(int cycle) {
	Assert (ROB_SIZE >= 300);
	Assert (numSideBuffs <= 2 && numSideBuffs > 0);
	Assert (numActiveSideBuffs <= numSideBuffs);
	int iWinWalkIndx = -1;
	int *iSideBfWalkIndx = new int [numSideBuffs];
	for (int j = 0; j < numActiveSideBuffs; j++)
		iSideBfWalkIndx[j] = -1;

	//bool aSBdraining = false;
	/*-----STAT-----*/
	//for (int i = 0; i < numSideBuffs; i++) {
	//	if (iSideBuff[i]->isFree()==true) SBoffCycles[i]++;
	//}
	//for (int i = 0; i < numSideBuffs; i++) {
	//	if (iSideBuff[i]->isFree()==false) {
	//		if (mainStreamInsCount >= mainStreamBound) {
	//			aSBdraining = true;
	//			break;
	//		} else {
	//			SBactiveCycles[i]++;
	//			totSBactiveCycles++;
	//		}
	//	}
	//}
	/*-----STAT-----*/

	//Clear the iWindow & other queues
	if (iSideBuff[numActiveSideBuffs-1]->isFree() == false && mainStreamInsCount >= mainStreamBound) {//Drain SB
		Assert(mainStreamBound != -1);
		//if (mainStreamInsCount == mainStreamBound) numOnSideBuffs--;
		for (int j = 0; j < numActiveSideBuffs; j++) {
			if (iSideBuff[j]->NumElements() > 0) {
				for (int i = 0; i < iSideBuff[j]->NumElements(); i++) {
					if (iSideBuff[j]->Nth(i)->isReady(cycle) == true) {
						//TODO how many dependencies instructions 
						//have at this point? 1 hopefully?
						iSideBuff[j]->Nth(i)->setStatus(sideReady,-1,-1);
						iResStation->Append(iSideBuff[j]->Nth(i));
						iSideBfWalkIndx[j] = i;
					} else {
						break; //in-order side buff ins issue
					}
				}
			}
			if (iSideBfWalkIndx[j] >= 0) {
			       for (int i = iSideBfWalkIndx[j]; i >= 0; i--) {
					if (iSideBuff[j]->Nth(i)->getStatus() == sideReady)
						iSideBuff[j]->RemoveAt(i);
			       }
			}
			if (iSideBuff[j]->NumElements() > 0)
				break; //in-order SB drain sequence
		}
		bool allSBdrained = true;
		for (int j = 0; j < numActiveSideBuffs; j++) {
			if (iSideBuff[j]->NumElements() > 0) {
				allSBdrained = false;
				break;
			}
		}
		//if (drainedSBcount == numActiveSideBuffs) {allSBdrained = true; printf("hi\n");}
		//else allSBdrained = false;
		if (allSBdrained == true) {
			//Free all SB's together
			for (int j = 0; j < numActiveSideBuffs; j++)
				iSideBuff[j]->setFree();
			numActiveSideBuffs = 1; //default
			mainStreamBound = -1;
			//drainedSBcount = 0;
		}
	} else {
		printf("%d,%d,%d,%d,%d\n",cycle,numActiveSideBuffs,mainStreamInsCount,mainStreamBound,iSideBuff[numActiveSideBuffs-1]->isFree());
		for (int i = 0; i < iWindow->NumElements(); i++) {
		       	//Transfer to Reservation Station
		       	if (iWindow->Nth(i)->isReady(cycle) == true) {
		       		iWindow->Nth(i)->setStatus(ready,-1,-1);
		       		iResStation->Append(iWindow->Nth(i));
		       		iWinWalkIndx = i;
		       	} else {
		       		break; //in-order issue dependency
		       	}
		}
	}

	/*-----STAT-----*/
	if (numOnSideBuffs == 0) {totSBoffCycles++;}
	/*-----STAT-----*/
	//Updaate the lists
	if (iWinWalkIndx >=0 ) {
		for (int i = iWinWalkIndx; i >= 0; i--) {
		 	Assert(iWindow->Nth(i)->getStatus() != sideBuffer);
		 	if (iWindow->Nth(i)->getStatus() == ready)
		 		iWindow->RemoveAt(i);
		}
	}
	//for (int j = 0; j < numActiveSideBuffs; j++) {
	//	if (iSideBfWalkIndx[j] >= 0) {
	//	       for (int i = iSideBfWalkIndx[j]; i >= 0; i--) {
	//			if (iSideBuff[j]->Nth(i)->getStatus() == sideReady)
	//				iSideBuff[j]->RemoveAt(i);
	//	       }
	//	}
	//}

	/*-----STAT-----*/
	if (iResStation->NumElements() == 0) {emptyResStation++;}
	//else {
		printf("cycle/SB_status/msic/msb/sb %d, %d, %d, %d, %d, %d| ",cycle,iSideBuff[0]->isFree(),mainStreamInsCount,mainStreamBound, iSideBuff[1]->NumElements(),iWindow->Nth(0)->_guardian);
		printf("%lld,%d,%llu(%s)|%lld,%d,%llu (%s)|", iWindow->Nth(0)->_ancestors->Nth(0)->getInsID(),
							 iWindow->Nth(0)->_ancestors->Nth(0)->getStatus(),
							 iWindow->Nth(0)->_ancestors->Nth(0)->getMemAddr(),
							 iWindow->Nth(0)->_ancestors->Nth(0)->getCmdStr(),
							 //iWindow->Nth(0)->_ancestors->Nth(0)->getSideBuffNum(),
							 iWindow->Nth(0)->getInsID(),
							 iWindow->Nth(0)->getStatus(),
							 iWindow->Nth(0)->getMemAddr(),
							 iWindow->Nth(0)->getCmdStr());
		if (iSideBuff[0]->NumElements()>0) printf("%lld,%d\n",iSideBuff[0]->Nth(0)->getInsID(),iSideBuff[0]->NumElements());
		else printf("\n");
	//}
	iSideBufSize  += iSideBuff[0]->NumElements();
	iWinSize      += iWindow->NumElements();
	iROBSize      += iROB->NumElements();
	iResStnSize   += iResStation->NumElements();
	Assert(iSideBuff[0]->NumElements() >= 0 && 
	       iWindow->NumElements()      >= 0 &&
	       iROB->NumElements()         >= 0 &&
	       iResStation->NumElements()  >= 0);
	/*-----STAT-----*/

	//Check resource availability & execute ins
	while (iResStation->NumElements() > 0) {
	       int prevInsCount = insCount;
	       for (int i = 0; i < NUM_FUNC_UNIT; i++) {
			if (aluAvail[i]==true) {
				if (aluKind[i] == MEM && iResStation->Nth(0)->getType() == MEM) {
					//Implementing two MEM issue per cycle (two mem units)
					aluAvail[i] = false;
					instruction* ins = iResStation->Nth(0);
					if (ins->getMemType() == READ) {
						long int insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
						int latency = 0;
						int completeCycle = 0;
						//Non-blocking Load Op
						if (inFlightLDops.count(insAddr) > 0) {
							latency = inFlightLDops.find(insAddr)->second - cycle;
							executeIns(ins, cycle, latency);
							if (iSideBuff[numActiveSideBuffs-1]->isFree() == false) mainStreamInsCount++;
							getLatency(0,latency); //Gen hit latency stat
							completeCycle = cycle+latency;
						} else {
							latency = findLatency(ins);
							executeIns(ins, cycle, latency);
							if (iSideBuff[numActiveSideBuffs-1]->isFree() == false) mainStreamInsCount++;
							completeCycle = cycle+latency;
							insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
							inFlightLDops.insert(pair<long int,int>(insAddr,completeCycle));
						}
						//X Level Deep Staic Cnotingent Scheduling Activation
						Assert (latency > 0 && completeCycle > 0);
						if (latency > xLevDeepLatLevel) {
							for (int sb = 0; sb < numSideBuffs; sb++) {
								if (iSideBuff[sb]->isFree() == true) {
									iSideBuff[sb]->setBusy();
									numActiveSideBuffs = sb + 1;
									numOnSideBuffs++; //STAT
									ins->notifyAllDepGoToSideBuff(sb,ins->getInsID(),numSideBuffs);
									for (int m = 0; m < numSideBuffs; m++)
										if (m != sb && iSideBuff[m]->NumElements() > 0)
											iSideBuff[m]->Nth(0)->notifyAllDepGoToSideBuff(iSideBuff[m]->Nth(0)->getSideBuffNum(), iSideBuff[m]->Nth(0)->getCauseOfSBinsID(), numSideBuffs);
									//Move Dependent Ins's into SB
									//if (sb > 0) {
									//	for (int j = 1; j <= sb; j++) {
									//		iSideBfWalkIndx[sb-j] = -1;
									//		for (int i = 0; i < iSideBuff[sb-j]->NumElements() && iSideBuff[sb]->NumElements() < SBlength; i++) {
									//			if (iSideBuff[sb-j]->Nth(i)->getSideBuffNum() == sb) {
									//				iSideBuff[sb-j]->Nth(i)->setStatus(sideBuffer,-1,-1);
									//				iSideBuff[sb]->Append(iSideBuff[sb-j]->Nth(i));
									//				SBlist->Append(i);
									//				iSideBfWalkIndx[sb-j] = i;
									//			}
									//		}
									//		for (int i = iSideBfWalkIndx[sb-j]; i >= 0; i--) {
									//			if (iSideBuff[sb-j]->Nth(i)->getSideBuffNum() == sb) {
									//				iSideBuff[sb-j]->RemoveAt(i);
									//			}
									//			if (iSideBuff[sb-j]->NumElements() == 0) {
									//				drainedSBcount++;
									//			}
									//		}
									//	}
									//} else {
									//	Assert(iSideBuff[sb]->NumElements() == 0);
									//}
									iWinWalkIndx = -1;
									int SBtoSBNumElements = 0; //iSideBuff[sb]->NumElements();
									int i = 0;
									for (i = 0; i < iWindow->NumElements() && iSideBuff[sb]->NumElements() < SBlength; i++) {
										if (iWindow->Nth(i)->isGotoSideBuff() == true) {
											iWindow->Nth(i)->setStatus(sideBuffer,-1,-1);
											int s = iWindow->Nth(i)->getSideBuffNum();
											Assert(s >= 0 && s < numSideBuffs);
											iSideBuff[s]->Append(iWindow->Nth(i));
											SBlist->Append(i);
											iWinWalkIndx = i;
										}
									}
									/*-----STAT-----*/
									//if (i >= iWindow->NumElements() && iSideBuff[sb]->NumElements() < SBlength) {
									//	windowSatration++; //TODO make it an array
									//}
									/*-----STAT-----*/
									Assert ((iWinWalkIndx == -1 && iSideBuff[sb]->NumElements() == 0) || iWinWalkIndx != -1);
									Assert (iSideBuff[sb]->NumElements() >= SBtoSBNumElements);
									mainStreamInsCount = 0; //TODO verify moving this line to here is not wrong (with -c 5)
									if (iSideBuff[sb]->NumElements() == 0) 
										mainStreamBound = 0;
									else
										mainStreamBound = (iWinWalkIndx + 1) + (iResStation->NumElements() - 1) - (iSideBuff[sb]->NumElements() - SBtoSBNumElements);
									//Update iWindow
									//if (iWinWalkIndx >= 0) {scheduleInsInFlight(SBlist, delList, iWindow, iWinWalkIndx, cycle);}
									for (int m = iWinWalkIndx; m >= 0; m--)
										if (iWindow->Nth(m)->getStatus() == sideBuffer) iWindow->RemoveAt(m);

									//Fill up the SB
									Assert(sb >= 0 && sb < numSideBuffs);
									/*-----STAT-----*/
									//totNumSBactivations++;
									//numSBactivations[sb]++;
									//if (iWinWalkIndx >= 0) {
									//	totFrameSize   += (iWinWalkIndx + 1) + (iResStation->NumElements() - 1);
									//	totMainStreamBound += mainStreamBound;
									//}
									/*-----STAT-----*/
									for (int m = 0; m < numSideBuffs; m++)
										if (iSideBuff[m]->NumElements() > 0) {
											iSideBuff[m]->Nth(0)->notifyAllDepGetOutSideBuff(iSideBuff[m]->Nth(0)->getSideBuffNum(), iSideBuff[m]->Nth(0)->getCauseOfSBinsID(), numSideBuffs);
										}
									break;
								} else { //TODO should this be inside the for loop or ouside?
									/*-----STAT-----*/
									longLatOpWhenSPisON++;
									if (mainStreamInsCount >= mainStreamBound)
										longLatOpWhenSPisDraining++;
									else
										longLatOpWhenSPisWaiting++;
									/*-----STAT-----*/
								}
							}
						}
					} else if (ins->getMemType() == WRITE) {
						int latency = findLatency(ins);
						executeIns(ins, cycle, latency);
						if (iSideBuff[numActiveSideBuffs-1]->isFree() == false) mainStreamInsCount++;
					} else {
						Assert(ins->getMemType() == WRITE || ins->getMemType() == READ);
					}
					aluFreeTime[i] = cycle+1;//non-blocking LD/ST
					iResStation->RemoveAt(0);
					/*-----STAT-----*/
					//for (int i = 0; i < numSideBuffs; i++) {
					if (iSideBuff[0]->isFree()==false) { // && iSideBuff[i]->getExpiration() > cycle) {
						InsCountWhenSBon[0]++;
						totInsCountWhenSBon++;
					}
					//}
					/*-----STAT-----*/
					break;
				} else if (aluKind[i] == ALU && 
					   (iResStation->Nth(0)->getType() == ALU ||
					    iResStation->Nth(0)->getType() == FPU)) {
					aluAvail[i] = false;
					instruction* ins = iResStation->Nth(0);
					int latency = findLatency(ins);
					executeIns(ins, cycle, latency);
					if (iSideBuff[numActiveSideBuffs-1]->isFree() == false) mainStreamInsCount++;
					aluFreeTime[i] = cycle+latency;
					iResStation->RemoveAt(0);
					/*-----STAT-----*/
					//for (int i = 0; i < numSideBuffs; i++) {
					if (iSideBuff[0]->isFree()==false) { // && iSideBuff[i]->getExpiration() > cycle) {
						InsCountWhenSBon[0]++;
						totInsCountWhenSBon++;
					}
					//}
					/*-----STAT-----*/
					break;
				}
			}
	       }
	       if (insCount-prevInsCount == 0) {break;} //in-order issue struct hazard
	}
	/*-----STAT-----*/
	inFlightLDopsSize += inFlightLDops.size();
	/*-----STAT-----*/
	delete [] iSideBfWalkIndx;
}


/***************************************
 * 1 LEVEL DEEP Static Contingent Sch. Core Implementation
 ***************************************/
//int mainStreamInsCount = 0;
//int mainStreamBound = -1;
//List<int> *delList = new List<int>;
//List<int> *SBlist = new List<int>;
void run1LD_STAT(int cycle) {
	Assert (ROB_SIZE >= 300);
	//Overwrite num side buffs to be 1 always
	numSideBuffs = 1;
	int iWinWalkIndx = -1;
	int *iSideBfWalkIndx = new int [numSideBuffs];
	iSideBfWalkIndx[0]=-1;

	//bool aSBdraining = false;
	/*-----STAT-----*/
	for (int i = 0; i < numSideBuffs; i++) {
		if (iSideBuff[i]->isFree()==true) SBoffCycles[i]++;
	}
	for (int i = 0; i < numSideBuffs; i++) {
		if (iSideBuff[i]->isFree()==false) {
			if (mainStreamInsCount >= mainStreamBound) {
				//aSBdraining = true;
				break;
			} else {
				SBactiveCycles[i]++;
				totSBactiveCycles++;
			}
		}
	}
	/*-----STAT-----*/

	//Clear the iWindow & other queues
	if (iSideBuff[0]->isFree() == false && mainStreamInsCount >= mainStreamBound) {//Drain SB
		Assert(mainStreamBound != -1);
		if (mainStreamInsCount == mainStreamBound) numOnSideBuffs--;
		if (iSideBuff[0]->NumElements() > 0) {
			for (int i = 0; i < iSideBuff[0]->NumElements(); i++) {
				if (iSideBuff[0]->Nth(i)->isReady(cycle) == true) {
					//TODO how many dependencies instructions 
					//have at this point? 1 hopefully?
					iSideBuff[0]->Nth(i)->setStatus(sideReady,-1,-1);
					iResStation->Append(iSideBuff[0]->Nth(i));
					iSideBfWalkIndx[0] = i;
				} else {
					break; //in-order side buff
				}
			}
		} else {
			iSideBuff[0]->setFree();
			mainStreamInsCount = 0;
			mainStreamBound = -1;
		}
	} else {
		for (int i = 0; i < iWindow->NumElements(); i++) {
		       	//Transfer to Reservation Station
		       	if (iWindow->Nth(i)->isReady(cycle) == true) {
		       		iWindow->Nth(i)->setStatus(ready,-1,-1);
		       		iResStation->Append(iWindow->Nth(i));
		       		iWinWalkIndx = i;
		       	} else {
		       		break; //in-order issue dependency
		       	}
		}
	}

	/*-----STAT-----*/
	if (numOnSideBuffs == 0) {totSBoffCycles++;}
	/*-----STAT-----*/
	//Updaate the lists
	if (iWinWalkIndx >=0 ) {
		for (int i = iWinWalkIndx; i >= 0; i--) {
		 	Assert(iWindow->Nth(i)->getStatus() != sideBuffer);
		 	if (iWindow->Nth(i)->getStatus() == ready)
		 		iWindow->RemoveAt(i);
		}
	}
	if (iSideBfWalkIndx[0] >= 0) {
	       for (int i = iSideBfWalkIndx[0]; i >= 0; i--) {
			if (iSideBuff[0]->Nth(i)->getStatus() == sideReady)
				iSideBuff[0]->RemoveAt(i);
	       }
	}

	/*-----STAT-----*/
	if (iResStation->NumElements() == 0) {emptyResStation++;}
	//else {
		//printf("cycle/SB_status/msic/msb/sb %d, %d, %d, %d, %d, %d| ",cycle,iSideBuff[0]->isFree(),mainStreamInsCount,mainStreamBound, iSideBuff[0]->NumElements(),iWindow->Nth(0)->_guardian);
		//printf("%d,%d,%ld(%s),|%d,%d,%ld (%s)|", iWindow->Nth(0)->_ancestors->Nth(0)->getInsID(),
		//					 iWindow->Nth(0)->_ancestors->Nth(0)->getStatus(),
		//					 iWindow->Nth(0)->_ancestors->Nth(0)->getMemAddr(),
		//					 iWindow->Nth(0)->_ancestors->Nth(0)->getCmdStr(),
		//					 iWindow->Nth(0)->getInsID(),
		//					 iWindow->Nth(0)->getStatus(),
		//					 iWindow->Nth(0)->getMemAddr(),
		//					 iWindow->Nth(0)->getCmdStr());
		//if (iSideBuff[0]->NumElements()>0) printf("%d\n",iSideBuff[0]->Nth(0)->getInsID());
		//else printf("\n");
	//}
	iSideBufSize  += iSideBuff[0]->NumElements();
	iWinSize      += iWindow->NumElements();
	iROBSize      += iROB->NumElements();
	iResStnSize   += iResStation->NumElements();
	Assert(iSideBuff[0]->NumElements() >= 0 && 
	       iWindow->NumElements()      >= 0 &&
	       iROB->NumElements()         >= 0 &&
	       iResStation->NumElements()  >= 0);
	/*-----STAT-----*/

	//Check resource availability & execute ins
	while (iResStation->NumElements() > 0) {
	       int prevInsCount = insCount;
	       for (int i = 0; i < NUM_FUNC_UNIT; i++) {
			if (aluAvail[i]==true) {
				if (aluKind[i] == MEM && iResStation->Nth(0)->getType() == MEM) {
					//Implementing two MEM issue per cycle (two mem units)
					aluAvail[i] = false;
					instruction* ins = iResStation->Nth(0);
					if (ins->getMemType() == READ) {
						long int insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
						int latency = 0;
						int completeCycle = 0;
						//Non-blocking Load Op
						if (inFlightLDops.count(insAddr) > 0) {
							latency = inFlightLDops.find(insAddr)->second - cycle;
							executeIns(ins, cycle, latency);
							if (iSideBuff[0]->isFree() == false) mainStreamInsCount++;
							getLatency(0,latency); //Gen hit latency stat
							completeCycle = cycle+latency;
						} else {
							latency = findLatency(ins);
							executeIns(ins, cycle, latency);
							if (iSideBuff[0]->isFree() == false) mainStreamInsCount++;
							completeCycle = cycle+latency;
							insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
							inFlightLDops.insert(pair<long int,int>(insAddr,completeCycle));
						}
						//1 Level Deep Staic Cnotingent Scheduling Activation
						Assert (latency > 0 && completeCycle > 0);
						if (latency > xLevDeepLatLevel) {
							for (int sb = 0; sb < numSideBuffs; sb++) {
								if (iSideBuff[sb]->isFree() == true) {
									iSideBuff[sb]->setBusy();
									numOnSideBuffs++;
									ins->notifyAllDepGoToSideBuff(sb,ins->getInsID(),numSideBuffs);
									iWinWalkIndx = -1;
									//Collect Dependent Ins into SB
									int i = 0;
									//for (i = 0; i < iWindow->NumElements() && iSideBuff[sb]->NumElements() < SBlength; i++) {
									//	if (iWindow->Nth(i)->isGotoSideBuff() == true) {
									//		iWindow->Nth(i)->setStatus(sideBuffer,-1,-1);
									//		iSideBuff[sb]->Append(iWindow->Nth(i));
									//		SBlist->Append(i);
									//		iWinWalkIndx = i;
									//	}
									//}

									/* This is just an experiement */
									int diff = -1;
									for (i = 0; i < iWindow->NumElements() && diff < SBlength; i++) {
										if (iWindow->Nth(i)->isGotoSideBuff() == true) {
											iWindow->Nth(i)->setStatus(sideBuffer,-1,-1);
											iSideBuff[sb]->Append(iWindow->Nth(i));
											SBlist->Append(i);
											iWinWalkIndx = i;
										}
										diff = (iWinWalkIndx + 1) + (iResStation->NumElements() - 1) - iSideBuff[sb]->NumElements();
										Assert(diff >= 0);
									}
									/* This is just an experiement */

									/*-----STAT-----*/
									if (i >= iWindow->NumElements() && iSideBuff[sb]->NumElements() < SBlength) { //TODO this stat is broken
										windowSatration++; //TODO make it an array
									}
									/*-----STAT-----*/
									Assert ((iWinWalkIndx == -1 && iSideBuff[sb]->NumElements() == 0) || iWinWalkIndx != -1);
									if (iSideBuff[sb]->NumElements() == 0) 
										mainStreamBound = 0;
									else
										mainStreamBound = (iWinWalkIndx + 1) + (iResStation->NumElements() - 1) - iSideBuff[sb]->NumElements();
									//Update iWindow
									if (iWinWalkIndx >= 0) {scheduleInsInFlight(SBlist, delList, iWindow, iWinWalkIndx, cycle);}

									//Fill up the SB
									Assert(sb >= 0 && sb < numSideBuffs);
									/*-----STAT-----*/
									totNumSBactivations++;
									numSBactivations[sb]++;
									if (iWinWalkIndx >= 0) {
										totFrameSize   += (iWinWalkIndx + 1) + (iResStation->NumElements() - 1);
										totMainStreamBound += mainStreamBound;
									}
									/*-----STAT-----*/
									break;
								} else { //TODO should this be inside the for loop or ouside?
									/*-----STAT-----*/
									longLatOpWhenSPisON++;
									if (mainStreamInsCount >= mainStreamBound)
										longLatOpWhenSPisDraining++;
									else
										longLatOpWhenSPisWaiting++;
									/*-----STAT-----*/
								}
							}
						}
					} else if (ins->getMemType() == WRITE) {
						int latency = findLatency(ins);
						executeIns(ins, cycle, latency);
						if (iSideBuff[0]->isFree() == false) mainStreamInsCount++;
					} else {
						Assert(ins->getMemType() == WRITE || ins->getMemType() == READ);
					}
					aluFreeTime[i] = cycle+1;//non-blocking LD/ST
					iResStation->RemoveAt(0);
					/*-----STAT-----*/
					//for (int i = 0; i < numSideBuffs; i++) {
					if (iSideBuff[0]->isFree()==false) { // && iSideBuff[i]->getExpiration() > cycle) {
						InsCountWhenSBon[0]++;
						totInsCountWhenSBon++;
					}
					//}
					/*-----STAT-----*/
					break;
				} else if (aluKind[i] == ALU &&	
					   (iResStation->Nth(0)->getType() == ALU ||
					    iResStation->Nth(0)->getType() == FPU)) {
					aluAvail[i] = false;
					instruction* ins = iResStation->Nth(0);
					int latency = findLatency(ins);
					executeIns(ins, cycle, latency);
					if (iSideBuff[0]->isFree() == false) mainStreamInsCount++;
					aluFreeTime[i] = cycle+latency;
					iResStation->RemoveAt(0);
					/*-----STAT-----*/
					//for (int i = 0; i < numSideBuffs; i++) {
					if (iSideBuff[0]->isFree()==false) { // && iSideBuff[i]->getExpiration() > cycle) {
						InsCountWhenSBon[0]++;
						totInsCountWhenSBon++;
					}
					//}
					/*-----STAT-----*/
					break;
				}
			}
	       }
	       if (insCount-prevInsCount == 0) {break;} //in-order issue struct hazard
	}
	/*-----STAT-----*/
	inFlightLDopsSize += inFlightLDops.size();
	/*-----STAT-----*/
	delete [] iSideBfWalkIndx;
}

/***************************************
 * Dual-Lane IN ORDER Core implementation
 ***************************************/
void updateResStn(List<instruction*>* iResStation, List<instruction*>* iWindow) {
	int iWinWalkIndx = -1;
	for (int i = 0; i < iWindow->NumElements(); i++) {
		if (iWindow->Nth(i)->isReady(cycle) == true) {
			iWindow->Nth(i)->setStatus(ready,-1,-1);
			iResStation->Append(iWindow->Nth(i));
			iWinWalkIndx = i;
		} else {
			//printf("ins %s not ready\n", iWindow->Nth(i)->getCmdStr());
			break; //in-order issue dependency
		}
	}
	//Updaate the fetch list
	for (int i = iWinWalkIndx; i >= 0; i--) {
		if (iWindow->Nth(i)->getStatus() == ready)
			iWindow->RemoveAt(i);
	}
}
void runDueLaneInOcore(int cycle, List<instruction*> *iWindow1, List<instruction*> *iWindow2, List<instruction*> *iWindow3, List<instruction*> *iWindow4, int arbitrate) {
	int aluBound = NUM_FUNC_UNIT;
	//Cosntruct the ready list
	if (arbitrate == 1) {
		updateResStn(iResStation, iWindow1);
	} else if (arbitrate == 2) {
		updateResStn(iResStation, iWindow2);
	} else if (arbitrate == 3) {
		updateResStn(iResStation, iWindow3);
	} else if (arbitrate == 4) {
		updateResStn(iResStation, iWindow4);
	} else {
		Assert (arbitrate == 1 || arbitrate ==2 || arbitrate ==3 || arbitrate == 4);
	}
	/*-----STAT-----*/
	iWinSize      += iWindow1->NumElements()+iWindow2->NumElements()+iWindow3->NumElements()+iWindow4->NumElements();
	iROBSize      += iROB->NumElements();
	iResStnSize   += iResStation->NumElements();
	/*-----STAT-----*/
	//Check resource availability & execute ins
	if (iResStation->NumElements() == 0) {
		if (arbitrate == 1) {
			updateResStn(iResStation, iWindow2);
			updateResStn(iResStation, iWindow3);
			updateResStn(iResStation, iWindow4);
		} else if (arbitrate == 2) {
			updateResStn(iResStation, iWindow1);
			updateResStn(iResStation, iWindow3);
			updateResStn(iResStation, iWindow4);
		} else if (arbitrate == 3) {
			updateResStn(iResStation, iWindow1);
			updateResStn(iResStation, iWindow2);
			updateResStn(iResStation, iWindow4);
		} else if (arbitrate == 4) {
			updateResStn(iResStation, iWindow1);
			updateResStn(iResStation, iWindow2);
			updateResStn(iResStation, iWindow3);
		} else {
			Assert (arbitrate == 1 || arbitrate ==2 || arbitrate ==3 || arbitrate == 4);
		}
	}
	//if (iResStation->NumElements() == 0) {
	//	printf("(%d,%d,%d)", iWindow1->NumElements(),
	//	                       iWindow2->NumElements(),
	//			       iWindow3->NumElements());
	//	emptyResStation++;
	//}
	//Choose the number of functional units
	if (numFU == 1) {aluBound = 1;}
	else		{aluBound = NUM_FUNC_UNIT;}
	while (iResStation->NumElements() > 0) {
		int prevInsCount = insCount;
		for (int i = 0; i < aluBound; i++) {
			if (aluAvail[i]==true) {
				//printf("size of iResStation = %d\n", iResStation->NumElements());
				if ((numFU == 1 || aluKind[i] == MEM) && 
				    iResStation->Nth(0)->getType() == MEM) {
					aluAvail[i] = false;
					instruction* ins = iResStation->Nth(0);
					if (ins->getMemType() == READ) {
						long int insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
						if (inFlightLDops.count(insAddr) > 0) {//Non-blocking Load Op
							/*-----STAT-----*/
							nonBlockingMemOp++;
							/*-----STAT-----*/
							int tempLat = inFlightLDops.find(insAddr)->second - cycle;
							executeIns(ins, cycle, tempLat);
							//printf("%s, ",ins->getCmdStr());
							//Generate hit latency stat
							ins->setCacheHitLevel(getLatency(0,tempLat));
						} else {
							int latency = findLatency(ins);
							executeIns(ins, cycle, latency);
							//printf("%s, ",ins->getCmdStr());
							int completeCycle = cycle+latency;
							insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
							inFlightLDops.insert(pair<long int,int>(insAddr,completeCycle));
						}
					} else if (ins->getMemType() == WRITE) {
						int latency = findLatency(ins);
						executeIns(ins, cycle, latency);
							//printf("%s, ",ins->getCmdStr());
					} else {
						Assert(ins->getMemType() == WRITE || ins->getMemType() == READ);
					}
					aluFreeTime[i] = cycle+1;//non-blocking LD/ST
					iResStation->RemoveAt(0);
					break;
				} else if ((numFU == 1 || aluKind[i] == ALU) && 
					   (iResStation->Nth(0)->getType() == ALU ||
					    iResStation->Nth(0)->getType() == FPU)) {
					aluAvail[i] = false;
					instruction* ins = iResStation->Nth(0);
					int latency = findLatency(ins);
					executeIns(ins, cycle, latency);
					//printf("%s, ",ins->getCmdStr());
					aluFreeTime[i] = cycle+latency;
					iResStation->RemoveAt(0);
					break;
				}
			}
		}
		if (insCount-prevInsCount == 0) break; //in-order issue struct hazard
	}
	//printf("\n");
	/*-----STAT-----*/
	inFlightLDopsSize += inFlightLDops.size();
	/*-----STAT-----*/
}

/***************************************
 * IN ORDER Core implementation
 ***************************************/
long int runningFragNumber;
void runInOcore(int cycle, List<instruction*> *iWindow) {
	//long int localRunningFragNumber;
	//Cosntruct the ready list
	int iWinWalkIndx = -1;
	int aluBound = NUM_FUNC_UNIT;
	for (int i = 0; i < iWindow->NumElements(); i++) {
		if (iWindow->Nth(i)->isReady(cycle) == true) {
			iWindow->Nth(i)->setStatus(ready,-1,-1);
			iResStation->Append(iWindow->Nth(i));
			iWinWalkIndx = i;
		} else {
			//printf("ins %s not ready\n", iWindow->Nth(i)->getCmdStr());
			break; //in-order issue dependency
		}
	}
	//Updaate the fetch list
	for (int i = iWinWalkIndx; i >= 0; i--) {
		if (iWindow->Nth(i)->getStatus() == ready)
			iWindow->RemoveAt(i);
	}
	/*-----STAT-----*/
	iWinSize      += iWindow->NumElements();
	iROBSize      += iROB->NumElements();
	iResStnSize   += iResStation->NumElements();
	/*-----STAT-----*/
	//Check resource availability & execute ins
	//static int inter=0; static int intera=0;
	if (iResStation->NumElements() == 0) {
		emptyResStation++;
		//if (localRunningFragNumber == runningFragNumber) {
		//	inter++;
		//	printf("Inter-fragment Stall %d\n", inter);
		//} else {
		//	intera++;
		//	//printf("Intera-fragment Stall %d\n", intera);
		//}
	}
	//localRunningFragNumber = runningFragNumber;
	//Choose the number of functional units
	if (numFU == 1) {aluBound = 1;}
	else		{aluBound = NUM_FUNC_UNIT;}
	while (iResStation->NumElements() > 0) {
		int prevInsCount = insCount;
		for (int i = 0; i < aluBound; i++) {
			if (aluAvail[i]==true) {
				//printf("size of iResStation = %d\n", iResStation->NumElements());
				if ((numFU == 1 || aluKind[i] == MEM) && 
				    iResStation->Nth(0)->getType() == MEM) {
					aluAvail[i] = false;
					instruction* ins = iResStation->Nth(0);
					if (ins->getMemType() == READ) {
						long int insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
						if (inFlightLDops.count(insAddr) > 0) {//Non-blocking Load Op
							/*-----STAT-----*/
							nonBlockingMemOp++;
							/*-----STAT-----*/
							int tempLat = inFlightLDops.find(insAddr)->second - cycle;
							executeIns(ins, cycle, tempLat);
							//printf("%s, ",ins->getCmdStr());
							//Generate hit latency stat
							ins->setCacheHitLevel(getLatency(0,tempLat));
						} else {
							int latency = findLatency(ins);
							executeIns(ins, cycle, latency);
							//printf("%s, ",ins->getCmdStr());
							int completeCycle = cycle+latency;
							insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
							inFlightLDops.insert(pair<long int,int>(insAddr,completeCycle));
						}
					} else if (ins->getMemType() == WRITE) {
						int latency = findLatency(ins);
						executeIns(ins, cycle, latency);
							//printf("%s, ",ins->getCmdStr());
					} else {
						Assert(ins->getMemType() == WRITE || ins->getMemType() == READ);
					}
					aluFreeTime[i] = cycle+1;//non-blocking LD/ST
					iResStation->RemoveAt(0);
					break;
				} else if ((numFU == 1 || aluKind[i] == ALU) && 
					   (iResStation->Nth(0)->getType() == ALU ||
					    iResStation->Nth(0)->getType() == BR  ||
					    iResStation->Nth(0)->getType() == ASSIGN ||
					    iResStation->Nth(0)->getType() == FPU)) {
					aluAvail[i] = false;
					instruction* ins = iResStation->Nth(0);
					int latency = findLatency(ins);
					executeIns(ins, cycle, latency);
					//printf("%s, ",ins->getCmdStr());
					aluFreeTime[i] = cycle+latency;
					iResStation->RemoveAt(0);
					break;
				}
			}
		}
		if (insCount-prevInsCount == 0) {
			break; //in-order issue struct hazard
		}
	}
	if (reportTrace == true) printSTALL (cycle);
	//printf("\n");
	/*-----STAT-----*/
	inFlightLDopsSize += inFlightLDops.size();
	/*-----STAT-----*/
}

/***************************************
 * IN ORDER Core implementation - with N EU's  (default: N=4)
 ***************************************/
void runInOcore2(int cycle) {
	//Cosntruct the ready list
	//int iWinWalkIndx = -1;
	//int wbb_bypass_count = 0, br_bypass_count = 0;
	//float wbb_chain_accuracy = 1.0, br_chain_accuracy = 1.0;
	set<long int> list_of_wbb;
	for (int i = 0; i < iWindow->NumElements(); i++) {
		/*-----STAT-----*/
		//if (iWindow->Nth(i)->getType() == BR
		//    && iWindow->Nth(i)->isReady(cycle) == false
		//	&& iWindow->Nth(i)->getBrBias() >= 0.05 
		//	&& iWindow->Nth(i)->getBrBias() <= 0.95) {
		//	list_of_wbb.insert(iWindow->Nth(i)->getInsAddr());
		//	wbb_bypass_count++;
		//	wbb_chain_accuracy *= iWindow->Nth(i)->getBrAccuracy();
		//}
		//if (iWindow->Nth(i)->getType() == BR) {
		//	br_bypass_count++;
		//	br_chain_accuracy *= iWindow->Nth(i)->getBrAccuracy();
		//}
		/*-----STAT-----*/
		if (iWindow->Nth(i)->isReady(cycle) == true) {
			/*-----STAT-----*/
			////if (iWindow->Nth(i)->getInsAddr() == 4214686) { //bzip
			//if (iWindow->Nth(i)->getInsAddr() == 4248075) { //hmmer
			//	fprintf(junk,"\n\n");
			//	collect_stat = true;
			////} else if (iWindow->Nth(i)->getInsAddr() == 4214340 ||
			////           iWindow->Nth(i)->getInsAddr() == 4214683) { //bzip2
			//} else if ((iWindow->Nth(i)->getInsAddr() == 4248005 && 
			//			i+1 < iWindow->NumElements() &&
			//            iWindow->Nth(i+1)->getInsAddr() != 4248075)||
			//           iWindow->Nth(i)->getInsAddr() == 4248071) { //hmmer
			//	collect_stat = false;
			//}
			//if (collect_stat == true) {
			//	fprintf(junk,"%ld, ", iWindow->Nth(i)->getInsAddr());
			//}
			//if (num_ins_exe_cnt.find(iWindow->Nth(i)->getInsAddr()) != num_ins_exe_cnt.end())
			//	num_ins_exe_cnt[iWindow->Nth(i)->getInsAddr()] += 1;
			//else
			//	num_ins_exe_cnt[iWindow->Nth(i)->getInsAddr()] = 1;
			//if (num_bypassed_wbb.find(iWindow->Nth(i)->getInsAddr()) != num_bypassed_wbb.end())
			//	//num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] += list_of_wbb.size(); //static ins
			//	num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] += wbb_bypass_count; //dynamic ins
			//else
			//	//num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] = list_of_wbb.size(); //static ins
			//	num_bypassed_wbb[iWindow->Nth(i)->getInsAddr()] = wbb_bypass_count; //dynamic ins
			//if (ins_exe_hoist_accuracy.find(iWindow->Nth(i)->getInsAddr()) != ins_exe_hoist_accuracy.end())
			//	ins_exe_hoist_accuracy[iWindow->Nth(i)->getInsAddr()] += br_chain_accuracy;
			//else
			//	ins_exe_hoist_accuracy[iWindow->Nth(i)->getInsAddr()] = br_chain_accuracy;
			/*-----STAT-----*/
			iWindow->Nth(i)->setStatus(ready,-1,-1);
			int indx = findBestResStation();
			iResStations[indx]->Append(iWindow->Nth(i));
			//iWinWalkIndx= i;
		} else {
			break; //in-order issue dependency
		}
	}
	//quicksortInsList(iResStation,0,iResStation->NumElements()-1);
	//quicksortInsList(iMemBuf,0,iMemBuf->NumElements()-1);
	//printf("size of iROB        = %d\n", iROB->NumElements());
	//printf("size of iWindow     = %d\n", iWindow->NumElements());
	//printf("size of iResStation = %d\n", iResStation->NumElements());
	//printf("size of iMemBuf     = %d\n", iMemBuf->NumElements());
	//Updaate the fetch list
	for (int i = iWindow->NumElements()-1; i >= 0; i--) {
		if (iWindow->Nth(i)->getStatus() == ready)
			iWindow->RemoveAt(i);
	}
	/*-----STAT-----*/
	iWinSize      += iWindow->NumElements();
	iROBSize      += iROB->NumElements();
	for (int i = 0; i < NUM_FUNC_UNIT; i++)
		iResStnsSize[i] += iResStations[i]->NumElements();
	/*-----STAT-----*/
	//Check resource availability & execute ins
	for (int j = 0; j < NUM_FUNC_UNIT; j++) {
		if (aluAvail[j]==true) {
			if (iResStations[j]->NumElements() > 0 &&
			    iResStations[j]->Nth(0)->getStatus() == ready) {
				aluAvail[j] = false;
				instruction* ins = iResStations[j]->Nth(0);
				if (ins->getType() == MEM && ins->getMemType() == READ) {
					if (memoryModel == NAIVE_SPECUL) {
							executeIns(ins, cycle, LONG_LATENCY);
					} else {
						long int insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
						if (inFlightLDops.count(insAddr) > 0) {//Non-blocking Load Op
							int tempLat = inFlightLDops.find(insAddr)->second - cycle;
							executeIns(ins, cycle, tempLat);
							getLatency(0,tempLat); //Generate hit latency stat
						} else {
							int latency = findLatency(ins);
							executeIns(ins, cycle, latency);
							int completeCycle = cycle+latency;
							insAddr = ins->getMemAddr() >> (BLOCK_OFFSET+WORD_OFFSET);
							inFlightLDops.insert(pair<long int,int>(insAddr,completeCycle));
						}
					}
					aluFreeTime[j] = cycle+1;//non-blocking LD/ST
				} else if (ins->getType() == MEM && ins->getMemType() == WRITE) {
					if (memoryModel == NAIVE_SPECUL) {
						executeIns(ins, cycle, ST_LATENCY);
					} else {
						int latency = findLatency(ins);
						executeIns(ins, cycle, latency);
					}
					aluFreeTime[j] = cycle+1;//non-blocking LD/ST
				} else if (ins->getType() == ALU || 
						   ins->getType() == BR  || 
						   ins->getType() == ASSIGN || 
						   ins->getType() == FPU) {
					int latency = findLatency(ins);
					executeIns(ins, cycle, latency);
					aluFreeTime[j] = cycle+latency;
				} else {
					Assert(ins->getType() == MEM || ins->getType() == ALU ||
					       ins->getType() == BR  || ins->getType() == FPU ||
					       ins->getType() == BR  || ins->getType() == FPU);
				}
				iResStations[j]->RemoveAt(0);
			}
		}
	}
	/*-----STAT-----*/
	inFlightLDopsSize += inFlightLDops.size();
	/*-----STAT-----*/
}



/***************************************
 * Main Functions
 ***************************************/
void bkEnd_run () {
	//VLIW Scheduler
	if (reschedule == true) {
		bool addMoreIns = true;
		vliwScheduler* schedule = new vliwScheduler;
		while (true) {
			cycle++;
			if (eoc == false && addMoreIns == true) addIns(cycle);
			else if (eoc == true && iROB->NumElements() == 0) break; //TODO double check this line
			if (numFU == 1) { //VLIW scheduling (normal) for 1FU
				addMoreIns = schedule->scheduleInsStream_1FU(iROB,cycle,false,reScheduleFile);
			} else { //VLIW scheduling (normal) for multiple FU's
				addMoreIns = schedule->scheduleInsStream(iROB,cycle,UPLDhoist,reScheduleFile,unpredMemOpThreshold);
			}
			//Runtime Stat
			if (cycle%100000==0) {
				printf ("Cycle %ld\n", cycle);
				//printf("Ins Count = %d\n", insCount); //TODO add this line later
			}
		}
		delete schedule;
	}

	// Phrase Maker
	else if (makePhrase == true) {
		bool addMoreIns = true;
		vliwScheduler* schedule = new vliwScheduler;
		List<phrase*>* phList = new List<phrase*>;
		while (true) {
			cycle++;
			if (eoc == false && addMoreIns == true) schedule->parseIns(ROBsize, parse, cycle);
			else if (eoc == true) break; //TODO double check this line
			//while (iWindow->NumElements() > 0) {iWindow->RemoveAt(0);}//Drain it to avoid core dump (TODO optimize it out later)
			addMoreIns = schedule->schedulePhraseinsStream(iROB,cycle,UPLDhoist,reScheduleFile,unpredMemOpThreshold,phList);
			//Runtime Stat
			if (cycle%10000==0) {
				printf ("Cycle %ld\n", cycle);
				printf("Ins Count = %ld\n", insCount);
			}
		}
		delete schedule;
	}
	//----
	//else if (makePhrase == true) {
	//	printf("\n---WARNING: SET THE CORRECT unpredMemOpThreshold VALUE!!!---\n\n");
	//	int numPhrases = -1;
	//	phraseGen* phGen = new phraseGen;
	//	while (true) {
	//		cycle++;
	//		if (iROB->NumElements() == 0) printf("***********ROB DONE\n");
	//		if (iWindow->NumElements() == 0) printf("***********iWIN DONE\n");
	//		if (eoc == true) printf("END OF FILE\n");
	//		//if (numPhrases == 0) printf("PHRASES DONE\n");
	//		//else printf("num phrases = %d\n", numPhrases);
	//
	//		if (eoc == false) addIns(cycle);
	//		else if (eoc == true &&
	//			 numPhrases == 0 &&
	//		         iROB->NumElements() == 0) break;
	//		numPhrases = phGen->runPhraseGen(iROB, iWindow, phraseFile, depTables, eoc, ROBsize);
	//		if (cycle%300000==0) {
	//			printf ("Instruction: %d\n", iWindow->Nth(0)->getInsID());
	//		}
	//	}
	//	/*-----STAT-----*/
	//	totNumPhrase = phGen->getTotNumPhrases();
	//	totNumSoftBound = phGen->getTotNumSoftBound();
	//	totNumPhUnpredMemOp = phGen->getTotNumPhraseUPLD();
	//	totNumRootIns = phGen->getTotNumRootIns();
	//	totNumPhraseAncestors = phGen->getTotNumPhraseAncestors();
	//	totNumPhGenResets = phGen->getNumPhGenReset();
	//	insCount = phGen->getTotNumIns();
	//	totNumFrag = phGen->getTotNumFrags();
	//	totNumRootPh = phGen->getTotNumRootPh();
	//	totNumCritPathViol = phGen->getTotNumCritPathViolations();
	//	/*-----STAT-----*/
	//	//delete phGen;
	//}

	// Phrase Core
	else if (coreType == PHRASE) {
		List<phrase*> *iPh = new List<phrase*>;
		dot *d = new dot(0);
		long int phIndx = 0;
		long int phIdealLat;
		bool phStart = true;
		int phSize;
		int numInFlightPh = 0;
		if (eoc == false) addPhrase(iPh, cycle, d); //TODO temperary location
		phSize = iPh->Nth(0)->getPhraseSize_unsort();
		//long int critPath = iPh->Nth(0)->findCriticalPath();
		//phCritPathHist->addElem(critPath);
		for (int i = 0; i < phSize; i++) {
			iWindow->Append(iPh->Nth(0)->getNthIns_unsort(i));
		}
		numInFlightPh++;
		/*-----STAT-----*/
		phIdealLat = iPh->Nth(0)->getPhIdealLat();
		/*-----STAT-----*/
		while (true) {
			cycle++;
			if (phStart == true) phCycle++;
			//Free the busy ALU's
			freeALUs(cycle);
			//Complete executed phrases
			for (int i = 0; i < numInFlightPh; i++) {
				completeIns(cycle,iPh->Nth(i)->getInsList_unsort());
				commitIns(cycle,iPh->Nth(i)->getInsList_unsort());
				//Complete executed phrases
				completeIns(cycle,iPh->Nth(i)->getInsList_unsort());
			}
			//Add instructions to phrase
			if (eoc == false) addPhrase (iPh, cycle, d);
			//Check resources status - goto nxt iter if all busy
			if (!isALUfree()) continue;
			int temp = numInFlightPh;
			for (int i = temp-1; i >= 0; i--) {
				if (iPh->Nth(i)->getPhraseSize_unsort() == 0) {
					removePhrase(iPh, i);
					numInFlightPh--;
					Assert(numInFlightPh >= 0);
					//printf("removed:%d\n", numInFlightPh);
				}
			}
			//Go to Next Phrase
			if (iWindow->NumElements() == 0 && iPh->NumElements() > numInFlightPh) {
				phStart = false;
				//removePhrase(iPh, 0);
				//if (iPh->NumElements() == 0) break;
				phIndx++;
				/*-----STAT-----*/
				//printf("%d, %d\n", phIdealLat/4, phCycle);
				//Assert((phIdealLat/NUM_FUNC_UNIT-1) <= phCycle); //TODO this check is broken. fix it
				totPhStall += phCycle - phIdealLat;
				phIdealLat = iPh->Nth(0)->getPhIdealLat();
				/*-----STAT-----*/
				phCycle = 1;
				//Assert(iWindow->NumElements() == 0);
				phSize = iPh->Nth(numInFlightPh)->getPhraseSize_unsort();
				//critPath = iPh->Nth(numInFlightPh)->findCriticalPath();
				//phCritPathHist->addElem(iPh->Nth(numInFlightPh)->findCriticalPath());
				for (int i = 0; i < phSize; i++) {
					iWindow->Append(iPh->Nth(numInFlightPh)->getNthIns_unsort(i));
				}
				Assert(iWindow->NumElements() == iPh->Nth(numInFlightPh)->getPhraseSize_unsort());
				numInFlightPh++;
			}
			if (iPh->NumElements() == 0 && iWindow->NumElements() == 0) break;
			//Run the core
			runInOcore(cycle,iWindow);
			if (iWindow->NumElements() < phSize) phStart = true;
			//Runtime Stat
			if (cycle%200000==0) {
				ipc = (float)(insCount)/(float)(cycle);
				printf ("Cycle %ld\n", cycle);
				printf("IPC = %f\n", ipc);
				//phCritPathHist->report();
				//phSizeHist->report();
				//printf("------\n");
			}
		}
		d->finish();
		delete d;
		/*-----STAT-----*/
		totNumPhrase = phIndx+1;
		/*-----STAT-----*/
	}

	// Fragment Core
	else if (coreType == FRAGMENT) {
		List<fragment*> *iFr_wait  = new List<fragment*>;
		List<fragment*> *iFr_ready = new List<fragment*>;
		long int frIndx = 0;
		long int frIdealLat;
		bool frStart = true;
		int frSize;
		int numInFlightFrags = 0;
		if (eoc == false) addFrag (iFr_wait, cycle); //TODO temperary location
		findReadyFrag(iFr_ready, iFr_wait);
		numReadyFrags += iFr_ready->NumElements();
		frSize = iFr_ready->Nth(0)->getFragSize();
		for (int i = 0; i < frSize; i++) {
			iWindow->Append(iFr_ready->Nth(0)->getNthIns(i));
		}
		numInFlightFrags++;
		/*-----STAT-----*/
		frIdealLat = iFr_ready->Nth(0)->getFrIdealLat();
		/*-----STAT-----*/
		while (true) {
			cycle++;
			if (frStart == true) frCycle++;
			//Free the busy ALU's
			freeALUs(cycle);
			for (int i = 0; i < numInFlightFrags; i++) {
				//Complete executed fragments
				completeIns(cycle,iFr_ready->Nth(i)->getInsList());
				commitIns(cycle,iFr_ready->Nth(i)->getInsList());
				//Complete executed fragments
				completeIns(cycle,iFr_ready->Nth(i)->getInsList());
			}
			//Move Fragments from Wait List to Ready List
			findReadyFrag(iFr_ready, iFr_wait);
			//Add instructions to fragment
			if (eoc == false) addFrag (iFr_wait, cycle);
			//Check resources status - goto nxt iter if all busy
			if (!isALUfree()) continue;
			int temp = numInFlightFrags;
			for (int i = temp-1; i >= 0; i--) {
				if (iFr_ready->Nth(i)->getFragSize() == 0) {
					iFr_ready->Nth(i)->setEnd(cycle);
					frLatHist->addElem(iFr_ready->Nth(i)->getLat());
					removeFrag(iFr_ready, i);
					numInFlightFrags--;
					Assert(numInFlightFrags >= 0);
					//printf("removed:%d\n", numInFlightFrags);
				}
			}
			//printf("num: %d,%d,%d,%d\n", iFr_ready->NumElements(), iFr_ready->Nth(0)->getFragSize(), iWindow->NumElements(),numInFlightFrags);
			//Go to Next Frag
			if (iWindow->NumElements() == 0 && iFr_ready->NumElements() > numInFlightFrags) {
				//printf("num ready frags=%d\n",iFr_ready->NumElements());
				frStart = false;
				//removeFrag(iFr_ready, 0);
				frIndx++;
				/*-----STAT-----*/
				//printf("%d, %d\n", frIdealLat/4, frCycle);
				//Assert((frIdealLat/NUM_FUNC_UNIT-1) <= frCycle); //TODO this check is broken. fix it
				totFrStall += frCycle - frIdealLat/NUM_FUNC_UNIT;
				//printf("%d\n",frCycle - frIdealLat/4);
				frIdealLat = iFr_ready->Nth(0)->getFrIdealLat();
				/*-----STAT-----*/
				frCycle = 1;
				//Assert(iWindow->NumElements() == 0);
				//quicksortFragScore(iFr_ready, numInFlightFrags, iFr_ready->NumElements()-1, -1);
				frSize = iFr_ready->Nth(numInFlightFrags)->getFragSize();
				runningFragNumber = iFr_ready->Nth(numInFlightFrags)->getFragNum();
				//printf("%d\n", numInFlightFrags);
				iFr_ready->Nth(numInFlightFrags)->setStart(cycle);
				for (int i = 0; i < frSize; i++) {
					iWindow->Append(iFr_ready->Nth(numInFlightFrags)->getNthIns(i));
				}
				Assert(iWindow->NumElements() == iFr_ready->Nth(numInFlightFrags)->getFragSize());
				numInFlightFrags++;
				//printf("added:%d\n", numInFlightFrags);
				/*-----STAT-----*/
				if (frSize < phraseSizeBound) {
					totNumRealFrag++;
					totSizRealFrag += frSize;
				} else {
					totNumOfSigleFragPhrases++;
				}
				numReadyFrags += iFr_ready->NumElements();
				//printf("num ready frags = %d, %d\n", iFr_ready->NumElements(), cycle);
				/*-----STAT-----*/
			}
			if (iFr_wait->NumElements() == 0 && iFr_ready->NumElements() == 0 && iWindow->NumElements() == 0) break;
			/*-----STAT-----*/
			if (iFr_ready->NumElements()==0) {interFragStallCycle++; printf("empty\n");}
			/*-----STAT-----*/
			//Run the core
			runInOcore(cycle,iWindow);
			if (iWindow->NumElements() < frSize) frStart = true;
			//Runtime Stat
			if (cycle%200000==0) {
				ipc = (float)(insCount)/(float)(cycle);
				printf ("Cycle %ld\n", cycle);
				printf("IPC = %f\n", ipc);
				//frSizeHist->report();
				//printf("------\n");
				//frLatHist->report();
			}
		}
		delete iFr_wait;
		delete iFr_ready;
		/*-----STAT-----*/
		totNumFrag = frIndx+1;
		/*-----STAT-----*/
	}
	else if (coreType == FRAGMENT2) {
		List<fragment*> *iFr_wait  = new List<fragment*>;
		List<fragment*> *iFr_ready = new List<fragment*>;
		List<instruction*> *iWindow1  = new List<instruction*>;
		List<instruction*> *iWindow2  = new List<instruction*>;
		List<instruction*> *iWindow3  = new List<instruction*>;
		List<instruction*> *iWindow4  = new List<instruction*>;
		long int frIndx = 0;
		//long int frIdealLat;
		bool frStart = true;
		int frSize;
		int numInFlightFrags = 0;
		int arbitrate = -1;
		if (eoc == false) addFrag (iFr_wait, cycle); //TODO temperary location
		findReadyFrag(iFr_ready, iFr_wait);
		numReadyFrags += iFr_ready->NumElements();
		frSize = iFr_ready->Nth(0)->getFragSize();
		for (int i = 0; i < frSize; i++) {
			iWindow1->Append(iFr_ready->Nth(0)->getNthIns(i));
			arbitrate = 1;
		}
		numInFlightFrags++;
		if (iFr_ready->NumElements() > 1) {
			frSize = iFr_ready->Nth(1)->getFragSize();
			for (int i = 0; i < frSize; i++) {
				iWindow2->Append(iFr_ready->Nth(1)->getNthIns(i));
				arbitrate = 1; //TODO is this correect?
			}
			numInFlightFrags++;
		}
		/*-----STAT-----*/
		//frIdealLat = iFr_ready->Nth(0)->getFrIdealLat();
		/*-----STAT-----*/
		while (true) {
			cycle++;
			if (frStart == true) frCycle++;
			//Free the busy ALU's
			freeALUs(cycle);
			for (int i = 0; i < numInFlightFrags; i++) {
				//Complete executed fragments
				completeIns(cycle,iFr_ready->Nth(i)->getInsList());
				commitIns(cycle,iFr_ready->Nth(i)->getInsList());
				//Complete executed fragments
				completeIns(cycle,iFr_ready->Nth(i)->getInsList());
			}
			//Move Fragments from Wait List to Ready List
			findReadyFrag(iFr_ready, iFr_wait);
			//Add instructions to fragment
			if (eoc == false) addFrag (iFr_wait, cycle);
			//Check resources status - goto nxt iter if all busy
			if (!isALUfree()) continue;
			//if (iFr_ready->NumElements()) {
			//	printf("num: %d,%d,%d,%d\n", iFr_ready->NumElements(), 
			//				     iFr_ready->Nth(0)->getFragSize(), 
			//				     iWindow1->NumElements(),
			//				     numInFlightFrags);
			//}
			int temp = numInFlightFrags;
			for (int i = temp-1; i >= 0; i--) {
				if (iFr_ready->Nth(i)->getFragSize() == 0) {
					removeFrag(iFr_ready, i);
					numInFlightFrags--;
					Assert(numInFlightFrags >= 0);
				}
			}
			//Go to Next Frag
			if (iWindow1->NumElements() == 0 && iFr_ready->NumElements() > numInFlightFrags) {
				//printf("******* 1 \n");
				//int whichWin = -1;
				//if (iFr_ready->Nth(0)->getFragSize() == 0)
				//	whichWin = 0;
				//else if (iFr_ready->Nth(1)->getFragSize() == 0)
				//	whichWin = 1;
				//else
				//	Assert(whichWin == 0 || whichWin == 1);
				//frSize = iFr_ready->Nth(numInFlightFrags)->getFragSize();
				//removeFrag(iFr_ready, whichWin);
				//quicksortFragScore(iFr_ready, 0, iFr_ready->NumElements()-1, -1);
				frSize = iFr_ready->Nth(numInFlightFrags)->getFragSize();
				for (int i = 0; i < frSize; i++) {
					iWindow1->Append(iFr_ready->Nth(numInFlightFrags)->getNthIns(i));
				}
				Assert(iWindow1->NumElements() == iFr_ready->Nth(numInFlightFrags)->getFragSize());
				numInFlightFrags++;
				arbitrate = 2;
			} if (iWindow2->NumElements() == 0 && iFr_ready->NumElements() > numInFlightFrags) {
				//printf("******* 2 \n");
				//int whichWin = -1;
				//if (iFr_ready->Nth(0)->getFragSize() == 0)
				//	whichWin = 0;
				//else if (iFr_ready->Nth(1)->getFragSize() == 0)
				//	whichWin = 1;
				//else
				//	Assert(whichWin == 0 || whichWin == 1);
				//removeFrag(iFr_ready, whichWin);
				//quicksortFragScore(iFr_ready, 0, iFr_ready->NumElements()-1, -1);
				frSize = iFr_ready->Nth(numInFlightFrags)->getFragSize();
				for (int i = 0; i < frSize; i++) {
					iWindow2->Append(iFr_ready->Nth(numInFlightFrags)->getNthIns(i));
				}
				Assert(iWindow2->NumElements() == iFr_ready->Nth(numInFlightFrags)->getFragSize());
				numInFlightFrags++;
				arbitrate = 3;
			} if (iWindow3->NumElements() == 0 && iFr_ready->NumElements() > numInFlightFrags) {
				//printf("******* 2 \n");
				//int whichWin = -1;
				//if (iFr_ready->Nth(0)->getFragSize() == 0)
				//	whichWin = 0;
				//else if (iFr_ready->Nth(1)->getFragSize() == 0)
				//	whichWin = 1;
				//else
				//	Assert(whichWin == 0 || whichWin == 1);
				//removeFrag(iFr_ready, whichWin);
				//quicksortFragScore(iFr_ready, 0, iFr_ready->NumElements()-1, -1);
				frSize = iFr_ready->Nth(numInFlightFrags)->getFragSize();
				for (int i = 0; i < frSize; i++) {
					iWindow3->Append(iFr_ready->Nth(numInFlightFrags)->getNthIns(i));
				}
				Assert(iWindow3->NumElements() == iFr_ready->Nth(numInFlightFrags)->getFragSize());
				numInFlightFrags++;
				arbitrate = 4;
			} if (iWindow4->NumElements() == 0 && iFr_ready->NumElements() > numInFlightFrags) {
				//printf("******* 2 \n");
				//int whichWin = -1;
				//if (iFr_ready->Nth(0)->getFragSize() == 0)
				//	whichWin = 0;
				//else if (iFr_ready->Nth(1)->getFragSize() == 0)
				//	whichWin = 1;
				//else
				//	Assert(whichWin == 0 || whichWin == 1);
				//removeFrag(iFr_ready, whichWin);
				//quicksortFragScore(iFr_ready, 0, iFr_ready->NumElements()-1, -1);
				frSize = iFr_ready->Nth(numInFlightFrags)->getFragSize();
				for (int i = 0; i < frSize; i++) {
					iWindow4->Append(iFr_ready->Nth(numInFlightFrags)->getNthIns(i));
				}
				Assert(iWindow4->NumElements() == iFr_ready->Nth(numInFlightFrags)->getFragSize());
				numInFlightFrags++;
				arbitrate = 1;
			}
			/*-----STAT-----*/
			if (iFr_ready->NumElements()== 0 && iFr_wait->NumElements()>0) interFragStallCycle++;
			if (iFr_wait->NumElements() == 0 && 
			    iFr_ready->NumElements()== 0 && 
			    iWindow1->NumElements() == 0 && 
			    iWindow2->NumElements() == 0 &&
			    iWindow3->NumElements() == 0 &&
			    iWindow4->NumElements() == 0) break;
			/*-----STAT-----*/
			//Run the core
			runDueLaneInOcore(cycle,iWindow1,iWindow2,iWindow3,iWindow4,arbitrate); //TODO fix arbitration
			if (iWindow1->NumElements() < frSize) frStart = true;
			//Runtime Stat
			if (cycle%200000==0) {
				ipc = (float)(insCount)/(float)(cycle);
				printf ("Cycle %ld\n", cycle);
				printf("IPC = %f\n", ipc);
			}
		}
		delete iFr_wait;
		delete iFr_ready;
		delete iWindow1;
		delete iWindow2;
		delete iWindow3;
		delete iWindow4;
		/*-----STAT-----*/
		totNumFrag = frIndx+1;
		/*-----STAT-----*/
	} else if (coreType == PHRASEBLOCK) {
		//long int phIndx = 0;
		//long int phIdealLat;
		//bool phStart = true;
		//int phSize;
		//cout << "addPhraseblock\n";
		addPhraseblock(_pbLists, cycle);
		while (true) {
			cycle++;
			//Free the busy ALU's
			//cout << "freeALUs\n";
			freeALUs(cycle);
			//Complete executed phrases
			//cout << "completePB\n";
			completePB(cycle);
			//Commit executed instructions
			//cout << "commitPB\n";
			if (commitPB(cycle) == -1 && cycle != 1 && eoc == true) {break;}
			//Complete executed phrases
			//cout << "completePB\n";
			completePB(cycle);
			//LSQ
			//if (memoryModel == NAIVE_SPECUL) {
			//	//Run LSQ
			//	exeMemPipeStage(pbROB, cycle, loadStoreQue);
			//	//SQ Access Cache for Write
			//	runStoreQueue(loadStoreQue, cycle);
			//	getLSQsize(loadStoreQue); /*STAT*/
			//}
			if (!perfectRegRen) getRRsize(GRF); /*STAT*/
			//Add instructions to phrase
			//cout << "addPhraseblock\n";
			addPhraseblock (_pbLists, cycle);
			if (eof) {eof=false; break;}
			//Check resources status - goto nxt iter if all busy
			//cout << "isALUfree\n";
			if (!isALUfree()) continue;
			//Run the core
			//cout << "runPhraseblockCore\n";
			runPhraseblockCore(cycle,_pbLists);
			//Runtime Stat
			if (cycle%300000==0) {
				ipc = (float)(insCount)/(float)(cycle);
				printf ("Cycle %ld\n", cycle);
				printf("IPC = %f\n", ipc);
				printf("Squash Cnt = %d\n", squashCount);
				printf("EOF: %d EOC: %d\n", eof, eoc);
			}
		}
		/*-----STAT-----*/
		//totNumPhrase = phIndx+1;
		/*-----STAT-----*/
	} else if (coreType == DOT) {
		printf("hey\n");
		dot *d = new dot(0);
		if (eoc == false) addIns(cycle);
		d->runDot(iWindow,0);
	}
	//core implementations
	else {
		if (debug) {
			printf("CYCLE IN = %ld\n", cycle);
			//long int cycle_in = cycle; //TODO cycle_in must be defined outside this scope
		}
		while (true) {
			cycle++;
			//Free the busy ALU's
			freeALUs(cycle);
			//Complete executed instructions
			completeIns(cycle,iROB);
			//Commit executed instructions
			if (commitIns(cycle,iROB) == -1 && cycle != 1 && eoc == true) break;
			//Complete executed instructions
			completeIns(cycle,iROB);
			//LSQ
			if (memoryModel == NAIVE_SPECUL) {
				//Run LSQ
				exeMemPipeStage(iROB, cycle, loadStoreQue);
				//SQ Access Cache for Write
				runStoreQueue(loadStoreQue, cycle);
				getLSQsize(loadStoreQue); /*STAT*/
			}
			if (!perfectRegRen && coreType != IN_ORDER) getRRsize(GRF); /*STAT*/
			//Add instructions to iROB
			if (eoc == false) addIns(cycle);
			if (eof) {eof=false;break;}
			//Check resources status - goto nxt iter if all busy
			if (!isALUfree()) continue;
			//Run the core
			if (coreType == OUT_OF_ORDER) {
				if (numFU == 1) {
					runOOOcoreSingleIssue(cycle);
				} else if (numFU == 4) {
					runOOOcore2(cycle);
				} else {
					runOOOcore(cycle);}
			} else if (coreType == STRAND) {
			 	runStrandcore(cycle);
			} else if (coreType == IN_ORDER) {
				if (numFU == 4)
					runInOcore2(cycle);
				else
					runInOcore(cycle,iWindow);
			} else if (coreType == X_LEVEL_DEEP_DYN) {
			 	runxLDcore_DYN(cycle);
			} else if (coreType == ONE_LEVE_DEEP_STAT) {
			 	run1LD_STAT(cycle);
			} else if (coreType == X_LEVE_DEEP_STAT) {
			 	runxLD_STAT(cycle);
			}else {
				printf("ERROR: core name is not set correctly!\n");
				Assert(coreType >= IN_ORDER && coreType <= X_LEVEL_DEEP_DYN);
			}
			//Runtime Stat
			if (cycle%300000==0) {
				ipc = (float)(insCount)/(float)(cycle);
				printf ("Cycle %ld\n", cycle);
				printf("IPC = %f\n", ipc);
				printf("Squash Cnt = %d\n", squashCount);
			}
		}
		//if (debug) printf("CYCLE OUT = %ld (diff = %ld)\n", cycle, cycle-cycle_in);
		if (reportTraceAndHitMiss == true) {
			printf("STORE Unpredictable Memory Accesses\n");
			computeMissRates();
			printMissRatetoFile();
			fclose (pinFile);
			//if((pinFile=fopen(inFileName.c_str(), "r")) == NULL) {
			//	printf("ERROR: Cannot open file(s).\n");
			//	exit(1);
			//}
			//eoc = false;
			//if (evaltMissRtCorrel == 1) {
			//	correlationHist = new hist(1001, 0, 1001);
			//}
			//while (true) {
			//	if (eoc == false) addIns(cycle);
			//	else break;
			//	while (iWindow->NumElements() > 0) {iWindow->RemoveAt(0);}//Drain it to avoid core dump
			//	while (iROB->NumElements() > 0) {
			//		instruction *ins = iROB->Nth(0);
			//		long int insAddr = ins->getInsAddr();
			//		if (ins->getMemType() == READ &&
			//		    memRdMissRateTable.count(insAddr) > 0) {
			//			float missRate = memRdMissRateTable.find(insAddr)->second;
			//			/*-----STAT-----*/
			//			if (missRate > unpredMemOpThreshold) {unpredMemOpCnt++;}
			//			/*-----STAT-----*/
			//			//For measuring UPLD measurement accuracy
			//			//if (missRate > unpredMemOpThreshold && ins->getMissrate() <= unpredMemOpThreshold)
			//			//	printf("1: %f - %f = %f\n",ins->getMissrate(), missRate, ins->getMissrate()-missRate);
			//			//else if (missRate <= unpredMemOpThreshold && ins->getMissrate() > unpredMemOpThreshold)
			//			//	printf("2: %f - %f = %f\n",ins->getMissrate(), missRate, ins->getMissrate()-missRate);

			//			if (evaltMissRtCorrel == 0) {
			//				ins->setMissRate(missRate);
			//			} else {
			//				float missRateDiff = missRate - ins->getMissrate();
			//				long int temp = (long int) (missRate * 1000.0);
			//				if (temp < 0) temp *= -1; //take abs
			//				correlationHist->addElem(temp);
			//			}
			//		} //miss rate is zero by default
			//		if (evaltMissRtCorrel == 0) {
			//			createTraceAndHitMiss(0);
			//		}
			//		iROB->Nth(0)->notifyAllDepICompleted();
			//		iROB->Nth(0)->delDepTableEntris(depTables, coreType);
			//		delete iROB->Nth(0);
			//		iROB->RemoveAt(0);
			//	}
			//}
			//if (evaltMissRtCorrel == 1) {
			//	correlationHist->report();
			//}
		}
	}
	//Final IPC computation
	ipc = (float)(insCount)/(float)(cycle);
}

void bkEnd_finish () {
	//Print Stats on screen
	//printf("line Number = %ld\n", lineNum);
	printf("------------------------------------------\n");
	printf("COMMENT           = %s\n", comment);
	printf("CORE TYPE         = %d\n", coreType);
	printf("Perf Reg Renaming = %d (Size: %d)\n", perfectRegRen, GRRF_SIZE);
	printf("LSQ Model         = %d (LQ: %d, SQ %d)\n", memoryModel, LQ_SIZE, SQ_SIZE);
	if (coreType == PHRASEBLOCK) printf("Num Ins Buff = %d\n", NUM_PHRASEBLKS);
	printf("ROB Size		  = %d\n", ROBsize);
	printf("1 LD Lat Lvl      = %ld\n", oneLevDeepLatLevel);
	printf("Frame Buffer Size	  = %ld\n", SBlength);
	printf("Num Side Buffs		  = %ld\n", numSideBuffs);
	printf("IN FILE	                  = %s\n", inFileName.c_str());
	printf("OUT FILE                  = %s\n", outFileName1.c_str());
	if (reschedule == true) printf("VLIW TRACE FILE = %s\n", reScheduleFileName.c_str());
	if (makePhrase == true) printf("PHRASE TRACE FILE = %s\n", phrasingFileName.c_str());
	if (reportTraceAndHitMiss == true) 
		printf("UNPRED MEM TRACE FILE = %s\n", reScheduleFileName.c_str());
	printf("IPC                       = %f\n", ipc);
	printf("Corrupt Instruction Conut = %ld\n", corruptInsCount);
	printf("Miss  Count            = %ld\n", missCount(cacheLat[0]));
	printf("Hit   Count            = %ld\n", hitCount(0));
	printf("Miss  Latency          = %ld\n", missLatency(cacheLat[0]));
	printf("Hit   Latency          = %ld\n", hitLatency(0));
	printf("TOTAL Latency          = %ld\n", cycle);
	printf("TOTAL MEM Latency      = %ld\n", totalLatency(0));
	printf("FU  Utilization Count  = %ld, %ld, %ld, %ld\n\n",aluStat[0],aluStat[1],aluStat[2],aluStat[3]);
	printf("STALL Event Rate = %ld (%f)\n",fetchStallCycle,(float)fetchStallCycle/(float)cycle);

	printf("Miss  Rate             = %f\n", (float)missCount(cacheLat[0])/(float)(hitCount(0)+missCount(cacheLat[0])));
	printf("Hit   Rate             = %f\n", (float)hitCount(0)/(float)(hitCount(0)+missCount(cacheLat[0])));
	printf("Miss  Avg Latency      = %f\n", (float)missLatency(cacheLat[0])/(float)totalLatency(0));
	printf("Hit   Avg Latency      = %f\n", (float)hitLatency(0)/(float)totalLatency(0));
	printf("FU  Utilization Rate   = %f, %f, %f, %f\n\n",(float)aluStat[0]/(float)cycle,
							     (float)aluStat[1]/(float)cycle,
							     (float)aluStat[2]/(float)cycle,
							     (float)aluStat[3]/(float)cycle);

	printf("L1  Hit Rate           = %f\n", (float)L1hitCount()/(float)(hitCount(0)+missCount(cacheLat[0])));
	printf("L2  Hit Rate           = %f\n", (float)L2hitCount()/(float)(hitCount(0)+missCount(cacheLat[0])));
	printf("L3  Hit Rate           = %f\n", (float)L3hitCount()/(float)(hitCount(0)+missCount(cacheLat[0])));
	printf("MEM Hit Rate           = %f\n\n", (float)MEMhitCount()/(float)(hitCount(0)+missCount(cacheLat[0])));

	printf("Ins ROB   Utilization  = %f\n", (float)iROBSize/(float)cycle);
	printf("Ins Win   Utilization  = %f\n", (float)iWinSize/(float)cycle);
	printf("Res Stn   Utilization  = %f\n", (float)iResStnSize/(float)cycle);
	printf("Ins LQ    Utilization  = %f\n", (float)lqSize/(float)cycle);
	printf("Ins SQ    Utilization  = %f\n", (float)sqSize/(float)cycle);
	printf("Ins RR    Utilization  = %f\n", (float)rrSize/(float)cycle);
	printf("Ins LRF   Utilization  = %lld (%f)\n",lrfCount,(double)lrfCount/(double)insCount);
	printf("Ins GRF   Utilization  = %lld (%f)\n",grfCount,(double)grfCount/(double)insCount);
	printf("LRF WR    Utilization  = %lld (%f)\n",lrfWrCountPerIns,(double)lrfWrCountPerIns/(double)insCount);
	printf("LRF RD    Utilization  = %lld (%f)\n",lrfRdCountPerIns,(double)lrfRdCountPerIns/(double)insCount);
	printf("GRF WR    Utilization  = %lld (%f)\n",grfWrCountPerIns,(double)grfWrCountPerIns/(double)insCount);
	printf("GRF RD    Utilization  = %lld (%f)\n",grfRdCountPerIns,(double)grfRdCountPerIns/(double)insCount);
	printf("Mem Buf   Utilization  = %f\n", (float)iMemBuffSize/(float)cycle);
	printf("Side Buf  Utilization  = %f\n", (float)iSideBufSize/(float)cycle);
	printf("In Flight Utilization  = %f\n\n",(float)inFlightLDopsSize/(float)cycle);

	printf("Tot Num Squash Events   = %d,   Ratio: (%f)\n",squashCount, (float)squashCount/(float)cycle);
	printf("Tot Num Squash Ins      = %lld, Ratio: (%f)\n",squashInsCount, (float)squashInsCount/(float)insCount);
	printf("Tot Num Squash BR Ins   = %lld, Ratio: (%f)\n",squashBRinsCount, (float)squashBRinsCount/(float)insCount);
	printf("Tot Num Squash LD Ins   = %lld, Ratio: (%f)\n",squashLQinsCount, (float)squashLQinsCount/(float)insCount);
	printf("Tot Num Squash ST Ins   = %lld, Ratio: (%f)\n",squashSQinsCount, (float)squashSQinsCount/(float)insCount);
	printf("Tot Num Squash RS Ins   = %lld, Ratio: (%f)\n",squashRSinsCount, (float)squashRSinsCount/(float)insCount);
	printf("Tot Num Squash iWin Ins = %lld, Ratio: (%f)\n",squashWinInsCount, (float)squashWinInsCount/(float)insCount);
	printf("Tot Num Squash iPhr Ins = %lld, Ratio: (%f)\n",squashPhrInsCount, (float)squashPhrInsCount/(float)insCount);
	printf("Tot Num Squash GRF Reg = %lld, Ratio: (%f)\n",squashRegRenCount, (float)squashRegRenCount/(float)insCount);
	printf("Tot Num Squash LRF Reg = %lld, Ratio: (%f)\n",squashLRFcount, (float)squashLRFcount/(float)insCount);
	printf("Tot Num Squash GRF WR Reg (ALL)   = %lld\n",num_squashed_grf_wr_reg);
	printf("Tot Num Squash GRF RD Reg (ALL)   = %lld\n",num_squashed_grf_rd_reg);
	printf("Tot Num Squash LRF WR Reg (ALL)   = %lld\n",num_squashed_lrf_wr_reg);
	printf("Tot Num Squash LRF RD Reg (ALL)   = %lld\n",num_squashed_lrf_rd_reg);
	printf("Tot Num Squash LRF WR Reg (FETCH) = %lld\n",num_squashed_lrf_wr_fetch_reg);
	printf("Tot Num Squash LRF RD Reg (FETCH) = %lld\n",num_squashed_lrf_rd_fetch_reg);
	printf("Tot Num Squash LRF WR Reg (READY) = %lld\n",num_squashed_lrf_wr_ready_reg);
	printf("Tot Num Squash LRF RD Reg (READY) = %lld\n",num_squashed_lrf_rd_ready_reg);
	printf("Tot Num Squash LRF WR Reg (EXECT) = %lld\n",num_squashed_lrf_wr_execute_reg);
	printf("Tot Num Squash LRF RD Reg (EXECT) = %lld\n",num_squashed_lrf_rd_execute_reg);
	printf("Tot Num Squash LRF WR Reg (COMPL) = %lld\n",num_squashed_lrf_wr_complete_reg);
	printf("Tot Num Squash LRF RD Reg (COMPL) = %lld\n",num_squashed_lrf_rd_complete_reg);
	printf("Tot Num Squash GRF WR Reg (FETCH) = %lld\n",num_squashed_grf_wr_fetch_reg);
	printf("Tot Num Squash GRF RD Reg (FETCH) = %lld\n",num_squashed_grf_rd_fetch_reg);
	printf("Tot Num Squash GRF WR Reg (READY) = %lld\n",num_squashed_grf_wr_ready_reg);
	printf("Tot Num Squash GRF RD Reg (READY) = %lld\n",num_squashed_grf_rd_ready_reg);
	printf("Tot Num Squash GRF WR Reg (EXECT) = %lld\n",num_squashed_grf_wr_execute_reg);
	printf("Tot Num Squash GRF RD Reg (EXECT) = %lld\n",num_squashed_grf_rd_execute_reg);
	printf("Tot Num Squash GRF WR Reg (COMPL) = %lld\n",num_squashed_grf_wr_complete_reg);
	printf("Tot Num Squash GRF RD Reg (COMPL) = %lld\n",num_squashed_grf_rd_complete_reg);
	printf("Tot Num SB Activations            = %ld\n",totNumSBactivations);
	printf("Tot Num SB ReActivations          = %ld\n",totNumSBreactivations);
	printf("Tot SB Reactivation Rate          = %f\n",(float)totNumSBreactivations/(float)totNumSBactivations);
	printf("Total SB Size                     = %lu\n",totalSBsize);
	printf("Tot Avg SB Size                   = %f\n",(float)totalSBsize/(float)totNumSBactivations);
	printf("Max SB Size	                      = %ld\n",maxSBsize);
	printf("Min SB Size	                      = %ld\n",minSBsize);
	printf("Total Number of Execute  Ins      = %ld\n",executeInsCount);
	printf("Total Number of Complete Ins      = %ld\n",completeInsCount);
	printf("Total Number of Commit Ins        = %ld\n",insCount);
	printf("Total Num Ins Visit SB            = %ld\n",totInsVisitingSBcount);
	printf("Total Rate of Ins Visiting SB     = %f\n",(float)totInsVisitingSBcount/(float)insCount);
	printf("Tot Ins Exe When SB ON            = %ld\n",totInsCountWhenSBon);
	printf("Tot Ins Exe When SB ON IPC        = %f\n",(float)totInsCountWhenSBon/(float)totSBactiveCycles);
	printf("Tot SB ON Avg. Time               = %f\n\n",(float)(cycle-totSBoffCycles)/(float)cycle);
	printf("Tot Num of Off SB Dep. Ins/Active Win = %f\n",(float)numDepInOtherSBs/(float)totNumSBactivations);
	printf("Num Empty Res Stn Cycles	  = %ld\n", emptyResStation);
	printf("Num Empty Res Stn Cycles/All Cycl = %f\n\n", (float)emptyResStation/(float)cycle);

	if (reportTraceAndHitMiss) {
		printf("Unpredictability Threshold	  = %f\n", (double)unpredMemOpThreshold/(double)COEFF);
		printf("Num Unpred. Ins:		  = %ld\n", unpredMemInsCnt);
		printf("Num Unpred. Ops:		  = %ld\n", unpredMemOpCnt);
		printf("Unpred. Ins Rate:		  = %f\n\n", (float)unpredMemOpCnt/(float)insCount);
	}
	if (makePhrase) {
		printf("Max Phrases Size		  = %d\n", phraseSizeBound);
		printf("Unpredictable Ins Threshold	  = %f\n", (double)unpredMemOpThreshold/(double)COEFF);
		printf("Num Phrases			  = %ld\n", totNumPhrase);
		printf("Avg. Num Ins/Phrase		  = %f\n", (float)insCount/(float)totNumPhrase);
		printf("Tot Num SoftBound Partitions 	  = %ld\n", totNumSoftBound);
		printf("Avg. Num SoftBound/Phrase 	  = %f\n", (float)totNumSoftBound/(float)totNumPhrase);
		printf("Tot Num Unpred. Ins 	          = %ld\n", totNumPhUnpredMemOp);
		printf("Num Unpred. Ins/Phrase 		  = %f\n", (float)totNumPhUnpredMemOp/(float)totNumPhrase);
		printf("Num Unpred. Ins/Total Ins 	  = %f\n", (float)totNumPhUnpredMemOp/(float)insCount);
		printf("Tot Num Root Ins 		  = %ld\n", totNumRootIns);
		printf("Num Root Ins/Phrase 		  = %f\n", (float)totNumRootIns/(float)totNumPhrase);
		printf("Tot Num Phrase Ancestors 	  = %ld\n", totNumPhraseAncestors);
		printf("NumPhrase Ancestors/Phrase 	  = %f\n", (float)totNumPhraseAncestors/(float)totNumPhrase);
		printf("Num of Resetting Times 		  = %ld\n\n", totNumPhGenResets);
		printf("Num Fragment			  = %ld\n", totNumFrag);
		printf("Num Root Phrases		  = %ld\n", totNumRootPh);
		printf("Root Phrases Rate		  = %f\n", (double)totNumRootPh/(double)totNumPhrase);
		printf("Num Critical Path Violations	  = %ld\n", totNumCritPathViol);
		printf("Critical Path Violations Rate	  = %f\n", (double)totNumCritPathViol/(double)totNumPhrase);
		printf("Avg. Num Ins/Fragment		  = %f\n", (float)insCount/(float)totNumFrag);
		printf("Avg. Num Fragment/Phrase	  = %f\n", (float)totNumFrag/(float)totNumPhrase);
	}
	if (coreType == PHRASE || coreType == FRAGMENT) {
		printf("Num Phrases			  = %ld\n", totNumPhrase);
		printf("Avg. Num Ins/Fragment		  = %f\n", (float)insCount/(float)totNumFrag);
		printf("Avg. Num Fragment/Phrase	  = %f\n", (float)totNumFrag/(float)totNumPhrase);
		printf("Avg. Num Ins/Phrase		  = %f\n", (float)insCount/(float)totNumPhrase);
		printf("Latency/Phrase			  = %f\n", (float)totalLatency(0)/(float)totNumPhrase);
		printf("Tot Phrase Stall Time		  = %ld\n", totPhStall);
		printf("Avg. Phrase Stall Time		  = %f\n", (float)totPhStall/(float)totNumPhrase);
		printf("Unexpected Miss Count		  = %ld\n", unexpectedMiss);
		printf("Unexpected Miss Rate		  = %f\n", (float)unexpectedMiss/(float)missCount(cacheLat[0]));
		printf("Unexpected Miss Latency		  = %ld\n", unexpecteedLat);
		printf("Unexpected Avg. Miss Lat	  = %f\n", (float)unexpecteedLat/(float)unexpectedMiss);
		printf("Unexpected Miss Lat Rate	  = %f\n", (float)unexpecteedLat/(float)missLatency(0));
	}
	if (coreType == FRAGMENT) {
		printf("Num Fragment			  = %ld\n", totNumFrag);
		printf("Tot Fragment Stall Time		  = %ld\n", totFrStall);
		printf("Avg. Fragment  Stall Time	  = %f\n", (float)totFrStall/(float)totNumFrag);
		printf("Num Real Fragment		  = %ld\n", totNumRealFrag);
		printf("Real Frag/Tot Frag		  = %f\n", (float)totNumRealFrag/(float)totNumFrag);
		printf("Avg. Num Ins/Real Frag		  = %f\n", (float)totSizRealFrag/(float)totNumRealFrag);
		printf("Num of Single Frag Wavefront	  = %ld\n", totNumOfSigleFragPhrases);
		printf("Avg. Num Real Frag/Phrase	  = %f\n", (double)totNumRealFrag/(double)(totNumPhrase-totNumOfSigleFragPhrases));
		printf("Avg. Num Ready Frags		  = %f\n", (double)numReadyFrags/(double)totNumFrag);
		printf("Inter-Fragment Stall Time	  = %ld\n", interFragStallCycle);
		printf("Inter-Fragment Stall Rate	  = %f\n\n", (double)interFragStallCycle/(double)cycle);
	}



	printf("---    ---    ---    ---    ---    ----   \n");
	for (int  i = 0; i < numSideBuffs; i++) {
		printf("SB INDEX		= %d\n",i);
		printf("Num SB Activations      = %ld\n",numSBactivations[i]);
		printf("Num SB ReActivations    = %ld\n",numSBreactivations[i]);
		printf("SB Reactivation Rate    = %f\n",(float)numSBreactivations[i]/(float)numSBactivations[i]);
		printf("SB Size			= %lu\n",SBsize[i]);
		printf("Avg SB Size             = %f\n",(float)SBsize[i]/(float)numSBactivations[i]);
		printf("Num Ins Visit SB	= %ld\n",insVisitingSBcount[i]);
		printf("Rate of Ins Visiting SB = %f\n",(float)insVisitingSBcount[i]/(float)insCount);
		printf("Ins Exe When SB ON      = %ld\n",InsCountWhenSBon[i]);
		printf("Ins Exe When SB ON IPC  = %f\n",(float)InsCountWhenSBon[i]/(float)SBactiveCycles[i]);
		printf("SB ON Avg. Time         = %f\n\n",(float)(cycle-SBoffCycles[i])/(float)cycle);
		printf("Frame Win Avg. Size    = %f\n",(float)(totFrameSize)/(float)numSBactivations[i]);
		printf("Frame NonSB Ins Avg Cnt= %f\n",(float)(totMainStreamBound)/(float)numSBactivations[i]);
		printf("Frame Window Sat Rate  = %f\n",(float)(windowSatration)/(float)numSBactivations[i]);  //TODO THIS IS NOT A VECTOR ELEMENT, fix
		printf("Frame ON longLat op Rate  = %f\n",(float)(longLatOpWhenSPisON)/(float)numSBactivations[i]);  //TODO THIS IS NOT A VECTOR ELEMENT, fix
		printf("Frame ON-Drain longLat Rat= %f\n",(float)(longLatOpWhenSPisDraining)/(float)numSBactivations[i]);  //TODO THIS IS NOT A VECTOR ELEMENT, fix
		printf("Frame ON-Wait longLat Rate= %f\n",(float)(longLatOpWhenSPisWaiting)/(float)numSBactivations[i]);  //TODO THIS IS NOT A VECTOR ELEMENT, fix
		printf("---    ---    ---    ---    ---    ----   \n");
	}

	printf("\nNumber of MEM Ops       = %ld\n",numMemOps);
	printf("Number of ALU Ops         = %ld\n",numALUOps);
	printf("Number of FPU Ops         = %ld\n",numFPUOps);
	printf("Number of BR  Ops         = %ld\n",numBROps);
	printf("Number of ASSIGN  Ops     = %ld\n",numAssignOps);
	printf("Number of READ Ops        = %ld\n",numReadOps);
	printf("Number of WRITE Op        = %ld\n",numWriteOps);
	printf("Number Mis-Pred BR Ops (Fetch)     = %ld, Rate: %f\n",missPredBROps_fetch, (float)missPredBROps_fetch/(float)numBROps);
	printf("Number Mis-Pred BR Ops (Commit)    = %ld, Rate: %f\n",missPredBROps, (float)missPredBROps/(float)numBROps);
	printf("Number Mis-Pred_NT BR Ops (Fetch)  = %ld, Rate: %f\n",missPredBROps_NT_fetch, (float)missPredBROps_NT_fetch/(float)numBROps);
	printf("Number Mis-Pred_NT BR Ops (Commit) = %ld, Rate: %f\n",missPredBROps_NT, (float)missPredBROps_NT/(float)numBROps);
	printf("Number Pred_NT R Ops (Fetch)       = %ld, Rate: %f\n",numBrOps_predT_fetch, (float)numBrOps_predT_fetch/(float)numBROps);
	printf("Number Pred_T BR Ops (Commit)      = %ld, Rate: %f\n",numBrOps_predT, (float)numBrOps_predT/(float)numBROps);
	printf("Store FWD READ Ops        = %ld\n",stFwdMemOp);
	printf("Store FWD READ/All READ   = %f\n",(float)stFwdMemOp/(float)numReadOps);
	printf("Non-blocking READ Ops     = %ld\n",nonBlockingMemOp);
	printf("Non-blck READ/All READ    = %f\n",(float)nonBlockingMemOp/(float)numReadOps);
	printf("Cache Axes READ Ops       = %ld\n",maxSBsize);
	printf("Cache Axes READ/All READ  = %f\n",(float)maxSBsize/(float)numReadOps);
	printf("MEM Ops	Rate		= %f\n",(float)numMemOps/(float)insCount);
	printf("ALU Ops	Rate		= %f\n",(float)numALUOps/(float)insCount);
	printf("FPU Ops	Rate		= %f\n",(float)numFPUOps/(float)insCount);
	printf("BR  Ops	Rate		= %f\n",(float)numBROps/(float)insCount);
	printf("ASSIGN  Ops	Rate	= %f\n",(float)numAssignOps/(float)insCount);
	printf("READ Ops Rate		= %f\n",(float)numReadOps/(float)insCount);
	printf("WRITE Ops Rate		= %f\n",(float)numWriteOps/(float)insCount);
	printf("READ/MEM  Ops Rate	= %f\n",(float)numReadOps/(float)numMemOps);
	printf("WRITE/MEM Ops Rate	= %f\n\n",(float)numWriteOps/(float)numMemOps);
	printf("------------------------------------------\n");

	/* STRAND EXE STAT */
	if (coreType == STRAND || coreType == PHRASEBLOCK) {
		printf("STRAND COUNT: %lld\n", dyn_num_strands);
		printf("TOT NUM INS IN STRAND: %lld\n", total_num_strand_ins);
		printf("AVG. STRAND SIZE: %f\n", (double)total_num_strand_ins/(double)dyn_num_strands);
		printf("Percent of Ins in Strands: %f\n", (double)total_num_strand_ins/(double)insCount*100.0);
	} 
	/* PHRASEBLOCK EXE STAT */
	else if (coreType == PHRASEBLOCK) {
		printf("Total Num PB/BB Ran: %ld\n", bbCount);
		printf("AVG. Active BB/PB Ratio: %f\n", (double)activeBuffCnt/(double)cycle);
		printf("Average # of Cycles b/w BP Lookup & Update: %f\n", (double)br_pred_update_dist/(double)brInsCount);
	}
	for (int i = 0; i < NUM_FUNC_UNIT; i++)
		printf("Res Stn %d Utilization  = %f\n", i, (float)iResStnsSize[i]/(float)cycle);
	for (int i = 0; i < NUM_PHRASEBLKS; i++)
		printf("Phraseblock %d Utilization  = %f\n", i, (float)pbListsSize[i]/(float)cycle);

	/* Report the branches whose accuracy was not recorded: */
	if (branchProfileFlag) {
		set<long int>::iterator it;
		printf("Report the branches whose accuracy was not recorded:\n");
		for (it = missingAccuracyBranches.begin(); it != missingAccuracyBranches.end(); it++) {
			printf("%ld, ", *it);
		}
		printf("\n");
	}

	//Print Stats into file
	//for (int i = 0; i < 1; i++) {
	//	if (i == 0) outFile = outFile1;
	//	else	    outFile = outFile2;
	outFile = outFile1;

		fprintf(outFile, "------------------------------------------\n");
		fprintf(outFile,"COMMENT		    = %s\n", comment);
		fprintf(outFile, "CORE TYPE         = %d\n", coreType);
		fprintf(outFile, "Perf Reg Renaming = %d (Size: %d)\n", perfectRegRen, GRRF_SIZE);
		fprintf(outFile, "LSQ Model         = %d (LQ: %d, SQ %d)\n", memoryModel, LQ_SIZE, SQ_SIZE);
		if (coreType == PHRASEBLOCK) fprintf(outFile, "Num Ins Buff = %d\n", NUM_PHRASEBLKS);
		fprintf(outFile, "ROB Size		    = %d\n", ROBsize);
		fprintf(outFile, "1 LD Lat Lvl              = %ld\n", oneLevDeepLatLevel);
		fprintf(outFile, "Frame Buffer Size	  = %ld\n", SBlength);
		fprintf(outFile, "Num Side Buffs	    = %ld\n", numSideBuffs);
		fprintf(outFile, "IN FILE		    = %s\n", inFileName.c_str());
		fprintf(outFile, "OUT FILE                  = %s\n", outFileName1.c_str());
		if (reschedule == true) fprintf(outFile,"VLIW TRACE FILE = %s\n", reScheduleFileName.c_str());
		if (makePhrase == true) fprintf(outFile,"PHRASE TRACE FILE = %s\n", phrasingFileName.c_str());
		if (reportTraceAndHitMiss == true) 
					fprintf(outFile,"UNPRED MEM TRACE FILE = %s\n", reScheduleFileName.c_str());
		fprintf(outFile, "IPC                       = %f\n", ipc);
		fprintf(outFile, "Corrupt Instruction Conut = %ld\n", corruptInsCount);
		fprintf(outFile, "Miss  Count               = %ld\n", missCount(cacheLat[0]));
		fprintf(outFile, "Hit   Count               = %ld\n", hitCount(0));
		fprintf(outFile, "Miss  Latency             = %ld\n", missLatency(0));
		fprintf(outFile, "Hit   Latency             = %ld\n", hitLatency(0));
		fprintf(outFile, "TOTAL Latency             = %ld\n", cycle);
		fprintf(outFile, "TOTAL MEM Latency         = %ld\n", totalLatency(0));
		fprintf(outFile, "FU  Utilization Count = %ld, %ld, %ld, %ld\n\n",aluStat[0],aluStat[1],aluStat[2],aluStat[3]);
		fprintf(outFile, "STALL Event Rate = %ld (%f)\n",fetchStallCycle,(float)fetchStallCycle/(float)cycle);
		
		fprintf(outFile, "Miss  Rate             = %f\n", (float)missCount(cacheLat[0])/(float)(hitCount(0)+missCount(cacheLat[0])));
		fprintf(outFile, "Hit   Rate             = %f\n", (float)hitCount(0)/(float)(hitCount(0)+missCount(cacheLat[0])));
		fprintf(outFile, "Miss  Avg Latency      = %f\n", (float)missLatency(cacheLat[0])/(float)totalLatency(0));
		fprintf(outFile, "Hit   Avg Latency      = %f\n", (float)hitLatency(0)/(float)totalLatency(0));
		fprintf(outFile, "FU  Utilization Rate   = %f, %f, %f, %f\n\n", (float)aluStat[0]/(float)cycle,
										(float)aluStat[1]/(float)cycle,
										(float)aluStat[2]/(float)cycle,
										(float)aluStat[3]/(float)cycle);
		
		fprintf(outFile, "L1  Hit Rate           = %f\n",   (float)L1hitCount()/(float)(hitCount(0)+missCount(cacheLat[0])));
		fprintf(outFile, "L2  Hit Rate           = %f\n",   (float)L2hitCount()/(float)(hitCount(0)+missCount(cacheLat[0])));
		fprintf(outFile, "L3  Hit Rate           = %f\n",   (float)L3hitCount()/(float)(hitCount(0)+missCount(cacheLat[0])));
		fprintf(outFile, "MEM Hit Rate           = %f\n\n", (float)MEMhitCount()/(float)(hitCount(0)+missCount(cacheLat[0])));
		
		fprintf(outFile, "Ins ROB   Utilization  = %f\n", (float)iROBSize/(float)cycle);
		fprintf(outFile, "Ins Win   Utilization  = %f\n", (float)iWinSize/(float)cycle);
		fprintf(outFile, "Res Stn   Utilization  = %f\n", (float)iResStnSize/(float)cycle);
		fprintf(outFile, "Ins LQ    Utilization  = %f\n", (float)lqSize/(float)cycle);
		fprintf(outFile, "Ins SQ    Utilization  = %f\n", (float)sqSize/(float)cycle);
		fprintf(outFile, "Ins RR    Utilization  = %f\n", (float)rrSize/(float)cycle);
		fprintf(outFile, "Ins LRF   Utilization  = %lld (%f)\n",lrfCount,(double)lrfCount/(double)insCount);
		fprintf(outFile, "Ins GRF   Utilization  = %lld (%f)\n",grfCount,(double)grfCount/(double)insCount);
		fprintf(outFile, "LRF WR    Utilization  = %lld (%f)\n",lrfWrCountPerIns,(double)lrfWrCountPerIns/(double)insCount);
		fprintf(outFile, "LRF RD    Utilization  = %lld (%f)\n",lrfRdCountPerIns,(double)lrfRdCountPerIns/(double)insCount);
		fprintf(outFile, "GRF WR    Utilization  = %lld (%f)\n",grfWrCountPerIns,(double)grfWrCountPerIns/(double)insCount);
		fprintf(outFile, "GRF RD    Utilization  = %lld (%f)\n",grfRdCountPerIns,(double)grfRdCountPerIns/(double)insCount);
		fprintf(outFile, "Mem Buf   Utilization  = %f\n", (float)iMemBuffSize/(float)cycle);
		fprintf(outFile, "Side Buf  Utilization  = %f\n", (float)iSideBufSize/(float)cycle);
		fprintf(outFile, "In Flight Utilization  = %f\n", (float)inFlightLDopsSize/(float)cycle);

		fprintf(outFile, "Tot Num Squash Events = %d, Ratio: (%f)\n",squashCount, (float)squashCount/(float)cycle);
		fprintf(outFile, "Tot Num Squash Ins    = %lld, Ratio: (%f)\n",squashInsCount, (float)squashInsCount/(float)insCount);
		fprintf(outFile, "Tot Num Squash BR Ins   = %lld, Ratio: (%f)\n",squashBRinsCount, (float)squashBRinsCount/(float)insCount);
		fprintf(outFile, "Tot Num Squash LD Ins = %lld, Ratio: (%f)\n",squashLQinsCount, (float)squashLQinsCount/(float)insCount);
		fprintf(outFile, "Tot Num Squash ST Ins = %lld, Ratio: (%f)\n",squashSQinsCount, (float)squashSQinsCount/(float)insCount);
		fprintf(outFile, "Tot Num Squash RS Ins = %lld, Ratio: (%f)\n",squashRSinsCount, (float)squashRSinsCount/(float)insCount);
		fprintf(outFile, "Tot Num Squash iWin Ins = %lld, Ratio: (%f)\n",squashWinInsCount, (float)squashWinInsCount/(float)insCount);
		fprintf(outFile, "Tot Num Squash iPhr Ins = %lld, Ratio: (%f)\n",squashPhrInsCount, (float)squashPhrInsCount/(float)insCount);
		fprintf(outFile, "Tot Num Squash GRF Reg = %lld, Ratio: (%f)\n",squashRegRenCount, (float)squashRegRenCount/(float)insCount);
		fprintf(outFile, "Tot Num Squash LRF Reg = %lld, Ratio: (%f)\n",squashLRFcount, (float)squashLRFcount/(float)insCount);
		fprintf(outFile, "Tot Num Squash GRF WR Reg (ALL)   = %lld\n",num_squashed_grf_wr_reg);
		fprintf(outFile, "Tot Num Squash GRF RD Reg (ALL)   = %lld\n",num_squashed_grf_rd_reg);
		fprintf(outFile, "Tot Num Squash LRF WR Reg (ALL)   = %lld\n",num_squashed_lrf_wr_reg);
		fprintf(outFile, "Tot Num Squash LRF RD Reg (ALL)   = %lld\n",num_squashed_lrf_rd_reg);
		fprintf(outFile, "Tot Num Squash LRF WR Reg (FETCH) = %lld\n",num_squashed_lrf_wr_fetch_reg);
		fprintf(outFile, "Tot Num Squash LRF RD Reg (FETCH) = %lld\n",num_squashed_lrf_rd_fetch_reg);
		fprintf(outFile, "Tot Num Squash LRF WR Reg (READY) = %lld\n",num_squashed_lrf_wr_ready_reg);
		fprintf(outFile, "Tot Num Squash LRF RD Reg (READY) = %lld\n",num_squashed_lrf_rd_ready_reg);
		fprintf(outFile, "Tot Num Squash LRF WR Reg (EXECT) = %lld\n",num_squashed_lrf_wr_execute_reg);
		fprintf(outFile, "Tot Num Squash LRF RD Reg (EXECT) = %lld\n",num_squashed_lrf_rd_execute_reg);
		fprintf(outFile, "Tot Num Squash LRF WR Reg (COMPL) = %lld\n",num_squashed_lrf_wr_complete_reg);
		fprintf(outFile, "Tot Num Squash LRF RD Reg (COMPL) = %lld\n",num_squashed_lrf_rd_complete_reg);
		fprintf(outFile, "Tot Num Squash GRF WR Reg (FETCH) = %lld\n",num_squashed_grf_wr_fetch_reg);
		fprintf(outFile, "Tot Num Squash GRF RD Reg (FETCH) = %lld\n",num_squashed_grf_rd_fetch_reg);
		fprintf(outFile, "Tot Num Squash GRF WR Reg (READY) = %lld\n",num_squashed_grf_wr_ready_reg);
		fprintf(outFile, "Tot Num Squash GRF RD Reg (READY) = %lld\n",num_squashed_grf_rd_ready_reg);
		fprintf(outFile, "Tot Num Squash GRF WR Reg (EXECT) = %lld\n",num_squashed_grf_wr_execute_reg);
		fprintf(outFile, "Tot Num Squash GRF RD Reg (EXECT) = %lld\n",num_squashed_grf_rd_execute_reg);
		fprintf(outFile, "Tot Num Squash GRF WR Reg (COMPL) = %lld\n",num_squashed_grf_wr_complete_reg);
		fprintf(outFile, "Tot Num Squash GRF RD Reg (COMPL) = %lld\n",num_squashed_grf_rd_complete_reg);
		fprintf(outFile, "Tot Num SB Activations            = %ld\n",totNumSBactivations);
		fprintf(outFile, "Tot Num SB ReActivations          = %ld\n",totNumSBreactivations);
		fprintf(outFile, "Tot SB Reactivation Rate          = %f\n",(float)totNumSBreactivations/(float)totNumSBactivations);
		fprintf(outFile, "Total SB Size                     = %lu\n",totalSBsize);
		fprintf(outFile, "Tot Avg SB Size                   = %f\n",(float)totalSBsize/(float)totNumSBactivations);
		fprintf(outFile, "Max SB Size	                    = %ld\n",maxSBsize);
		fprintf(outFile, "Min SB Size	                    = %ld\n",minSBsize);
		fprintf(outFile, "Total Number of Execute  Ins      = %ld\n",executeInsCount);
		fprintf(outFile, "Total Number of Complete Ins      = %ld\n",completeInsCount);
		fprintf(outFile, "Total Number of Commit Ins        = %ld\n",insCount);
		fprintf(outFile, "Total Num Ins Visit SB            = %ld\n",totInsVisitingSBcount);
		fprintf(outFile, "Total Rate of Ins Visiting SB     = %f\n",(float)totInsVisitingSBcount/(float)insCount);
		fprintf(outFile, "Tot Ins Exe When SB ON            = %ld\n",totInsCountWhenSBon);
		fprintf(outFile, "Tot Ins Exe When SB ON IPC        = %f\n",(float)totInsCountWhenSBon/(float)totSBactiveCycles);
		fprintf(outFile, "SB ON Avg. Time                   = %f\n\n",(float)(cycle-totSBoffCycles)/(float)cycle);
		fprintf(outFile, "Tot Num of Off SB Dep. Ins/Active Win = %f\n",(float)numDepInOtherSBs/(float)totNumSBactivations);
		fprintf(outFile, "Num Empty Res Stn Cycles	    = %ld\n", emptyResStation);
		fprintf(outFile, "Num Empty Res Stn Cycles/All Cycl = %f\n\n", (float)emptyResStation/(float)cycle);

		if (reportTraceAndHitMiss) {
			fprintf(outFile, "Unpredictability Threshold= %f\n", (double)unpredMemOpThreshold/(double)COEFF);
			fprintf(outFile, "Num Unpred. Ins:	    = %ld\n", unpredMemInsCnt);
			fprintf(outFile, "Num Unpred. Ops:	    = %ld\n", unpredMemOpCnt);
			fprintf(outFile, "Unpred. Ins Rate:	    = %f\n", (float)unpredMemOpCnt/(float)insCount);
		}
		if (makePhrase) {
			fprintf(outFile, "Max Phrases Size		  = %d\n", phraseSizeBound);
			fprintf(outFile, "Unpredictable Ins Threshold	  = %f\n", (double)unpredMemOpThreshold/(double)COEFF);
			fprintf(outFile, "Num Phrases			  = %ld\n", totNumPhrase);
			fprintf(outFile, "Avg. Num Ins/Phrase		  = %f\n", (float)insCount/(float)totNumPhrase);
			fprintf(outFile, "Tot Num SoftBound Partitions 	  = %ld\n", totNumSoftBound);
			fprintf(outFile, "Avg. Num SoftBound/Phrase 	  = %f\n", (float)totNumSoftBound/(float)totNumPhrase);
			fprintf(outFile, "Tot Num Unpred. Ins 	          = %ld\n", totNumPhUnpredMemOp);
			fprintf(outFile, "Num Unpred. Ins/Phrase 	  = %f\n", (float)totNumPhUnpredMemOp/(float)totNumPhrase);
			fprintf(outFile, "Num Unpred. Ins/Total Ins 	  = %f\n", (float)totNumPhUnpredMemOp/(float)insCount);
			fprintf(outFile, "Tot Num Root Ins 		  = %ld\n", totNumRootIns);
			fprintf(outFile, "Num Root Ins/Phrase 		  = %f\n", (float)totNumRootIns/(float)totNumPhrase);
			fprintf(outFile, "Tot Num Phrase Ancestors 	  = %ld\n", totNumPhraseAncestors);
			fprintf(outFile, "NumPhrase Ancestors/Phrase 	  = %f\n", (float)totNumPhraseAncestors/(float)totNumPhrase);
			fprintf(outFile, "Num of Resetting Times 	  = %ld\n\n", totNumPhGenResets);
			fprintf(outFile, "Num Fragment			  = %ld\n", totNumFrag);
			fprintf(outFile, "Num Root Phrases		  = %ld\n", totNumRootPh);
			fprintf(outFile, "Root Phrases Rate		  = %f\n", (double)totNumRootPh/(double)totNumPhrase);
			fprintf(outFile, "Num Critical Path Violations	  = %ld\n", totNumCritPathViol);
			fprintf(outFile, "Critical Path Violations Rate	  = %f\n", (double)totNumCritPathViol/(double)totNumPhrase);
			fprintf(outFile, "Avg. Num Ins/Fragment		  = %f\n", (float)insCount/(float)totNumFrag);
			fprintf(outFile, "Avg. Num Fragment/Phrase	  = %f\n", (float)totNumFrag/(float)totNumPhrase);
		}
		if (coreType == PHRASE || coreType == FRAGMENT) {
			fprintf(outFile, "Num Phrases			  = %ld\n", totNumPhrase);
			fprintf(outFile, "Avg. Num Ins/Fragment		  = %f\n", (float)insCount/(float)totNumFrag);
			fprintf(outFile, "Avg. Num Fragment/Phrase	  = %f\n", (float)totNumFrag/(float)totNumPhrase);
			fprintf(outFile, "Avg. Num Ins/Phrase		  = %f\n", (float)insCount/(float)totNumPhrase);
			fprintf(outFile, "Latency/Phrase		  = %f\n", (float)totalLatency(0)/(float)totNumPhrase);
			fprintf(outFile, "Tot Phrase Stall Time		  = %ld\n", totPhStall);
			fprintf(outFile, "Avg. Phrase Stall Time	  = %f\n", (float)totPhStall/(float)totNumPhrase);
			fprintf(outFile, "Unexpected Miss Count		  = %ld\n", unexpectedMiss);
			fprintf(outFile, "Unexpected Miss Rate		  = %f\n", (float)unexpectedMiss/(float)missCount(cacheLat[0]));
			fprintf(outFile, "Unexpected Miss Latency	  = %ld\n", unexpecteedLat);
			fprintf(outFile, "Unexpected Avg. Miss Lat	  = %f\n", (float)unexpecteedLat/(float)unexpectedMiss);
			fprintf(outFile, "Unexpected Miss Lat Rate	  = %f\n", (float)unexpecteedLat/(float)missLatency(0));
		}
		if (coreType == FRAGMENT) {
			fprintf(outFile, "Num Fragment			  = %ld\n", totNumFrag);
			fprintf(outFile, "Tot Fragment Stall Time	  = %ld\n", totFrStall);
			fprintf(outFile, "Avg. Fragment  Stall Time	  = %f\n", (float)totFrStall/(float)totNumFrag);
			fprintf(outFile, "Num Real Fragment		  = %ld\n", totNumRealFrag);
			fprintf(outFile, "Real Frag/Tot Frag		  = %f\n", (float)totNumRealFrag/(float)totNumFrag);
			fprintf(outFile, "Avg. Num Ins/Real Frag	  = %f\n", (float)totSizRealFrag/(float)totNumRealFrag);
			fprintf(outFile, "Num of Single Frag Wavefront	  = %ld\n", totNumOfSigleFragPhrases);
			fprintf(outFile, "Avg. Num Real Frag/Phrase	  = %f\n", (double)totNumRealFrag/(double)(totNumPhrase-totNumOfSigleFragPhrases));
			fprintf(outFile, "Avg. Num Ready Frags		  = %f\n", (double)numReadyFrags/(double)totNumFrag);
			fprintf(outFile, "Inter-Fragment Stall Time	  = %ld\n", interFragStallCycle);
			fprintf(outFile, "Inter-Fragment Stall Rate	  = %f\n\n", (double)interFragStallCycle/(double)cycle);

		}


		fprintf(outFile, "---    ---    ---    ---    ---    ----   \n");
		for (int  i = 0; i < numSideBuffs; i++) {
			fprintf(outFile, "SB INDEX		  = %d\n",i);
			fprintf(outFile, "Num SB Activations      = %ld\n",numSBactivations[i]);
			fprintf(outFile, "Num SB ReActivations    = %ld\n",numSBreactivations[i]);
			fprintf(outFile, "SB Reactivation Rate    = %f\n",(float)numSBreactivations[i]/(float)numSBactivations[i]);
			fprintf(outFile, "SB Size		  = %lu\n",SBsize[i]);
			fprintf(outFile, "Avg SB Size             = %f\n",(float)SBsize[i]/(float)numSBactivations[i]);
			fprintf(outFile, "Num Ins Visit SB	  = %ld\n",insVisitingSBcount[i]);
			fprintf(outFile, "Rate of Ins Visiting SB = %f\n",(float)insVisitingSBcount[i]/(float)insCount);
			fprintf(outFile, "Ins Exe When SB ON      = %ld\n",InsCountWhenSBon[i]);
			fprintf(outFile, "Ins Exe When SB ON IPC  = %f\n",(float)InsCountWhenSBon[i]/(float)SBactiveCycles[i]);
			fprintf(outFile, "SB ON Avg. Time         = %f\n\n",(float)(cycle-SBoffCycles[i])/(float)cycle);
			fprintf(outFile, "Frame Win Avg. Size    = %f\n",(float)(totFrameSize)/(float)numSBactivations[i]);
			fprintf(outFile, "Frame NonSB Ins Avg Cnt= %f\n",(float)(totMainStreamBound)/(float)numSBactivations[i]);
			fprintf(outFile, "Frame Window Sat Rate  = %f\n",(float)(windowSatration)/(float)numSBactivations[i]);  //TODO THIS IS NOT A VECTOR ELEMENT, fix
			fprintf(outFile, "Frame ON longLat op Rate  = %f\n",(float)(longLatOpWhenSPisON)/(float)numSBactivations[i]);  //TODO THIS IS NOT A VECTOR ELEMENT, fix
			fprintf(outFile, "Frame ON-Drain longLat Rat= %f\n",(float)(longLatOpWhenSPisDraining)/(float)numSBactivations[i]);  //TODO THIS IS NOT A VECTOR ELEMENT, fix
			fprintf(outFile, "Frame ON-Wait longLat Rate= %f\n",(float)(longLatOpWhenSPisWaiting)/(float)numSBactivations[i]);  //TODO THIS IS NOT A VECTOR ELEMENT, fix
			fprintf(outFile, "---    ---    ---    ---    ---    ----   \n");
		}


		fprintf(outFile, "\nNumber of MEM Ops       = %ld\n",numMemOps);
		fprintf(outFile, "Number of ALU Ops         = %ld\n",numALUOps);
		fprintf(outFile, "Number of FPU Ops         = %ld\n",numFPUOps);
		fprintf(outFile, "Number of BR  Ops         = %ld\n",numBROps);
		fprintf(outFile, "Number of ASSIGN  Ops     = %ld\n",numAssignOps);
		fprintf(outFile, "Number of READ Ops	    = %ld\n",numReadOps);
		fprintf(outFile, "Number of WRITE Ops	    = %ld\n",numWriteOps);
		fprintf(outFile, "Number Mis-Pred BR Ops (Fetch)     = %ld, Rate: %f\n",missPredBROps_fetch, (float)missPredBROps_fetch/(float)numBROps);
		fprintf(outFile, "Number Mis-Pred BR Ops (Commit)    = %ld, Rate: %f\n",missPredBROps, (float)missPredBROps/(float)numBROps);
		fprintf(outFile, "Number Mis-Pred_NT BR Ops (Fetch)  = %ld, Rate: %f\n",missPredBROps_NT_fetch, (float)missPredBROps_NT_fetch/(float)numBROps);
		fprintf(outFile, "Number Mis-Pred_NT BR Ops (Commit) = %ld, Rate: %f\n",missPredBROps_NT, (float)missPredBROps_NT/(float)numBROps);
		fprintf(outFile, "Number Pred_NT R Ops (Fetch)       = %ld, Rate: %f\n",numBrOps_predT_fetch, (float)numBrOps_predT_fetch/(float)numBROps);
		fprintf(outFile, "Number Pred_T BR Ops (Commit)      = %ld, Rate: %f\n",numBrOps_predT, (float)numBrOps_predT/(float)numBROps);
		fprintf(outFile, "Store FWD READ Ops        = %ld\n",stFwdMemOp);
		fprintf(outFile, "Store FWD READ/All READ   = %f\n",(float)stFwdMemOp/(float)numReadOps);
		fprintf(outFile, "Non-blocking READ Ops     = %ld\n",nonBlockingMemOp);
		fprintf(outFile, "Non-blck READ/All READ    = %f\n",(float)nonBlockingMemOp/(float)numReadOps);
		fprintf(outFile, "Cache Axes READ Ops       = %ld\n",maxSBsize);
		fprintf(outFile, "Cache Axes READ/All READ  = %f\n",(float)maxSBsize/(float)numReadOps);
		fprintf(outFile, "MEM Ops Rate		= %f\n",(float)numMemOps/(float)insCount);
		fprintf(outFile, "ALU Ops Rate		= %f\n",(float)numALUOps/(float)insCount);
		fprintf(outFile, "FPU Ops Rate		= %f\n",(float)numFPUOps/(float)insCount);
		fprintf(outFile, "BR  Ops Rate		= %f\n",(float)numBROps/(float)insCount);
		fprintf(outFile, "ASSIGN  Ops Rate	= %f\n",(float)numAssignOps/(float)insCount);
		fprintf(outFile, "READ Ops Rate		= %f\n",(float)numReadOps/(float)insCount);
		fprintf(outFile, "WRITE Ops Rate	= %f\n",(float)numWriteOps/(float)insCount);
		fprintf(outFile, "READ/MEM  Ops Rate	= %f\n",(float)numReadOps/(float)numMemOps);
		fprintf(outFile, "WRITE/MEM Ops Rate	= %f\n\n",(float)numWriteOps/(float)numMemOps);
		fprintf(outFile, "------------------------------------------\n");
	//}
	//fprintf(outFile, "SB Sizes:\n");
	//for (int x = 0; x < SBsizeList->NumElements(); x++)
	//	fprintf(outFile, "%d\n", SBsizeList->Nth(x));
	//fprintf(outFile, "------------------------------------------\n");
	//fprintf(outFile, "SB Sizes (SORTED):\n");
	//quicksort(SBsizeList,0,SBsizeList->NumElements()-1,cycle);
	//for (int x = 0; x < SBsizeList->NumElements(); x++)
	//	fprintf(outFile, "%d\n", SBsizeList->Nth(x));

	if (coreType == FRAGMENT || coreType == FRAGMENT2) {
		frSizeHist->report(outFile, "Frag Size Hist");
		frLatHist->report(outFile, "Frag Latency Hist");
	} 
	else if (coreType == PHRASE) {
		//phCritPathHist->report(outFile, "Phrase Critical Path Hist");
		//phSizeHist->report(outFile, "Phrase Size Hist");
	}
	if (coreType == STRAND || coreType == PHRASEBLOCK) {
		fprintf(outFile, "STRAND COUNT: %lld\n", dyn_num_strands);
		fprintf(outFile, "TOT NUM INS IN STRAND: %lld\n", total_num_strand_ins);
		fprintf(outFile, "AVG. STRAND SIZE: %f\n", (double)total_num_strand_ins/(double)dyn_num_strands);
		fprintf(outFile, "Percent of Ins in Strands: %f\n", (double)total_num_strand_ins/(double)insCount*100.0);
	} 
	/* PHRASEBLOCK EXE STAT */
	else if (coreType == PHRASEBLOCK) {
		fprintf(outFile, "Total Num PB/BB Ran: %ld\n", bbCount);
		fprintf(outFile, "AVG. Active BB/PB Ratio: %f\n", (double)activeBuffCnt/(double)cycle);
		fprintf(outFile, "Average # of Cycles b/w BP Lookup & Update: %f\n", (double)br_pred_update_dist/(double)brInsCount);
	}
	for (int i = 0; i < NUM_FUNC_UNIT; i++)
		fprintf(outFile, "Res Stn %d Utilization  = %f\n", i, (float)iResStnsSize[i]/(float)cycle);
	for (int i = 0; i < NUM_PHRASEBLKS; i++)
		fprintf(outFile, "Phraseblock %d Utilization  = %f\n", i, (float)pbListsSize[i]/(float)cycle);
	if (reportTraceAndHitMiss == true) {
		correlationHist->report(outFile, "Miss Rate Correlation between INO Schedule and the 'new' schedule");
	}
	numParentsHist->report(outFile, "Num of Ancestors Hist");
	numChildrenHist->report(outFile, "Num of Dependents Hist");
	numParentsHist->report();
	numChildrenHist->report();

	map<long int, int>::iterator wbb_it;
	for (wbb_it = num_ins_exe_cnt.end(); wbb_it != num_ins_exe_cnt.begin(); wbb_it--) {
		long int insAddr = wbb_it->first;
		fprintf(wbbSkipCountFile, "%ld, %d, %d, %f\n",	insAddr,
														num_ins_exe_cnt[insAddr],
														num_bypassed_wbb[insAddr],
														float(num_bypassed_wbb[insAddr])/float(num_ins_exe_cnt[insAddr]));
	}
	map<long int, int>::iterator br_it;
	for (br_it = num_ins_exe_cnt.end(); br_it != num_ins_exe_cnt.begin(); br_it--) {
		long int insAddr = br_it->first;
		fprintf(brSkipAccuracyFile, "%ld, %d, %f, %f\n",insAddr,
														num_ins_exe_cnt[insAddr],
														ins_exe_hoist_accuracy[insAddr],
														float(ins_exe_hoist_accuracy[insAddr])/float(num_ins_exe_cnt[insAddr]));
	}
	fclose(wbbSkipCountFile);
	fclose(brSkipAccuracyFile);
	fclose (pinFile);
	fclose (outFile);
	if (reschedule == true) fclose (reScheduleFile);
	if (makePhrase == true) fclose (phraseFile);
	printf("DONE TRACING!\n");

	delete iResStation;
	delete iMemBuf;
	delete iWindow;
	delete iROB;
	//delete iSideBuff[0];
	delete _L1;
	delete _L2;
	delete _L3;
	delete [] iSideBuff;

	delete [] numSBreactivations;
	delete [] insVisitingSBcount;
	delete [] numSBactivations;
	delete [] InsCountWhenSBon;
	delete [] SBactiveCycles;
	delete [] SBoffCycles;
	delete [] SBsize;
}

void bkEnd_heading(int argc, char const * argv[]) {
	printf("-----------------------------------------\n");
	printf("COMMAND:	  ");
	for (int i = 0; i < argc; i++) {
		printf("%s ", argv[i]);
	}
	printf("\n");
	printf("CACHE LAT:	 = ");
	for (int i = 0; i < MEM_HIGHERARCHY; i++) {
		printf("%d, ", cacheLat[i]);
	}
	printf("\n");
	printf("CORE TYPE         = %d\n", coreType);
	printf("Perf Reg Renaming = %d (Size: %d)\n", perfectRegRen, GRRF_SIZE);
	printf("LSQ Model         = %d (LQ: %d, SQ %d)\n", memoryModel, LQ_SIZE, SQ_SIZE);
	printf("ROB Size	      = %d\n", ROBsize);
	printf("1 LD Lat Lvl      = %ld\n", oneLevDeepLatLevel);
	printf("Frame Buffer Size = %ld\n", SBlength);
	printf("Num Side Buffs	  = %ld\n", numSideBuffs);
	printf("IN FILE		      = %s\n", inFileName.c_str());
	printf("OUT FILE          = %s\n", outFileName1.c_str());
	if (reschedule == true) printf("VLIW TRACE FILE = %s\n", reScheduleFileName.c_str());
	if (makePhrase == true) printf("PHRASE TRACE FILE = %s\n", phrasingFileName.c_str());
	if (reportTraceAndHitMiss == true) 
				printf("UNPRED MEM TRACE FILE = %s\n", reScheduleFileName.c_str());
	printf("COMMENT		 = %s\n", comment);
	printf("-----------------------------------------\n");

	outFile = outFile1;
	fprintf(outFile, "-----------------------------------------\n");
	fprintf(outFile, "COMMAND:	    ");
	for (int i = 0; i < argc; i++) {
		fprintf(outFile, "%s ", argv[i]);
	}
	fprintf(outFile, "\n");
	fprintf(outFile, "CACHE LAT:	   = ");
	for (int i = 0; i < MEM_HIGHERARCHY; i++) {
		fprintf(outFile, "%d, ", cacheLat[i]);
	}
	fprintf(outFile, "\n");
	fprintf(outFile, "CORE TYPE        = %d\n", coreType);
	fprintf(outFile, "ROB Size		   = %d\n", ROBsize);
	fprintf(outFile, "1 LD Lat Lvl     = %ld\n", oneLevDeepLatLevel);
	fprintf(outFile, "Frame Buffer Size= %ld\n", SBlength);
	fprintf(outFile, "Num Side Buffs   = %ld\n", numSideBuffs);
	fprintf(outFile, "IN FILE	   = %s\n", inFileName.c_str());
	fprintf(outFile, "OUT FILE         = %s\n", outFileName1.c_str());
	if (reschedule == true) fprintf(outFile,"VLIW TRACE FILE = %s\n", reScheduleFileName.c_str());
	if (makePhrase == true) fprintf(outFile,"PHRASE TRACE FILE = %s\n", phrasingFileName.c_str());
	if (reportTraceAndHitMiss == true) 
				fprintf(outFile,"UNPRED MEM TRACE FILE = %s\n", reScheduleFileName.c_str());
	fprintf(outFile,"COMMENT		 = %s\n", comment);
	fprintf(outFile, "-----------------------------------------\n");
}
