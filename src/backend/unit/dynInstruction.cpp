/*******************************************************************************
 * dynInstruction.cpp
 *******************************************************************************/

#include "dynInstruction.h"

dynInstruction::dynInstruction ()
    : unit ("dynInstruction")
{
    _insStage = NO_STAGE;
    _lq_state = LQ_NO_STATE;
    _sq_state = SQ_NO_STATE;
    _is_on_wrong_path = false;
}

dynInstruction::~dynInstruction () {}

// ***********************
// ** SET INS ATRIBUTES **
// ***********************
void dynInstruction::setInsAddr (ADDRS insAddr) {
	_insAddr = insAddr;
}

void dynInstruction::setInsID (INS_ID seqNum) {
#ifdef ASSERTION
    Assert (seqNum > 0);
#endif
	_seqNum = seqNum;
}

void dynInstruction::setInsType (INS_TYPE insType) {
	_insType = insType;
}

void dynInstruction::setPR (PR pr, AXES_TYPE type) {
    if  (type == READ) {
        _p_rdReg.Append (pr);
        _p_rdReg_waitList.Append (pr);
    } else if  (type == WRITE) {
        _p_wrReg.Append (pr);
    } else {
        Assert (true == false && "wrong instruction type");
    }
}

void dynInstruction::setAR (AR ar, AXES_TYPE type) {
    if  (type == READ) {
        _a_rdReg.Append (ar);
        _a_rdReg_waitList.Append (ar);
    } else if  (type == WRITE) {
        _a_wrReg.Append (ar);
    } else {
        Assert (true == false && "wrong instruction type");
    }
}

void dynInstruction::setMemAtr (MEM_TYPE memType, ADDRS memAddr, BYTES memAxesSize, 
                                bool stackRd, bool stackWr) {
    _memType = memType;
    _memAddr = memAddr;
    _memAxesSize = memAxesSize;
    _stackRd = stackRd;
    _stackWr = stackWr;
}

void dynInstruction::setMemAtr (MEM_TYPE memType, BYTES memAxesSize) {
    _memType = memType;
    _memAxesSize = memAxesSize;
}

void dynInstruction::setBrAtr (ADDRS brTarget, ADDRS brFalThru, bool hasFalThru, bool brTaken, bool isCall, bool isRet, bool isJump, bool isDirBrOrCallOrJmp) {
    _brTarget = brTarget;
    _brTaken = brTaken;
    if (isJump) _brType = JMP;
    else if (isCall) _brType = CALL;
    else if (isRet) _brType = RET;
    else if (isDirBrOrCallOrJmp) _brType = DIR_BR;
    else  _brType = INDIR_BR;
}

void dynInstruction::setBrAtr (ADDRS brTarget, bool isCall, bool isRet, bool isJump) {
    _brTarget = brTarget;
    if (isJump) _brType = JMP;
    else if (isCall) _brType = CALL;
    else if (isRet) _brType = RET;
}

void dynInstruction::setPipeStage (PIPE_STAGE insStage) {
#ifdef ASSERTION
    switch (insStage) {
        case FETCH: Assert (_insStage == NO_STAGE); break;
        case DECODE: Assert (_insStage == FETCH); break;
        case DISPATCH: Assert (_insStage == DECODE); break;
        case ISSUE: Assert (_insStage == DISPATCH); break;
        case EXECUTE: Assert (_insStage == ISSUE); break;
        case MEM_ACCESS: Assert (_insStage == EXECUTE); break;
        case COMPLETE: Assert (_insStage == MEM_ACCESS || _insStage == EXECUTE); break;
        case COMMIT: Assert (_insStage == COMPLETE); break;
        default: Assert (true == false && "Invalid Pipeine stage"); break;
    }
#endif
    _insStage = insStage;
}

void dynInstruction::setSQstate (SQ_STATE state) {
#ifdef ASSERTION
    Assert (getMemType () == STORE);
    switch (state) {
        case SQ_ADDR_WAIT: Assert (_sq_state == SQ_NO_STATE); break;
        case SQ_COMPLETE: Assert (_sq_state == SQ_ADDR_WAIT); break;
        case SQ_COMMIT: Assert (_sq_state == SQ_COMPLETE); break;
        case SQ_CACHE_DISPATCH: Assert (_sq_state == SQ_COMMIT); break;
        default: Assert (true == false && "Invalid Pipeine stage"); break;
    }
#endif
    _sq_state = state;
}

