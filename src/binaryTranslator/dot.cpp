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
		if ( (_outFile=fopen (cfg_file.c_str (), "w+")) == NULL) {
		    Assert ("Cannot open dot file (s).");
		}		
	} else if (mode = 1) { //Make Phrase CFG
        string cfg_file = out_dir + (*program_name) + "_cfg_phrase.dot";
		if ( (_outFile=fopen (cfg_file.c_str (), "w+")) == NULL) {
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
	//createSubGraph (subGraphID);
    cout << "Defn" << endl;
	defn ();
    cout << "Create Graph" << endl;
	createGraph ();
    cout << "Close Block" << endl;
	closeBlock ();
    cout << "Done with DOT file" << endl;
}

dot::~dot () {
    delete _interiorBB;
}

void dot::setupBox (string color) {
	fprintf (_outFile, "[  style = \"filled\" penwidth = 1 fillcolor = \"%s\" fontname = \"Courier New\" shape = \"Mrecord\"\n"
			   "\t\tlabel =<<table border=\"0\" cellborder=\"0\" cellpadding=\"3\" bgcolor=\"%s\">\n", color.c_str (), color.c_str ());
}

void dot::finishBox () {
	fprintf (_outFile, "\t\t</table>> ];\n");
}

void dot::init () {
	fprintf (_outFile, "digraph G {\n"
			  "\tgraph [fontsize=30 labelloc=\"t\" label=\"\" splines=true overlap=false rankdir = \"LR\"];\n"
			  "\tratio = auto;\n");
}

void dot::createSubGraph (int subGraphID) {
	fprintf (_outFile, "\n\n");
	fprintf (_outFile, "\tsubgraph cluster_%d {\n", subGraphID);
	fprintf (_outFile, "\tlabel = \"Wavefront #%d\";\n", subGraphID);
}

void dot::defn () {
	for (int j = 0; j < _bBlist->NumElements (); j++) {
        basicblock* bb = _bBlist->Nth (j);
//        if (!(bb->getID () >= 4260864 && bb->getID () <= 4262750)) continue;
		_insList = bb->getInsList ();
		ADDR bbID = bb->getID ();
		//if (bbID == 18446744073709551615) continue; //TODO remove/fix this problem
		fprintf (_outFile, "\t\"%llu\" ", bbID);
		string color;
		if (bb->isAPhraseblock ()) color.append ("lightgrey");
		else									 color.append ("white");	
		setupBox (color);
		map<long int, vector<long int> > phiFuncs = bb->getPhiFuncs ();
		if (phiFuncs.size () > 0) {
			for (map<long int, vector<long int> >::iterator it = phiFuncs.begin (); it != phiFuncs.end (); it++) {
				vector<long int> phiVec;
				phiVec = it->second;
				fprintf (_outFile, "\t\t<tr><td align=\"left\" port=\"r1\"> PHI: %d_ -- ",it->first);
				for (vector<long int>::iterator it2 = phiVec.begin (); it2 != phiVec.end (); it2++) {
					fprintf (_outFile, ",%d_%d", it->first,*it2);				
				}
				fprintf (_outFile, "</td></tr>\n");
			}
		}
        /*-TODO delete this-*/
		fprintf (_outFile, "\t\t<tr><td align=\"left\" port=\"r1\"></td></tr>\n");
				fprintf (_outFile, "\t\t<tr><td align=\"left\" port=\"r1\"> IN/DEF: ");				
                set<long int> inset = bb->getInSet ();
				for (set<long int>::iterator it = inset.begin(); it != inset.end(); it++) {
				    fprintf (_outFile, ",%dI\n", *it);
				}
                set<long int> defset = bb->getDefSet ();
				for (set<long int>::iterator it = defset.begin(); it != defset.end(); it++) {
				    fprintf (_outFile, ",%dD\n", *it);
				}
		fprintf (_outFile, "</td></tr>\n");				
        /* ---------------- */
		for (int i = 0; i < _insList->NumElements (); i++) {
            instruction* ins = _insList->Nth (i);
			ADDR insAddr = ins->getInsAddr ();
			double missRate = ins->getLdMissRate ();
			if (missRate > UPLD_THRESHOLD)
				fprintf (_outFile, "\t\t<tr><td align=\"left\" port=\"r1\"> %llx, %s (UPLD-%.2f) </td></tr>\n", insAddr, ins->getOpCode (), missRate);
			else {
				fprintf (_outFile, "\t\t<tr><td align=\"left\" port=\"r1\"> %llx, %s", insAddr, ins->getOpCode ());				
				for (int j = 0; j < ins->getNumReg (); j++) {
					if (ins->getNthRegType (j) == READ)
						fprintf (_outFile, ",%dR\n", ins->getNthReg (j));
					else
						fprintf (_outFile, ",%dW\n", ins->getNthReg (j));
				}
				fprintf (_outFile, ",%s", ins->getInsAsm ());				
/*				fprintf (_outFile, "\t\t<tr><td align=\"left\" port=\"r1\"> %llx, %s", insAddr, _insList->Nth (i)->getOpCode ());				
				for (int j = 0; j < _insList->Nth (i)->getNumReadReg (); j++) {
					fprintf (_outFile, ",%d_%dR", _insList->Nth (i)->getNthReadReg (j),_insList->Nth (i)->getReadRegSubscript (_insList->Nth (i)->getNthReadReg (j)));				
				}
				for (int j = 0; j < _insList->Nth (i)->getNumWriteReg (); j++) {
					fprintf (_outFile, ",%d_%dW", _insList->Nth (i)->getNthWriteReg (j),_insList->Nth (i)->getWriteRegSubscript (_insList->Nth (i)->getNthWriteReg (j)));				
				}
*/				fprintf (_outFile, "</td></tr>\n");
			}
		}
		finishBox ();
	}
}

void dot::closeBlock () {
	fprintf (_outFile, "\t}\n");
}

void dot::getInteriorBB () {
    for (int i = 0; i < _bBlist->NumElements (); i++) {
        basicblock* bb = _bBlist->Nth (i);
        if (bb->getSDominatorSize () == 0) {
            _interiorBB->Append (bb);
            cout << "\tDominator Interior BB: " << hex << bb->getID () << endl;
        }
    }
}

void dot::createGraph () {
    getInteriorBB ();
    for (int j = 0; j < _interiorBB->NumElements (); j++) {
        basicblock* bb = _interiorBB->Nth (j);
        ADDR bbID = bb->getID ();
        makeLink (bb);
//        if (!(bb->getID () >= 4260864 && bb->getID () <= 4262750)) continue;
    }
}

void dot::makeLink (basicblock* bb) {
    map<ADDR,basicblock*> children = bb->getChildren ();
    map<ADDR,basicblock*>::iterator it;
    for (it = children.begin (); it != children.end (); it++) {
        basicblock* child = it->second;
        fprintf (_outFile, "\t%llu -> %llu[label = \"%.3f\"]\n", bb->getID (), child->getID (), 1.0-bb->getTakenBias ());
    }
    for (it = children.begin (); it != children.end (); it++) {
        basicblock* child = it->second;
        makeLink (child);
    }
}

void dot::finish () {
	fprintf (_outFile, "}\n");
	//chdir ("dotFiles");
	//system ("make");
	//chdir ("../");
	fclose (_outFile);
}
