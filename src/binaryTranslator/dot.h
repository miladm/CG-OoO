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
		dot (int mode, string *program_name);
		~dot ();
		void runDot (List<basicblock*>* list);
		void init ();
		void defn (FILE*);
		void createSubGraph (int, FILE*);
		void createCFG (FILE*);
		void createDT (FILE*);
		void finish (FILE*);
		void closeBlock (FILE*);
		void setupBox (string, FILE*);
		void finishBox (FILE*);
        void getInteriorBB ();
        void makeLink (basicblock*, FILE*);

	private:
		char* _fillColor;
		char* _style;
		char* _nodeName;
		char* _nodeCode;
		char* _fontColor;
		char* _label;

		string fileName;
		FILE* _outFile;
		FILE* _outDomTreeFile;
		List<instruction*>* _insList;
		List<basicblock*>* _bBlist;

        List<basicblock*>* _interiorBB;
};

#endif
