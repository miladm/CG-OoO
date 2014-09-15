/*******************************************************************************
 *  annotateTrace.cpp
 ******************************************************************************/
#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include "annotateTrace.h"
#include "stat.h"

void dumpBB (List<basicblock*>* bbList, List<instruction*>* insList, map<ADDR, basicblock*> &bbHeaders) {
	if (insList->NumElements() == 0) return;
	printf("BB %llx\n", insList->Nth(0)->getInsAddr());
	if (bbHeaders.find(insList->Nth(0)->getInsAddr()) == bbHeaders.end()) Assert("SHIT");
	// basicblock* bb = findBB(bbList, insList->Nth(0)->getInsAddr());
	basicblock* bb;
	{ //Find Basicblock
		for (int i = 0; i < bbList->NumElements(); i++) {
			int size = bbList->Nth(i)->getBbSize();
			bb = bbList->Nth(i);
			if (bb->getInsList()->Nth(0)->getInsAddr() == insList->Nth(0)->getInsAddr()) goto exit_point;
		}
		Assert("Didn't find the bb");
	}
	exit_point:
	int listSize = insList->NumElements();
	if (bb == NULL || listSize != bb->getInsList()->NumElements()) Assert("Wrong number of instructions");
	for (int i = 0; i < listSize; i++) {
		printf("%llx", insList->Nth(0)->getInsAddr());
		if (bb->getInsList()->Nth(i)->getInsAddr() != insList->Nth(0)->getInsAddr()) printf(" SHOOT");
		printf("\n");
		delete insList->Nth(0);
		insList->RemoveAt(0);
	}
}

long int storeToFile(bool brFound, ADDR brAddr, basicblock* bb, map<ADDR, string> &dynBBMap, FILE* traceFileOutput) {
	long int insCounter = 0;
	// Print BB/PB Header
	if (brFound == true) {
		fprintf(traceFileOutput, "H,%llu\n", brAddr); // TOOD Update this for phraseblock
	}
	List<instruction*>* listSchBBList = bb->getInsList_ListSchedule();
	for (int i = 0; i < listSchBBList->NumElements(); i++) {
		if (dynBBMap.find(listSchBBList->Nth(i)->getInsAddr()) != dynBBMap.end()) {
			insCounter++;
			fprintf(traceFileOutput, "%s", (dynBBMap[listSchBBList->Nth(i)->getInsAddr()]).c_str());
		}
	}
	// map<ADDR,string>::iterator it;
	// for (it = dynBBMap.begin(); it != dynBBMap.end(); it++) {
	// 	fprintf(traceFileOutput, "%s", (it->second).c_str());
	// } For storing exatly what comes out of trace (with no list scheduling)
	fprintf(traceFileOutput, "}\n{\n");
	return insCounter;
}


void storeToFile(map<ADDR,bool> &brFoundMap, map<ADDR,ADDR> &brAddrMap, basicblock* bb, map<ADDR, map<ADDR,string> > &dynBBListMap, FILE* traceFileOutput) {
	// // Print BB/PB Header
	// if (brFound == true) {
	// 	fprintf(traceFileOutput, "H,%llu\n", brAddr); // TOOD Update this for phraseblock
	// }
	//Consolidate maps into one
	map<ADDR,string> dynBBMap;
	for (map<ADDR, map<ADDR,string> >::iterator it = dynBBListMap.begin(); it != dynBBListMap.end(); it++) {
		map<ADDR,string> dynBBMap_partial = it->second;
		fprintf(traceFileOutput, "hey0: %d\n", dynBBMap_partial.size());
		for (map<ADDR,string>::iterator it2 = dynBBMap_partial.begin(); it2 != dynBBMap_partial.end(); it2++) {
			dynBBMap.insert(pair<ADDR,string> (it2->first,it2->second));
		}
	}
	List<instruction*>* listSchBBList = bb->getInsList_ListSchedule();
	fprintf(traceFileOutput, "hey1: %d - %d - %d\n",dynBBMap.size(), bb->getBbSize(), dynBBListMap.size());
	for (int i = 0; i < listSchBBList->NumElements(); i++) {
		fprintf(traceFileOutput, "%s", (dynBBMap[listSchBBList->Nth(i)->getInsAddr()]).c_str());
	}
	// // map<ADDR,string>::iterator it;
	// // for (it = dynBBMap.begin(); it != dynBBMap.end(); it++) {
	// // 	fprintf(traceFileOutput, "%s", (it->second).c_str());
	// // } For storing exatly what comes out of trace (with no list scheduling)
	fprintf(traceFileOutput, "PB}\n{\n");
}

