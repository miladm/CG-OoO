/*******************************************************************************
 *  regFile.cpp
 ******************************************************************************/

#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include "utility.h"
#include "regFile.h"

using namespace std;

regFile::regFile()
{
    numRegs = NUM_REGISTERS+INIT_RENAME_REG_NUM; //TODO this is NOT Accrate
    //Construct the register file map
    setupRegFile();
    //initialize the register file status
    for (int i = 0; i < NUM_REGISTERS; i++)
	RFstatus[i] = -1;
}
/*
void regFile::setRegStat(int regNum, int insNum) {
	Assert (regNum > 0 && regNum < numRegs && insNum >= 0);
	RFstatus[regNum] = insNum;
}

void regFile::resetRegStat(int regNum) {
	Assert (regNum > 0 && regNum < numRegs);
	RFstatus[regNum] = -1;
}

int regFile::getRegStat(int regNum) {
	Assert (regNum > 0 && regNum < numRegs);
	return RFstatus[regNum];
}
*/
long int regFile::getRegNum(const char* regName) {
	if (RF.count(regName) > 0)
		return RF.find(regName)->second;
	else if (atoi(regName) >= INIT_RENAME_REG_NUM) {
		numRegs++;
		return atoi(regName);
	} else {
		printf("\tERROR: Missing X86 Regiter: %s\n",regName);
		// Assert(true == false && "Didn't find the required register.");
		return INVALID_REG;
	}
}

long int regFile::getNumberOfRegs() {
	return numRegs;
}

//TODO the following two functions are not that useful
int  regFile::addToRegFile(long int reg) {
	string strReg = itos(reg);
	RF.insert(pair<string,long int>(strReg,reg));
}

string regFile::itos(long int number) {
   stringstream ss (stringstream::in | stringstream::out);
   ss << number;
   return ss.str();
}


