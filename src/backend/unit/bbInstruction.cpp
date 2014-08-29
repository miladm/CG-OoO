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
    if  (type == READ) {
        _a_rdReg.Append (ar);
        _a_rdReg_waitList.Append (ar);
    } else if  (type == WRITE) {
        _a_wrReg.Append (ar);
    } else {
        Assert (true == false && "wrong instruction type");
    }
}

// ***********************
// ** GET INS ATRIBUTES **
// ***********************
dynBasicblock* bbInstruction::getBB () { return _bb; }

WIDTH bbInstruction::getBBWinID () {Assert (_bbWin_id > -1); return _bbWin_id;}

WIDTH bbInstruction::getNumRdAR () { return _a_rdReg_waitList.NumElements(); }

WIDTH bbInstruction::getTotNumRdAR () { return _a_rdReg.NumElements(); }

WIDTH bbInstruction::getNumRdPR () { return _p_rdReg_waitList.NumElements(); }

List<AR>* bbInstruction::getARrdList () {return &_a_rdReg_waitList;}

List<AR>* bbInstruction::getARwrList () {return &_a_wrReg;}

List<PR>* bbInstruction::getPRrdList () {return &_p_rdReg_waitList;}

List<PR>* bbInstruction::getPRwrList () {return &_p_wrReg;}

// ***********************
// ** INS CONTROL       **
// ***********************

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
