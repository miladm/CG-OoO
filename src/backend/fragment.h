/*******************************************************************************
 * fragment.h
 * the fragment class
 ******************************************************************************/
#ifndef _FRAGMENT_H_
#define _FRAGMENT_H_

#include "../lib/list.h"
#include "latency.h"
#include "instruction.h"
#include "quickSort.h"
#include "binarySearch.h"
#include "vliwScheduler.h"

class instruction;

class fragment {
	public:
		fragment();
		~fragment();
		void setFragID(long int id);
		void setMyPhID(long int id);
		INS_ID getFragID();
		void addToFrag(instruction* ins);
		instruction* getNthIns(int i);
		List<instruction*>* getInsList();
		int getFragSize();
		void setNumBits(int numBits);
		int getNumBits();
		void computeFrIdealLat(instruction* ins);
		long int getFrIdealLat();
		void setAllFrAncestors(instruction* ins);
		void removeAncestorIns(instruction* ins);
		bool isReady();
		void computeFrScore();
		double getFrScore();
		long double getFrRelAvgAge();
		double getFrRelNumDepLinksToNxtPh();
		double getFrRelNumUPLDops();
		double getFrRelSize();
		void setPhAvgAge(long double phAvgAge);
		void setPhNumDepLinksToNxtPh(int phNumDepLinksToNxtPh);
		void setPhNumUPLDops(int phNumUPLDops);
		void setPhSize(int phSize);
		void setScore(double frScore);
		void VLIWfrag(int cycle, int numFU);
		void setFragNum(long int num);
		long int getFragNum();
		void setStart(long int start);
		void setEnd(long int start);
		long int getLat();


	private:
		List<instruction*>* _allAncestors; //All ancestors
		List<instruction*>* _fragmentInsList;
		List<instruction*>* _fragmentInsList_sort;
		List<instruction*>* _fragmentInsList_vliw;
		List<int>*	    _memReadAncestorsID;
		INS_ID _id;
		long int _frNum;
		long int _muPhID;
		int _numBits;
		int _idealLat;
		double _frScore;
		long double _phAvgAge;
		double _phNumDepLinksToNxtPh;
		double _phNumUPLDops;
		double _phSize;
		long int _numUPMEMops;
		//STAT
		long int _startCycle;
		long int _endCycle;
		long int _latency;
};

#endif
