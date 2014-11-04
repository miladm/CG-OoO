/*******************************************************************************
 *  dot.cpp
 ******************************************************************************/

#include <vector>
#include "dot.h"
#include <stdio.h>
#include <stdlib.h>

extern float unpredMemOpThreshold;

dot::dot (int mode, string *program_name) {
    string out_dir = "/home/milad/esc_project/svn/PARS/src/binaryTranslator/dotFiles/";
	if (mode == 0) { //Make CFG
        string cfg_file = out_dir + (*program_name) + "_cfg.dot";
		if ((_outFile=fopen (cfg_file.c_str (), "w+")) == NULL) {
		    Assert ("Cannot open dot file (s).");
		}		
        string dt_file = out_dir + (*program_name) + "_dt.dot";
		if ((_outDomTreeFile=fopen (dt_file.c_str (), "w+")) == NULL) {
		    Assert ("Cannot open dot file (s).");
		}		
	} else if (mode = 1) { //Make Phrase CFG
        string cfg_file = out_dir + (*program_name) + "_cfg_phrase.dot";
		if ((_outFile=fopen (cfg_file.c_str (), "w+")) == NULL) {
		    Assert ("Cannot open dot file (s).");
		}		
        string dt_file = out_dir + (*program_name) + "_dt_phrase.dot";
		if ((_outDomTreeFile=fopen (dt_file.c_str (), "w+")) == NULL) {
		    Assert ("Cannot open dot file (s).");
		}		
	} else {
		Assert ("ILLEGAL Control Flow Graph file.");
	}
    _interiorBB = new List<basicblock*>;
	init ();
}

void dot::runDot (List<basicblock*>* list) {
	_bBlist = list;
	//createSubGraph (subGraphID, _outFile);
	defn (_outFile);
	defn (_outDomTreeFile);

	createCFG (_outFile);
	createDT (_outDomTreeFile);

	closeBlock (_outFile);
	closeBlock (_outDomTreeFile);
}

dot::~dot () {
    delete _interiorBB;
}

void dot::setupBox (string color, FILE* outFile) {
	fprintf (outFile, "[  style = \"filled\" penwidth = 1 fillcolor = \"%s\" fontname = \"Courier New\" shape = \"Mrecord\"\n"
			   "\t\tlabel =<<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"%s\">\n", color.c_str (), color.c_str ());
}

void dot::finishBox (FILE* outFile) {
	fprintf (outFile, "\t\t</table>> ];\n");
}

void dot::init () {
	fprintf (_outFile, "digraph G {\n"
			  "\tgraph [fontsize=30 labelloc=\"t\" label=\"\" splines=true overlap=false rankdir = \"LR\"];\n"
			  "\tratio = auto;\n");

	fprintf (_outDomTreeFile, "digraph G {\n"
			  "\tgraph [fontsize=30 labelloc=\"t\" label=\"\" splines=true overlap=false rankdir = \"LR\"];\n"
			  "\tratio = auto;\n");
}

void dot::createSubGraph (int subGraphID, FILE* outFile) {
	fprintf (outFile, "\n\n");
	fprintf (outFile, "\tsubgraph cluster_%d {\n", subGraphID);
	fprintf (outFile, "\tlabel = \"Wavefront #%d\";\n", subGraphID);
}

