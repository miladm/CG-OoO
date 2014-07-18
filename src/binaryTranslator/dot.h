/*******************************************************************************
 *  dot.h
 ******************************************************************************/

#ifndef _DOT_H
#define _DOT_H

#include <string>
#include <string.h>
#include "instruction.h"
#include "basicblock.h"
#include "list.h"
#include <string>
#include <iostream>

class dot {
	public:
		dot(int mode, string *program_name);
		~dot();
		void runDot(List<basicblock*>* list);
		void init();
		void defn();
		void createSubGraph(int subGraphID);
		void createGraph();
		void finish();
		void closeBlock();
		void setupBox(string color);
		void finishBox();

	private:
		char* _fillColor;
		char* _style;
		char* _nodeName;
		char* _nodeCode;
		char* _fontColor;
		char* _label;

		string fileName;
		FILE* _outFile;
		List<instruction*>* _insList;
		List<basicblock*>* _bBlist;
};

#endif
