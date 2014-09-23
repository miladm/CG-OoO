#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string>
#include <list>
#include <time.h>

#include "registerAllocate.h"
#include "make_instruction.h"
#include "make_basicblock.h"
#include "make_phraseblock.h"
#include "annotateTrace.h"
#include "dominator.h"
#include "config.h"
#include "global.h"
#include "logGen.h"
#include "list.h"
#include "stat.h"
#include "ssa.h"
#include "dot.h"

void finish (List<basicblock*> *bbList, List<basicblock*> *phBBList, std::string *program_name, SCH_MODE sch_mode, REG_ALLOC_MODE reg_alloc_mode) {
	/* STAT Generation Functions */
	// printf("FILE NAME: %s\n", (*program_name).c_str());
	StatBBSizeStat(bbList, program_name);
	StatNum_interBB_and_intra_BB_regs(bbList, program_name);
	/* ------------------------- */
	for (int i = 0; i < bbList->NumElements(); i++) {
		// printf("BB Size: %d\n", bbList->Nth(i)->getBbSize());
		//bbList->Nth(i)->printBb();
	}
	dot cfg(0, program_name);
	cfg.runDot(bbList);
	dot cfg_phrase(1, program_name);
	cfg_phrase.runDot(phBBList);
	writeToFile(bbList, program_name, sch_mode, reg_alloc_mode);
	// writeToFile(pbList, program_name, sch_mode, reg_alloc_mode);
}

int main(int argc, char* argv[])
{
	Assert(argc == 4 && "USAGE: ./PhraseFormer <program_name> <reg_alloc_method> <scheduling_method>");
	unsigned t0=clock(),t1;
	//SETUP VARIABLES
	List<instruction*>* insList = new List<instruction*>;
	List<basicblock*>* bbList   = new List<basicblock*>;
	List<basicblock*>* phBBList = new List<basicblock*>;
	//Linst<phraseblock*> *phList = new List<phraseblock*>;
	map<int,variable*> varList;
	map<ADDR, basicblock*> bbMap;
	map<ADDR, double> brBiasMap;
	map<ADDR, double> bpAccuracyMap;
	map<ADDR, double> upldMap;
	map<ADDR,set<ADDR> > memRdAddrMap;
	map<ADDR,set<ADDR> > memWrAddrMap;
	map<ADDR,instruction*> insAddrMap;
	set<ADDR> brDstSet;
	std::string program_name = argv[1];

    /*-- REGISTER ALLOCATION MODE --*/
	REG_ALLOC_MODE reg_alloc_mode;
    cout << argv[2] << " " << argv[3] << endl;
    if (strcmp (argv[2], "global_reg") == 0) reg_alloc_mode = GLOBAL;
    else if (strcmp (argv[2], "local_global_reg") == 0) reg_alloc_mode = LOCAL_GLOBAL;
    else Assert (true == false && "Wrong register allocation mode specified");

    /*-- STATIC CODE SCHEDULING MODE --*/
    SCH_MODE sch_mode;
    if (strcmp (argv[3], "list_sch") == 0) sch_mode = LIST_SCH;
    else if (strcmp (argv[3], "no_list_sch") == 0) sch_mode = NO_LIST_SCH;
    else Assert (true == false && "Wrong static code scheduling mode specified");

    Assert (!(sch_mode == LIST_SCH && reg_alloc_mode == GLOBAL)); /*-- BAD COMBO --*/

	printf("-------------\nPROGRAM NAME: %s\n-------------\n\n", program_name.c_str());
	//init(argc, argv);
	printf("- Configure Program -\n");
	parse_config_file();
	printf("- Parse Instructions -\n");
	parse_instruction(insList, &insAddrMap, &brDstSet, &brBiasMap, &bpAccuracyMap, &upldMap, memRdAddrMap, memWrAddrMap, &program_name);
	printf("- Make Basic Blocks -\n");
	make_basicblock(insList, bbList, varList, &brDstSet, &bbMap, &insAddrMap, sch_mode);
	printf("- Build Dominance Frontier -\n");
	setup_dominance_frontier(bbList);
	printf("- Build SSA -\n");
	build_ssa_form(bbList, varList);
	printf("- Register Allocation -\n");
	allocate_register(bbList, insList, reg_alloc_mode);
	// printf("- Make Phraesblocks -\n");
	// phBBList = make_phraseblock(bbList, &brBiasMap, &bpAccuracyMap);
	printf("- Annotate Trace -\n");
	//annotateTrace_forBB(bbList, &insAddrMap, &program_name);
	// annotateTrace_forPB(phBBList, &insAddrMap, &program_name);
	/* remove the code below */
	// printf("num BB's: %d\n", bbList->NumElements());
	// for (int i =0 ; i < insList->NumElements(); i++) {
	// // 	if (insList->Nth(i)->isAlreadyAssignedArcRegs())
	// // 		;// printf("GOOD\n");
	// // 	else	
	// // 		printf("SHIT\n");
	// 	instruction* ins = insList->Nth(i);
	// 	printf("%llx: ",ins->getInsAddr());
	// 	for (int i = 0; i < ins->getNumReadReg(); i++) {
	// 		printf("(%d->%d),",ins->getNthReadReg(i),ins->getReadRegSubscript(ins->getNthReadReg(i)));
	// 	}
	// 	for (int i = 0; i < ins->getNumWriteReg(); i++) {
	// 		printf("(%d->%d),",ins->getNthWriteReg(i),ins->getWriteRegSubscript(ins->getNthWriteReg(i)));
	// 	}
	// 	printf("\n");
	// }
	/* ---------------------*/
	printf("- Make Dot Files & Stat Data -\n");
	finish(bbList, phBBList, &program_name, sch_mode, reg_alloc_mode);
	t1=clock()-t0;
	printf("\n-------------\nEXECUTION COMPLETED SUCCESSFULLY. (Time: %f Minutes)\n-------------\n", ((float)t1)/CLOCKS_PER_SEC/60.0);
	return 0;
}
