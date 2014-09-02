/*******************************************************************************
 * staticCodeParser.cpp
 *******************************************************************************/

#include "staticCodeParser.h"

staticCodeParser::staticCodeParser (g_variable *g_var, config *g_cfg) {
	_g_var = g_var;
	_g_cfg = g_cfg;
	char *program_name = _g_cfg->getProgName ();
	string file = "/home/milad/esc_project/svn/PARS/src/binaryTranslator/output_files/"+string (program_name)+"_obj.s";
	if ( (_inFile  = fopen (file.c_str (), "r")) == NULL) 
		Assert ("Unable to open the input static code file.");
	if (g_var->g_verbose_level & V_FRONTEND) std::cout << "STATIC CODE FILE: " << file.c_str () << std::endl;
    createDB ();
	parse ();
}

staticCodeParser::~staticCodeParser () {
	;//TODO iterate through the maps and delete objects
}

void staticCodeParser::getRegisters (ADDRS insAddr, string registers) {
    //    Assert (_insObjMap.find (insAddr) != _insObjMap.end ());
    //    stInstruction* ins = _insObjMap[insAddr];
    int scanStatus, i = 0, j = 0;
    string r_reg[4], w_reg[4];
    for (int i = 0; i < 4; i++) {
        r_reg[i] = "null";
        w_reg[i] = "null";
    }
    string insAdr = iTos (insAddr);
    while (true) {
        AR reg;
        int typeTemp;
        char s[100];
        scanStatus = sscanf (registers.c_str (), "%u#%d,%s", &reg, &typeTemp, s);
        if (scanStatus != 3) break;
        Assert (typeTemp == 1 || typeTemp == 2);
        registers = string (s);
        AXES_TYPE reg_axes_type = (typeTemp == 1 ? READ : WRITE);
        //        ins->setAR (reg, reg_axes_type);
        if (reg_axes_type == READ) {r_reg[i] = (i < 4) ? iTos ((ADDRS) reg) : "null"; i++;}
        else if (reg_axes_type == WRITE) {w_reg[j] = (j < 4) ? iTos ((ADDRS) reg) : "null"; j++;}
        else {Assert (true == false);}
    }
    string cmd = "INSERT INTO perlbench_400 VALUES (" + insAdr + ","
        + r_reg[0] + "," 
        + r_reg[1] + ","
        + r_reg[2] + "," 
        + r_reg[3] + ","
        + w_reg[0] + "," 
        + w_reg[1] + ","
        + w_reg[2] + "," 
        + w_reg[3] + ");";
    sql_db (":memory:", cmd.c_str ());
}

//PRE:
//	.s files must be avilaable from the compilation stage
//Parse .s File
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
			scanStatus = fscanf (_inFile, ",%ld\n", &bbAddr);
			Assert (scanStatus != EOF);
//			makeNewBB (bbAddr);
		} else if (insType == '}') {
			if (scanStatus == EOF) break;
			scanStatus = fscanf (_inFile, "\n");
		} else if (insType == 'H') {
			Assert (scanStatus != EOF);
			scanStatus = fscanf (_inFile, ",%ld\n", &insAddr);
			Assert (scanStatus != EOF);
//			addBBheader (insAddr, bbAddr);
		} else if (insType == 'j' || insType == 'c' || insType == 'b' || insType == 'r') {
			Assert (scanStatus != EOF);
			scanStatus = fscanf (_inFile, ",%ld,-brTaken-,%ld,%s\n", &insAddr, &brDest, regs_dummy);
			string registers (regs_dummy,100);
			Assert (scanStatus != EOF);
//			makeNewIns (insType, insAddr, brDest, registers, memAccessSize);
            getRegisters (insAddr, registers);
//			addToBB (insAddr, bbAddr);
		} else if (insType == 'R' || insType == 'W') {
			Assert (scanStatus != EOF);
			scanStatus = fscanf (_inFile, ",-memAddr-,%ld,%d,%s\n", &insAddr, &memAccessSize, regs_dummy);
			string registers (regs_dummy,100);
			Assert (scanStatus != EOF);
//			makeNewIns (insType, insAddr, brDest, registers, memAccessSize);
            getRegisters (insAddr, registers);
//			addToBB (insAddr, bbAddr);
		} else if (insType == 'o') {
			Assert (scanStatus != EOF);
			scanStatus = fscanf (_inFile, ",%ld,%s\n", &insAddr, regs_dummy);
			string registers (regs_dummy,100);
			Assert (scanStatus != EOF);
//			makeNewIns (insType, insAddr, brDest, registers, memAccessSize);
            getRegisters (insAddr, registers);
//			addToBB (insAddr, bbAddr);
		} else {
			printf ("Parsed Value: %c", insType);
			Assert (true == false && "Unrecognized character parsed");
		}
	}
	printf ("Static code fetch DONE\n");
}

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

