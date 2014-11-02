/*******************************************************************************
 *  variable.h
 ******************************************************************************/

#ifndef _VARIABLE_H
#define _VARIABLE_H

#include <string>
#include <vector>
#include <string.h>
#include "basicblock.h"
#include "list.h"
#include <string>
#include <iostream>

class variable {
	public:
		variable(long int var);
		~variable();
		long int getID();
		void addBB(basicblock* bb);
		int getNumAssignedBB();
		basicblock* getNthAssignedBB(int indx);
		int getC();
		void setC(int c);
		void popFromStack();
		void pushToStack(int s);
		int getTopStack();
		basicblock* getTopBbStack();
		//Hack
		void popHackPushes();

	private:
		long int _var;
		int _c; //number of processed assignments to _var 
		vector<long int> _s; //the index for var_i is kept here
		List<basicblock*>* bbList;
    public:
		int _hackPushCount;
};

#endif
