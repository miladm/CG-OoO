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
#include "stats.h"

typedef enum {BR_DST, NO_BR_DST} REACHING_TYPE;

class sub_block {
    public:
        sub_block () { _insList = new List<instruction*>; }
        ~sub_block () { delete _insList; }
        set<ADDR> _upld_set;
        List<instruction*>* _insList;
        SUB_BLK_ID _id;
};

class basicblock {
	public:
		basicblock ();
		~basicblock ();
		basicblock& operator= (const basicblock& bb);
		void transferPointersToNewList (List<basicblock*>* bbList);
		
		//INSTRUCTION
		void addIns (instruction*, REACHING_TYPE);
		void addIns (instruction*, ADDR);
        void forceAssignBBID ();
		void addMovIns (instruction*);
        bool isThisBBfallThru (basicblock*);
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
		void resetBBbrHeader ();
		ADDR getBBbrHeader ();
		bool hasHeader ();
        ADDR getBBtail ();
		
		//POINTERS TO OTHER BB
		bool hasFallThrough ();
		void setFallThrough (basicblock*);
		void resetFallThrough ();
		bool hasTakenTarget ();
		void setTakenTarget (basicblock*);
		void resetTakenTarget ();
		void setDescendent (basicblock*);
        void resetDescendents ();
		void setAncestor (basicblock*); //Do not use outside class!
        void resetAncestor (ADDR);
		basicblock* getNxtBB ();
		basicblock* getFallThrough ();
		basicblock* getTakenTarget ();
		basicblock* getNthDescendent (int indx);
		basicblock* getNthAncestor (int indx);
		List<basicblock*>* getAncestorList ();
		List<basicblock*>* getDescendentList ();
		int getNumDescendents ();
		int getNumAncestors ();

		//VISIT
		void setAsVisited ();
		void setAsUnvisited ();
		bool isVisited ();
		bool isRegAllocated ();
		void setRegAllocated ();
		
		//DOMINATOR / LOOP
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
		void resetDF ();
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
		
		//SSA / PHI-FUNCTION
		void insertPhiFunc (long int var);
		map<long int, vector<long int> > getPhiFuncs ();
		void replaceNthPhiOperand (long int var, int indx, long int subscript);
		void setPhiWriteVar (long int var, long int subscript);
		int elimPhiFuncs (ADDR&, map<ADDR,instruction*>*);
		void insertMOVop (long int dst_var, long int dst_subs, long int src_var, long int src_subs, ADDR insAddr);
	
		//PROFILE
		float getTakenBias ();
		int getListIndx ();
		void setListIndx (int listIndx);
		
		//PHRASEBLOCK
		void addBBtoPBList (ADDR bbID);
		List<ADDR>* getBBListForPB ();
		bool isAPhraseblock ();
		
		// PHRASE
		void basicblockToPhrase ();
		
		// LIST-SCHEDULING FUNCTIONS
		void addToBB_ListSchedule (instruction* ins);
		List<instruction*>* getInsList_ListSchedule ();


        // STATS
        void setsupStats ();
        void reportStats ();
		
        // UPLD
        void findRootUPLD ();
        void markUPLDroot (instruction*, ADDR);
        void markUPLDroots ();
        void makeSubBlocks ();

		// DATA-FLOW ANALYSIS
    private:
		void updateDefSet (long int reg);
		void updateUseSet (long int reg);
    public:
		bool update_InOutSet (REG_ALLOC_MODE);
        void update_locGlbSet ();
        int update_locToGlb ();
		void setupDefUseSets ();
		void renameAllInsRegs ();
		set<long int> getInSet ();
		set<long int> getDefSet ();

        // LOCAL REGISTERS
		set<long int> getLocalRegSet ();
		bool isInLocalRegSet (long int reg);
		void updateLocalRegSet ();
		int getLiveVarSize () { return _inSet.size () + _defSet.size (); }

    private:
        int _sub_blk_id;
        map<SUB_BLK_ID, sub_block*> sub_blk_map;

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
        List<instruction*>* _upld_roots;
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
		std::set<long int> _bbLocGlbRegSet;

        block_stats _stats;
};
#endif

