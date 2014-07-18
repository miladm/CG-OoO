#ifndef _SIDE_BUFF_H
#define _SIDE_BUFF_H

#include "instruction.h"
#include "../lib/list.h"

class sideBuff {
	private:
		List<instruction*> *_sideBuff;
		bool _free;
		int _expCycle;

	public:
		sideBuff();
		~sideBuff();
		int NumElements();
		instruction* Nth(int i);
		void setBusy();
		bool isFree();
		void setFree();
		void Append(instruction* ins);
		void InsertAt(instruction* ins, int index);
		void setExpiration (int expCycle);
		int getExpiration ();
		void RemoveAt(int i);
};

#endif
