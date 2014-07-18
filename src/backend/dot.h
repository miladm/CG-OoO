#include <string>
#include <string.h>
#include "instruction.h"
#include "../lib/list.h"


class dot {
	public:
		dot(int fileCode);
		~dot();
		void runDot(List<instruction*>* list, int subGraphID);
		void init();
		void defn();
		void createSubGraph(int subGraphID);
		void createGraph();
		void finish();
		void closeBlock();
		

	private:
		char* _fillColor;
		char* _style;
		char* _nodeName;
		char* _nodeCode;
		char* _fontColor;
		char* _label;

		string fileName;
		FILE* _outFile;
		List<instruction*>* _list;
};
