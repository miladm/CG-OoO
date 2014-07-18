/*******************************************************************************
 *  dependencyTable.cpp
 ******************************************************************************/

#include "utility.h"
#include "dependencyTable.h"
#include "instruction.h"

bool reschedule;

dependencyTable::dependencyTable() {
	branchList = new List<instruction*>;
	memWrites = new List<instruction*>;
}

dependencyTable::~dependencyTable() {
	delete branchList;
	delete memWrites;
}

void dependencyTable::addAddr (long int addr, instruction* ins, tableType table) {
	Assert (table != REG_WRITE && table != REG_READ);

	if (table == MEM_READ) {
		if (memReadList.count(addr) > 0) {
			memReadList.erase(addr);
		}
		memReadList.insert(pair<long int,instruction*>(addr,ins));
		//printf("ADD: Address READ Map Size = %d\n", memReadList.size());
	} else if (table == MEM_WRITE) {
		if (memWriteList.count(addr) > 0) {
			memWriteList.erase(addr);
		}
		memWriteList.insert(pair<long int,instruction*>(addr,ins));
		//printf("ADD: Address WRITE Map Size = %d\n", memWriteList.size());
	}
}

void dependencyTable::delAddr (long int addr, instruction* ins, tableType table) {
	Assert (table != REG_WRITE && table != REG_READ);
	if (table == MEM_READ) {
		if (memReadList.count(addr) > 0 &&
		    memReadList.find(addr)->second->getInsAddr() == ins->getInsAddr()) {
			memReadList.erase(addr);
		}
		//printf("DEL: Address READ Map Size = %d\n", memReadList.size());
	} else if (table == MEM_WRITE) {
		if (memWriteList.count(addr) > 0 &&
		    memWriteList.find(addr)->second->getInsAddr() == ins->getInsAddr()) {
			memWriteList.erase(addr);
		}
		//printf("DEL: Address WRITE Map Size = %d\n", memWriteList.size());
	}
}

instruction* dependencyTable::addrLookup (long int addr, tableType table) {
	Assert (table != REG_WRITE && table != REG_READ);

	if (table == MEM_READ && memReadList.count(addr) > 0) {
		return memReadList.find(addr)->second;
	} else if (table == MEM_WRITE && memWriteList.count(addr) > 0) {
		return memWriteList.find(addr)->second;
	} else {
		return NULL; //Item was not found
	}
}



void dependencyTable::addReg  (int indx, long int reg, instruction* ins, tableType table) {
	Assert (table != MEM_READ && table != MEM_WRITE);

	if (table == REG_WRITE) {
		if (regWriteList.count(reg) > 0 || regReadList.count(reg) > 0) {
			regWriteList.erase(reg);
			ins->renameWriteReg(indx, reg);
		} else if (regReadList.count(reg) > 0) {
			/*We assume that no regiter is ever read before having been written B4*/
			printf ("\tERROR: Register Renaming Violation (%s, line: %d)\n", __FILE__, __LINE__);
		}
		regWriteList.insert(pair<long int,instruction*>(reg,ins));
		//printf("ADD: Address REG Map Size = %d\n", regWriteList.size());
	} else if (table == REG_READ) {
		if (regReadList.count(reg) > 0) {
			regReadList.erase(reg);
		}
		regReadList.insert(pair<long int,instruction*>(reg,ins));
		//printf("ADD: Address REG Map Size = %d\n", regReadList.size());
	}
}

void dependencyTable::delReg  (long int reg, instruction* ins, tableType table){
	Assert (table != MEM_READ && table != MEM_WRITE);

	if (table == REG_WRITE) {
		if (regWriteList.count(reg) > 0 &&
		    regWriteList.find(reg)->second->getInsAddr() == ins->getInsAddr()) {
			regWriteList.erase(reg);
		}
		//printf("DEL: Address REG Map Size = %d\n", regWriteList.size());
	} else if (table == REG_READ) {
		if (regReadList.count(reg) > 0 &&
		    regReadList.find(reg)->second->getInsAddr() == ins->getInsAddr()) {
			regReadList.erase(reg);
		}
		//printf("DEL: Address REG Map Size = %d\n", regReadList.size());
	}
}

instruction* dependencyTable::regLookup (long int reg, tableType table) {
	Assert (table != MEM_READ && table != MEM_WRITE);

	if (table == REG_WRITE && regWriteList.count(reg) > 0)
		return regWriteList.find(reg)->second;
	else if (table == REG_READ && regReadList.count(reg) > 0)
		return regReadList.find(reg)->second;
	else
		return NULL; //Item not found
}


void dependencyTable::addBr  (instruction* ins) {
	branchList->Append(ins);
}

void dependencyTable::delBr  (instruction* ins){
	int initBrListSiz = branchList->NumElements();
	for (int i = 0; i < branchList->NumElements(); i++) {
		if (branchList->Nth(i)->getInsAddr() == ins->getInsAddr()) {
			branchList->RemoveAt(i);
			break;
		}
	}
	//if (branchList->NumElements() != initBrListSiz-1) printf("%d,%d\n", initBrListSiz,branchList->NumElements());
	Assert(branchList->NumElements() == initBrListSiz-1 || (initBrListSiz == 0));
}

List<instruction*>* dependencyTable::brLookup () {
	return branchList;
}


void dependencyTable::addWr  (instruction* ins) {
	memWrites->Append(ins);
}

void dependencyTable::delWr  (instruction* ins){
	int initBrListSiz = memWrites->NumElements();
	for (int i = 0; i < memWrites->NumElements(); i++) {
		if (memWrites->Nth(i)->getInsAddr() == ins->getInsAddr()) {
			memWrites->RemoveAt(i);
			break;
		}
	}
}

List<instruction*>* dependencyTable::wrLookup () {
	return memWrites;
}