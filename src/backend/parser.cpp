/*******************************************************************************
 *  parser.cpp
 *  Parses instructions from file
 ******************************************************************************/
#include "bkEnd.h"
#include "parser.h"
#include "../frontend/tournament.hh"
#include "dependencyTable.h"
#include "../global/global.h"

extern bool debug;
extern brMode branchMode;
extern int corruptInsCount;
extern bool makePhrase;
extern int parseHitMiss;
extern long int numALUOps;
extern int parseHitMiss;
extern long int numMemOps;
extern long int numALUOps;
extern long int numFPUOps;
extern long int numBROps;
extern long int missPredBROps;
extern long int numAssignOps;
extern long int missPredBROps;
extern long int numReadOps;
extern long int numWriteOps;
extern int cycle;
extern bool eof;
extern int insID;
extern core coreType;
extern List<instruction*> *iWindow;
extern List<instruction*> *iROB;
extern bool reschedule;
extern int numSideBuffs;
//extern hist* numParentsHist;
extern dependencyTable *depTables;
extern int cacheLat[];
extern regFile *RF;
extern FILE* pinFile;
extern long int insParseCap;
extern TournamentBP *predictor;

bool parser::parseIns(instruction* newIns) {
	//int activeWinRange = iROB->NumElements() - iWinPointer;
	newIns->setStatus(FETCH, cycle, -1);
	while(true) {
		int result = fetchIns(newIns);
		if (result == -1 || (insParseCap > 0 && insParseCap <= insID)) {
			printf("Reached End of File.\n");
			eof = true; //TODO add this line later
			//iROB->RemoveAt(iROB->NumElements()-1);
			delete newIns;
			return false;
		} //EOF
		else if (result == -2) {
			//iROB->RemoveAt(iROB->NumElements()-1);
			//i--;
			continue; //Skip the line and read next line
		} else {
			insID++;
			//printf("insID = %d\n",insID);
			newIns->setInsID(insID);
			newIns->noRRdependencyTable(depTables, coreType);
			//TODO put the block back in when you have the right parameters defined.
			// newIns->br_dependencyTable(depTables);
			// /* Manage LSQ */ 
			// if (memoryModel == PERFECT) {
			// 	newIns->perfetc_MemDependencyTable(depTables, coreType, numSideBuffs);
			// } else if (memoryModel == TOTAL_ORDER) {
			// 	newIns->totalOrder_MemDependencyTable(totalOrderLSQ);
			// }
			/*-----STAT-----*/
			//numParentsHist->addElem(newIns->getNumAncestors());
			/*-----STAT-----*/
			if (reschedule == false && makePhrase == false)
				iWindow->Append(newIns);
			iROB->Append(newIns);
			newIns->findPhraseAncestors(); //TODO this should go somewhere else
			//printf("TEST: newIndx = %d, ROB size = %d\n", *iWindow->Nth(iWindow->NumElements()-1), iROB->NumElements());
			break;
		}
	}
	return true;
}