bool isNotRepeated(ADDR insAddr, List<instruction*>* insList) {
	for (int i = 0; i < insList->NumElements(); i++) {
		if (insList->Nth(i)->getInsAddr() == insAddr)
			return false;
	}
	return true;
}

long int findBB(ADDR bbHead, List<map<ADDR,string> > *dynBBList) {
	for (int i = 0; i < dynBBList->NumElements(); i++) {
		if ((dynBBList->Nth(i)).find(bbHead) != (dynBBList->Nth(i)).end()) {
		//if ((*it).find(bbHead) != (*it).end()) { //TODO how do i identify this BB?
			return i; 
		}
	}
	return -1;
}

void removeBB(ADDR bbHead, List<map<ADDR,string> > *dynBBList) {
	int indx = 0;
	for (int i = 0; i < dynBBList->NumElements(); i++) {
		if ((dynBBList->Nth(i)).find(bbHead) != (dynBBList->Nth(i)).end()) {
			dynBBList->RemoveAt(i); 
			return;
		}
	}
	Assert("Element was not removed from List of Map");
}

void annotateTrace_forBB (List<basicblock*>* bbList, map<ADDR,instruction*> *insAddrMap, string *program_name) {
	for (int i = 0; i < bbList->NumElements(); i++) 
		int size = bbList->Nth(i)->getBbSize();
	FILE *traceFileInput, *traceFileOutput;
	if ((traceFileInput  = fopen(("/scratch/tracesim/specint2006/"+(*program_name)+".trace").c_str(), "r")) == NULL) //TODO PARAMETRIZE this line
		Assert("Unable to open the input file.");
	if ((traceFileOutput  = fopen(("/scratch/tracesim/specint2006/bb_trace_archReg/"+(*program_name)+".trace").c_str(), "w")) == NULL) //TODO PARAMETRIZE this line
		Assert("Unable to open the output file.");
	map<ADDR, basicblock*> bbHeaders;
	map<int, int> bbSizeHist;

	/* Create a list of program BB's for use later */
	for (int i =  0; i < bbList->NumElements(); i++) {
		basicblock* bb = bbList->Nth(i);
		bbHeaders.insert(pair<ADDR, basicblock*> (bb->getInsList()->Nth(0)->getInsAddr(), bb));
	}

	/* Walk through the trace */
	long long int insCounter = 0, insSkipCounter = 0, bbSize = 0, printedInsCounter = 0, printedPhiInsCounter = 0;
	List<instruction*>* insList = new List<instruction*>;
	ADDR insAddr, brAddr, junk_addr, brDstAddr;
	bool new_bb = false, new_pb = false;
	basicblock *currentBB = NULL, *prevBB = NULL;
	char insType, junk_s[1000];
	map<ADDR,string> dynBBMap;
	int brTaken = -1;
	//Get rid of first line (blank line perhaps?)
	if (fgets (junk_s, 1000, traceFileInput) == NULL)
		Assert("Trace file is empty!");
	bool brFound = false;
	fprintf(traceFileOutput, "{\n");
	while((insType = fgetc (traceFileInput)) != EOF) {
		insCounter++;
		if (insType == 'R' || insType == 'W') {
			/* TODO put these two lines back */
			// insType = 'M';
			if (fscanf(traceFileInput, ",%llu,%llu", &junk_addr, &insAddr) == EOF) 
				Assert("Invalid end of line detected in trace file.");
			if (fgets (junk_s, 1000, traceFileInput) == NULL) break; //grab the rest of line
			// Eliminate trace instructions that are not mapped in the CFG
			if (insAddrMap->find(insAddr) == insAddrMap->end()) {
				Assert(new_bb == false && "Illegal value for new_bb flag.");
				insSkipCounter++;
				continue;
				// printf("skipping instruction %llx\n", insAddr);
			}
			//Find out if this ins belongs to new BB
			if (bbHeaders.find(insAddr) != bbHeaders.end()) {
				new_bb = true;
				prevBB = currentBB;
				currentBB = bbHeaders[insAddr];
			} else if (currentBB->isInsAddrInBB(insAddr) == false) {
				new_bb = true;
				prevBB = currentBB;
				currentBB = bbHeaders[((*insAddrMap)[insAddr])->getMy_BB_id()];
			}
		} else {
			/* TODO put these two lines back */
			// if (insType == 'B') insType = 'b';
			// else insType = 'o';
			if (insType == 'B') {
				if (fscanf(traceFileInput, ",%llu,%d,%llu", &insAddr, &brTaken, &brDstAddr) == EOF)
					Assert("Invalid end of line detected in trace file.");
			} else if (insType == 'A' || insType == 'F') {
				if (fscanf(traceFileInput, ",%llu", &insAddr) == EOF)
					Assert("Invalid end of line detected in trace file.");
			} else {
				Assert("Invalid Trace Instruction Type.");
			}
			if (fgets (junk_s, 1000, traceFileInput) == NULL) break; //grab the rest of line
			// Eliminate trace instructions that are not mapped in the CFG
			if (insAddrMap->find(insAddr) == insAddrMap->end()) {
				Assert(new_bb == false && "Illegal value for new_bb flag.");
				insSkipCounter++;
				continue;
			}
			//Find out if this ins belongs to new BB
			if (bbHeaders.find(insAddr) != bbHeaders.end()) {
				new_bb = true;
				prevBB = currentBB;
				currentBB = bbHeaders[insAddr];
			} else if (currentBB->isInsAddrInBB(insAddr) == false) {
				new_bb = true;
				prevBB = currentBB;
				currentBB = bbHeaders[((*insAddrMap)[insAddr])->getMy_BB_id()];
			}
		}
		Assert(currentBB->isInsAddrInBB(insAddr) == true && "Trace instruction does not belong to current BB.");
		if (new_bb) {
			map<ADDR,string>::iterator it;
			// Print BB/PB Header
			if (brFound == true) {
				fprintf(traceFileOutput, "H,%llu\n", brAddr);
			}
			// Store BB/PB Ins's
			if (prevBB == NULL) prevBB = currentBB; //for first BB
			List<instruction*>* listSchBBList = prevBB->getInsList_ListSchedule();
//			List<instruction*>* listSchBBList = prevBB->getInsList(); //Replace this with above line for list-scheduling
			for (int i = 0; i < listSchBBList->NumElements(); i++) {
				// if (dynBBMap.find(listSchBBList->Nth(i)->getInsAddr()) == dynBBMap.end()) 
					// printf("didn't find ins in map %d, %d, %d\n", dynBBMap.size(), listSchBBList->NumElements(), dynBBMap.size() < listSchBBList->NumElements());
				Assert (listSchBBList->Nth(i)->isAlreadyAssignedArcRegs() == true);
				if (listSchBBList->Nth(i)->getInsAddr() == PHI_INS_ADDR) {
					fprintf(traceFileOutput, "A,%d,%d#%d,%d#%d,\n", PHI_INS_ADDR,listSchBBList->Nth(i)->getNthArchReg(0),listSchBBList->Nth(i)->getNthRegType(0),listSchBBList->Nth(i)->getNthArchReg(1),listSchBBList->Nth(i)->getNthRegType(1));
					printedInsCounter++;
					printedPhiInsCounter++;
				} else if (dynBBMap.find(listSchBBList->Nth(i)->getInsAddr()) != dynBBMap.end()) {
					fprintf(traceFileOutput, "%s", (dynBBMap[listSchBBList->Nth(i)->getInsAddr()]).c_str());
					printedInsCounter++;
				}
			}
			// for (it = dynBBMap.begin(); it != dynBBMap.end(); it++) {
			// 	fprintf(traceFileOutput, "%s", (it->second).c_str());
			// } For storing exatly what comes out of trace (with no list scheduling)
			fprintf(traceFileOutput, "}\n{\n");
			new_bb = false;
			brFound = false;
			bbSizeHist[bbSize]++;
			bbSize = 0;
			dynBBMap.clear();
 		}
		bbSize++;
		// Construct ins string & store in ins-map
		std::stringstream ss1, ss2;
		string registers = ((*insAddrMap)[insAddr])->getArchRegisterStr();
		if (insType == 'R' || insType == 'W') {
			ss1 << insType << "," << junk_addr << "," << insAddr << "," << registers;
			// ss1 << insType << "," << junk_addr << "," << insAddr << junk_s; (original reg names)
		} else if (insType == 'B') {
			ss1 << insType << "," << insAddr << "," << brTaken << "," << brDstAddr << "," << registers;
			// ss1 << insType << "," << insAddr << "," << brTaken << "," << brDstAddr << junk_s; (original reg names)
		} else { //A & F for insType
			ss1 << insType << "," << insAddr << "," << registers;
			// ss1 << insType << "," << insAddr << junk_s; (original reg names)
		}
		std::string s = ss1.str();
		if (dynBBMap.find(insAddr) != dynBBMap.end()) {
			//Trace runs micro-ops, so it can have ins-addr replicas
			string t = dynBBMap[insAddr];
			ss2 << t << s;
			s = ss2.str();
			dynBBMap[insAddr] = s;
		} else {
			dynBBMap.insert(pair<ADDR,string> (insAddr,s));
		}
		// Crearte basicblock headers
		if (insType == 'B') {
			if (brFound != false) printf("%llu\n", brAddr);
			Assert(brFound == false && "Illegal value for brFound variable.");
			brFound = true;
			brAddr = insAddr;
		}
	}
	if (brFound) {
		fprintf(traceFileOutput, "H:%llu\n", brAddr);
		brFound = false;
	}
	bbSizeHist[bbSize]++;
	fprintf(traceFileOutput, "}\n");
	printf("\tNumber of Trace Instructions Skipped: %lld (out of %lld)\n", insSkipCounter, insCounter);
	printf("\tNumber of Trace Instructions Printed: %lld (out of %lld)\n", printedInsCounter, insCounter-insSkipCounter);
	printf("\tNumber of Trace Phi Instructions Printed: %lld (out of %lld)\n", printedPhiInsCounter, insCounter-printedPhiInsCounter);
	DynBBSizeStat(bbSizeHist, program_name);
}

