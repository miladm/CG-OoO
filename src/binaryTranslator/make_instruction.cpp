#include "make_instruction.h"
#include "regFile.h"
#include "dependencyTable.h"
#include "config.h"

// char* input_asm_file;

void parseRegisters (instruction* newIns, FILE* input_assembly) {
	regFile* RF = new regFile;
	char reg[REG_STRING_SIZE];
	int read_write = 0;
	long int regCode, specialRegCode;

	while (true) {
		if (fgets (reg, REG_STRING_SIZE, input_assembly) == NULL) 
            Assert (0 && "Register name not found");
		if (reg[0] == '-') break; //SOME OPS HAVE NO REG

		if (fscanf (input_assembly, "%d\n", &read_write) == EOF) 
            Assert (0 && "Register type not found");
		reg[strlen (reg) - 1] = 0; //CUT OUT NEWLINE

		regCode = RF->getRegNum (reg);
		specialRegCode = RF->getSpecialRegNum (reg);
        Assert ((regCode == INVALID_REG || specialRegCode == INVALID_REG) && 
                "One of the two fields must return invalid");

		if (regCode != INVALID_REG) {
			newIns->setRegister (&regCode, &read_write);
        } else if (specialRegCode != INVALID_REG) {
			newIns->setSpecialRegister (&specialRegCode, &read_write);
        } else {
            printf ("\t\tWARNING: Unrecorgnized register %s\n", reg);
        }
	}
	delete RF;
}