void staticCodeParser::addToBB (ADDRINT insAddr, ADDRINT bbAddr) {
	#ifdef ASSERTION
	Assert (bbAddr != 0 && insAddr != 0);
	#endif
	_bbMap[bbAddr]->bbInsSet.push_back (insAddr);
}

//Make a new static instruction and add it to instruction list
void staticCodeParser::makeNewIns (char insType, ADDRINT insAddr, ADDRINT brDest, string registers, ADDRINT memAccessSize) {
	Assert (insType != 'z' && insAddr != 0);
	//Assert (_insMap.find (insAddr) == _insMap.end () && "Instruction address already present in insMap");
	//printf ("Instruction address already present in insMap\n");
//	insObj * newIns = new insObj; //last elem uninitialized
//	newIns->ins_str = "\0"; //Allocate later when brTaken and/or memAddr are known
//	newIns->registers = registers;
//	newIns->insType[0] = insType; //TODO expand this for several u-ops - this changes memAddr and brDst allocations below too
//	newIns->insAddr = insAddr;
//	newIns->brDest = ( (insType == 'j' || insType == 'c' || insType == 'b' || insType == 'r') ?  brDest : 0);
//	newIns->memAccessSize = ( (insType == 'R' || insType == 'W') ?  memAccessSize : 0);
//	_insMap.insert (pair<ADDRINT,insObj*> (insAddr, newIns));

    stInstruction* newInsObj = new stInstruction;
    newInsObj->setInsAddr (insAddr);
// TODO this code is removed for memory efficiency - put it back?
//    if (insType == 'j' || insType == 'c' || insType == 'b' || insType == 'r') {
//        newInsObj->setInsType (BR);
//        bool isJump = (insType == 'j' ? true : false);
//        bool isCall = (insType == 'c' ? true : false);
//        bool isRet  = (insType == 'r' ? true : false);
//        newInsObj->setBrAtr (brDest, isCall, isRet, isJump);
//    } else if (insType == 'R') {
//        newInsObj->setInsType (MEM);
//        newInsObj->setMemAtr (LOAD, memAccessSize);
//    } else if (insType == 'W') {
//        newInsObj->setInsType (MEM);
//        newInsObj->setMemAtr (STORE, memAccessSize);
//    } else {
//        newInsObj->setInsType (ALU);
//    }
	_insObjMap.insert (pair<ADDRS,stInstruction*> (insAddr, newInsObj));
}

bool staticCodeParser::isInsIn_insMap (ADDRINT insAddr) {
	Assert (insAddr != 0);
	std::stringstream ss;
	if (_insMap.find (insAddr) != _insMap.end ()) {
		return true;
	} else {
		return false;
	}
}

std::string staticCodeParser::getBrIns (ADDRINT insAddr, BOOL hasFT, ADDRINT tgAddr, ADDRINT ftAddr, BOOL isTaken) {
	Assert (insAddr != 0);
	std::stringstream ss;
	if (_insMap.find (insAddr) != _insMap.end ()) {
		char insType = _insMap[insAddr]->insType[0];
		if (insType == 'j' || insType == 'c' || insType == 'b' || insType == 'r') {
//            if (tgAddr == 0) cout << "warning - zero tgAddr" << endl;
			//Assert (tgAddr != 0 && "invalid branch dst address value.");
			ss << "B" << "," << insAddr << "," << isTaken  << "," << tgAddr << "," << _insMap[insAddr]->registers;
		} else {
			//Assert (true == false && "Unrecognized instruction type - expecting a branch instruction");
			ss << "MILAD";
		}
	} else {
		Assert (true == false && "Didn't find the instruction stream");
	}
	string ins_str = (ss.str () + string ("\n"));
	return ins_str;
}