void annotateTrace_forPB (List<basicblock*>* pbList, map<ADDR,instruction*> *insAddrMap, string *program_name) {
	for (int i = 0; i < pbList->NumElements(); i++) 
		int size = pbList->Nth(i)->getBbSize();
	FILE *traceFileInput, *traceFileOutput;
	if ((traceFileInput  = fopen(("/scratch/tracesim/specint2006/"+(*program_name)+".trace").c_str(), "r")) == NULL)
		Assert("Unable to open the input file.");
	if ((traceFileOutput  = fopen(("/scratch/tracesim/specint2006/bb_trace/"+(*program_name)+"_pb.trace").c_str(), "w")) == NULL)
		Assert("Unable to open the output file.");
	map<ADDR, basicblock*> bbHeaders;
	map<ADDR, list<basicblock*> > pbHeaders;
	map<int, int> bbSizeHist;

	/* Create a list of program BB's for use later */
	for (int i =  0; i < pbList->NumElements(); i++) {
		basicblock* bb = pbList->Nth(i);
		if (bb->isAPhraseblock()) {
			printf("found Phraseblock\n");
			if (pbHeaders.find(bb->getInsList()->Nth(0)->getInsAddr()) != pbHeaders.end()) {
				(pbHeaders[bb->getInsList()->Nth(0)->getInsAddr()]).push_back(bb);
			} else {
				list<basicblock*> lst;
				lst.push_back(bb);
				pbHeaders.insert(pair<ADDR, list<basicblock*> > (bb->getInsList()->Nth(0)->getInsAddr(), lst));
			}
		} else {
			bbHeaders.insert(pair<ADDR, basicblock*> (bb->getInsList()->Nth(0)->getInsAddr(), bb));
		}
	}

	/* Walk through the trace */
	long long int insCounter = 0, insSkipCounter = 0, bbSize = 0, printedInsCounter = 0;
	List<instruction*>* insList = new List<instruction*>;
	ADDR insAddr, brAddr, junk_addr, brDstAddr, bbAddr, prevBbAddr;
	bool new_bb = false, new_pb = false;
	basicblock *currentBB = NULL, *prevBB = NULL;
	// List<basicblock*> *bb_list = new List<basicblock*>;
	// List<ADDR> br_addr_list    = new List<ADDR>;
	// List<bool> br_found_list   = new List<bool>;
	// List<int> bb_size_list     = new List<int>;
	char insType, junk_s[1000];
	map<ADDR,string> dynBBMap;
	List<map<ADDR,string> >* dynBBList = new List<map<ADDR,string> >;
	map<ADDR, basicblock*> statBBListMap;
	list<ADDR> dynBBAddrList;
	map<ADDR, bool> brFoundMap;
	map<ADDR, ADDR> brAddrMap;
	int brTaken = -1;
	//Get rif of first line (blank line perhaps?)
	if (fgets (junk_s, 1000, traceFileInput) == NULL)
		Assert("Trace file is empty!");
	bool brFound = false;
	fprintf(traceFileOutput, "{\n");
	while((insType = fgetc (traceFileInput)) != EOF) {
		insCounter++;
		if (insCounter % 10000 == 0) printf("ins count: %d\n", insCounter);
		if (insType == 'R' || insType == 'W') {
			/* TODO put these two lines back */
			// insType = 'M';
			if (fscanf(traceFileInput, ",%llu,%llu", &junk_addr, &insAddr) == EOF) 
				Assert("Invalid end of line detected in trace file.");
		} else {
			/* TODO put these two lines back */
			// if (insType == 'B') insType = 'b';
			// else insType = 'o';
			if (insType == 'B') {
				if (fscanf(traceFileInput, ",%llu,%d,%llu", &insAddr, &brTaken, &brDstAddr) == EOF)
					Assert("Invalid end of line detected in trace file.");
			} else if (insType == 'A' || insType == 'F') {
				if (fscanf(traceFileInput, ",%llu", &insAddr) == EOF)
					Assert("Invalid end of line detected in trace file.");
			} else {
				Assert("Invalid Trace Instruction Type.");
			}
		}
		if (fgets (junk_s, 1000, traceFileInput) == NULL) break; //grab the rest of line
		// Eliminate trace instructions that are not mapped in the CFG
		if (insAddrMap->find(insAddr) == insAddrMap->end()) {
			Assert(new_bb == false && "Illegal value for new_bb flag.");
			insSkipCounter++;
			continue;
			// printf("skipping instruction %llx\n", insAddr);
		}
		//Find out if this ins belongs to new BB
		if (bbHeaders.find(insAddr) != bbHeaders.end()) {
			new_bb = true;
			prevBbAddr = bbAddr;
			bbAddr = insAddr;
			prevBB = currentBB;
			currentBB = bbHeaders[insAddr];
			if (pbHeaders.find(insAddr) != pbHeaders.end()) {
				// Assert(new_pb == false);
				// currentPB = pbHeaders[insAddr];
				new_pb = true;
			}
		} else if (currentBB->isInsAddrInBB(insAddr) == false) {
			new_bb = true;
			prevBbAddr = bbAddr;
			bbAddr = ((*insAddrMap)[insAddr])->getMy_BB_id();
			prevBB = currentBB;
			currentBB = bbHeaders[((*insAddrMap)[insAddr])->getMy_BB_id()];
			if (pbHeaders.find(insAddr) != pbHeaders.end()) {
			// 	Assert(new_pb == false);
			// 	// currentPB = pbHeaders[((*insAddrMap)[insAddr])->getMy_PB_id()];
			// 	currentPB = ((*insAddrMap)[insAddr])->getMy_PB_id();
				new_pb = true; //TODO is this condution check valid? take care of it when building phraeblocks
			}
		}
		
		Assert(currentBB->isInsAddrInBB(insAddr) == true && "Trace instruction does not belong to current BB.");
		
		if (new_bb) {
			if (dynBBAddrList.size() >= 20) {
				if (pbHeaders.find(dynBBAddrList.front()) != pbHeaders.end()) {//a phraseblock header?
					// printf("found PB?\n");
					list<basicblock*> candidatePBs = pbHeaders.find(dynBBAddrList.front())->second;
					bool pbDetected = false;
					//Match a candidate PB to the list of trace BB's in flight
					for (list<basicblock*>::iterator it = candidatePBs.begin(); it != candidatePBs.end(); it++) {
						List<ADDR>* bbHeadList = (*it)->getBBListForPB();
						pbDetected = true;
						map<ADDR, map<ADDR,string> > listOfBBforPBMap;
						list<ADDR> listOfBBforPB;
						//Cross-ref every BB with the trace BB's in flight
						for (int i = 0; i < bbHeadList->NumElements(); i++) {
							long int bbIndx = findBB(bbHeadList->Nth(i), dynBBList);
							if (bbIndx == -1) {
								pbDetected = false;
								break;
							} else {
								listOfBBforPBMap.insert(pair<ADDR, map<ADDR,string> > (bbHeadList->Nth(i), dynBBList->Nth(bbIndx)));
								listOfBBforPB.push_back(bbHeadList->Nth(i));
							}
						}
						if (pbDetected) {
							// printf("detected PB\n");
							basicblock* detectedPB = (*it);
							storeToFile(brFoundMap, brAddrMap, detectedPB, listOfBBforPBMap, traceFileOutput);
							// Clear processed BB's in map
							for (list<ADDR>::iterator it2 = listOfBBforPB.begin(); it2 != listOfBBforPB.end(); it2++) {
								removeBB(*it2, dynBBList);
								for (list<ADDR>::iterator it3 = dynBBAddrList.end(); it3 !=  dynBBAddrList.begin(); it3--)
									if (*it3 == *it2) {
										dynBBAddrList.erase(it3);
										break;
									}
							}
							break;
							//do we break here?
						}
					}
					if (!pbDetected) {
						// printf("didn't detect a phraseblock\n");
						map<ADDR,string> bbMap = dynBBList->Nth(0);
						if (bbMap.size() == 0) {printf(" - size: %d\n", bbMap.size());}
						printedInsCounter += storeToFile(brFound, brAddr, (statBBListMap.find(dynBBAddrList.front()))->second, bbMap, traceFileOutput);
						dynBBList->RemoveAt(0);// dump the BB in the first spot 
						dynBBAddrList.pop_front();
					}
					//search through the rest of list for finding a PB
					// if found, dump the BB's into a PB and move on
					// else dump the first BB
				} else {
					map<ADDR,string> bbMap = dynBBList->Nth(0);
					if (bbMap.size() == 0) {printf(" - size: %d\n", bbMap.size());}
					printedInsCounter += storeToFile(brFound, brAddr, (statBBListMap.find(dynBBAddrList.front()))->second, bbMap, traceFileOutput);
					dynBBList->RemoveAt(0);// dump the BB in the first spot 
					dynBBAddrList.pop_front();
				}
			}
			if (prevBB == NULL) { //for first BB
				prevBB = currentBB;
				prevBbAddr = bbAddr;
			}
			if (dynBBMap.size() > 0) {
				// printf("REAR: %llx, %d, %d\n", prevBbAddr,dynBBAddrList.size(), dynBBMap.size());
				dynBBList->Append(dynBBMap); //map does not work here (addr replication :s)
				statBBListMap.insert(pair<ADDR,basicblock*> (prevBbAddr, prevBB));
				dynBBAddrList.push_back(prevBbAddr);
				brFoundMap.insert(pair<ADDR,bool> (prevBbAddr,brFound));
				brAddrMap.insert(pair<ADDR,ADDR> (prevBbAddr,brAddr));
			} else {
				printf("empty BB\n");
			}
			brFound = false;
			bbSizeHist[bbSize]++;
			dynBBMap.clear();
			bbSize = 0; //We are not measuring PB size here! (we can add values though)
			new_bb = false;
		}
		bbSize++;
		// Construct ins string & store in ins-map
		std::stringstream ss1, ss2;
		string registers = ((*insAddrMap)[insAddr])->getRegisterStr();
		if (insType == 'R' || insType == 'W') {
			ss1 << insType << "," << junk_addr << "," << insAddr << "," << registers;
			// ss1 << insType << "," << junk_addr << "," << insAddr << junk_s; (original reg names)
		} else if (insType == 'B') {
			ss1 << insType << "," << insAddr << "," << brTaken << "," << brDstAddr << "," << registers;
			// ss1 << insType << "," << insAddr << "," << brTaken << "," << brDstAddr << junk_s; (original reg names)
		} else { //A & F for insType
			ss1 << insType << "," << insAddr << "," << registers;
			// ss1 << insType << "," << insAddr << junk_s; (original reg names)
		}
		std::string s = ss1.str();
		if (dynBBMap.find(insAddr) != dynBBMap.end()) {
			//Trace runs micro-ops, so it can have ins-addr replicas
			string t = dynBBMap[insAddr];
			ss2 << t << s;
			s = ss2.str();
			dynBBMap[insAddr] = s;
		} else {
			dynBBMap.insert(pair<ADDR,string> (insAddr,s));
		}
		// Crearte basicblock headers
		if (insType == 'B') {
			if (brFound != false) printf("%llu\n", brAddr);
			Assert(brFound == false && "Illegal value for brFound variable.");
			// insType = 'b';
			brFound = true;
			brAddr = insAddr;
		}
	}
	if (brFound) {
		fprintf(traceFileOutput, "H:%llu\n", brAddr);
		brFound = false;
	}
	bbSizeHist[bbSize]++;
	fprintf(traceFileOutput, "}\n");
	printf("\tNumber of Trace Instructions Skipped: %lld (out of %lld)\n", insSkipCounter, insCounter);
	printf("\tNumber of Trace Instructions Printed: %lld (out of %lld)\n", printedInsCounter, insCounter-insSkipCounter);
	DynBBSizeStat(bbSizeHist, program_name);
}




