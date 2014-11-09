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
	RF.insert (pair<string,long int> ("rax",21));
	RF.insert (pair<string,long int> ("rbx",22));
	RF.insert (pair<string,long int> ("rcx",23));
	RF.insert (pair<string,long int> ("rdx",24));

	RF.insert (pair<string,long int> ("eax",21));
	RF.insert (pair<string,long int> ("ebx",22));
	RF.insert (pair<string,long int> ("ecx",23));
	RF.insert (pair<string,long int> ("edx",24));

	RF.insert (pair<string,long int> ("ax", 21));
	RF.insert (pair<string,long int> ("bx", 22));
	RF.insert (pair<string,long int> ("cx", 23));
	RF.insert (pair<string,long int> ("dx", 24));

	RF.insert (pair<string,long int> ("al", 21));
	RF.insert (pair<string,long int> ("bl", 22));
	RF.insert (pair<string,long int> ("cl", 23));
	RF.insert (pair<string,long int> ("dl", 24));

	RF.insert (pair<string,long int> ("ah", 21));
	RF.insert (pair<string,long int> ("bh", 22));
	RF.insert (pair<string,long int> ("ch", 23));
	RF.insert (pair<string,long int> ("dh", 24));

	// 64-BIT MODE-ONLY GENERAL PURPOSE REGISTERS
	RF.insert (pair<string,long int> ("r8" , 25));
	RF.insert (pair<string,long int> ("r9" , 26));
	RF.insert (pair<string,long int> ("r10", 27));
	RF.insert (pair<string,long int> ("r11", 28));
	RF.insert (pair<string,long int> ("r12", 29));
	RF.insert (pair<string,long int> ("r13", 30));
	RF.insert (pair<string,long int> ("r14", 31));
	RF.insert (pair<string,long int> ("r15", 32));

	RF.insert (pair<string,long int> ("r8d" ,25));
	RF.insert (pair<string,long int> ("r9d" ,26));
	RF.insert (pair<string,long int> ("r10d",27));
	RF.insert (pair<string,long int> ("r11d",28));
	RF.insert (pair<string,long int> ("r12d",29));
	RF.insert (pair<string,long int> ("r13d",30));
	RF.insert (pair<string,long int> ("r14d",31));
	RF.insert (pair<string,long int> ("r15d",32));

	RF.insert (pair<string,long int> ("r8w" ,25));
	RF.insert (pair<string,long int> ("r9w" ,26));
	RF.insert (pair<string,long int> ("r10w",27));
	RF.insert (pair<string,long int> ("r11w",28));
	RF.insert (pair<string,long int> ("r12w",29));
	RF.insert (pair<string,long int> ("r13w",30));
	RF.insert (pair<string,long int> ("r14w",31));
	RF.insert (pair<string,long int> ("r15w",32));
                                               
	RF.insert (pair<string,long int> ("r8b" ,25));
	RF.insert (pair<string,long int> ("r9b" ,26));
	RF.insert (pair<string,long int> ("r10b",27));
	RF.insert (pair<string,long int> ("r11b",28));
	RF.insert (pair<string,long int> ("r12b",29));
	RF.insert (pair<string,long int> ("r13b",30));
	RF.insert (pair<string,long int> ("r14b",31));
	RF.insert (pair<string,long int> ("r15b",32));

	//XMM REGISTERS
	RF.insert (pair<string,long int> ("xmm0", 33));
	RF.insert (pair<string,long int> ("xmm1", 34));
	RF.insert (pair<string,long int> ("xmm2", 35));
	RF.insert (pair<string,long int> ("xmm3", 36));
	RF.insert (pair<string,long int> ("xmm4", 37));
	RF.insert (pair<string,long int> ("xmm5", 38));
	RF.insert (pair<string,long int> ("xmm6", 39));
	RF.insert (pair<string,long int> ("xmm7", 40));
	RF.insert (pair<string,long int> ("xmm8", 41));
	RF.insert (pair<string,long int> ("xmm9", 42));
	RF.insert (pair<string,long int> ("xmm10",43));
	RF.insert (pair<string,long int> ("xmm11",44));
	RF.insert (pair<string,long int> ("xmm12",45));
	RF.insert (pair<string,long int> ("xmm13",46));
	RF.insert (pair<string,long int> ("xmm14",47));
	RF.insert (pair<string,long int> ("xmm15",48));

	//YMM REGISTERS
	RF.insert (pair<string,long int> ("ymm0", 33));
	RF.insert (pair<string,long int> ("ymm1", 34));
	RF.insert (pair<string,long int> ("ymm2", 35));
	RF.insert (pair<string,long int> ("ymm3", 36));
	RF.insert (pair<string,long int> ("ymm4", 37));
	RF.insert (pair<string,long int> ("ymm5", 38));
	RF.insert (pair<string,long int> ("ymm6", 39));
	RF.insert (pair<string,long int> ("ymm7", 40));
	RF.insert (pair<string,long int> ("ymm8", 41));
	RF.insert (pair<string,long int> ("ymm9", 42));
	RF.insert (pair<string,long int> ("ymm10",43));
	RF.insert (pair<string,long int> ("ymm11",44));
	RF.insert (pair<string,long int> ("ymm12",45));
	RF.insert (pair<string,long int> ("ymm13",46));
	RF.insert (pair<string,long int> ("ymm14",47));
	RF.insert (pair<string,long int> ("ymm15",48));

	//ZMM REGISTERS
	RF.insert (pair<string,long int> ("zmm0", 33));
	RF.insert (pair<string,long int> ("zmm1", 34));
	RF.insert (pair<string,long int> ("zmm2", 35));
	RF.insert (pair<string,long int> ("zmm3", 36));
	RF.insert (pair<string,long int> ("zmm4", 37));
	RF.insert (pair<string,long int> ("zmm5", 38));
	RF.insert (pair<string,long int> ("zmm6", 39));
	RF.insert (pair<string,long int> ("zmm7", 40));
	RF.insert (pair<string,long int> ("zmm8", 41));
	RF.insert (pair<string,long int> ("zmm9", 42));
	RF.insert (pair<string,long int> ("zmm10",43));
	RF.insert (pair<string,long int> ("zmm11",44));
	RF.insert (pair<string,long int> ("zmm12",45));
	RF.insert (pair<string,long int> ("zmm13",46));
	RF.insert (pair<string,long int> ("zmm14",47));
	RF.insert (pair<string,long int> ("zmm15",48));
	RF.insert (pair<string,long int> ("zmm16",49));
	RF.insert (pair<string,long int> ("zmm17",50));
	RF.insert (pair<string,long int> ("zmm18",51));
	RF.insert (pair<string,long int> ("zmm19",52));
	RF.insert (pair<string,long int> ("zmm20",53));
	RF.insert (pair<string,long int> ("zmm21",54));
	RF.insert (pair<string,long int> ("zmm22",55));
	RF.insert (pair<string,long int> ("zmm23",56));
	RF.insert (pair<string,long int> ("zmm24",57));
	RF.insert (pair<string,long int> ("zmm25",58));
	RF.insert (pair<string,long int> ("zmm26",59));
	RF.insert (pair<string,long int> ("zmm27",60));
	RF.insert (pair<string,long int> ("zmm28",61));
	RF.insert (pair<string,long int> ("zmm29",62));
	RF.insert (pair<string,long int> ("zmm30",63));
	RF.insert (pair<string,long int> ("zmm31",64));

	RF.insert (pair<string,long int> ("mxcsr",65));

	RF.insert (pair<string,long int> ("x",66));
	RF.insert (pair<string,long int> ("s",67));

