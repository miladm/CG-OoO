/*******************************************************************************
 *  dependencyTable.h
 ******************************************************************************/

#ifndef _DEP_TABLE_H_
#define _DEP_TABLE_H_


#include <stdint.h>
#include <stdlib.h>
#include <map>
#include "global.h"
#include "instruction.h"
class instruction;  /*used in another version of C compiler*/

typedef enum {MEM_READ, MEM_WRITE, REG_WRITE, REG_READ} tableType;

class dependencyTable {
	private:
		std::map<ADDR,instruction*> memReadList;
		std::map<ADDR,instruction*> memWriteList;
		std::map<ADDR,instruction*> regWriteList;
		std::map<ADDR,instruction*> regReadList;
		List<instruction*>* branchList;
		List<instruction*>* memWrites;

	public:
		dependencyTable();
		~dependencyTable();
		void addAddr (long int addr, instruction* ins, tableType table);
		void delAddr (long int addr, instruction* ins, tableType table);
		instruction* addrLookup (long int addr, tableType table);

		void addReg  (int indx, long int reg, instruction* ins, tableType table);
		void delReg  (long int reg, instruction* ins, tableType table);
		instruction* regLookup (long int reg, tableType table);

		void addBr  (instruction* ins);
		void delBr  (instruction* ins);
		List<instruction*>* brLookup ();
		
		void addWr (instruction* ins);
		void delWr (instruction* ins);
		List<instruction*>* wrLookup();
};

#endif
