/*******************************************************************************
 *  logGen.cpp
 ******************************************************************************/

#include "logGen.h"

void writeToFile (List<basicblock*> *bbList, string *program_name) {
	FILE* outFile;
	if ((outFile  = fopen(("/home/milad/esc_project/svn/PARS/src/binaryTranslator/output_files/"+(*program_name)+"_obj.s").c_str(), "w")) == NULL) 
		Assert("Unable to open the output file.");
	
	for (int i =  0; i < bbList->NumElements(); i++) {
		basicblock* bb = bbList->Nth(i);
		fprintf(outFile, "{,%ld\n", bb->getID());
		if (bb->hasHeader()) {
			fprintf(outFile, "H,%ld\n", bb->getBBbrHeader());
		}
		for (int j = 0; j < bb->getBbSize_ListSch(); j++) {
			instruction* ins = bb->getInsList_ListSchedule()->Nth(j);
			if (ins->getType() == 'M') {
				if (ins->isWrMemType()) {
					fprintf(outFile, "W,-memAddr-,%ld,%d,%s", ins->getInsAddr(), ins->getMemAccessSize(), ins->getArchRegisterStr().c_str());
				} else if (ins->isRdMemType()) {
					fprintf(outFile, "R,-memAddr-,%ld,%d,%s", ins->getInsAddr(), ins->getMemAccessSize(), ins->getArchRegisterStr().c_str());
				} else {
					Assert(true == false && "ERROR: A memory operation is either read or write");
				}
			} else if (ins->getType() == 'j' || ins->getType() == 'c' || ins->getType() == 'b' || ins->getType() == 'r') {
				fprintf(outFile, "%c,%ld,-brTaken-,%ld,%s", ins->getType(), ins->getInsAddr(), ins->getBrDst(), ins->getArchRegisterStr().c_str());
			} else if (ins->getType() == 'o') { //A, D, F for ins->getType()
				fprintf(outFile, "%c,%ld,%s", ins->getType(), ins->getInsAddr(), ins->getArchRegisterStr().c_str());
			} else {
				Assert(true == false && "Unrecognized instruction type");
			}
		}
		fprintf(outFile, "}\n");
	}
}