//This function sets up the x86 register file names map
void regFile::setupRegFile () {
	//General Purpose Registers
	RF.insert(pair<string,long int>("rax",  1));
	RF.insert(pair<string,long int>("rbx",  2));
	RF.insert(pair<string,long int>("rcx",  3));
	RF.insert(pair<string,long int>("rdx",  4));

	RF.insert(pair<string,long int>("eax",  1));
	RF.insert(pair<string,long int>("ebx",  2));
	RF.insert(pair<string,long int>("ecx",  3));
	RF.insert(pair<string,long int>("edx",  4));

	RF.insert(pair<string,long int>("ax",   1));
	RF.insert(pair<string,long int>("bx",   2));
	RF.insert(pair<string,long int>("cx",   3));
	RF.insert(pair<string,long int>("dx",   4));

	RF.insert(pair<string,long int>("al",   1));
	RF.insert(pair<string,long int>("bl",   2));
	RF.insert(pair<string,long int>("cl",   3));
	RF.insert(pair<string,long int>("dl",   4));

	RF.insert(pair<string,long int>("ah",   1));
	RF.insert(pair<string,long int>("bh",   2));
	RF.insert(pair<string,long int>("ch",   3));
	RF.insert(pair<string,long int>("dh",   4));

	// Segment Registers
	RF.insert(pair<string,long int>("cs",   5));
	RF.insert(pair<string,long int>("ds",   6));
	RF.insert(pair<string,long int>("ss",   7));
	RF.insert(pair<string,long int>("es",   8));
	RF.insert(pair<string,long int>("fs",   9));
	RF.insert(pair<string,long int>("gs",  10));

	// Pointer Registers
	RF.insert(pair<string,long int>("rsp", 11));
	RF.insert(pair<string,long int>("rbp", 12));

	RF.insert(pair<string,long int>("esp", 11));
	RF.insert(pair<string,long int>("ebp", 12));

	RF.insert(pair<string,long int>("sp",  11));
	RF.insert(pair<string,long int>("bp",  12));

	RF.insert(pair<string,long int>("spl", 11));
	RF.insert(pair<string,long int>("bpl", 12));

	// Index Registers
	RF.insert(pair<string,long int>("rsi", 13));
	RF.insert(pair<string,long int>("rdi", 14));

	RF.insert(pair<string,long int>("esi", 13));
	RF.insert(pair<string,long int>("edi", 14));

	RF.insert(pair<string,long int>("si",  13));
	RF.insert(pair<string,long int>("di",  14));

	RF.insert(pair<string,long int>("sil", 13));
	RF.insert(pair<string,long int>("dil", 14));

	// Instruction Pointer Register
	RF.insert(pair<string,long int>("rip", 15));

	RF.insert(pair<string,long int>("eip", 15));

	RF.insert(pair<string,long int>("ip",  15));

	// 64-bit mode-only General Purpose Registers
	RF.insert(pair<string,long int>("r8" , 16));
	RF.insert(pair<string,long int>("r9" , 17));
	RF.insert(pair<string,long int>("r10", 18));
	RF.insert(pair<string,long int>("r11", 19));
	RF.insert(pair<string,long int>("r12", 20));
	RF.insert(pair<string,long int>("r13", 21));
	RF.insert(pair<string,long int>("r14", 22));
	RF.insert(pair<string,long int>("r15", 23));

	RF.insert(pair<string,long int>("r8d" ,16));
	RF.insert(pair<string,long int>("r9d" ,17));
	RF.insert(pair<string,long int>("r10d",18));
	RF.insert(pair<string,long int>("r11d",19));
	RF.insert(pair<string,long int>("r12d",20));
	RF.insert(pair<string,long int>("r13d",21));
	RF.insert(pair<string,long int>("r14d",22));
	RF.insert(pair<string,long int>("r15d",23));

	RF.insert(pair<string,long int>("r8w" ,16));
	RF.insert(pair<string,long int>("r9w" ,17));
	RF.insert(pair<string,long int>("r10w",18));
	RF.insert(pair<string,long int>("r11w",19));
	RF.insert(pair<string,long int>("r12w",20));
	RF.insert(pair<string,long int>("r13w",21));
	RF.insert(pair<string,long int>("r14w",22));
	RF.insert(pair<string,long int>("r15w",23));

	RF.insert(pair<string,long int>("r8b" ,16));
	RF.insert(pair<string,long int>("r9b" ,17));
	RF.insert(pair<string,long int>("r10b",18));
	RF.insert(pair<string,long int>("r11b",19));
	RF.insert(pair<string,long int>("r12b",20));
	RF.insert(pair<string,long int>("r13b",21));
	RF.insert(pair<string,long int>("r14b",22));
	RF.insert(pair<string,long int>("r15b",23));

	// Flag
	RF.insert(pair<string,long int>("rflags",24));

	//XMM registers
	RF.insert(pair<string,long int>("xmm0", 25));
	RF.insert(pair<string,long int>("xmm1", 26));
	RF.insert(pair<string,long int>("xmm2", 27));
	RF.insert(pair<string,long int>("xmm3", 28));
	RF.insert(pair<string,long int>("xmm4", 29));
	RF.insert(pair<string,long int>("xmm5", 30));
	RF.insert(pair<string,long int>("xmm6", 31));
	RF.insert(pair<string,long int>("xmm7", 32));
	RF.insert(pair<string,long int>("xmm8", 33));
	RF.insert(pair<string,long int>("xmm9", 34));
	RF.insert(pair<string,long int>("xmm10",35));
	RF.insert(pair<string,long int>("xmm11",36));
	RF.insert(pair<string,long int>("xmm12",37));
	RF.insert(pair<string,long int>("xmm13",38));
	RF.insert(pair<string,long int>("xmm14",39));
	RF.insert(pair<string,long int>("xmm15",40));


	//YMM registers
	RF.insert(pair<string,long int>("ymm0", 25));
	RF.insert(pair<string,long int>("ymm1", 26));
	RF.insert(pair<string,long int>("ymm2", 27));
	RF.insert(pair<string,long int>("ymm3", 28));
	RF.insert(pair<string,long int>("ymm4", 29));
	RF.insert(pair<string,long int>("ymm5", 30));
	RF.insert(pair<string,long int>("ymm6", 31));
	RF.insert(pair<string,long int>("ymm7", 32));
	RF.insert(pair<string,long int>("ymm8", 33));
	RF.insert(pair<string,long int>("ymm9", 34));
	RF.insert(pair<string,long int>("ymm10",35));
	RF.insert(pair<string,long int>("ymm11",36));
	RF.insert(pair<string,long int>("ymm12",37));
	RF.insert(pair<string,long int>("ymm13",38));
	RF.insert(pair<string,long int>("ymm14",39));
	RF.insert(pair<string,long int>("ymm15",40));


	//ZMM registers
	RF.insert(pair<string,long int>("ymm0", 25));
	RF.insert(pair<string,long int>("ymm1", 26));
	RF.insert(pair<string,long int>("ymm2", 27));
	RF.insert(pair<string,long int>("ymm3", 28));
	RF.insert(pair<string,long int>("ymm4", 29));
	RF.insert(pair<string,long int>("ymm5", 30));
	RF.insert(pair<string,long int>("ymm6", 31));
	RF.insert(pair<string,long int>("ymm7", 32));
	RF.insert(pair<string,long int>("xmm8", 33));
	RF.insert(pair<string,long int>("xmm9", 34));
	RF.insert(pair<string,long int>("xmm10",35));
	RF.insert(pair<string,long int>("xmm11",36));
	RF.insert(pair<string,long int>("xmm12",37));
	RF.insert(pair<string,long int>("xmm13",38));
	RF.insert(pair<string,long int>("xmm14",39));
	RF.insert(pair<string,long int>("xmm15",40));
	RF.insert(pair<string,long int>("ymm16",41));
	RF.insert(pair<string,long int>("ymm17",42));
	RF.insert(pair<string,long int>("ymm18",43));
	RF.insert(pair<string,long int>("ymm19",44));
	RF.insert(pair<string,long int>("ymm20",45));
	RF.insert(pair<string,long int>("ymm21",46));
	RF.insert(pair<string,long int>("ymm22",47));
	RF.insert(pair<string,long int>("ymm23",48));
	RF.insert(pair<string,long int>("xmm24",49));
	RF.insert(pair<string,long int>("xmm25",50));
	RF.insert(pair<string,long int>("xmm26",51));
	RF.insert(pair<string,long int>("xmm27",52));
	RF.insert(pair<string,long int>("xmm28",53));
	RF.insert(pair<string,long int>("xmm29",54));
	RF.insert(pair<string,long int>("xmm30",55));
	RF.insert(pair<string,long int>("xmm31",56));

	RF.insert(pair<string,long int>("st0",57));
	RF.insert(pair<string,long int>("st1",58));
	RF.insert(pair<string,long int>("st2",59));
	RF.insert(pair<string,long int>("st3",60));
	RF.insert(pair<string,long int>("st4",61));
	RF.insert(pair<string,long int>("st5",62));
	RF.insert(pair<string,long int>("st6",63));
	RF.insert(pair<string,long int>("st7",64));
	
	RF.insert(pair<string,long int>("mxcsr",65));

	RF.insert(pair<string,long int>("x",66));
	RF.insert(pair<string,long int>("s",67));

	//TODO To be verified
	//RF.insert(pair<string,long int>("p",41));
	//RF.insert(pair<string,long int>("l",42));
	//RF.insert(pair<string,long int>("i",42));
	//RF.insert(pair<string,long int>("di",42));
}



