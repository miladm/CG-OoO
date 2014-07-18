/*******************************************************************************
 * regFile.h
 ******************************************************************************/

#ifndef _RF_H
#define _RF_H

#include <string.h>
#include <map>
#include "../global/global.h"

class regFile {
	private:
		map<string,long int> RF;
		int RFstatus[NUM_REGISTERS];
		long int numRegs;

		void setupRegFile();
		//string itos(long int number);
	public:
		regFile();
		~regFile(){}
		//void setRegStat(int  regNum, int insNum);
		//void resetRegStat(int  regNum);
		//int  getRegStat(int regNum);
		long int  getRegNum(const char* regName);
		//int  addToRegFile(long int reg);
		long int getNumberOfRegs(); //TODO may need to be long int
};

#endif
