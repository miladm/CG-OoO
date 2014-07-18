/*******************************************************************************
 * phraseGen.h
 * generated phrases for each instruction thru interacting with the instruction class
 ******************************************************************************/
#ifndef _PHRASE_GEN_H
#define _PHRASE_GEN_H

#include "../global/global.h"
#include "phrase.h"
//#include "instruction.h"

class phraseGen {
	public:
		phraseGen();
		~phraseGen();
		int runPhraseGen(List<instruction*> *iROB, List<instruction*> *iWindow, FILE* phraseFile, dependencyTable *depTables, bool eof, int ROBsize);
		void genPhraseStat(int indx);
		void closeOldPhrase(int phIndx);
		int findMyPhrase(instruction* ins);
		void storeToFile(int indx, FILE* phraseFile, dependencyTable *depTables);
		void findReadyPhrases(FILE* phraseFile, dependencyTable *depTables, List<instruction*> *iROB);
		int commitIns (int cycle, List<instruction*> *iROB);
		bool isPhraseOld(int phIndx);
		bool isPhraseReady(int phIndx);
		int takeMax(int a, int b);
		/* STAT */
		long int getTotNumPhrases();
		long int getTotNumSoftBound();
		long int getTotNumPhraseUPLD();
		long int getTotNumRootIns();
		long int getTotNumPhraseAncestors();
		long int getNumPhGenReset();
		long int getTotNumIns();
		long int getTotNumFrags ();
		long int getTotNumRootPh ();
		long int getTotNumCritPathViolations();
		

	private:
		List<phrase*> *openPh;
		List<phrase*> *closePh;
		List<int>     *closedPhIDs;
		long int topPhraseID;
		long int resetCount;
		/* STAT */
		long int totNumPhraseAncestors;
		long int totNumRootIns;
		long int totNumPhUnpredMemOp;
		long int insCount;
		long int totNumSoftBounds;
		long int totRootPhrases;
		long int totNumFragments;
		long int numCritPathViolations;
};

#endif
