/*********************************************************************************
 *  registerRename.cpp
 ********************************************************************************/

#include "registerRename.h"

bb_registerRename::bb_registerRename (sysClock* clk, string rf_name)
    : unit (rf_name, clk), 
      _a_rf_size (GARF_HI - GARF_LO + 1), 
      _a_rf_hi (GARF_HI), 
      _a_rf_lo (GARF_LO), 
      _p_rf_size (GPRF_HI - GPRF_LO + 1), 
      _p_rf_hi (GPRF_HI), 
      _p_rf_lo (GPRF_LO), 
      _wr_port (4, WRITE, clk, rf_name + ".wr_wire"),
      _rd_port (8, READ,  clk, rf_name + ".rd_wire")
{
    _cycle = START_CYCLE;

    /*-- INITIALIZE ALL TABLES --*/
    PR PR_counter = _p_rf_lo;

    /*-- INITIALIZE ARCHITECTURAL REGISTER DOMAIN --*/
    for (AR a_reg = _a_rf_lo; a_reg <= _a_rf_hi; a_reg++) {
        bb_regElem* p_reg = new bb_regElem (PR_counter, ARCH_REG);
        _fRAT.insert (pair<AR, bb_regElem*> (a_reg, p_reg));
        _cRAT.insert (pair<AR, bb_regElem*> (a_reg, p_reg));
        _RF.insert (pair<PR, bb_regElem*> (PR_counter, p_reg));
        PR_counter++;
    }

    /*-- INITIALIZE RENAME REGISTER DOMAIN --*/
    while (PR_counter <= _p_rf_hi) {
        bb_regElem* p_reg = new bb_regElem (PR_counter, AVAILABLE);
        _availablePRset.push_back (p_reg);
        _RF.insert (pair<PR, bb_regElem*> (PR_counter, p_reg));
        PR_counter++;
    }

    Assert (_availablePRset.size () == GRRF_SIZE && "Invalid Rename table structure initialization.");
    Assert (_RF.size () == GPRF_SIZE && "Invalid Rename table structure initialization.");
    Assert (_fRAT.size () == GARF_SIZE && "Invalid Rename table structure initialization.");
    Assert (_cRAT.size () == GARF_SIZE && "Invalid Rename table structure initialization.");
}

bb_registerRename::bb_registerRename (AR a_rf_lo, 
                                     AR a_rf_hi, 
                                     WIDTH rd_port_cnt, 
                                     WIDTH wr_port_cnt, 
                                     sysClock* clk,
                                     string rf_name) 
    : unit (rf_name, clk),
      _a_rf_size (a_rf_hi - a_rf_lo + 1), 
      _a_rf_hi (a_rf_hi), 
      _a_rf_lo (a_rf_lo), 
      _p_rf_size (GPRF_HI - GPRF_LO + 1), 
      _p_rf_hi (GPRF_HI), 
      _p_rf_lo (GPRF_LO), 
      _wr_port (wr_port_cnt, WRITE, clk, rf_name + ".wr_wire"),
      _rd_port (rd_port_cnt, READ,  clk, rf_name + ".rd_wire")
{
    _cycle = START_CYCLE;

	/*-- INITIALIZE ALL TABLES --*/
	PR PR_counter = _p_rf_lo;

	/*-- INITIALIZE ARCHITECTURAL REGISTER DOMAIN --*/
	for (AR a_reg = _a_rf_lo; a_reg <= _a_rf_hi; a_reg++) {
        bb_regElem* p_reg = new bb_regElem (PR_counter, ARCH_REG);
		_fRAT.insert (pair<AR, bb_regElem*> (a_reg, p_reg));
		_cRAT.insert (pair<AR, bb_regElem*> (a_reg, p_reg));
        _RF.insert (pair<PR, bb_regElem*> (PR_counter, p_reg));
		PR_counter++;
	}

	/*-- INITIALIZE RENAME REGISTER DOMAIN --*/
	while (PR_counter <= _p_rf_hi) {
        bb_regElem* p_reg = new bb_regElem (PR_counter, AVAILABLE);
		_availablePRset.push_back (p_reg);
        _RF.insert (pair<PR, bb_regElem*> (PR_counter, p_reg));
		PR_counter++;
	}

    Assert (rd_port_cnt == RD_TO_WR_WIRE_CNT_RATIO * wr_port_cnt && "Must have twice as many read ports than write ports.");
	Assert (_availablePRset.size () == GRRF_SIZE && "Invalid Rename table structure initialization.");
	Assert (_RF.size () == _p_rf_size && "Invalid Rename table structure initialization.");
	Assert (_fRAT.size () == _a_rf_size && "Invalid Rename table structure initialization.");
	Assert (_cRAT.size () == _a_rf_size && "Invalid Rename table structure initialization.");
}

