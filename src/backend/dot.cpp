#include "dot.h"
#include <stdio.h>
#include <stdlib.h>

extern float unpredMemOpThreshold;

dot::dot(int fileCode) {
	//char* hi = new char[10];
	//itoa(fileCode, hi, 10);

	if((_outFile=fopen("dotFiles/hi.dot", "w+")) == NULL) {
	    printf("ERROR: Cannot open file(s).\n");
	    exit(1);
	}
	init();
}

void dot::runDot(List<instruction*>* list, int subGraphID) {
	_list = list;
	createSubGraph(subGraphID);
	defn();
	createGraph();
	closeBlock();
}

dot::~dot() {
}

void dot::init() {
	fprintf(_outFile, "digraph G {\n");
}

void dot::createSubGraph(int subGraphID) {
	fprintf(_outFile, "\n\n");
	fprintf(_outFile, "\tsubgraph cluster_%d {\n", subGraphID);
	fprintf(_outFile, "\tlabel = \"Wavefront #%d\";\n", subGraphID);
}

void dot::defn() {
	for (int i = 0; i < _list->NumElements(); i++) {
		if (_list->Nth(i)->getMissrate() <= unpredMemOpThreshold) {
			fprintf(_outFile, "\t\tn%llu[label=\"n%llu\"];\n",_list->Nth(i)->getInsAddr(),_list->Nth(i)->getInsAddr());
		} else {
			fprintf(_outFile, "\t\tn%llu[label=\"n%llu\",style=filled,fillcolor=orangered];\n",_list->Nth(i)->getInsAddr(),_list->Nth(i)->getInsAddr());
		}
	}
}

void dot::closeBlock() {
	fprintf(_outFile, "\t}\n");
}

void dot::createGraph() {
	for (int i = 0; i < _list->NumElements(); i++) {
		for (int j = 0; j < _list->Nth(i)->getNumAncestors(); j++) {
			if (_list->Nth(i)->getNthAncestor(j)->getMissrate() <= unpredMemOpThreshold) {
				fprintf(_outFile, "\t\tn%llu->n%llu[label=\"%d\"];\n",_list->Nth(i)->getNthAncestor(j)->getInsAddr(),_list->Nth(i)->getInsAddr(),_list->Nth(i)->getNthAncestor(j)->getMyPathLen());
			} else {
				fprintf(_outFile, "\t\tn%llu->n%llu[label=\"%d\",color=orangered];\n",_list->Nth(i)->getNthAncestor(j)->getInsAddr(),_list->Nth(i)->getInsAddr(),_list->Nth(i)->getNthAncestor(j)->getMyPathLen());
			}
		}
	}
}

void dot::finish() {
	fprintf(_outFile, "}\n");
	//chdir("dotFiles");
	//system("make");
	//chdir("../");
	fclose(_outFile);
}