std::string staticCodeParser::getMemIns (ADDRINT insAddr, ADDRINT memAccessSize, ADDRINT memAddr) {
	Assert (insAddr != 0);
	std::stringstream ss;
	if (_insMap.find (insAddr) != _insMap.end ()) {
		char insType = _insMap[insAddr]->insType[0];
		//if (insType == 'W' || insType == 'R') {
            //if (memAddr == 0) cout << "warning - zero memAddr" << endl;
			//Assert (memAddr != 0 && "invalid memory address value.");
			//Assert (_insMap[insAddr]->memAccessSize == memAccessSize && "Unexpected memory access size");
			ss << insType << "," << memAddr << "," << insAddr << "," << memAccessSize << "," << _insMap[insAddr]->registers;
		//} else {
			//Assert (true == false && "Unrecognized instruction type - expecting a memory instruction");
			//ss << "MILAD";
		//}
	} else {
		Assert (true == false && "Didn't find the instruction stream");
	}
	string ins_str = (ss.str () + string ("\n"));
	return ins_str;
}

std::string staticCodeParser::getIns (ADDRINT insAddr) {
	Assert (insAddr != 0);
	std::stringstream ss;
	if (_insMap.find (insAddr) != _insMap.end ()) {
		char insType = _insMap[insAddr]->insType[0];
		if (insType == 'o') { //A, D, F for ins->getType ()
			ss << "A" << "," << insAddr << "," << _insMap[insAddr]->registers;
		} else {
			//Assert (true == false && "Unrecognized instruction type - expecting a ALU instruction");
			ss << "MILAD";
		}
	} else {
		Assert (true == false && "Didn't find the instruction stream");
	}
	string ins_str = (ss.str () + string ("\n"));
	return ins_str;
}

stInstruction* staticCodeParser::getInsObj (ADDRINT insAddr) {
    Assert (_insObjMap.find (insAddr) != _insObjMap.end ());
	return _insObjMap[insAddr];
}

bool staticCodeParser::isNewBB (ADDRINT insAddr) {
	bool newBB = (_bbMap.find (insAddr) != _bbMap.end () ? true : false);
	return newBB;
}

// Create BB top
std::string staticCodeParser::getBB_top (ADDRINT bbAddr) {
	Assert (bbAddr == _bbMap[bbAddr]->bbAddr && "Instruction is not the first instruction in BB");
	std::stringstream ss;
	ss << bbAddr;
	string strg = (string ("{,") + ss.str () + string (",\n")); //TODO add a comma at the end of line
	return strg;
}

// Create BB bottom
std::string staticCodeParser::getBB_bottom () {
	string strg = (string ("}\n"));
	return strg;
}

// Does BB have a header?
BOOL staticCodeParser::BBhasHeader (ADDRINT bbAddr) {
	Assert (bbAddr == _bbMap[bbAddr]->bbAddr && "Instruction is not the first instruction in BB");
	return _bbMap[bbAddr]->bbHasHeader;
}

// Get the header string for the BB
std::string staticCodeParser::getBBheader (ADDRINT bbAddr) {
	Assert (bbAddr == _bbMap[bbAddr]->bbAddr && "Instruction is not the first instruction in BB");
	std::stringstream ss;
	ss << _bbMap[bbAddr]->bbHeader;
	string strg = (string ("H,") + ss.str () + string (",\n"));
	return strg;
}

void staticCodeParser::createDB () {
    string db_name = ":memory:";
    string cmd = "CREATE TABLE perlbench_400 (ins_addr INT PRIMARY KEY, a_rd_reg_0 INT, a_rd_reg_1 INT, a_rd_reg_2 INT, a_rd_reg3 INT, a_wr_reg_0 INT, a_wr_reg_1 INT, a_wr_reg_2 INT, a_wr_reg_3 INT);";
    sql_db_open (db_name.c_str (), cmd.c_str ());
}

string staticCodeParser::iTos (ADDRS value) {
    ostringstream ss;
    ss << value;
    return ss.str ();
}

