/*******************************************************************************
 * phrase.h
 * the phrase class
 ******************************************************************************/
#ifndef _PHRASE_H
#define _PHRASE_H

#include "../lib/list.h"
#include "latency.h"
#include "quickSort.h"
#include "binarySearch.h"
#include "unique.h"
#include "fragment.h"

#define MAX_NUM_INS_VISITORS 100
#define CRIT_PATH_LEN_LIMIT 10

typedef enum {noPhStatus, phOpen, phClosed, phWaitList, phReady, phDone} phraseState;

class instruction;

class phrase {
	public:
		phrase();
		phrase(int id);
		~phrase();

		void setMemReadAncestors(instruction* ins);
		void setAllPhAncestors(instruction* ins);

		instruction* getNthIns(int i);
		instruction* getNthIns_unsort(int i);
		List<instruction*>* getInsList_unsort();
		void addToPhrase(instruction* ins);
		void addToPhrase_Light(instruction* ins); //Low compute overhead version of addPhrase()
		void removeFromPhrase(int indx);
		void removeAncestorIns(instruction* ins);

		bool chkMemRdAncestors(instruction* ins);
		bool chkMemRdAncestorsV1(instruction* ins);

		void printToFilePhrase(FILE* phraseFile);

		void incPhraseAge();
		int getPhraseAge();

		void setState(phraseState state);
		phraseState getState();

		void VLIWphrase(int cycle, int numFU);
		int findCriticalPath();
		
		bool isPhraseComplete();

		/* Fragment */
		void makeFragment();
		long int encodeAndReset(bool* fragID, int size);
		int findFragID(instruction* ins, bool* fragID, instruction* root);
		void printToFileFragment(FILE* phraseFile);
		void breakFragmentInHalf();

		/* STAT */
		int getPhraseSize();
		int getPhraseSize_unsort();
		int getNumAllPhraseAncestors();
		instruction* getNthPhraseAncestor(int i);
		int getPhraseID();
		long int getNumUPLDops();
		int getNumPhraseAncestors();
		int getNumRootIns();
		void computePhIdealLat(instruction* ins);
		long int getPhIdealLat();
		long int getNumSoftBounds();
		long int getNumUPLDancestors();
		long int getNumFrags();
		long double getPhAvgAge();
		void computePhAvgAge();

	private:
		List<instruction*>* _phraseInsList_sort;
		List<instruction*>* _phraseInsList_vliw;
		List<instruction*>* _phraseInsList;
		List<instruction*>* _allAncestors; //All ancestors
		List<instruction*>* _memReadAncestors; //Only Unpredictab LD op ancestors
		List<int>*	    _memReadAncestorsID; //Only Unpredictab LD op ancestors
		List<fragment*>*    _frags;

		int _id;
		int _phAge;
		phraseState _state;

		long int _numUPMEMops;
		int	 _numRootIns;
		int	 _numPhraseAncestors;
		long int _idealLat;
		long int _numOfFragments;
		long double _phAvgAge;
};

#endif
