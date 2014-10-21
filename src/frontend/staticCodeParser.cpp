/*******************************************************************************
 * staticCodeParser.cpp
 *******************************************************************************/

#include "staticCodeParser.h"

staticCodeParser::staticCodeParser (config *g_cfg) 
    : s_missing_static_bb_cnt (g_stats.newScalarStat ("staticCodeParser", "s_missing_static_bb_cnt", "Number of times static version of a BB not existed.", 0, NO_PRINT_ZERO))
{
	_g_cfg = g_cfg;
    string reg_alloc_mode_s, sch_mode_s;
	string bench_name = _g_cfg->getProgName ();
    SCH_MODE sch_mode = _g_cfg->getSchMode ();
    REG_ALLOC_MODE reg_alloc_mode = _g_cfg->getRegAllocMode ();

    if (sch_mode == NO_LIST_SCH) sch_mode_s = "no_list_sch";
    else if (sch_mode == LIST_SCH) sch_mode_s = "list_sch";
    else Assert ("invalid scheduling mode");

    if (reg_alloc_mode == GLOBAL) reg_alloc_mode_s = "global_reg";
    else if (reg_alloc_mode == LOCAL_GLOBAL) reg_alloc_mode_s = "local_global_reg";
    else Assert ("invalid reg allocation mode");

    string in_dir = "/home/milad/esc_project/svn/PARS/src/binaryTranslator/output_files/";
    string in_file_path = in_dir + reg_alloc_mode_s + "/" + sch_mode_s + "/" + string (bench_name) + "_obj.s";
//	string file = "/home/milad/esc_project/svn/PARS/src/binaryTranslator/output_files/" + reg_alloc_mode_s + "/" + sch_mode_s + "/" + string (bench_name)+"_obj.s";
//	string file = "/home/milad/esc_project/svn/memTraceMilad/TraceSim/phraseblock_framework/output_files/"+string (bench_name)+"_obj.s";
	if ( (_inFile  = fopen (in_file_path.c_str (), "r")) == NULL) 
		Assert ("Unable to open the input static code file.");
	if (g_var.g_verbose_level & V_FRONTEND) cout << "STATIC CODE FILE: " << in_file_path.c_str () << endl;
	parse ();
}

staticCodeParser::~staticCodeParser () {
    map<ADDRINT, stInstruction*>::iterator st_ins_it;
    map<ADDRINT, bbObj*>::iterator bb_it;
    for (st_ins_it = _insObjMap.begin (); st_ins_it != _insObjMap.end (); st_ins_it++)
        delete st_ins_it->second;
    for (bb_it = _bbMap.begin (); bb_it != _bbMap.end (); bb_it++)
        delete bb_it->second;
}

/*--
 * PRE:
 * 	.s files must be avilaable from the compilation stage
 * Parse .s File
 --*/