void dot::defn (FILE* outFile) {
	for (int j = 0; j < _bBlist->NumElements (); j++) {
        basicblock* bb = _bBlist->Nth (j);
//        if (! (bb->getID () >= 4260864 && bb->getID () <= 4262750)) continue;
		_insList = bb->getInsList ();
		ADDR bbID = bb->getID ();
		//if (bbID == 18446744073709551615) continue; //TODO remove/fix this problem
		fprintf (outFile, "\t\"%llu\" ", bbID);
		string color;
		if (bb->isAPhraseblock ()) color.append ("lightgrey");
		else									 color.append ("white");	
		setupBox (color, outFile);
		map<long int, vector<long int> > phiFuncs = bb->getPhiFuncs ();
		if (phiFuncs.size () > 0) {
			for (map<long int, vector<long int> >::iterator it = phiFuncs.begin (); it != phiFuncs.end (); it++) {
				vector<long int> phiVec;
				phiVec = it->second;
				fprintf (outFile, "\t\t<tr><td align=\"left\" port=\"r1\"> PHI: %d_ -- ",it->first);
				for (vector<long int>::iterator it2 = phiVec.begin (); it2 != phiVec.end (); it2++) {
					fprintf (outFile, ",%d_%d", it->first,*it2);				
				}
				fprintf (outFile, "</td></tr>\n");
			}
		}
        /*-TODO delete this-*/
		fprintf (outFile, "\t\t<tr><td align=\"left\" port=\"r1\"></td></tr>\n");
				fprintf (outFile, "\t\t<tr><td align=\"left\" port=\"r1\"> IN/DEF: ");				
                set<long int> inset = bb->getInSet ();
				for (set<long int>::iterator it = inset.begin (); it != inset.end (); it++) {
				    fprintf (outFile, ",%dI\n", *it);
				}
                set<long int> defset = bb->getDefSet ();
				for (set<long int>::iterator it = defset.begin (); it != defset.end (); it++) {
				    fprintf (outFile, ",%dD\n", *it);
				}
		fprintf (outFile, "</td></tr>\n");				
        /* ---------------- */
		for (int i = 0; i < _insList->NumElements (); i++) {
            instruction* ins = _insList->Nth (i);
			ADDR insAddr = ins->getInsAddr ();
			double missRate = ins->getLdMissRate ();
			if (missRate > UPLD_THRESHOLD)
				fprintf (outFile, "\t\t<tr><td align=\"left\" port=\"r1\"> %llx, %s (UPLD-%.2f) </td></tr>\n", insAddr, ins->getOpCode (), missRate);
			else {
				fprintf (outFile, "\t\t<tr><td align=\"left\" port=\"r1\"> %llx, %s", insAddr, ins->getOpCode ());				
				for (int j = 0; j < ins->getNumReg (); j++) {
					if (ins->getNthRegType (j) == READ)
						fprintf (outFile, ",%dR\n", ins->getNthReg (j));
					else
						fprintf (outFile, ",%dW\n", ins->getNthReg (j));
				}
				fprintf (outFile, ",%s", ins->getInsAsm ());				
/*				fprintf (outFile, "\t\t<tr><td align=\"left\" port=\"r1\"> %llx, %s", insAddr, _insList->Nth (i)->getOpCode ());				
				for (int j = 0; j < _insList->Nth (i)->getNumReadReg (); j++) {
					fprintf (outFile, ",%d_%dR", _insList->Nth (i)->getNthReadReg (j),_insList->Nth (i)->getReadRegSubscript (_insList->Nth (i)->getNthReadReg (j)));				
				}
				for (int j = 0; j < _insList->Nth (i)->getNumWriteReg (); j++) {
					fprintf (outFile, ",%d_%dW", _insList->Nth (i)->getNthWriteReg (j),_insList->Nth (i)->getWriteRegSubscript (_insList->Nth (i)->getNthWriteReg (j)));				
				}
*/				fprintf (outFile, "</td></tr>\n");
			}
		}
		finishBox (outFile);
	}
}

void dot::createCFG (FILE* outFile) {
	for (int j = 0; j < _bBlist->NumElements (); j++) {
		ADDR bbID = _bBlist->Nth (j)->getID ();
		basicblock* bb = _bBlist->Nth (j);
//        if (! (bb->getID () >= 4260864 && bb->getID () <= 4262750)) continue;
		for (int i = 0; i < bb->getNumDescendents (); i++) {
			if (bb->getLastInsDst () == -1) {
				fprintf (outFile, "\t%llu -> %llu[label = \"%.3f\"]\n", bbID, bb->getNthDescendent (i)->getID (), 1.0-bb->getTakenBias ());
			} else if (bb->getLastInsDst () == bb->getNthDescendent (i)->getID ()) {
				fprintf (outFile, "\t%llu -> %llu[label = \"%.3f\"]\n", bbID, bb->getNthDescendent (i)->getID (), bb->getTakenBias ());	
			} else {
				fprintf (outFile, "\t%llu -> %llu[label = \"%.3f\"]\n", bbID, bb->getNthDescendent (i)->getID (), 1.0-bb->getTakenBias ());
			}
		}
	}
}

void dot::createDT (FILE* outFile) {
    getInteriorBB ();
    for (int j = 0; j < _interiorBB->NumElements (); j++) {
        basicblock* bb = _interiorBB->Nth (j);
        ADDR bbID = bb->getID ();
        makeLink (bb, outFile);
//        if (! (bb->getID () >= 4260864 && bb->getID () <= 4262750)) continue;
    }
}

void dot::closeBlock (FILE* outFile) {
	fprintf (outFile, "\t}\n");
}

void dot::getInteriorBB () {
    for (int i = 0; i < _bBlist->NumElements (); i++) {
        basicblock* bb = _bBlist->Nth (i);
        if (bb->getSDominatorSize () == 0) {
            _interiorBB->Append (bb);
        }
    }
}

void dot::makeLink (basicblock* bb, FILE* outFile) {
    map<ADDR,basicblock*> children = bb->getChildren ();
    map<ADDR,basicblock*>::iterator it;
    for (it = children.begin (); it != children.end (); it++) {
        basicblock* child = it->second;
        fprintf (outFile, "\t%llu -> %llu[label = \"%.3f\"]\n", bb->getID (), child->getID (), 1.0-bb->getTakenBias ());
    }
    for (it = children.begin (); it != children.end (); it++) {
        basicblock* child = it->second;
        makeLink (child, outFile);
    }
}

void dot::finish (FILE* outFile) {
	fprintf (outFile, "}\n");
	//chdir ("dotFiles");
	//system ("make");
	//chdir ("../");
	fclose (outFile);
}
