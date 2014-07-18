#include "dependencyTable.h"
#include "utility.h"
#include "instruction.h"

extern bool reschedule;

dependencyTable::dependencyTable() {
	branchList = new List<instruction*>;
}


dependencyTable::~dependencyTable() {
	delete branchList;
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
		    memReadList.find(addr)->second->getInsID() == ins->getInsID()) {
			memReadList.erase(addr);
		}
		//printf("DEL: Address READ Map Size = %d\n", memReadList.size());
	} else if (table == MEM_WRITE) {
		if (memWriteList.count(addr) > 0 &&
		    memWriteList.find(addr)->second->getInsID() == ins->getInsID()) {
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



void dependencyTable::addReg  (long int reg, instruction* ins, tableType table) {
	Assert (table != MEM_READ && table != MEM_WRITE);

	if (table == REG_WRITE) {
		if (regWriteList.count(reg) > 0) {
			regWriteList.erase(reg);
			//if (reschedule == true) {ins->renameWriteReg(reg);} //NOTE: comment it out if need to do recursive scheduling for missRate refinement
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

void dependencyTable::delReg  (long int reg, instruction* ins, tableType table) {
	Assert (table != MEM_READ && table != MEM_WRITE);

	if (table == REG_WRITE) {
		if (regWriteList.find(reg) != regWriteList.end() &&
		    regWriteList.find(reg)->second->getInsID() == ins->getInsID()) {
			regWriteList.erase(reg);
		}
		//printf("DEL: Address REG Map Size = %d\n", regWriteList.size());
	} else if (table == REG_READ) {
		if (regReadList.find(reg) != regReadList.end() &&
		    regReadList.find(reg)->second->getInsID() == ins->getInsID()) {
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
		if (branchList->Nth(i)->getInsID() == ins->getInsID()) {
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

void dependencyTable::flush_depTables() {
	//This step is redundant as all must already be zero in size
	//(just a check to avoid accumulating bugs)
	memReadList.clear();
	memWriteList.clear();
	regWriteList.clear();
	regReadList.clear();
	for (int i = 0; i < branchList->NumElements(); i++)
		branchList->RemoveAt(i);
}
