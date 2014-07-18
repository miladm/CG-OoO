#include "make_instruction.h"
#include "regFile.h"
#include "dependencyTable.h"
#include "config.h"

// char* input_asm_file;

void parseRegisters(instruction* newIns, FILE* input_assembly) {
	regFile* RF = new regFile;
	char reg[REG_STRING_SIZE];
	int read_write = 0;
	long int regCode;
	while(1) {
		if (fgets(reg, REG_STRING_SIZE, input_assembly) == NULL) Assert("Register name not found");
		if (reg[0] == '-') break; //some ops have no reg
		if (fscanf(input_assembly, "%d\n", &read_write) == EOF) Assert("Register type not found");
		reg[strlen(reg) - 1] = 0; //cut out newline
		regCode = RF->getRegNum(reg);
		if (regCode != INVALID_REG) {
			newIns->setRegister(&regCode, &read_write);
		}
		// printf("debug: %d, %d, %s\n", read_write, regCode, reg);
	}
	delete RF;
}

void parse_instruction(List<instruction*> *insList, 
					   map<ADDR,instruction*> *insAddrMap,
                       set<ADDR> *brDstSet,
		       		map<ADDR, double> *brBiasMap,
		       		map<ADDR, double> *bpAccuracyMap,
			   		map<ADDR, double> *upldMap,
			   		map<ADDR,set<ADDR> > &memRdAddrMap,
			   		map<ADDR,set<ADDR> > &memWrAddrMap,
			   		std::string *program_name) {
	char c[INS_STRING_SIZE], ins[INS_STRING_SIZE];
	ADDR insAddr, brDst;
	FILE * input_assembly;
	// dependencyTable* depTables = new dependencyTable; /* DISABLED */
	// if ((input_assembly = fopen(input_asm_file, "r")) == NULL) {
	if ((input_assembly = fopen(("/home/milad/esc_project/svn/memTraceMilad/TraceSim/phraseblock_framework/input_files/"+(*program_name)+".s").c_str(), "r")) == NULL) {
		Assert("Cannot open assembly file.");
	}
	FILE * input_brBias;
	if ((input_brBias = fopen(("/home/milad/esc_project/svn/memTraceMilad/TraceSim/phraseblock_framework/input_files/"+(*program_name)+"_bias.csv").c_str(), "r")) == NULL) {
		Assert("Cannot open branch bias file.");
	}
	FILE * input_bpAccuracy;
	if ((input_bpAccuracy = fopen(("/home/milad/esc_project/svn/memTraceMilad/TraceSim/phraseblock_framework/input_files/"+(*program_name)+"_bpAccuracy.csv").c_str(), "r")) == NULL) {
		Assert("Cannot open branch prediction accuracy file.");
	}
	FILE * input_upld;
	if ((input_upld = fopen("input_files/input_upld.csv", "r")) == NULL) {
		Assert("Cannot open unpred load ops file.");
	}

	FILE * input_mem;
	if ((input_mem = fopen("frontend/mem_trace.csv", "r")) == NULL) {
		Assert("Cannot open mem addresses file.");
	}

	//Parse branch bias numbers
	printf("\tRead branch bias profile file: %s\n", ("input_files/"+(*program_name)+"_bias.csv").c_str());
	while (1) {
		ADDR addr;
		double bias;
		if (fscanf(input_brBias, "%lx, %lf\n", &addr, &bias) == EOF) break;
		brBiasMap->insert(pair<ADDR, double> (addr,bias));
	}
	//Parse branch prediction numbers
	printf("\tRead branch prediction accuracy profile file: %s\n", ("input_files/"+(*program_name)+"_bpAccuracy.csv").c_str());
	while (1) {
		ADDR addr;
		double accuracy;
		if (fscanf(input_bpAccuracy, "%lx, %lf\n", &addr, &accuracy) == EOF) break;
		bpAccuracyMap->insert(pair<ADDR, double> (addr,accuracy));
	}
	//Parse unpredictable load numbers
	printf("\tRead UPLD profile file: input_files/input_upld.csv\n");
	while (1) {
		ADDR addr;
		double missRate;
		if (fscanf(input_upld, "(%ld, %lf)\n", &addr, &missRate) == EOF) break;
		upldMap->insert(pair<ADDR, double> (addr,missRate));
		
	}
	//Parse memory access addresses
	// while (1) {
	// 	ADDR insAddr, dataAddr;
	// 	char type;
	// 	if (fscanf(input_mem, "%llx: %c %llx\n", &insAddr, &type, &dataAddr) == EOF) break;
	// 	if (type == 'R') {
	// 		memRdAddrMap[insAddr].insert(dataAddr);
	// 	} else if (type == 'W') {
	// 		memWrAddrMap[insAddr].insert(dataAddr);
	// 	} else {
	// 		Assert("Invalid memory address access type.");
	// 	}
	// }
	//Parse assembly instructions
	while (1) {
		// printf("---\n");
		instruction *newIns = new instruction;
		while(1) {
			if (fgets(c, OPCODE_STRING_SIZE, input_assembly) == NULL) break;
			// printf("debug: insType: %s\n", c);
			if (c[0] == 'j'  || c[0] == 'b'|| c[0] == 'c' || c[0] == 'r' || c[0] == 'o') {
				//Find instruction type
				newIns->setType(c[0]);				
			} else if (c[0] == 'R' || c[0] == 'W') {
				//Find if instruction is a memory op
				int memAccessSize = -1;
				if (c[0] == 'W') newIns->setWrMemType();
				else if (c[0] == 'R') newIns->setRdMemType();
				if (fscanf(input_assembly, "%d\n", &memAccessSize) == EOF) break;
				newIns->setMemAccessSize(memAccessSize);
				newIns->setType('M');//TODO: must not set if already set
			} else if (c[0] == '#') {
				//Set the instruction opcode mnemonic
				newIns->setOpCode(c);
				break;
			} else {
				Assert("Instruction Type was not recognized.");
			}
		}
		//Set instruction disassembled code
		if (fgets(ins, INS_STRING_SIZE, input_assembly) == NULL) break;
		newIns->setInsAsm(ins);
		// printf("debug: ASM: %s\n", ins);
		//Set instruction address
		if (fscanf(input_assembly, "%llx\n", &insAddr) == EOF) break;
		newIns->setInsAddr(insAddr);
		// printf("debug: insAddr = %llx\n", insAddr);
		//Setup instruction branch destination/bias/accuracy information
		if (newIns->getType() == 'j'  || newIns->getType() == 'b'|| newIns->getType() == 'c') {
			if (fscanf(input_assembly, "%llx\n", &brDst) == EOF) break;
			newIns->setBrDst(brDst);
			brDstSet->insert(brDst);
			//Setup branch prediction accuracy information
			if (bpAccuracyMap->find(insAddr) != bpAccuracyMap->end()) {
				newIns->setBPaccuracy((*bpAccuracyMap)[insAddr]);
			} else if (newIns->getType() == 'b') {
				;// printf("ERROR: branch instruction prediction accuracy not found! (%s, line: %d)\n" , __FILE__, __LINE__);
				//exit(1);
			}
			//Setup branch bias information
			if (brBiasMap->find(insAddr) != brBiasMap->end()) {
				newIns->setBrTakenBias((*brBiasMap)[insAddr]);
			} else if (newIns->getType() == 'b') {
				;// printf("ERROR: branch instruction bias not found! (%s, line: %d)\n" , __FILE__, __LINE__);
				//exit(1);
			}
		} else if (newIns->getType() == 'M') {
			if (memRdAddrMap.find(insAddr) != memRdAddrMap.end()) {
				newIns->setRdAddrSet(memRdAddrMap[insAddr]);
			} 
			if (memWrAddrMap.find(insAddr) != memWrAddrMap.end()) {
				newIns->setWrAddrSet(memWrAddrMap[insAddr]);
			}
		}
		//Set miss-rate if instruction is a UPLD
		if (upldMap->find(insAddr) != upldMap->end() && newIns->isRdMemType() == true) {
			newIns->setLdMissRate((*upldMap)[insAddr]);
		}
		//Set instruction registers (read & write)
		parseRegisters(newIns, input_assembly);
		// newIns->setOpCode(opCode); done somewhere else
		insList->Append(newIns);
		insAddrMap->insert(pair<ADDR, instruction*> (newIns->getInsAddr(), newIns));
		// newIns->dependencyTableCheck(depTables); /* DISABLED */
	}

	//Close files
	fclose(input_assembly);
	fclose(input_brBias);
	fclose(input_bpAccuracy);
	fclose(input_upld);
	fclose(input_mem);
}