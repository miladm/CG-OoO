/*******************************************************************************
 *  basicblock.h
 ******************************************************************************/

#ifndef _BASICBLOCK_H
#define _BASICBLOCK_H

#include <stdint.h>
#include <stdlib.h>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include "list.h"
#include "global.h"
#include "instruction.h"
#include "phrase.h"

typedef enum {BR_DST, NO_BR_DST} REACHING_TYPE;

class basicblock {
	public:
		basicblock ();
		~basicblock ();
		basicblock& operator= (const basicblock& bb);
		void transferPointersToNewList (List<basicblock*>* bbList);
		
		//Instruction
		void addIns (instruction*, REACHING_TYPE);
		void addIns (instruction*, ADDR);
		ADDR getLastInsDst ();
		ADDR getLastInsFallThru ();
		instruction* getLastIns ();
		List<instruction*>* getInsList ();
		bool isInsAddrInBB (ADDR insAddr);

        //DEPENDENCY TABLE
        void brDependencyTableCheck ();

		//BB INFO
		void printBb ();
		int getBbSize ();
		int getBbSize_ListSch ();
		ADDR getID ();
		void setBBbrHeader (ADDR brAddr);
		ADDR getBBbrHeader ();
		bool hasHeader ();
        ADDR getBBtail ();
		
		//Pointers to other BB
		void setFallThrough (basicblock* bb);
		void setTakenTarget (basicblock* bb);
		void setDescendent (basicblock* bb);
		void setAncestor (basicblock* bb); //Do not use outside class!
		basicblock* getNxtBB ();
		basicblock* getFallThrough ();
		basicblock* getTakenTarget ();
		basicblock* getNthDescendent (int indx);
		basicblock* getNthAncestor (int indx);
		List<basicblock*>* getAncestorList ();
		List<basicblock*>* getDescendentList ();
		int getNumDescendents ();
		int getNumAncestors ();

		//Visit
		void setAsVisited ();
		void setAsUnvisited ();
		bool isVisited ();
		bool isRegAllocated ();
		void setRegAllocated ();
		
		//Dominator / Loop
		bool setDominators ();
		map<ADDR,basicblock*> getDominators ();
		bool setDominators (map<ADDR,basicblock*> &intersection);
		bool setDominators (List<basicblock*>* bbList);
		void buildImmediateDominators ();
		void buildSDominators ();
		void buildDomTree ();
		void addChild (basicblock *child);
		int getChildrenSize ();
		map<ADDR,basicblock*> getChildren ();
		map<ADDR,basicblock*> getDF ();
		void addToDFset (basicblock *node);
		bool isInIDom (ADDR nodeID);
		bool isASDominator (ADDR nodeID);
		int getSDominatorSize ();
		bool isBackEdge ();
		void setupBackEdge ();
		ADDR getBackEdgeDest ();
		int getNumBackEdgeSource ();
		void setAsBackEdgeSource (basicblock* bb);
		List<basicblock*>* getBackEdgeSourceList ();
		basicblock* getNthBackEdgeSource (int i);
 		int numNonBackEdgeAncestors ();
		void markAsEntryPoint ();
		bool isEntryPoint ();
		bool getAllasDominators ();
		void setAllasDominators (bool domSetIsAll);
		
		//SSA / Phi-function
		void insertPhiFunc (long int var);
		map<long int, vector<long int> > getPhiFuncs ();
		void replaceNthPhiOperand (long int var, int indx, long int subscript);
		void setPhiWriteVar (long int var, long int subscript);
		void insertMOVop ();
		int elimPhiFuncs ();
		void insertMOVop (long int dst_var, long int dst_subs, long int src_var, long int src_subs);
	
		//Profile
		float getTakenBias ();
		int getListIndx ();
		void setListIndx (int listIndx);
		
		//Phraseblock
		void addBBtoPBList (ADDR bbID);
		List<ADDR>* getBBListForPB ();
		bool isAPhraseblock ();
		
		// Phrase
		void basicblockToPhrase ();
		
		// List-Scheduling Functions
		void addToBB_ListSchedule (instruction* ins);
		List<instruction*>* getInsList_ListSchedule ();
		
		// Data-flow analysis
		void updateDefSet (long int reg);
		void updateUseSet (long int reg);
		void updateLocalRegSet ();
		bool update_InOutSet ();
		void setupDefUseSets ();
		void renameAllInsRegs ();
		set<long int> getInSet ();
		set<long int> getDefSet ();
		set<long int> getLocalRegSet ();
		bool isInLocalRegSet (long int reg);
		int getLiveVarSize () {
			// for (set<long int>::iterator it = _inSet.begin (); it != _inSet.end (); it++) {
			// 	printf (", %ld", *it);
			// }
			// printf ("\n");
			return _inSet.size ()+_defSet.size ();
		}

	private:
		int _listIndx;
		bool _visited;
        bool _regAllocated;
		bool _entryPoint;
		bool _domSetIsAll;
		bool _hasBrHeader;
		ADDR _brHeaderAddr;
		ADDR bbID;
		basicblock* _fallThroughBB;
		basicblock* _takenTargetBB;
		List<basicblock*>* _ancestorBbList;
		List<basicblock*>* _descendantBbList;
		List<basicblock*>* _backEdgeSourceBbList;
		List<instruction*>* _insListSchList;
		List<instruction*>* _insList_orig;
		List<instruction*>* _insList;
		List<phrase*> *_phList;
		List<ADDR>* _bbListForPhraseblock;
		std::map<ADDR, basicblock*> _dominatorMap;
		std::map<ADDR, basicblock*> _sDominatorMap;
		std::map<ADDR, basicblock*> _parentsMap;  //BB's this is immediately domniated by
		std::map<ADDR, basicblock*> _childrenMap; //BB's this immediately dominates
		std::map<ADDR, basicblock*> _dominanceFrontier; //BB's this immediately dominates
		std::set<ADDR> _dominatorSet;
		std::set<ADDR> _idomSet;
		std::set<ADDR> _insAddrList;
		ADDR _backEdgeDest;
		std::map<long int, vector<long int> > _phiFuncMap;
		std::map<long int, long int> _phiDestMap;

		std::set<long int> _useSet;
		std::set<long int> _defSet;
		std::set<long int> _inSet;
		std::set<long int> _outSet;
		std::set<long int> _localRegSet;
};
#endif