// if ()
// // Store BB/PB Ins's
// if (prevBB == NULL) prevBB = currentBB; //for first BB
// if (new_pb) { //PB Mode
// 	// br_found_list->Append(brFound);
// 	// br_addr_list->Append(brAddr);
// 	// bb_size_list->Append(bbSize);
// 	// bb_list->Append(currentBB);
// 	if (found_full_pb) {
// 		if (num_pb_found == 0) {
// 			//Switch to BB Mode (may not need this one)
// 		} else if (num_pb_found == 1) {
// 			//Dupmp PB
// 		} else {
// 			Assert("More than one PB candidate found.");
// 		}
// 		new_pb = false;
// 		dynBBMap.clear();
// 	} else if (found_partial_pb) {
// 		//Keep fetching stuff
// 	} else {
// 		//Switch to BB Mode
// 		for (int i = 0; i < bb_list->NumElements(); i++) {
// 			basicblock* bb = bb_list->Nth(i);
// 			bool br_addr = br_addr_list->Nth(i);
// 			bool br_found = br_found_list->Nth(i);
// 			int bb_size = bb_size_list->Nth(i);
// 			storeToFile(br_found, br_addr, bb, dynBBMap, traceFileOutput);
// 			bbSizeHist[bb_size]++;
// 		}
// 		new_pb = false;
// 		brFound = false; //TODO what to do with this?
// 		dynBBMap.clear();
// 	}
// } else { //BB Mode