//	RF.insert (pair<string,long int> ("x87", 68));
}

void regFile::setupSpecialRegFile () {
	// SEGMENT REGISTERS
	SRF.insert (pair<string,long int> ("cs",  1));
	SRF.insert (pair<string,long int> ("ds",  2));
	SRF.insert (pair<string,long int> ("ss",  3));
	SRF.insert (pair<string,long int> ("es",  4));
	SRF.insert (pair<string,long int> ("fs",  5));
	SRF.insert (pair<string,long int> ("gs",  6));

	// POINTER REGISTERS
	SRF.insert (pair<string,long int> ("rsp", 7));
	SRF.insert (pair<string,long int> ("rbp", 8));

	SRF.insert (pair<string,long int> ("esp", 7));
	SRF.insert (pair<string,long int> ("ebp", 8));

	SRF.insert (pair<string,long int> ("sp",  7));
	SRF.insert (pair<string,long int> ("bp",  8));

	SRF.insert (pair<string,long int> ("spl", 7));
	SRF.insert (pair<string,long int> ("bpl", 8));

	// INDEX REGISTERS
	SRF.insert (pair<string,long int> ("rsi", 9));
	SRF.insert (pair<string,long int> ("rdi",10));

	SRF.insert (pair<string,long int> ("esi", 9));
	SRF.insert (pair<string,long int> ("edi",10));

	SRF.insert (pair<string,long int> ("si",  9));
	SRF.insert (pair<string,long int> ("di", 10));

	SRF.insert (pair<string,long int> ("sil", 9));
	SRF.insert (pair<string,long int> ("dil",10));

	// INSTRUCTION POINTER REGISTER
	SRF.insert (pair<string,long int> ("rip",11));

	SRF.insert (pair<string,long int> ("eip",11));

	SRF.insert (pair<string,long int> ("ip", 11));

	// FLAG
	SRF.insert (pair<string,long int> ("rflags",12));

    // X87 NON-STRICT STACK REGISTERS
	SRF.insert (pair<string,long int> ("st0",13));
	SRF.insert (pair<string,long int> ("st1",14));
	SRF.insert (pair<string,long int> ("st2",15));
	SRF.insert (pair<string,long int> ("st3",16));
	SRF.insert (pair<string,long int> ("st4",17));
	SRF.insert (pair<string,long int> ("st5",18));
	SRF.insert (pair<string,long int> ("st6",19));
	SRF.insert (pair<string,long int> ("st7",20));
}