bb_registerRename::~bb_registerRename () {
    map <PR, bb_regElem*>::iterator it;
    for (it = _RF.begin (); it != _RF.end (); it++) {
        delete it->second;
    }
}

PR bb_registerRename::renameReg (AR a_reg) {
	Assert (_fRAT.find (a_reg) != _fRAT.end () && "The arch. reg was not found in Fetch-RAT!");
	return _fRAT[a_reg]->_reg;
}

void bb_registerRename::update_fRAT (AR a_reg, PR p_reg) {
	Assert (_RF.find (p_reg) != _RF.end ());
	Assert (_RF[p_reg]->_reg_state == AVAILABLE);
	if (_fRAT.find (a_reg) != _fRAT.end ()) {
		_fRAT.erase (a_reg);
	}
	_fRAT.insert (pair<AR, bb_regElem*> (a_reg, _RF[p_reg]));
}

void bb_registerRename::update_cRAT (AR a_reg, PR p_reg) {
	Assert (_cRAT.find (a_reg) != _cRAT.end ());
	Assert (_cRAT[a_reg]->_reg_state == AVAILABLE);
	Assert (_RF.find (p_reg) != _RF.end ());
	Assert (_RF[p_reg]->_reg_state == ARCH_REG);

	_cRAT.erase (a_reg);
	_cRAT.insert (pair<AR, bb_regElem*> (a_reg, _RF[p_reg]));
}

bool bb_registerRename::isAnyPRavailable () {
    return (_availablePRset.size () == 0) ? false : true;
}

PR bb_registerRename::getAvailablePR () {
	Assert (_availablePRset.size () > 0 && "Invalid use-set size.");
	bb_regElem* p_reg = _availablePRset.back ();
	Assert (p_reg->_reg_state == AVAILABLE && "Register State is Invalid - Register must be in Available State");
	_availablePRset.pop_back ();
	return p_reg->_reg;
}

void bb_registerRename::setAsAvailablePR (PR p_reg) {
	Assert (_RF.find (p_reg) != _RF.end () && "Physical reg not found!");
	_availablePRset.push_back (_RF[p_reg]);
	_RF[p_reg]->_prev_pr = NULL;
	Assert (_availablePRset.size () <= GRRF_SIZE && "Rename table has grown too large (size violation).");
}

int bb_registerRename::getNumAvailablePR () {
	return _availablePRset.size ();
}

PR bb_registerRename::getPrevPR (PR p_reg) {
	Assert (_RF.find (p_reg) != _RF.end () && "Reg not found in ARST!");
	Assert (_RF[p_reg]->_prev_pr != NULL && "The previous PR not found.");
	return _RF[p_reg]->_prev_pr->_reg;
}

