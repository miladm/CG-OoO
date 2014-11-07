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
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include <map>

#include "list.h"
#include "global.h"
// #include "registerRename.h"

class dependencyTable; /*USED IN ANOTHER VERSION OF C COMPILER*/

class instruction {
	public:
		instruction ();
		~instruction ();

		void setOpCode (const char *);
		void setInsAddr (ADDR);
		void setInsDstAddr (ADDR, bool);
		void setInsFallThruAddr (ADDR, bool);
		void setInsDst (instruction*);
		void setInsFallThru (instruction*);
		void setInsAsm (const char *);
		void setType (const char);
		void setBrTakenBias (double brBias);
		void setBPaccuracy (double bpAccuracy);
		void setLdMissRate (double missRate);
		void setRegister (long int *r, int *rt);
		void setReadVar (int var, int subscript);
		void setWriteVar (int var, int subscript);
		void setArchReg (long int r);
		void setWrMemType ();
		void setRdMemType ();
		void setMemAccessSize (int memSize); //in bytes

		const char *getOpCode ();
		void resetInsDst ();
		void resetInsFallThru ();
		ADDR getInsAddr ();
		ADDR getInsDstAddr ();
		ADDR getInsFallThruAddr ();
		instruction* getInsFallThru ();
		const char *getInsAsm ();
		const char getType ();
		double getBrTakenBias ();
		double getBPaccuracy ();
		double getLdMissRate ();
		int getLatency ();
		int getNthRegType (int i);
		long int getNthReg (int i);
		long int getNthArchReg (int indx);
		long int getNthOldWriteReg (int i);
		long int getNthReadReg (int i);
		long int getNthWriteReg (int i);
		void removeNthRegister (int i);
		int getNumReg ();
		int getNumReadReg ();
		int getNumWriteReg ();
		long int getReadRegSubscript (long int var);
		long int getWriteRegSubscript (long int var);
		void makeUniqueRegs ();
		string getRegisterStr (); //legacy code
		string getArchRegisterStr ();
		int getMemAccessSize (); //in bytes
		bool isRdMemType ();
		bool isWrMemType ();
		bool hasDst ();
		bool hasFallThru ();
		
		/* DEPENDENCY CHECKING METHODS */
		List<instruction*>* getDependents ();
		List<instruction*>* getAncestors ();
		List<instruction*>* getRegAncestors ();
		void setAsDependent (instruction* ins);
		void setAsAncestor (instruction* ins);
		void setAsRegAncestor (instruction* ins);
		void dependencyTableCheck (dependencyTable *depTables);
		bool isInsRepeated (instruction* ins, List<instruction*>*ancestors);
		void setRdAddrSet (set<ADDR> &addrSet);
		void setWrAddrSet (set<ADDR> &addrSet);
		int getNumAncestors ();
		int getNumDependents ();

		/* REGISTER RENAMING */
	    void renameWriteReg (int indx, long int reg);                                                                                                                       
	    void renameReadReg (int indx, long int renReg);                                                                                                           
	    long int getRenamedReg (long int reg);                                                                                                                    
	    bool isRepeated (instruction* temp, List<instruction*>*ancestors);                                                                                        
		
		/* BB/PB RELATED */
		bool isLongestPathSet ();
		int getLongestPath ();
		void resetLongestPath ();
		void setLongestPath (int longestPath);
		void setMy_BBorPB_id (ADDR id);
		ADDR getMy_BB_id ();
		set<ADDR> getMy_PB_id ();
		ADDR getMy_first_PB_id ();
		
		/* REG ALLOCATION */
		void allocatedRegister (long int r_allocated, regKind rk);
		bool isAlreadyAssignedArcRegs ();
		
		/* DATA-FLOW ANALYSIS */
    private:
		void updateDefSet (long int reg);
		void updateUseSet (long int reg);
		void updateLocalRegSet ();
    public:
		bool update_InOutSet (REG_ALLOC_MODE, set<long int>&, bool);
		void setupDefUseSets ();
		void renameAllInsRegs ();
		set<long int> getInSet ();
		set<long int> getOutSet ();
		set<long int> getDefSet ();
		set<long int> getLocalRegSet ();
		bool isInLocalRegSet (long int reg);
		int getLiveVarSize () { return _inSet.size ()+_defSet.size (); }


	private:
		char _opCode[OPCODE_STRING_SIZE];
		char _command[INS_STRING_SIZE];
		set<ADDR> _myBBs;
		ADDR _insAddr;

        /* REGS */
        List<long int> *_r; //SSA REGISTER LIST
        List<long int> *_r_read; //SSA REGISTER LIST
        List<long int> *_r_write; //SSA REGISTER LIST
        List<long int> *_r_write_old; //SSA REGISTER LIST (NOT RENAMED BY SSA)
        List<long int> *_r_allocated; //REGISTER LIST
        List<int> *_rt; //REGISTER TYPE LIST
		List<regKind> *_rk; //REGISTER KIND (LRF=0 VS. GRF=1)

        /* DATA DEPENDENCIES */
		List<instruction*>* _ancestors;
		List<instruction*>* _dependents;
		List<instruction*>* _regAncestors;

        /* SSA */
		map<int,int> _readVar;
		map<int,int> _writeVar;

        /* MEM */
		map<long int,long int> writeRegRenMap;
	    map<long int,long int> readRegRenMap;
        set<ADDR> _memRdAddr;
        set<ADDR> _memWrAddr;
		int _memSize;
		bool _memRead;
		bool _memWrite;

        /* NEXT INS's */
		ADDR _insDstAddr;
		ADDR _insFallThruAddr;
		instruction* _insDst;
		instruction* _insFallThru;
		bool _hasDst;
		bool _hasFallThru;

        /* SCHEDULING */
		int _latency;
		int _longestPath;
		char _insType;
		double _brBias;
		double _bpAccuracy;
		double _missRate;
        MEM_SCH_MODE _mem_sch_mode;

        /* LIVENESS */
		std::set<long int> _useSet;
		std::set<long int> _defSet;
		std::set<long int> _inSet;
		std::set<long int> _outSet;
		std::set<long int> _localRegSet;
};

#endif
