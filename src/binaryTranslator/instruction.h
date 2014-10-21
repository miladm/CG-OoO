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
#include <set>
#include <map>
#include <iostream>
#include <sstream>
#include <string>

#include "list.h"
#include "global.h"
// #include "registerRename.h"

class dependencyTable; /*used in another version of C compiler*/

class instruction {
	public:
		instruction ();
		~instruction ();

		void setOpCode (const char *);
		void setInsAddr (ADDR);
		void setInsDst (ADDR, bool);
		void setInsFallThru (ADDR, bool);
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
		ADDR getInsAddr ();
		ADDR getInsDst ();
		ADDR getInsFallThru ();
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
		
		/* Dependency checking methods */
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

		/* Register Renaming */
	    void renameWriteReg (int indx, long int reg);                                                                                                                       
	    void renameReadReg (int indx, long int renReg);                                                                                                           
	    long int getRenamedReg (long int reg);                                                                                                                    
	    bool isRepeated (instruction* temp, List<instruction*>*ancestors);                                                                                        
		
		/* BB/PB Related */
		bool isLongestPathSet ();
		int getLongestPath ();
		void resetLongestPath ();
		void setLongestPath (int longestPath);
		void setMy_BBorPB_id (ADDR id);
		ADDR getMy_BB_id ();
		set<ADDR> getMy_PB_id ();
		ADDR getMy_first_PB_id ();
		
		/* Reg Allocation */
		void allocatedRegister (long int r_allocated, regKind rk);
		bool isAlreadyAssignedArcRegs ();
		

	private:
		char _opCode[OPCODE_STRING_SIZE];
		char _command[INS_STRING_SIZE];
        List<long int> *_r;  //SSA Register List
        List<long int> *_r_read;  //SSA Register List
        List<long int> *_r_write;  //SSA Register List
        List<long int> *_r_write_old;  //SSA Register List (not renamed by SSA)
        List<long int> *_r_allocated;  //Register List
        List<int> *_rt; //Register Type List
		List<regKind> *_rk; //Register Kind (LRF=0 vs. GRF=1)
		List<instruction*>* _ancestors;
		List<instruction*>* _dependents;
		List<instruction*>* _regAncestors;
		map<int,int> _readVar;
		map<int,int> _writeVar;
		map<long int,long int> writeRegRenMap;
	    map<long int,long int> readRegRenMap;
        set<ADDR> _memRdAddr;
        set<ADDR> _memWrAddr;
		set<ADDR> _myBBs;
		ADDR _insAddr;
		ADDR _insDst;
		ADDR _insFallThru;
		int _latency;
		int _memSize;
		int _longestPath;
		bool _hasDst;
		bool _hasFallThru;
		bool _memRead;
		bool _memWrite;
		char _insType;
		double _brBias;
		double _bpAccuracy;
		double _missRate;
        MEM_SCH_MODE _mem_sch_mode;
};

#endif
