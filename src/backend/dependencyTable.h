#ifndef _DEP_TABLE_H_
#define _DEP_TABLE_H_

#include <map>
#include "../global/global.h"


typedef enum {MEM_READ, MEM_WRITE, REG_WRITE, REG_READ} tableType;
class instruction;

class dependencyTable {
private:
	map<long int,instruction*> memReadList;
	map<long int,instruction*> memWriteList;
	map<long int,instruction*> regWriteList;
	map<long int,instruction*> regReadList;
	List<instruction*>* branchList;

public:
	dependencyTable();
	~dependencyTable();
	void addAddr (long int addr, instruction* ins, tableType table);
	void delAddr (long int addr, instruction* ins, tableType table);
	instruction* addrLookup (long int addr, tableType table);

	void addReg  (long int reg, instruction* ins, tableType table);
	void delReg  (long int reg, instruction* ins, tableType table);
	instruction* regLookup (long int reg, tableType table);

	void addBr  (instruction* ins);
	void delBr  (instruction* ins);
	List<instruction*>* brLookup ();

	void flush_depTables();
};

#endif