int parser::fetchIns (instruction* ins) {
	int rt=-1;
	long int addr, insAddr, r=-1;
	long int brTaken = -1;
	double missRate;
	char c[INS_STRING_SIZE],cTemp[INS_STRING_SIZE];
	if (fgets (c, INS_STRING_SIZE, pinFile) == NULL) return -1; //EOF
	strcpy (cTemp, c);
	ins->setBrMode(branchMode);
	//if (debug) printf ("%s", c);
	switch (c[0]) {
		case 'A':  
			//printf ("%s", c);
			ins->setCmdStr(cTemp);
			resetInput (c, 0);
			ins->setType(ALU);
        		insAddr = getANumber(c);
        		if (insAddr == 0) {
				printf("WARNING: Instruction address corrupttion. Skipping %s\n", cTemp);
				corruptInsCount++;
        			return -2;
        		} else {
				ins->setInsAddr(insAddr);
			}
			if (makePhrase == true || parseHitMiss == 1) {
				missRate = (double)getANumber(c)/(double)COEFF;
				//Do nothing productive!
			}
			while (rt != 0) {
			    r = getReg(c);
			    rt = getRegType(c);
			    //if (debug) printf("r = %d, rt = %d\n", r,rt);
			    //if (rt > 0 && rt < 3 && r > 0 && r < RF->getNumberOfRegs()+1) {ins->setRegister(&r, &rt);}
			    //if (rt > 0 && rt < 3 && r > 0 && r < NUM_REGISTERS) {ins->setRegister(&r, &rt);}
			    if ((rt > 0 && rt < 3 && r > 0) || r < -99) {ins->setRegister(&r, &rt);}
			    else if (rt == -3 || r == -3) {break;} //reached the end of line
			    else {
			    	//if (debug)
				printf("WARNING: Instruction Corruption. Skipping %s\n",cTemp);
				corruptInsCount++;
			    	return -2;
			    }
			}
			ins->setPiepelineLat(ALU_LATENCY);
			/*-----STAT-----*/
			numALUOps++;
			/*-----STAT-----*/
			break;
		case 'F':  
			//printf ("%s", c);
			ins->setCmdStr(cTemp);
			resetInput (c, 0);
			ins->setType(FPU);
        		insAddr = getANumber(c);
        		if (insAddr == 0) {
				printf("WARNING: Instruction address corrupttion. Skipping %s\n", cTemp);
				corruptInsCount++;
        			return -2;
        		} else {
				ins->setInsAddr(insAddr);
			}
			if (makePhrase == true || parseHitMiss == 1) {
				missRate = (double)getANumber(c)/(double)COEFF;
				//Do nothing productive!
			}
			while (rt != 0) {
			    r = getReg(c);
			    rt = getRegType(c);
			    //if (debug) printf("r = %d, rt = %d\n", r,rt);
			    //if (rt > 0 && rt < 3 && r > 0 && r < RF->getNumberOfRegs()+1) {ins->setRegister(&r, &rt);}
			    //if (rt > 0 && rt < 3 && r > 0 && r < NUM_REGISTERS) {ins->setRegister(&r, &rt);}
			    if ((rt > 0 && rt < 3 && r > 0) || r < -99) {ins->setRegister(&r, &rt);}
			    else if (rt == -3 || r == -3) {break;} //reached the end of line
			    else {
			    	//if (debug)
				printf("WARNING: Instruction Corruption. Skipping %s\n",cTemp);
				corruptInsCount++;
			    	return -2;
			    }
			}
			ins->setPiepelineLat(FPU_LATENCY);
			/*-----STAT-----*/
			numFPUOps++;
			/*-----STAT-----*/
			break;
		case 'B':  
			//printf ("%s", c);
			ins->setCmdStr(cTemp);
			resetInput (c, 0);
			ins->setType(BR);
        		insAddr = getANumber(c);
        		if (insAddr == 0) {
				printf("WARNING: Instruction address corrupttion. Skipping %s\n", cTemp);
				corruptInsCount++;
        			return -2;
        		} else {
				ins->setInsAddr(insAddr);
			}
        		ins->setBrSide(getANumber(c));
			brTaken = getANumber(c);
			if (brTaken == 0) {
				printf("WARNING: Instruction Corruption. Skipping %s\n",cTemp);
				corruptInsCount++;
			    	return -2;
        		} else {
				ins->setBrTarget(brTaken);
			}
			ins->setBrForward();
			bool missPred;
			if (ins->getBrSide() == true)
				missPred = PredictAndUpdate(ins->getInsAddr(), 1);
			else
				missPred = PredictAndUpdate(ins->getInsAddr(), 0);
			ins->findMissPrediction(missPred);
			if (makePhrase == true || parseHitMiss == 1) {
				missRate = (double)getANumber(c)/(double)COEFF;
				//Do nothing productive!
			}
			while (rt != 0) {
			    r = getReg(c);
			    rt = getRegType(c);
			    //if (debug) printf("r = %d, rt = %d\n", r,rt);
			    //if (rt > 0 && rt < 3 && r > 0 && r < RF->getNumberOfRegs()+1) {ins->setRegister(&r, &rt);}
			    //if (rt > 0 && rt < 3 && r > 0 && r < NUM_REGISTERS) {ins->setRegister(&r, &rt);}
			    if (rt > 0 && rt < 3 && r > 0) {ins->setRegister(&r, &rt);}
			    else if (rt == -3 || r == -3) {break;} //reached the end of line
			    else {
			    	//if (debug)
				printf("WARNING: Instruction Corruption. Skipping %s\n",cTemp);
				corruptInsCount++;
			    	return -2;
			    }
			}
			if (ins->getMissPrediction()==true) {//Static branch prediction
				ins->setPiepelineLat(BR_LATENCY);
				missPredBROps++;
			} else {
				ins->setPiepelineLat(ALU_LATENCY);
			}
			/*-----STAT-----*/
			numBROps++;
			/*-----STAT-----*/
			break;
        	case 'R':  
        		//printf ("%s", c);
			ins->setCmdStr(cTemp);
        		resetInput (c, 0);
        		ins->setType(MEM);
        		ins->setMemType(READ);
        		addr = getAddr(c);
        		insAddr = getANumber(c);
        		if (addr == 0 || insAddr == 0) {
				printf("WARNING: Instruction address corrupttion. Skipping %s\n", cTemp);
				corruptInsCount++;
        			return -2;
        		} else {
				ins->setMemAddr(addr);
				ins->setInsAddr(insAddr);
			}
			if (makePhrase == true || parseHitMiss == 1) {
				missRate = (double)getANumber(c)/(double)COEFF;
				ins->setMissRate(missRate);
			}
        		while (rt != 0) {
        		    r  = getReg(c);
        		    rt = getRegType(c);
        		    //if (debug) printf("r = %d, rt = %d\n", r,rt);
        		    //if (rt > 0 && rt < 3 && r > 0 && r < RF->getNumberOfRegs()+1) {ins->setRegister(&r, &rt);}
        		    //if (rt > 0 && rt < 3 && r > 0 && r < NUM_REGISTERS) {ins->setRegister(&r, &rt);}
        		    if ((rt > 0 && rt < 3 && r > 0) || r < -99) {ins->setRegister(&r, &rt);}
        		    else if (rt == -3 || r == -3) {break;} //reached the end of line
        		    else { //when r = -2  or -1 or ...
        		    	//if (debug) 
				printf("WARNING: Instruction reg MAY BE corrupt. Skipping %s\n", cTemp);
				corruptInsCount++;
        			return -2;
        		    }
        		    if (rt == -1) break; //nothing was set!
        		}
			ins->setPiepelineLat(cacheLat[0]);
			/*-----STAT-----*/
			numMemOps++;
			numReadOps++;
			/*-----STAT-----*/
        		break;
        	case 'W':  
        		//printf ("%s", c);
			ins->setCmdStr(cTemp);
        		resetInput (c, 0);
        		ins->setType(MEM);
        		ins->setMemType(WRITE);
        		addr = getAddr(c);
        		insAddr = getANumber(c);
        		if (addr == 0 || insAddr == 0) {
				printf("WARNING: Instruction address corrupttion. Skipping %s\n", cTemp);
				corruptInsCount++;
        			return -2;
        		} else {
				ins->setMemAddr(addr);
				ins->setInsAddr(insAddr);
			}
			if (makePhrase == true || parseHitMiss == 1) {
				missRate = (double)getANumber(c)/(double)COEFF;
				ins->setMissRate(missRate);
			}
        		while (rt != 0) {
        		    r  = getReg(c);
        		    rt = getRegType(c);
        		    //if (debug) printf("r = %d, rt = %d\n", r,rt);
        		    //if (rt > 0 && rt < 3 && r > 0 && r < RF->getNumberOfRegs()+1) {ins->setRegister(&r, &rt);}
        		    //if (rt > 0 && rt < 3 && r > 0 && r < NUM_REGISTERS) {ins->setRegister(&r, &rt);}
        		    if (rt > 0 && rt < 3 && r > 0) {ins->setRegister(&r, &rt);}
        		    else if (rt == -3 || r == -3) {break;} //reached the end of line
        		    else { //when r = -2  or -1 or ...
        		    	//if (debug) 
				printf("WARNING: Instruction reg MAY BE corrupt. Skipping %s\n", cTemp);
				corruptInsCount++;
        			return -2;
        		    }
        		    if (rt == -1) break; //nothing was set!
        		}
			ins->setPiepelineLat(cacheLat[0]);
			/*-----STAT-----*/
			numMemOps++;
			numWriteOps++;
			/*-----STAT-----*/
        		break;
		case 'T':  
			//printf ("%s", c);
			ins->setCmdStr(cTemp);
			resetInput (c, 0);
			ins->setType(ASSIGN);
        		insAddr = getANumber(c);
        		if (insAddr == 0) {
				printf("WARNING: Instruction address corrupttion. Skipping %s\n", cTemp);
				corruptInsCount++;
        			return -2;
        		} else {
				ins->setInsAddr(insAddr);
			}
			if (makePhrase == true || parseHitMiss == 1) {
				missRate = (double)getANumber(c)/(double)COEFF;
				//Do nothing productive!
			}
			while (rt != 0) {
			    r = getReg(c);
			    rt = getRegType(c);
			    //if (debug) printf("r = %d, rt = %d\n", r,rt);
			    //if (rt > 0 && rt < 3 && r > 0 && r < RF->getNumberOfRegs()+1) {ins->setRegister(&r, &rt);}
			    //if (rt > 0 && rt < 3 && r > 0 && r < NUM_REGISTERS) {ins->setRegister(&r, &rt);}
			    if ((rt > 0 && rt < 3 && r > 0) || r < -99) {ins->setRegister(&r, &rt);}
			    else if (rt == -3 || r == -3) {break;} //reached the end of line
			    else {
			    	//if (debug)
				printf("WARNING: Instruction Corruption. Skipping %s\n",cTemp);
				corruptInsCount++;
			    	return -2;
			    }
			}
			ins->setPiepelineLat(ASSIGN_LATENCY);
			/*-----STAT-----*/
			numAssignOps++;
			/*-----STAT-----*/
			break;
		default:
			if (debug) printf("WARNING: Unrecognized Instruction. Skipping (%s)\n", cTemp);
			corruptInsCount++;
			return -2;
	};
	return 0; //Successful completion (TODO: fix return values)
}