void staticCodeParser::parse () {
	ADDRINT bbAddr = 0, insAddr = 0, brDest = 0;
	int memAccessSize = 0;
	char insType = 'z';
	char regs_dummy[100];

	int scanStatus = 0; //non-EOF init
	while (scanStatus != EOF) {
		scanStatus = fscanf (_inFile, "%c", &insType);
		if (insType == '{') {
			Assert (scanStatus != EOF);
			scanStatus = fscanf (_inFile, ",%lx\n", &bbAddr);
			Assert (scanStatus != EOF);
			makeNewBB (bbAddr);
		} else if (insType == '}') {
			if (scanStatus == EOF) break;
			scanStatus = fscanf (_inFile, "\n");
		} else if (insType == 'H') {
			Assert (scanStatus != EOF);
			scanStatus = fscanf (_inFile, ",%lx\n", &insAddr);
			Assert (scanStatus != EOF);
			addBBheader (insAddr, bbAddr);
		} else if (insType == 'j' || insType == 'c' || insType == 'b' || insType == 'r') {
			Assert (scanStatus != EOF);
			scanStatus = fscanf (_inFile, ",%lx,-brTaken-,%lx%s\n", &insAddr, &brDest, regs_dummy);
			string registers (regs_dummy, 100);
			Assert (scanStatus != EOF);
			makeNewIns (insType, insAddr, brDest, registers, memAccessSize);
            getRegisters (insAddr, registers);
			addToBB (insAddr, bbAddr, insType);
		} else if (insType == 'R' || insType == 'W') {
			Assert (scanStatus != EOF);
			scanStatus = fscanf (_inFile, ",-memAddr-,%lx,%d%s\n", &insAddr, &memAccessSize, regs_dummy);
			string registers (regs_dummy, 100);
			Assert (scanStatus != EOF);
			makeNewIns (insType, insAddr, brDest, registers, memAccessSize);
            getRegisters (insAddr, registers);
			addToBB (insAddr, bbAddr, insType);
		} else if (insType == 'o' || insType == 'n' || insType == 's') {
			Assert (scanStatus != EOF);
			scanStatus = fscanf (_inFile, ",%lx%s\n", &insAddr, regs_dummy);
			string registers (regs_dummy, 100);
			Assert (scanStatus != EOF);
			makeNewIns (insType, insAddr, brDest, registers, memAccessSize);
            getRegisters (insAddr, registers);
			addToBB (insAddr, bbAddr, insType);
		} else {
			printf ("Parsed Value: %c", insType);
			Assert (true == false && "Unrecognized character parsed");
		}
	}
	printf ("Static code fetch DONE\n");
}

void staticCodeParser::getRegisters (ADDRS insAddr, string registers) {
    Assert (_insObjMap.find (insAddr) != _insObjMap.end ());
    stInstruction* ins = _insObjMap[insAddr];
    int scanStatus;
    while (true) {
        AR reg;
        int typeTemp;
        char s[100];
        scanStatus = sscanf (registers.c_str (), ",%u#%d%s", &reg, &typeTemp, s);
        if (scanStatus != 3) break;
        Assert (typeTemp == 1 || typeTemp == 2);
        registers = string (s);
        AXES_TYPE reg_axes_type = (typeTemp == 1 ? READ : WRITE);
        ins->setAR (reg, reg_axes_type);
    }
}

/* ***************************** *
 * INS FUNCTIONS
 * ***************************** */

/*-- Make a new static instruction and add it to instruction list --*/
void staticCodeParser::makeNewIns (char insType, ADDRINT insAddr, ADDRINT brDest, string registers, ADDRINT memAccessSize) {
	Assert (insType != 'z' && insAddr != 0);

    stInstruction* newInsObj = new stInstruction;
    newInsObj->setInsAddr (insAddr);
    if (insType == 'j' || insType == 'c' || insType == 'b' || insType == 'r') {
        newInsObj->setInsType (BR);
        bool isJump = (insType == 'j' ? true : false);
        bool isCall = (insType == 'c' ? true : false);
        bool isRet  = (insType == 'r' ? true : false);
        newInsObj->setBrAtr (brDest, isCall, isRet, isJump);
    } else if (insType == 'R') {
        newInsObj->setInsType (MEM);
        newInsObj->setMemAtr (LOAD, memAccessSize);
    } else if (insType == 'W') {
        newInsObj->setInsType (MEM);
        newInsObj->setMemAtr (STORE, memAccessSize);
    } else {
        newInsObj->setInsType (ALU);
    }
	_insObjMap.insert (pair<ADDRS,stInstruction*> (insAddr, newInsObj));
}

BOOL staticCodeParser::hasIns (ADDRINT insAddr) {
	Assert (insAddr != 0);
	return (_insObjMap.find (insAddr) != _insObjMap.end ()) ? true : false;
}

stInstruction* staticCodeParser::getInsObj (ADDRINT insAddr) {
    Assert (_insObjMap.find (insAddr) != _insObjMap.end ());
	return _insObjMap[insAddr];
}

/* ***************************** *
 * BB FUNCTIONS
 * ***************************** */
