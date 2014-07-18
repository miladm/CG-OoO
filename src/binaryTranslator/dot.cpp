/*******************************************************************************
 *  dot.cpp
 ******************************************************************************/

#include <vector>
#include "dot.h"
#include <stdio.h>
#include <stdlib.h>

extern float unpredMemOpThreshold;

dot::dot(int mode, string *program_name) {
	if (mode == 0) { //Make CFG
		if((_outFile=fopen(("/home/milad/esc_project/svn/memTraceMilad/TraceSim/phraseblock_framework/dotFiles/"+(*program_name)+"_cfg.dot").c_str(), "w+")) == NULL) {
		    Assert("Cannot open dot file(s).");
		}		
	} else if (mode = 1) { //Make Phrase CFG
		if((_outFile=fopen(("/home/milad/esc_project/svn/memTraceMilad/TraceSim/phraseblock_framework/dotFiles/"+(*program_name)+"_cfg_phrase.dot").c_str(), "w+")) == NULL) {
		    Assert("Cannot open dot file(s).");
		}		
	} else {
		Assert("ILLEGAL Control Flow Graph file.");
	}
	init();
}

void dot::runDot(List<basicblock*>* list) {
	_bBlist = list;
	//createSubGraph(subGraphID);
	defn();
	createGraph();
	closeBlock();
}

dot::~dot() {
}

void dot::setupBox(string color) {
	fprintf(_outFile, "[  style = \"filled\" penwidth = 1 fillcolor = \"%s\" fontname = \"Courier New\" shape = \"Mrecord\"\n"
			   "\t\tlabel =<<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"%s\">\n", color.c_str(), color.c_str());
}

void dot::finishBox() {
	fprintf(_outFile, "\t\t</table>> ];\n");
}

void dot::init() {
	fprintf(_outFile, "digraph G {\n"
			  "\tgraph [fontsize=30 labelloc=\"t\" label=\"\" splines=true overlap=false rankdir = \"LR\"];\n"
			  "\tratio = auto;\n");
}

void dot::createSubGraph(int subGraphID) {
	fprintf(_outFile, "\n\n");
	fprintf(_outFile, "\tsubgraph cluster_%d {\n", subGraphID);
	fprintf(_outFile, "\tlabel = \"Wavefront #%d\";\n", subGraphID);
}

void dot::defn() {
	for (int j = 0; j < _bBlist->NumElements(); j++) {
		_insList = _bBlist->Nth(j)->getInsList();
		ADDR bbID = _bBlist->Nth(j)->getID();
		//if (bbID == 18446744073709551615) continue; //TODO remove/fix this problem
		fprintf(_outFile, "\t\"%llu\" ", bbID);
		string color;
		if (_bBlist->Nth(j)->isAPhraseblock())	color.append("lightgrey");
		else									color.append("white");	
		setupBox(color);
		map<long int, vector<long int> > phiFuncs = _bBlist->Nth(j)->getPhiFuncs();
		if (phiFuncs.size() > 0) {
			for (map<long int, vector<long int> >::iterator it = phiFuncs.begin(); it != phiFuncs.end(); it++) {
				vector<long int> phiVec;
				phiVec = it->second;
				fprintf(_outFile, "\t\t<tr><td align=\"left\" port=\"r1\"> PHI: %d_ -- ",it->first);
				for (vector<long int>::iterator it2 = phiVec.begin(); it2 != phiVec.end(); it2++) {
					fprintf(_outFile, ",%d_%d", it->first,*it2);				
				}
				fprintf(_outFile, "</td></tr>\n");
			}
		}
		fprintf(_outFile, "\t\t<tr><td align=\"left\" port=\"r1\"></td></tr>\n");
		for (int i = 0; i < _insList->NumElements(); i++) {
			ADDR insAddr = _insList->Nth(i)->getInsAddr();
			double missRate = _insList->Nth(i)->getLdMissRate();
			if (missRate > UPLD_THRESHOLD)
				fprintf(_outFile, "\t\t<tr><td align=\"left\" port=\"r1\"> %llx, %s (UPLD-%.2f) </td></tr>\n", insAddr, _insList->Nth(i)->getOpCode(), missRate);
			else {
				fprintf(_outFile, "\t\t<tr><td align=\"left\" port=\"r1\"> %llx, %s", insAddr, _insList->Nth(i)->getOpCode());				
				for (int j = 0; j < _insList->Nth(i)->getNumReg(); j++) {
					if (_insList->Nth(i)->getNthRegType(j) == READ)
						fprintf(_outFile, ",%dR\n", _insList->Nth(i)->getNthReg(j));
					else
						fprintf(_outFile, ",%dW\n", _insList->Nth(i)->getNthReg(j));
				}
/*				fprintf(_outFile, "\t\t<tr><td align=\"left\" port=\"r1\"> %llx, %s", insAddr, _insList->Nth(i)->getOpCode());				
				for (int j = 0; j < _insList->Nth(i)->getNumReadReg(); j++) {
					fprintf(_outFile, ",%d_%dR", _insList->Nth(i)->getNthReadReg(j),_insList->Nth(i)->getReadRegSubscript(_insList->Nth(i)->getNthReadReg(j)));				
				}
				for (int j = 0; j < _insList->Nth(i)->getNumWriteReg(); j++) {
					fprintf(_outFile, ",%d_%dW", _insList->Nth(i)->getNthWriteReg(j),_insList->Nth(i)->getWriteRegSubscript(_insList->Nth(i)->getNthWriteReg(j)));				
				}
*/				fprintf(_outFile, "</td></tr>\n");				
			}
		}
		finishBox();
	}
}

void dot::closeBlock() {
	fprintf(_outFile, "\t}\n");
}

void dot::createGraph() {
	for (int j = 0; j < _bBlist->NumElements(); j++) {
		ADDR bbID = _bBlist->Nth(j)->getID();
		basicblock* bb = _bBlist->Nth(j);
		//if (bbID == 18446744073709551615) continue; //TODO remove this
		for (int i = 0; i < bb->getNumDescendents(); i++) {
			if (bb->getLastInsDst() == -1) {
				fprintf(_outFile, "\t%llu -> %llu[label = \"%.3f\"]\n", bbID, bb->getNthDescendent(i)->getID(), 1.0-bb->getTakenBias());
			} else if (bb->getLastInsDst() == bb->getNthDescendent(i)->getID()) {
				fprintf(_outFile, "\t%llu -> %llu[label = \"%.3f\"]\n", bbID, bb->getNthDescendent(i)->getID(), bb->getTakenBias());	
			} else {
				fprintf(_outFile, "\t%llu -> %llu[label = \"%.3f\"]\n", bbID, bb->getNthDescendent(i)->getID(), 1.0-bb->getTakenBias());
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
