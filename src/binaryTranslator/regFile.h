/*******************************************************************************
 *  regFile.h
 ******************************************************************************/

#ifndef _RF_H
#define _RF_H

#include <map>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include "global.h"
#include "utility.h"

class regFile {
	public:
		regFile ();
		~regFile (){}
		long int  getRegNum (const char* regName);
		long int  getSpecialRegNum (const char* regName);

	private:
		void setupRegFile ();
		void setupSpecialRegFile ();

	private:
		std::map<std::string,long int> RF; /* REGISTER FILE */
		std::map<std::string,long int> SRF; /* SPECIAL REGISTER FILE */
};

#endif