void parser::resetInput (char *c, int i) {
	//this letter is never used in trace
	c[i] = 'z';
}

long int parser::getReg(char *c) {
	int i = 0, start = 0, end = 0;
	char temp[INS_STRING_SIZE];

	//Parse a x86 resgiter name
	while (c[i]=='z') {i++;}
	if (c[i] == ',') {
		resetInput(c,i);
		i++;
		start = i;
		while (c[i] != '#' && c[i] != '\0' && c[i] != '\n') {
			temp[i] = c[i];
			resetInput(c,i);
			if (debug) printf("temp[i] = %c\n", temp[i]);
			if (c[i] == ',') return -1; //corrupt ins
			i++;
		}
		end = i;
		if (c[i] == '\n' || c[i] == '\0') return -3; //End of line
	}
	int size = end - start;
	char * reg = new char[size+1];
	for (i=0; i < size; i++) {
		reg[i] = temp[i+start];
		if (debug) printf("reg [i] = %c, size = %d", reg[i], size);
	}
	reg[size] = '\0';
	//for (i=0; i < (end-start); i++) {
	//	if (debug) printf("%c", reg[i], size);
	//}
	long int result =  RF->getRegNum(reg);
	delete [] reg;
	return result;
}

int parser::getRegType(char *c) {
	int i = 0, start = 0, end = 0;
	char temp[INS_STRING_SIZE];

	//Parse a x86 resgiter name
	while (c[i]=='z') {i++;}
	if (c[i] == '#') {
		resetInput(c,i);
		i++;
		start = i;
		while (c[i] != ',' && c[i] != '\0' && c[i] != '\n') {
			temp[i] = c[i];
			resetInput(c,i);
			//if (debug) printf("temp[i] = %c\n", temp[i]);
			i++;
		}
		if (c[i] == '\n' || c[i] == '\0') return -3; //End of line
		end = i;
	}
	int size = end - start;
	char * reg = new char[size+1];
	for (i=0; i < size; i++) {
		reg[i] = temp[i+start];
		if (debug) printf("reg [i] = %c, size = %d\n", reg[i], size);
	}
	reg[size] = '\0';
	int result = (int) strtol(reg, NULL, 10);
	//if (debug) printf("priority: %c,%c,%c\n", reg[0],reg[1],reg[2]);
	delete [] reg;
	return result;
}

