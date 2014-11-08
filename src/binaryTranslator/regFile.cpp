/*******************************************************************************
 *  regFile.cpp
 ******************************************************************************/

#include "regFile.h"

using namespace std;

regFile::regFile () {
    // CONSTRUCT THE REGISTER FILE MAP
    setupRegFile ();
    setupSpecialRegFile ();
}

long int regFile::getRegNum (const char* regName) {
	if (RF.count (regName) > 0) {
		return RF.find (regName)->second;
	} else {
		return INVALID_REG;
	}
}

long int regFile::getSpecialRegNum (const char* regName) {
	if (SRF.count (regName) > 0) {
		return SRF.find (regName)->second;
	} else {
		return INVALID_REG;
	}
}

//THIS FUNCTION SETS UP THE X86 REGISTER FILE NAMES MAP
void regFile::setupRegFile () {
	//GENERAL PURPOSE REGISTERS
	RF.insert (pair<string,long int> ("rax",  1));
	RF.insert (pair<string,long int> ("rbx",  2));
	RF.insert (pair<string,long int> ("rcx",  3));
	RF.insert (pair<string,long int> ("rdx",  4));

	RF.insert (pair<string,long int> ("eax",  1));
	RF.insert (pair<string,long int> ("ebx",  2));
	RF.insert (pair<string,long int> ("ecx",  3));
	RF.insert (pair<string,long int> ("edx",  4));

	RF.insert (pair<string,long int> ("ax",   1));
	RF.insert (pair<string,long int> ("bx",   2));
	RF.insert (pair<string,long int> ("cx",   3));
	RF.insert (pair<string,long int> ("dx",   4));

	RF.insert (pair<string,long int> ("al",   1));
	RF.insert (pair<string,long int> ("bl",   2));
	RF.insert (pair<string,long int> ("cl",   3));
	RF.insert (pair<string,long int> ("dl",   4));

	RF.insert (pair<string,long int> ("ah",   1));
	RF.insert (pair<string,long int> ("bh",   2));
	RF.insert (pair<string,long int> ("ch",   3));
	RF.insert (pair<string,long int> ("dh",   4));

	// 64-BIT MODE-ONLY GENERAL PURPOSE REGISTERS
	RF.insert (pair<string,long int> ("r8" , 16));
	RF.insert (pair<string,long int> ("r9" , 17));
	RF.insert (pair<string,long int> ("r10", 18));
	RF.insert (pair<string,long int> ("r11", 19));
	RF.insert (pair<string,long int> ("r12", 20));
	RF.insert (pair<string,long int> ("r13", 21));
	RF.insert (pair<string,long int> ("r14", 22));
	RF.insert (pair<string,long int> ("r15", 23));

	RF.insert (pair<string,long int> ("r8d" ,16));
	RF.insert (pair<string,long int> ("r9d" ,17));
	RF.insert (pair<string,long int> ("r10d",18));
	RF.insert (pair<string,long int> ("r11d",19));
	RF.insert (pair<string,long int> ("r12d",20));
	RF.insert (pair<string,long int> ("r13d",21));
	RF.insert (pair<string,long int> ("r14d",22));
	RF.insert (pair<string,long int> ("r15d",23));

	RF.insert (pair<string,long int> ("r8w" ,16));
	RF.insert (pair<string,long int> ("r9w" ,17));
	RF.insert (pair<string,long int> ("r10w",18));
	RF.insert (pair<string,long int> ("r11w",19));
	RF.insert (pair<string,long int> ("r12w",20));
	RF.insert (pair<string,long int> ("r13w",21));
	RF.insert (pair<string,long int> ("r14w",22));
	RF.insert (pair<string,long int> ("r15w",23));

	RF.insert (pair<string,long int> ("r8b" ,16));
	RF.insert (pair<string,long int> ("r9b" ,17));
	RF.insert (pair<string,long int> ("r10b",18));
	RF.insert (pair<string,long int> ("r11b",19));
	RF.insert (pair<string,long int> ("r12b",20));
	RF.insert (pair<string,long int> ("r13b",21));
	RF.insert (pair<string,long int> ("r14b",22));
	RF.insert (pair<string,long int> ("r15b",23));

	//XMM REGISTERS
	RF.insert (pair<string,long int> ("xmm0", 25));
	RF.insert (pair<string,long int> ("xmm1", 26));
	RF.insert (pair<string,long int> ("xmm2", 27));
	RF.insert (pair<string,long int> ("xmm3", 28));
	RF.insert (pair<string,long int> ("xmm4", 29));
	RF.insert (pair<string,long int> ("xmm5", 30));
	RF.insert (pair<string,long int> ("xmm6", 31));
	RF.insert (pair<string,long int> ("xmm7", 32));
	RF.insert (pair<string,long int> ("xmm8", 33));
	RF.insert (pair<string,long int> ("xmm9", 34));
	RF.insert (pair<string,long int> ("xmm10",35));
	RF.insert (pair<string,long int> ("xmm11",36));
	RF.insert (pair<string,long int> ("xmm12",37));
	RF.insert (pair<string,long int> ("xmm13",38));
	RF.insert (pair<string,long int> ("xmm14",39));
	RF.insert (pair<string,long int> ("xmm15",40));

	//YMM REGISTERS
	RF.insert (pair<string,long int> ("ymm0", 25));
	RF.insert (pair<string,long int> ("ymm1", 26));
	RF.insert (pair<string,long int> ("ymm2", 27));
	RF.insert (pair<string,long int> ("ymm3", 28));
	RF.insert (pair<string,long int> ("ymm4", 29));
	RF.insert (pair<string,long int> ("ymm5", 30));
	RF.insert (pair<string,long int> ("ymm6", 31));
	RF.insert (pair<string,long int> ("ymm7", 32));
	RF.insert (pair<string,long int> ("ymm8", 33));
	RF.insert (pair<string,long int> ("ymm9", 34));
	RF.insert (pair<string,long int> ("ymm10",35));
	RF.insert (pair<string,long int> ("ymm11",36));
	RF.insert (pair<string,long int> ("ymm12",37));
	RF.insert (pair<string,long int> ("ymm13",38));
	RF.insert (pair<string,long int> ("ymm14",39));
	RF.insert (pair<string,long int> ("ymm15",40));

	//ZMM REGISTERS
	RF.insert (pair<string,long int> ("zmm0", 25));
	RF.insert (pair<string,long int> ("zmm1", 26));
	RF.insert (pair<string,long int> ("zmm2", 27));
	RF.insert (pair<string,long int> ("zmm3", 28));
	RF.insert (pair<string,long int> ("zmm4", 29));
	RF.insert (pair<string,long int> ("zmm5", 30));
	RF.insert (pair<string,long int> ("zmm6", 31));
	RF.insert (pair<string,long int> ("zmm7", 32));
	RF.insert (pair<string,long int> ("zmm8", 33));
	RF.insert (pair<string,long int> ("zmm9", 34));
	RF.insert (pair<string,long int> ("zmm10",35));
	RF.insert (pair<string,long int> ("zmm11",36));
	RF.insert (pair<string,long int> ("zmm12",37));
	RF.insert (pair<string,long int> ("zmm13",38));
	RF.insert (pair<string,long int> ("zmm14",39));
	RF.insert (pair<string,long int> ("zmm15",40));
	RF.insert (pair<string,long int> ("zmm16",41));
	RF.insert (pair<string,long int> ("zmm17",42));
	RF.insert (pair<string,long int> ("zmm18",43));
	RF.insert (pair<string,long int> ("zmm19",44));
	RF.insert (pair<string,long int> ("zmm20",45));
	RF.insert (pair<string,long int> ("zmm21",46));
	RF.insert (pair<string,long int> ("zmm22",47));
	RF.insert (pair<string,long int> ("zmm23",48));
	RF.insert (pair<string,long int> ("zmm24",49));
	RF.insert (pair<string,long int> ("zmm25",50));
	RF.insert (pair<string,long int> ("zmm26",51));
	RF.insert (pair<string,long int> ("zmm27",52));
	RF.insert (pair<string,long int> ("zmm28",53));
	RF.insert (pair<string,long int> ("zmm29",54));
	RF.insert (pair<string,long int> ("zmm30",55));
	RF.insert (pair<string,long int> ("zmm31",56));

	RF.insert (pair<string,long int> ("mxcsr",65));

	RF.insert (pair<string,long int> ("x",66));
	RF.insert (pair<string,long int> ("s",67));

	RF.insert (pair<string,long int> ("x87", 68));
}