void staticCodeParser::populateDB () {
    string db_name = ":memory:";
//    string cmd = "DROP TABLE perlbench_400;";
//    sql_db (db_name.c_str (), cmd.c_str ());
    string cmd = "CREATE TABLE perlbench_400 (ins_addr INT PRIMARY KEY, a_rd_reg_0 INT, a_rd_reg_1 INT, a_rd_reg_2 INT, a_rd_reg3 INT, a_wr_reg_0 INT, a_wr_reg_1 INT, a_wr_reg_2 INT, a_wr_reg_3 INT);";
//    sql_db_open (db_name.c_str (), cmd.c_str ());
    map<ADDRINT,stInstruction*>::iterator it;
    for (it = _insObjMap.begin (); it != _insObjMap.end (); it++) {
        stInstruction* ins = it->second;
        string insAdr = iTos (ins->_ins_addr);
        string r_reg[4], w_reg[4];
        for (int i = 0; i < 4; i++) {
            r_reg[i] = (i < ins->_a_rdReg.NumElements ()) ? iTos ((ADDRS) ins->_a_rdReg.Nth (i)) : "null";
        }
        for (int i = 0; i < 4; i++) {
            w_reg[i] = (i < ins->_a_wrReg.NumElements ()) ? iTos ((ADDRS) ins->_a_wrReg.Nth (i)) : "null";
        }
        cmd = "INSERT INTO perlbench_400 VALUES (" + insAdr + ","
            + r_reg[0] + "," 
            + r_reg[1] + ","
            + r_reg[2] + "," 
            + r_reg[3] + ","
            + w_reg[0] + "," 
            + w_reg[1] + ","
            + w_reg[2] + "," 
            + w_reg[3] + ")";
//        sql_db (db_name.c_str (), cmd.c_str ());
        delete ins;
    }
    cout << "delete map?" << endl;
    int x;
    cin >> x;
}

void staticCodeParser::openDB () {
    sqlite3* db = NULL; 
    sqlite3_open (":memory:", &db); 
    _g_var->g_db = db;
}

void staticCodeParser::closeDB () {
    sqlite3_close (_g_var->g_db);
}

bool staticCodeParser::exists (ADDRS ins_addr) {
    //    string db_name = "benchDB.db";
    //    string bench_name = "perlbench_400";
    //    string cmd = "SELECT COUNT(1) FROM TABLE " + bench_name + " WHERE ins_addr = " + iTos (ins_addr) + ";";
    //    sql_db (db_name.c_str (), cmd.c_str ());

    string cmd = "SELECT * FROM perlbench_400 WHERE ins_addr = " + iTos (ins_addr) + ";";
    sqlite3_stmt* stmt = NULL;
    sqlite3_prepare_v2 (_g_var->g_db, cmd.c_str (), -1, &stmt, NULL); 
    sqlite3_bind_int (stmt, 1, 42);
    int column_count = (sqlite3_step (stmt) == SQLITE_ROW) ? true : false;
    sqlite3_finalize (stmt); 
//    sqlite3_close (db);
    return column_count;
}

void staticCodeParser::testDB () {
//    string db_name = "benchDB.db";
    string cmd = "SELECT * FROM perlbench_400 WHERE ins_addr = "+ iTos (4195300) + ";";
//    sqlite3* db = NULL; 
//    sqlite3_open ("benchDB.db", &db); 
    sqlite3_stmt* stmt = NULL;
    sqlite3_prepare_v2 (_g_var->g_db, cmd.c_str (), -1, &stmt, NULL); 
    sqlite3_bind_int (stmt, 1, 42);
    if (sqlite3_step (stmt) == SQLITE_ROW) { 
        for (int i = 1; i < 5; i++) {
            int reg = sqlite3_column_int (stmt, i);
            if (reg == 0) break;
            cout << "rd reg: " << reg << endl;
        }
        for (int i = 5; i < 9; i++) {
            int reg = sqlite3_column_int (stmt, i);
            if (reg == 0) break;
            cout << "wr reg: " << reg << endl;
        }
    }
    sqlite3_finalize (stmt); 
//    sqlite3_close (db);
}
