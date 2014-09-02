/*******************************************************************************
 * bbInstruction.cpp
 *******************************************************************************/

#include "bbInstruction.h"

bbInstruction::bbInstruction (string class_name)
    : dynInstruction (class_name)
{ 
    _bbWin_id = -1;
}

bbInstruction::~bbInstruction () {}

// ***********************
// ** SET INS ATRIBUTES **
// ***********************
void bbInstruction::setBB (dynBasicblock* bb) {
#ifdef ASSERTION
    Assert (bb != NULL);
#endif
	_bb = bb;
}

void bbInstruction::setBBWinID (WIDTH bbWin_id) {
    Assert (bbWin_id > -1 && _bbWin_id == -1); 
    _bbWin_id = bbWin_id;
}

void bbInstruction::setPR (PR pr, AXES_TYPE type) {
    if  (type == READ) {
        _p_rdReg.Append (pr);
        _p_rdReg_waitList.Append (pr);
    } else if  (type == WRITE) {
        _p_wrReg.Append (pr);
    } else {
        Assert (true == false && "wrong instruction type");
    }
}

void bbInstruction::setAR (AR ar, AXES_TYPE type) {
    if (USE_LRF && getARtype (ar) == LOCAL_REG) {
        if  (type == READ) {
            _l_rdReg.Append (ar);
            _l_rdReg_waitList.Append (ar);
        } else if  (type == WRITE) {
            _l_wrReg.Append (ar);
        } else {
            Assert (true == false && "wrong instruction type");
        }
    } else {
        if  (type == READ) {
            _a_rdReg.Append (ar);
            _a_rdReg_waitList.Append (ar);
        } else if  (type == WRITE) {
            _a_wrReg.Append (ar);
        } else {
            Assert (true == false && "wrong instruction type");
        }
    }

}

REG_TYPE bbInstruction::getARtype (AR a_reg) {
#ifdef ASSERTION
    Assert (USE_LRF == true);
#endif
    if (a_reg >= LARF_LO && a_reg <= LARF_HI) return LOCAL_REG;
    else if (a_reg >= GARF_LO && a_reg <= GARF_HI) return GLOBAL_REG;
    else Assert (true == false && "Architectural register is neither LOCAL nor GLOBAL");
    return LOCAL_REG; //Placeholder for compiler
}

// ***********************
// ** GET INS ATRIBUTES **
// ***********************
dynBasicblock* bbInstruction::getBB () {return _bb;}

WIDTH bbInstruction::getBBWinID () {Assert (_bbWin_id > -1); return _bbWin_id;}

WIDTH bbInstruction::getNumRdAR () {return _a_rdReg_waitList.NumElements();}

WIDTH bbInstruction::getNumRdLAR () {return _l_rdReg_waitList.NumElements();}

WIDTH bbInstruction::getTotNumRdAR () {return _a_rdReg.NumElements();}

WIDTH bbInstruction::getTotNumRdLAR () {return _l_rdReg.NumElements();}

WIDTH bbInstruction::getNumRdPR () {return _p_rdReg_waitList.NumElements();}

List<AR>* bbInstruction::getARrdList () {return &_a_rdReg_waitList;}

List<AR>* bbInstruction::getARwrList () {return &_a_wrReg;}

List<PR>* bbInstruction::getPRrdList () {return &_p_rdReg_waitList;}

List<PR>* bbInstruction::getPRwrList () {return &_p_wrReg;}

List<AR>* bbInstruction::getLARrdList () {return &_l_rdReg_waitList;}

List<AR>* bbInstruction::getLARwrList () {return &_l_wrReg;}

// ***********************
// ** INS CONTROL       **
// ***********************