void regFile::setupSpecialRegFile () {
	// SEGMENT REGISTERS
	SRF.insert (pair<string,long int> ("cs",   5));
	SRF.insert (pair<string,long int> ("ds",   6));
	SRF.insert (pair<string,long int> ("ss",   7));
	SRF.insert (pair<string,long int> ("es",   8));
	SRF.insert (pair<string,long int> ("fs",   9));
	SRF.insert (pair<string,long int> ("gs",  10));

	// POINTER REGISTERS
	SRF.insert (pair<string,long int> ("rsp", 11));
	SRF.insert (pair<string,long int> ("rbp", 12));

	SRF.insert (pair<string,long int> ("esp", 11));
	SRF.insert (pair<string,long int> ("ebp", 12));

	SRF.insert (pair<string,long int> ("sp",  11));
	SRF.insert (pair<string,long int> ("bp",  12));

	SRF.insert (pair<string,long int> ("spl", 11));
	SRF.insert (pair<string,long int> ("bpl", 12));

	// INDEX REGISTERS
	SRF.insert (pair<string,long int> ("rsi", 13));
	SRF.insert (pair<string,long int> ("rdi", 14));

	SRF.insert (pair<string,long int> ("esi", 13));
	SRF.insert (pair<string,long int> ("edi", 14));

	SRF.insert (pair<string,long int> ("si",  13));
	SRF.insert (pair<string,long int> ("di",  14));

	SRF.insert (pair<string,long int> ("sil", 13));
	SRF.insert (pair<string,long int> ("dil", 14));

	// INSTRUCTION POINTER REGISTER
	SRF.insert (pair<string,long int> ("rip", 15));

	SRF.insert (pair<string,long int> ("eip", 15));

	SRF.insert (pair<string,long int> ("ip",  15));

	// FLAG
	SRF.insert (pair<string,long int> ("rflags",24));

    // X87 NON-STRICT STACK REGISTERS
	SRF.insert (pair<string,long int> ("st0",57));
	SRF.insert (pair<string,long int> ("st1",58));
	SRF.insert (pair<string,long int> ("st2",59));
	SRF.insert (pair<string,long int> ("st3",60));
	SRF.insert (pair<string,long int> ("st4",61));
	SRF.insert (pair<string,long int> ("st5",62));
	SRF.insert (pair<string,long int> ("st6",63));
	SRF.insert (pair<string,long int> ("st7",64));
}