long int parser::getANumber(char *c) {
	int i = 0, start = 0, end = 0;
	char temp[INS_STRING_SIZE];

	//Parse a x86 resgiter name
	while (c[i]=='z') {i++;}
	if (c[i] == ',' || c[i] == ' ') {
		resetInput(c,i);
		i++;
		start = i;
		while (c[i] != ',' && c[i] != '\0' && c[i] != '\n' && c[i] != ')') {
			temp[i] = c[i];
			resetInput(c,i);
			if (debug) printf("temp[i] = %c\n", temp[i]);
			i++;
		}
		//if (c[i] == '\n' || c[i] == '\0') return -3; //End of line (unsigned return)
		end = i;
	}
	int size = end - start;
	char * addr = new char[size];
	for (i=0; i < (end-start); i++) {
		addr[i] = temp[i+start];
		if (addr[i] < 48 || addr[i] > 57) return 0; //corrupt number
		if (debug) printf("addr [i] = %c, size = %d\n", addr[i], size);
	}
	long int result = 0;
	for (i = size-1; i >= 0; i--) {
		long int tenPow = 1;
		for (int j = 0; j < size-1-i; j++) {
			tenPow *= 10;}
		result += ((long int)addr[i]-48)*tenPow;
	}
	if (debug) printf("ins addr = %s, result = %ld\n", addr, result);
	//No negative or zero values allowed
	if (result <= 0) {
		delete [] addr;
		return 0; //corrupt number
	}
	delete [] addr;
	return result;
}

long int parser::getAddr(char *c) {
	int i = 0, start = 0, end = 0;
	char temp[INS_STRING_SIZE];

	//Parse a x86 resgiter name
	while (c[i]=='z') {i++;}
	if (c[i] == ',') {
		resetInput(c,i);
		i++;
		start = i;
		while (c[i] != ',' && c[i] != '\0' && c[i] != '\n') {
			temp[i] = c[i];
			resetInput(c,i);
			if (debug) printf("temp[i] = %c\n", temp[i]);
			i++;
		}
		//if (c[i] == '\n' || c[i] == '\0') return -3; //End of line (unsigned return)
		end = i;
	}
	int size = end - start;
	char * addr = new char[size];
	for (i=0; i < (end-start); i++) {
		addr[i] = temp[i+start];
		if (debug) printf("addr [i] = %c, size = %d\n", addr[i], size);
	}
	long int result = 0;
	result = strtol(addr, NULL, 10);
	//if (debug) printf("addr = %s, result = %lx\n", addr, result);
	if (result == 0) {
		delete [] addr;
		return 0; //corrupt number
	}
	delete [] addr;
	return result;
}