void dynInstruction::setLQstate (LQ_STATE state) {
#ifdef ASSERTION
    Assert (getMemType () == LOAD);
    dbg.print (DBG_TEST, "%s: %d %d (cyc:)\n", _c_name.c_str (), state, _lq_state);
    switch (state) {
        case LQ_ADDR_WAIT: Assert (_lq_state == LQ_NO_STATE); break;
        case LQ_PENDING_CACHE_DISPATCH: Assert (_lq_state == LQ_ADDR_WAIT); break;
        case LQ_FWD_FROM_SQ: Assert (_lq_state == LQ_PENDING_CACHE_DISPATCH); break;
        case LQ_MSHR_WAIT: Assert (_lq_state == LQ_PENDING_CACHE_DISPATCH); break;
        case LQ_CACHE_WAIT: Assert (_lq_state == LQ_PENDING_CACHE_DISPATCH); break;
        case LQ_COMPLETE: Assert (_lq_state == LQ_FWD_FROM_SQ || 
                                  _lq_state == LQ_MSHR_WAIT || 
                                  _lq_state == LQ_CACHE_WAIT); break;
        default: Assert (true == false && "Invalid Pipeine stage"); break;
    }
#endif
    _lq_state = state;
}

void dynInstruction::setWrongPath (bool is_on_wrong_path) {
    _is_on_wrong_path = is_on_wrong_path;
}

// ***********************
// ** GET INS ATRIBUTES **
// ***********************
ADDRS dynInstruction::getInsAddr () { return _insAddr; }

INS_ID dynInstruction::getInsID () { return _seqNum; }

INS_TYPE dynInstruction::getInsType () { return _insType; }

ADDRS dynInstruction::getMemAddr () { return _memAddr; }

MEM_TYPE dynInstruction::getMemType () { return _memType; }

BYTES dynInstruction::getMemAxesSize () { return _memAxesSize; }

ADDRS dynInstruction::getBrTarget () { return _brTarget; }

BR_TYPE dynInstruction::getBrType () { return _brType; }

bool dynInstruction::isBrTaken () { return _brTaken; }

PIPE_STAGE dynInstruction::getPipeStage () { return _insStage; }

SQ_STATE dynInstruction::getSQstate () {Assert (getMemType () == STORE); return _sq_state; }

LQ_STATE dynInstruction::getLQstate () {Assert (getMemType () == LOAD); return _lq_state; }

bool dynInstruction::isOnWrongPath () {return _is_on_wrong_path;}

WIDTH dynInstruction::getNumRdAR () { return _a_rdReg_waitList.NumElements(); }

WIDTH dynInstruction::getNumRdPR () { return _p_rdReg_waitList.NumElements(); }

List<AR>* dynInstruction::getARrdList () {return &_a_rdReg_waitList;}

List<AR>* dynInstruction::getARwrList () {return &_a_wrReg;}

List<PR>* dynInstruction::getPRrdList () {return &_p_rdReg_waitList;}

List<PR>* dynInstruction::getPRwrList () {return &_p_wrReg;}

// ***********************
// ** INS CONTROL       **
// ***********************

/* COPY REGISTERS FROM ONE OBJ TO ANOTHER */
void dynInstruction::copyRegsTo (dynInstruction* ins) {
    for (int i = 0; i < _p_rdReg.NumElements (); i++) {
        PR reg = _p_rdReg.Nth (i);
        ins->setPR (reg, READ);
    }
    for (int i = 0; i < _p_wrReg.NumElements (); i++) {
        PR reg = _p_wrReg.Nth (i);
        ins->setPR (reg, WRITE);
    }
    for (int i = 0; i < _a_rdReg.NumElements (); i++) {
        AR reg = _a_rdReg.Nth (i);
        ins->setAR (reg, READ);
    }
    for (int i = 0; i < _a_wrReg.NumElements (); i++) {
        AR reg = _a_wrReg.Nth (i);
        ins->setAR (reg, WRITE);
    }
}

/* Used to re-run ins after squash recovery */
void dynInstruction::resetStates () {
    while (_p_rdReg_waitList.NumElements () > 0) {
        _p_rdReg_waitList.RemoveAt(0);
    }
    while (_p_rdReg.NumElements () > 0) {
        _p_rdReg.RemoveAt(0);
    }
    while (_p_wrReg.NumElements () > 0) {
        _p_wrReg.RemoveAt(0);
    }
  //for (int i = 0; i < _p_rdReg.NumElements(); i++) {
  //    _p_rdReg_waitList.Append (_p_rdReg.Nth(i));
  //}

    while (_a_rdReg_waitList.NumElements () > 0) {
        _a_rdReg_waitList.RemoveAt(0);
    }
    for (int i = 0; i < _a_rdReg.NumElements(); i++) {
        _a_rdReg_waitList.Append (_a_rdReg.Nth(i));
    }

    _insStage = NO_STAGE;
    _lq_state = LQ_NO_STATE;
    _sq_state = SQ_NO_STATE;
}