/* COPY REGISTERS FROM ONE OBJ TO ANOTHER */
void bbInstruction::copyRegsTo (sqlite3* db) {
//    for (int i = 0; i < _p_rdReg.NumElements (); i++) {
//        PR reg = _p_rdReg.Nth (i);
//        ins->setPR (reg, READ);
//    }
//    for (int i = 0; i < _p_wrReg.NumElements (); i++) {
//        PR reg = _p_wrReg.Nth (i);
//        ins->setPR (reg, WRITE);
//    }
//    string db_name = "benchDB.db";
//    string cmd = "SELECT * FROM perlbench_400 WHERE ins_addr = "+ iTos (getInsAddr ()) + ";";
////    sql_db (db_name.c_str (), cmd.c_str ());
//    sqlite3* db = NULL; 
//    sqlite3_open("benchDB.db", &db); 
//    sqlite3_stmt* stmt = NULL;
//    sqlite3_prepare_v2(db, "SELECT COUNT(1) FROM TABLE " + bench_name + " WHERE ins_addr = " + iTos (ins_addr) + ";", &stmt, NULL); 
//    sqlite3_bind_int(stmt, 1, 42);
//    while (sqlite3_step(stmt) == SQLITE_ROW) { 
//        int row;
//        row = sqlite3_column_int(stmt, 0);
//    }
//    sqlite3_finalize(stmt); 
//    sqlite3_close(db);

//    string db_name = "benchDB.db";
    string cmd = "SELECT * FROM perlbench_400 WHERE ins_addr = "+ iTos (_ins_addr) + ";";
//    sqlite3* db = NULL; 
//    sqlite3_open ("benchDB.db", &db); 
    sqlite3_stmt* stmt = NULL;
    sqlite3_prepare_v2 (db, cmd.c_str (), -1, &stmt, NULL); 
    sqlite3_bind_int (stmt, 1, 42);
    if (sqlite3_step (stmt) == SQLITE_ROW) { 
        for (int i = 1; i < 5; i++) {
            int reg = sqlite3_column_int (stmt, i);
            if (reg == 0) break;
            setAR (reg, READ);
//            cout << "rd reg: " << reg << endl;
        }
        for (int i = 5; i < 9; i++) {
            int reg = sqlite3_column_int (stmt, i);
            if (reg == 0) break;
            setAR (reg, WRITE);
//            cout << "wr reg: " << reg << endl;
        }
    }
    sqlite3_finalize (stmt); 
//    sqlite3_close (db);

//    for (int i = 0; i < _a_rdReg.NumElements (); i++) {
//        AR reg = _a_rdReg.Nth (i);
//        ins->setAR (reg, READ);
//    }
//    for (int i = 0; i < _a_wrReg.NumElements (); i++) {
//        AR reg = _a_wrReg.Nth (i);
//        ins->setAR (reg, WRITE);
//    }
//    if (USE_LRF) {
//        for (int i = 0; i < _l_rdReg.NumElements (); i++) {
//            AR reg = _l_rdReg.Nth (i);
//            ins->setAR (reg, READ);
//        }
//        for (int i = 0; i < _l_wrReg.NumElements (); i++) {
//            AR reg = _l_wrReg.Nth (i);
//            ins->setAR (reg, WRITE);
//        }
//    }
}

//bool bbInstruction::exists () {
//    string db_name = "benchDB.db";
//    string bench_name = "perlbench_400";
//    string cmd = "SELECT COUNT(1) FROM TABLE " + bench_name + " WHERE ins_addr = " + iTos (ins_addr) + ";";
//    return sql_exists (db_name.c_str (), cmd.c_str ());
//}
//
string bbInstruction::iTos (ADDRS value) {
    ostringstream ss;
    ss << value;
    return ss.str ();
}

/* Used to re-run ins after squash recovery */
void bbInstruction::resetStates () {
    while (_p_rdReg_waitList.NumElements () > 0) {
        _p_rdReg_waitList.RemoveAt(0);
    }
    while (_p_rdReg.NumElements () > 0) {
        _p_rdReg.RemoveAt(0);
    }
    while (_p_wrReg.NumElements () > 0) {
        _p_wrReg.RemoveAt(0);
    }

    while (_a_rdReg_waitList.NumElements () > 0) {
        _a_rdReg_waitList.RemoveAt(0);
    }
    for (int i = 0; i < _a_rdReg.NumElements(); i++) {
        _a_rdReg_waitList.Append (_a_rdReg.Nth(i));
    }

    if (USE_LRF) {
        while (_l_rdReg_waitList.NumElements () > 0) {
            _l_rdReg_waitList.RemoveAt(0);
        }
        for (int i = 0; i < _l_rdReg.NumElements(); i++) {
            _l_rdReg_waitList.Append (_l_rdReg.Nth(i));
        }
    }

    _insStage = NO_STAGE;
    _lq_state = LQ_NO_STATE;
    _sq_state = SQ_NO_STATE;
    _is_mem_violation = false;
    _bbWin_id = -1;
}

/* Usage: for coarse grain execution - eg. BB */
void bbInstruction::resetWrongPath () {
    _is_on_wrong_path = false;
}