void bb_registerRename::squashRenameReg () {
	_fRAT.clear ();
	_availablePRset.clear ();
	map<PR, bb_regElem*> rf = _RF;

    map<AR, bb_regElem*>::iterator it;
    for (it = _cRAT.begin (); it != _cRAT.end (); it++) {
        AR a_reg = it->first;
        PR p_reg = it->second->_reg;
        bb_regElem* p_obj = it->second;
        p_obj->_reg_state = ARCH_REG;
        p_obj->_prev_pr = NULL;
        _fRAT.insert (pair<AR, bb_regElem*> (a_reg, p_obj));
        rf.erase (p_reg);
    }

	Assert (_fRAT.size () == _cRAT.size ());
    Assert (rf.size () == _RF.size () - _cRAT.size ());

    map<PR, bb_regElem*>::iterator it_pr;
    for (it_pr = rf.begin (); it_pr != rf.end (); it_pr++) {
        bb_regElem* p_obj = it_pr->second;
        PR p_reg = p_obj->_reg;
        p_obj->_reg_state = AVAILABLE;
        p_obj->_prev_pr = NULL;
        _availablePRset.push_back (_RF[p_reg]);
    }
    Assert (_availablePRset.size () <= GRRF_SIZE && "Rename table has grown too large (size violation).");
}

void bb_registerRename::updatePR (PR new_pr, PR prev_pr, REG_REN_STATE state) {
	Assert (_RF.find (new_pr) != _RF.end () && "Physical reg not found!");
	Assert (_RF.find (prev_pr) != _RF.end () && "Physical reg not found!");
	Assert ((_RF[new_pr]->_reg_state == AVAILABLE && state == RENAMED_INVALID) &&
		     "Phys Reg. State Transition Violation.");
    _RF[new_pr]->_prev_pr = _RF[prev_pr];
	_RF[new_pr]->_reg_state = state;
}

void bb_registerRename::updatePRstate (PR p_reg, REG_REN_STATE state) {
	Assert (_RF.find (p_reg) != _RF.end () && "Physical register value was not found!");
	Assert (((_RF[p_reg]->_reg_state == AVAILABLE && state == RENAMED_INVALID) ||
             (_RF[p_reg]->_reg_state == RENAMED_INVALID && state == RENAMED_VALID) ||
		     (_RF[p_reg]->_reg_state == RENAMED_VALID && state == ARCH_REG) ||
		     (_RF[p_reg]->_reg_state == ARCH_REG && state == AVAILABLE)) &&
		     "Phys Reg. State Transition Violation.");
	_RF[p_reg]->_reg_state = state;
}

void bb_registerRename::squashPRstate (PR p_reg) {
	Assert (_RF.find (p_reg) != _RF.end () && "Physical register was not found!");
	Assert (_RF[p_reg]->_reg_state == RENAMED_INVALID || _RF[p_reg]->_reg_state == RENAMED_VALID);
	_RF[p_reg]->_reg_state = AVAILABLE;
}

REG_REN_STATE bb_registerRename::getPRstate (PR p_reg) {
	Assert (_RF.find (p_reg) != _RF.end () && "Physical register was not found!");
	return _RF[p_reg]->_reg_state;
}

/*-- FIND IF REG DATA IS AVAILABLE FOR READ --*/
bool bb_registerRename::isPRvalid (PR p_reg) {
#ifdef ASSERTION
    Assert (p_reg >= _p_rf_lo && p_reg <= _p_rf_hi);
    Assert (_RF[p_reg]->_reg_state != AVAILABLE && 
            "Register must hold data to read from - call updateReg () first");
#endif
    REG_REN_STATE state = _RF[p_reg]->_reg_state;
    return (state == ARCH_REG || state == RENAMED_VALID) ? true : false;
}

bool bb_registerRename::hasFreeWire (AXES_TYPE axes_type) {
    if (axes_type == READ)
        return _rd_port.hasFreeWire ();
    else
        return _wr_port.hasFreeWire ();
}

void bb_registerRename::updateWireState (AXES_TYPE axes_type) {
    CYCLE now = _clk->now ();
    if (_cycle < now) {
        _wr_port.updateWireState ();
        _rd_port.updateWireState ();
        _cycle = now;
    } else if (_cycle == now) {
        if (axes_type == READ)
            _rd_port.updateWireState ();
        else
            _wr_port.updateWireState ();
    }
}

WIDTH bb_registerRename::getNumFreeWires (AXES_TYPE axes_type) {
    if (axes_type == READ)
        return _rd_port.getNumFreeWires ();
    else
        return _wr_port.getNumFreeWires ();
}
