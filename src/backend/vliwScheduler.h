/*******************************************************************************
 *  vliwScheduler.h
 *  vliwScheduler object. schedules the instructions into a VLIW schedule
 ******************************************************************************/

#ifndef _VLIWSCHEDULER_H_
#define _VLIWSCHEDULER_H_

#include "../lib/list.h"
#include "../global/global.h"
#include "dependencyTable.h"
#include "quickSort.h"
#include "phrase.h"
#include "parser.h"

class instruction;

class vliwScheduler {
    public:
	vliwScheduler();
	~vliwScheduler();
	void findMostCritIns(List<instruction*> *list, int cycle, bool UPLDhoisting);
	bool scheduleIns(List<instruction*>* inList, List<instruction*>* outList, int cycle, bool UPLDhoisting, long int phraseID);
	bool scheduleIns_1FU(List<instruction*>* inList, List<instruction*>* outList, int cycle, bool UPLDhoisting, long int phraseID);
	bool scheduleInsStream(List<instruction*>* inList, int cycle, bool UPLDhoisting, FILE* reScheduleFile, float unpredMemOpThreshold);
	bool scheduleInsStream_1FU(List<instruction*>* inList, int cycle, bool UPLDhoisting, FILE* reScheduleFile);
	void sortLists(List<instruction*> *rootALUins, List<instruction*> *rootMEMins, int cycle, bool UPLDhoisting, long int phraseID);
	void completeIns (int cycle,List<instruction*>* list, bool del);
	instruction* setupNewIns(instruction* ins, int cycle);
	void parseIns (int ROBsize, parser* parse, int cycle);
	void delFromInsMap(instruction* ins);
	void insertToInsMap(instruction* newIns);
	int getSizeOfInsMap();
	void injectIns(instruction* ins);
	
	
	//WAvefront Scheduling
	bool schedulePhraseinsStream(List<instruction*>* inList, int cycle, bool UPLDhoisting, FILE* reScheduleFile, float unpredMemOpThreshold, List<phrase*>* phList);
	

    private:
	List<instruction*> *rootALUins;
	List<instruction*> *rootMEMins;
	List<instruction*> *outListTemp;
	List<int> *rootInsIndx;
	long int rootSize;
	long int phID;
	map<long int,instruction*> insMap;
	long int tempReg;
};

#endif