void parse_instruction (List<instruction*> *insList, 
					   map<ADDR,instruction*> *insAddrMap,
                       set<ADDR> *brDstSet,
		       		map<ADDR, double> *brBiasMap,
		       		map<ADDR, double> *bpAccuracyMap,
			   		map<ADDR, double> *upldMap,
			   		map<ADDR,set<ADDR> > &memRdAddrMap,
			   		map<ADDR,set<ADDR> > &memWrAddrMap,
			   		std::string *program_name,
                    CLUSTER_MODE cluster_mode) {
	char c[INS_STRING_SIZE], ins[INS_STRING_SIZE];
	ADDR insAddr, insFallThru, insDst, brDst;
	bool hasFallThru, hasDst;
	FILE * input_assembly;
	FILE * input_brBias;
	FILE * input_bpAccuracy;
	FILE * input_upld;
	FILE * input_mem;

	// if ((input_assembly = fopen (input_asm_file, "r")) == NULL) {

    string input_path = "/home/milad/esc_project/svn/PARS/src/binaryTranslator/input_files/";
    string profile_path = "/home/milad/esc_project/svn/PARS/src/binaryTranslator/profile_files/ref/";

    /* ASSSEMBLY FILE FETCH */
	if ((input_assembly = fopen ((input_path + (*program_name) + ".s").c_str (), "r")) == NULL) {
		Assert ("Cannot open assembly file.");
	}

    /* BRANCH BIAS FILE FETCH */
    ostringstream lo_wbb, hi_wbb;
    lo_wbb << WBB_LOWER_BOUND;
    hi_wbb << WBB_UPPER_BOUND;
    string file_ext = "_wbb_" + lo_wbb.str () + "_" + hi_wbb.str () + ".csv";
	if ((input_brBias = fopen ((profile_path + (*program_name) + file_ext).c_str (), "r")) == NULL) {
		Assert ("Cannot open branch bias file.\n");
	}

    /* BRANCH ACCURACY FILE FETCH */
//    ostringstream upld_thr;
//    upld_thr << UPLD_THRESHOLD;
//    file_ext = "_upld_" + upld_thr.str () + ".csv";
//	if ((input_bpAccuracy = fopen ((input_path + (*program_name) + file_ext).c_str (), "r")) == NULL) {
//		Assert ("Cannot open branch prediction accuracy file.");
//	}

    /* UPLD FILE FETCH */
    ostringstream upld_thr;
    upld_thr << UPLD_THRESHOLD;
    file_ext = "_upld_" + upld_thr.str () + ".csv";
	if ((input_upld = fopen ((profile_path + (*program_name) + file_ext).c_str (), "r")) == NULL) {
		Assert ("Cannot open unpred load ops file.");
	}

    /* MEMORY TRACE FILE FETCH */
	if ((input_mem = fopen ("frontend/mem_trace.csv", "r")) == NULL) {
		Assert ("Cannot open mem addresses file.");
	}

	/* PARSE BRANCH BIAS NUMBERS */
	printf ("\tRead branch BIAS profile file");
	while (1) {
		ADDR addr;
		double bias;
        int taken, br;
		if (fscanf (input_brBias, "%lx,%lf,%d,%d\n", &addr, &bias, &taken, &br) == EOF) break;
		brBiasMap->insert (pair<ADDR, double> (addr,bias));
	}

	/* PARSE BRANCH PREDICTION NUMBERS */
//	printf ("\tRead branch prediction accuracy profile file\n");
//	while (1) {
//		ADDR addr;
//		double accuracy;
//		if (fscanf (input_bpAccuracy, "%lx, %lf\n", &addr, &accuracy) == EOF) break;
//		bpAccuracyMap->insert (pair<ADDR, double> (addr,accuracy));
//	}

	/* PARSE UNPREDICTABLE LOAD NUMBERS */
	printf ("\tRead UPLD profile file\n");
	while (1) {
		ADDR addr;
		double missRate;
        int miss_ld, ld;
		if (fscanf (input_upld, "%lx, %lf,%d,%d\n", &addr, &missRate, &miss_ld, &ld) == EOF) break;
		upldMap->insert (pair<ADDR, double> (addr, missRate));
	}

	//Parse memory access addresses
	// while (1) {
	// 	ADDR insAddr, dataAddr;
	// 	char type;
	// 	if (fscanf (input_mem, "%llx: %c %llx\n", &insAddr, &type, &dataAddr) == EOF) break;
	// 	if (type == 'R') {
	// 		memRdAddrMap[insAddr].insert (dataAddr);
	// 	} else if (type == 'W') {
	// 		memWrAddrMap[insAddr].insert (dataAddr);
	// 	} else {
	// 		Assert ("Invalid memory address access type.");
	// 	}
	// }

	/* PARSE ASSEMBLY INSTRUCTIONS */
	printf ("\tRead X86 .s file: %s\n", ("input_files/"+ (*program_name) + ".s").c_str ());
	while (1) {
		instruction *newIns = new instruction;
		while (1) {
			if (fgets (c, OPCODE_STRING_SIZE, input_assembly) == NULL) break;
			// printf ("debug: insType: %s\n", c);
			if (c[0] == 'j'  || c[0] == 'b'|| c[0] == 'c' || c[0] == 'r'  || c[0] == 'o'|| c[0] == 'n' || c[0] == 's') {
				/* SET INSTRUCTION TYPE */
				newIns->setType (c[0]);
			} else if (c[0] == 'R' || c[0] == 'W') {
				/* SET IF INSTRUCTION IS A MEMORY OP */
				int memAccessSize = -1;
				if (c[0] == 'W') newIns->setWrMemType ();
				else if (c[0] == 'R') newIns->setRdMemType ();
				if (fscanf (input_assembly, "%d\n", &memAccessSize) == EOF) break;
				newIns->setMemAccessSize (memAccessSize);
				newIns->setType ('M');//TODO: must not set if already set
			} else if (c[0] == '#') {
				/* SET THE INSTRUCTION OPCODE MNEMONIC */
				newIns->setOpCode (c);
				break;
			} else {
				Assert (0 && "Instruction Type was not recognized.");
			}
		}

		/* SET INSTRUCTION DISASSEMBLED CODE */
		if (fgets (ins, INS_STRING_SIZE, input_assembly) == NULL) break;
		newIns->setInsAsm (ins);
		// printf ("debug: ASM: %s\n", ins);

		/* SET INSTRUCTION ADDRESS */
		if (fscanf (input_assembly, "%llx\n", &insAddr) == EOF) break;
		newIns->setInsAddr (insAddr);
		// printf ("debug: insAddr = %llx\n", insAddr);

        /* SET INSTRUCTION FALL-THROUGH */
        insFallThru = 0;
		if (fscanf (input_assembly, "%d\n", &hasFallThru) == EOF) break;
		if (hasFallThru) {if (fscanf (input_assembly, "%llx\n", &insFallThru) == EOF) break;}
		newIns->setInsFallThruAddr (insFallThru, hasFallThru);

        /* SET INSTRUCTION DESTIATION */
        insDst = 0;
		if (fscanf (input_assembly, "%d\n", &hasDst) == EOF) break;
        if (hasDst) {if (fscanf (input_assembly, "%llx\n", &insDst) == EOF) break;}
		newIns->setInsDstAddr (insDst, hasDst);

		/* SETUP INSTRUCTION BRANCH DESTINATION INFORMATION */
		if (newIns->getType () == 'j' || newIns->getType () == 'b' || newIns->getType () == 'c') {
			/* SETUP BB START SET */
			if (newIns->hasDst ()) 
                brDstSet->insert (newIns->getInsDstAddr ());
			if (newIns->hasFallThru () && 
                (newIns->getType () == 'b' || newIns->getType () == 'c')) 
                brDstSet->insert (newIns->getInsFallThruAddr ());
        }

		/* SETUP INSTRUCTION BRANCH BIAS/ACCURACY INFORMATION */
		if (newIns->getType () == 'j' || newIns->getType () == 'b' || newIns->getType () == 'c') {
			/* SETUP BRANCH PREDICTION ACCURACY INFORMATION */
//			if (bpAccuracyMap->find (insAddr) != bpAccuracyMap->end ()) {
//				newIns->setBPaccuracy ((*bpAccuracyMap)[insAddr]);
//			} else if (newIns->getType () == 'b') {
//				;// printf ("\t\tERROR: branch instruction prediction accuracy not found! (%s, line: %d)\n" , __FILE__, __LINE__);
//				//exit (1);
//			}

			/* SETUP BRANCH BIAS INFORMATION */
			if (brBiasMap->find (insAddr) != brBiasMap->end ()) {
				newIns->setBrTakenBias ((*brBiasMap)[insAddr]);
			} else if (newIns->getType () == 'b') {
				;// printf ("\t\tERROR: branch instruction bias not found! (%s, line: %d)\n" , __FILE__, __LINE__);
				//exit (1);
			}
		}

        if (newIns->getType () == 'M') {
			if (memRdAddrMap.find (insAddr) != memRdAddrMap.end ()) {
				newIns->setRdAddrSet (memRdAddrMap[insAddr]);
			} 
			if (memWrAddrMap.find (insAddr) != memWrAddrMap.end ()) {
				newIns->setWrAddrSet (memWrAddrMap[insAddr]);
			}
		}

		/* SET MISS-RATE IF INSTRUCTION IS A UPLD */
		if (upldMap->find (insAddr) != upldMap->end () && newIns->isRdMemType () == true) {
			newIns->setLdMissRate ((*upldMap)[insAddr]);
            newIns->setUPLDins ();
		}

		/* SET INSTRUCTION REGISTERS (READ & WRITE) */
		parseRegisters (newIns, input_assembly);
		// newIns->setOpCode (opCode); done somewhere else
		insList->Append (newIns);
		insAddrMap->insert (pair<ADDR, instruction*> (newIns->getInsAddr (), newIns));
	}

    
    /* 
     * - IF AN INSTRUCTION NEXT INS IS NOT IN THE INSLIST, CLEAN THE INS ENTRIED
     * - THIS CLEANS UP THE DESTINATION OF CALL INSTRUCTIONS
     */
    int num_missing_ins = 0;
    for (int i = 0; i < insList->NumElements (); i++) {
        ADDR dst, fallThru;
        instruction* ins = insList->Nth (i);
        if (ins->hasDst ()) {
            dst = ins->getInsDstAddr ();
            if (insAddrMap->find (dst) == insAddrMap->end ()) {
                ins->resetInsDst (); num_missing_ins++;
            }
        }
        if (ins->hasFallThru ()) {
            fallThru = ins->getInsFallThruAddr ();
            if (insAddrMap->find (fallThru) == insAddrMap->end ()) {
                ins->resetInsFallThru (); num_missing_ins++;
            }
        }
    }
	printf ("\t\tWARNING: Number of missing instructions in insList: %d\n", num_missing_ins);

    for (int i = 0; i < insList->NumElements (); i++) {
        ADDR dst, fallThru;
        instruction* ins = insList->Nth (i);

        /* FIND DESTINATION POINTERS */
        if (ins->hasDst ()) {
            dst = ins->getInsDstAddr ();
            Assert (insAddrMap->find (dst) != insAddrMap->end ());
            instruction* dst_ins = (*insAddrMap)[dst];
            ins->setInsDst (dst_ins);
        }

        /* FIND FALL-THROUGH POINTERS */
        if (ins->hasFallThru ()) {
            fallThru = ins->getInsFallThruAddr ();
            Assert (insAddrMap->find (fallThru) != insAddrMap->end ());
            instruction* fallThru_ins = (*insAddrMap)[fallThru];
            ins->setInsFallThru (fallThru_ins);
        }
    }

	/* CLOSE FILES */
	fclose (input_assembly);
	fclose (input_brBias);
//	fclose (input_bpAccuracy);
	fclose (input_upld);
	fclose (input_mem);
}