void staticCodeParser::makeNewBB (ADDRINT bbAddr) {
	#ifdef ASSERTION
	//Assert (bbAddr != 0);
	//Assert (_bbMap.find (bbAddr) == _bbMap.end () && "BB address already present in bbMap");
	//printf ("WARNING: BB address already present in bbMap");
	#endif
	bbObj * newBB = new bbObj;
	newBB->bbHeader = 0;
	newBB->bbAddr = bbAddr;
	newBB->bbHasHeader = false;
	_bbMap.insert (pair<ADDRINT,bbObj*> (bbAddr, newBB));
}

void staticCodeParser::addBBheader (ADDRINT insAddr, ADDRINT bbAddr) {
	#ifdef ASSERTION
	Assert (bbAddr != 0 && insAddr != 0);
	#endif
	_bbMap[bbAddr]->bbHeader = insAddr;
	_bbMap[bbAddr]->bbHasHeader = true;
}

void staticCodeParser::addToBB (ADDRINT insAddr, ADDRINT bbAddr, char insType) {
	#ifdef ASSERTION
	Assert (bbAddr != 0 && insAddr != 0);
	#endif
	_bbMap[bbAddr]->bbInsList.push_back (insAddr);
    if (!_bbMap[bbAddr]->hasBr &&
        (insType == 'j' || insType == 'c' || insType == 'b' || insType == 'r')) {
        _bbMap[bbAddr]->hasBr = true;
        _bbMap[bbAddr]->brAddr = insAddr;
    } else if (_bbMap[bbAddr]->hasBr &&
        (insType == 'j' || insType == 'c' || insType == 'b' || insType == 'r')) {
//        Assert (true == false && "A BB can only hold one branch / jump"); //TODO put this back when the reason is uncovered
    }
}

BOOL staticCodeParser::isNewBB (ADDRINT insAddr) {
	bool newBB = (_bbMap.find (insAddr) != _bbMap.end () ? true : false);
	return newBB;
}

/*-- Create BB top --*/
string staticCodeParser::getBB_top (ADDRINT bbAddr) {
	Assert (bbAddr == _bbMap[bbAddr]->bbAddr && "Instruction is not the first instruction in BB");
	stringstream ss;
	ss << bbAddr;
	string strg = (string ("{,") + ss.str () + string (",\n")); //TODO add a comma at the end of line
	return strg;
}

/*-- Create BB bottom --*/
string staticCodeParser::getBB_bottom () {
	string strg = (string ("}\n"));
	return strg;
}

bool staticCodeParser::hasStaticBB (ADDRINT bbID) {
    return (_bbMap.find(bbID) != _bbMap.end());
}

BOOL staticCodeParser::bbHasBr (ADDRINT bbAddr) {
    return _bbMap[bbAddr]->hasBr;
}

ADDRINT staticCodeParser::getBBbr (ADDRINT bbAddr) {
    return _bbMap[bbAddr]->brAddr;
}

list<ADDRS>& staticCodeParser::getBBinsList (ADDRINT bbID) {
    if (_bbMap.find(bbID) == _bbMap.end()) {
        s_missing_static_bb_cnt++;
    }
    return _bbMap[bbID]->bbInsList;
}

/*-- Does BB have a header? --*/
BOOL staticCodeParser::BBhasHeader (ADDRINT bbAddr) {
	Assert (bbAddr == _bbMap[bbAddr]->bbAddr && "Instruction is not the first instruction in BB");
	return _bbMap[bbAddr]->bbHasHeader;
}

/*-- Get the header string for the BB --*/
string staticCodeParser::getBBheader (ADDRINT bbAddr) {
	Assert (bbAddr == _bbMap[bbAddr]->bbAddr && "Instruction is not the first instruction in BB");
	stringstream ss;
	ss << _bbMap[bbAddr]->bbHeader;
	string strg = (string ("H,") + ss.str () + string (",\n"));
	return strg;
}