/*
	//General Purpose Registers
	RegFile.insert(pair<string,int>("rax",  1));
	RegFile.insert(pair<string,int>("rbx",  2));
	RegFile.insert(pair<string,int>("rcx",  3));
	RegFile.insert(pair<string,int>("rdx",  4));

	RegFile.insert(pair<string,int>("eax",  5));
	RegFile.insert(pair<string,int>("ebx",  6));
	RegFile.insert(pair<string,int>("ecx",  7));
	RegFile.insert(pair<string,int>("edx",  8));

	RegFile.insert(pair<string,int>("ax",   9));
	RegFile.insert(pair<string,int>("bx",  10));
	RegFile.insert(pair<string,int>("cx",  11));
	RegFile.insert(pair<string,int>("dx",  12));

	RegFile.insert(pair<string,int>("al",  13));
	RegFile.insert(pair<string,int>("bl",  14));
	RegFile.insert(pair<string,int>("cl",  15));
	RegFile.insert(pair<string,int>("dl",  16));

	RegFile.insert(pair<string,int>("ah",  17));
	RegFile.insert(pair<string,int>("bh",  18));
	RegFile.insert(pair<string,int>("ch",  19));
	RegFile.insert(pair<string,int>("dh",  20));

	// Segment Registers
	RegFile.insert(pair<string,int>("cs",  21));
	RegFile.insert(pair<string,int>("ds",  22));
	RegFile.insert(pair<string,int>("ss",  23));
	RegFile.insert(pair<string,int>("es",  24));
	RegFile.insert(pair<string,int>("fs",  25));
	RegFile.insert(pair<string,int>("gs",  26));


	// Pointer Registers
	RegFile.insert(pair<string,int>("rsp", 27));
	RegFile.insert(pair<string,int>("rbp", 28));

	RegFile.insert(pair<string,int>("esp", 29));
	RegFile.insert(pair<string,int>("ebp", 30));

	RegFile.insert(pair<string,int>("sp",  31));
	RegFile.insert(pair<string,int>("bp",  32));

	RegFile.insert(pair<string,int>("spl", 33));
	RegFile.insert(pair<string,int>("bpl", 34));

	// Index Registers
	RegFile.insert(pair<string,int>("rsi", 35));
	RegFile.insert(pair<string,int>("rdi", 36));

	RegFile.insert(pair<string,int>("esi", 37));
	RegFile.insert(pair<string,int>("edi", 38));

	RegFile.insert(pair<string,int>("si",  39));
	RegFile.insert(pair<string,int>("di",  40));

	RegFile.insert(pair<string,int>("sil", 41));
	RegFile.insert(pair<string,int>("dil", 42));

	// Instruction Pointer Register
	RegFile.insert(pair<string,int>("rip", 43));

	RegFile.insert(pair<string,int>("eip", 44));

	RegFile.insert(pair<string,int>("ip",  45));

	// 64-bit mode-only General Purpose Registers
	RegFile.insert(pair<string,int>("r8" , 46));
	RegFile.insert(pair<string,int>("r9" , 47));
	RegFile.insert(pair<string,int>("r10", 48));
	RegFile.insert(pair<string,int>("r11", 49));
	RegFile.insert(pair<string,int>("r12", 50));
	RegFile.insert(pair<string,int>("r13", 51));
	RegFile.insert(pair<string,int>("r14", 52));
	RegFile.insert(pair<string,int>("r15", 53));

	RegFile.insert(pair<string,int>("r8d" ,54));
	RegFile.insert(pair<string,int>("r9d" ,55));
	RegFile.insert(pair<string,int>("r10d",56));
	RegFile.insert(pair<string,int>("r11d",57));
	RegFile.insert(pair<string,int>("r12d",58));
	RegFile.insert(pair<string,int>("r13d",59));
	RegFile.insert(pair<string,int>("r14d",60));
	RegFile.insert(pair<string,int>("r15d",61));

	RegFile.insert(pair<string,int>("r8w" ,62));
	RegFile.insert(pair<string,int>("r9w" ,63));
	RegFile.insert(pair<string,int>("r10w",64));
	RegFile.insert(pair<string,int>("r11w",65));
	RegFile.insert(pair<string,int>("r12w",66));
	RegFile.insert(pair<string,int>("r13w",67));
	RegFile.insert(pair<string,int>("r14w",68));
	RegFile.insert(pair<string,int>("r15w",69));

	RegFile.insert(pair<string,int>("r8b" ,70));
	RegFile.insert(pair<string,int>("r9b" ,71));
	RegFile.insert(pair<string,int>("r10b",72));
	RegFile.insert(pair<string,int>("r11b",73));
	RegFile.insert(pair<string,int>("r12b",74));
	RegFile.insert(pair<string,int>("r13b",75));
	RegFile.insert(pair<string,int>("r14b",76));
	RegFile.insert(pair<string,int>("r15b",77));

	// Flag
	RegFile.insert(pair<string,int>("rflags",78));
*/
