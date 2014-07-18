/*******************************************************************************
 *  registerRename.cpp
 ******************************************************************************/

#include "registerRename.h"

o3_registerRename::o3_registerRename (string rf_name)
    : unit (rf_name), 
      _a_rf_size (GARF_HI - GARF_LO + 1), 
      _a_rf_hi (GARF_HI), 
      _a_rf_lo (GARF_LO), 
      _p_rf_size (GPRF_HI - GPRF_LO + 1), 
      _p_rf_hi (GPRF_HI), 
      _p_rf_lo (GPRF_LO), 
      _wr_port_cnt (4), 
      _rd_port_cnt (8)
{
    _cycle = START_CYCLE;
    _num_free_wr_port = _wr_port_cnt;
    _num_free_rd_port = _rd_port_cnt;

    /* INITIALIZE ALL TABLES */
    PR PR_counter = _p_rf_lo;

    /* INITIALIZE ARCHITECTURAL REGISTER DOMAIN */
    for (AR a_reg = _a_rf_lo; a_reg <= _a_rf_hi; a_reg++) {
        o3_regElem* p_reg = new o3_regElem (PR_counter, ARCH_REG);
        _fRAT.insert (pair<AR, o3_regElem*> (a_reg, p_reg));
        _cRAT.insert (pair<AR, o3_regElem*> (a_reg, p_reg));
        _RF.insert (pair<PR, o3_regElem*> (PR_counter, p_reg));
        PR_counter++;
    }

    /* INITIALIZE RENAME REGISTER DOMAIN */
    while (PR_counter <= _p_rf_hi) {
        o3_regElem* p_reg = new o3_regElem (PR_counter, AVAILABLE);
        _availablePRset.push_back (p_reg);
        _RF.insert (pair<PR, o3_regElem*> (PR_counter, p_reg));
        PR_counter++;
    }

    Assert (_rd_port_cnt == 2 * _wr_port_cnt && "Must have twice as many read ports than write ports.");
    Assert (_availablePRset.size () == GRRF_SIZE && "Invalid Rename table structure initialization.");
    Assert (_RF.size () == GPRF_SIZE && "Invalid Rename table structure initialization.");
    Assert (_fRAT.size () == GARF_SIZE && "Invalid Rename table structure initialization.");
    Assert (_cRAT.size () == GARF_SIZE && "Invalid Rename table structure initialization.");
}

o3_registerRename::o3_registerRename (AR a_rf_lo, 
                                     AR a_rf_hi, 
                                     WIDTH rd_port_cnt, 
                                     WIDTH wr_port_cnt, 
                                     string rf_name) 
    : unit (rf_name), 
      _a_rf_size (a_rf_hi - a_rf_lo + 1), 
      _a_rf_hi (a_rf_hi), 
      _a_rf_lo (a_rf_lo), 
      _p_rf_size (GPRF_HI - GPRF_LO + 1), 
      _p_rf_hi (GPRF_HI), 
      _p_rf_lo (GPRF_LO), 
      _wr_port_cnt (wr_port_cnt), 
      _rd_port_cnt (rd_port_cnt)
{
    _cycle = START_CYCLE;
    _num_free_wr_port = _wr_port_cnt;
    _num_free_rd_port = _rd_port_cnt;

	/* INITIALIZE ALL TABLES */
	PR PR_counter = _p_rf_lo;

	/* INITIALIZE ARCHITECTURAL REGISTER DOMAIN */
	for (AR a_reg = _a_rf_lo; a_reg <= _a_rf_hi; a_reg++) {
        o3_regElem* p_reg = new o3_regElem (PR_counter, ARCH_REG);
		_fRAT.insert (pair<AR, o3_regElem*> (a_reg, p_reg));
		_cRAT.insert (pair<AR, o3_regElem*> (a_reg, p_reg));
        _RF.insert (pair<PR, o3_regElem*> (PR_counter, p_reg));
		PR_counter++;
	}

	/* INITIALIZE RENAME REGISTER DOMAIN */
	while (PR_counter <= _p_rf_hi) {
        o3_regElem* p_reg = new o3_regElem (PR_counter, AVAILABLE);
		_availablePRset.push_back (p_reg);
        _RF.insert (pair<PR, o3_regElem*> (PR_counter, p_reg));
		PR_counter++;
	}

    Assert (_rd_port_cnt == 2 * _wr_port_cnt && "Must have twice as many read ports than write ports.");
	Assert (_availablePRset.size () == GRRF_SIZE && "Invalid Rename table structure initialization.");
	Assert (_RF.size () == _p_rf_size && "Invalid Rename table structure initialization.");
	Assert (_fRAT.size () == _a_rf_size && "Invalid Rename table structure initialization.");
	Assert (_cRAT.size () == _a_rf_size && "Invalid Rename table structure initialization.");
}

o3_registerRename::~o3_registerRename () {
    map <PR, o3_regElem*>::iterator it;
    for (it = _RF.begin (); it != _RF.end (); it++) {
        delete it->second;
    }
}

PR o3_registerRename::renameReg (AR a_reg) {
	Assert (_fRAT.find (a_reg) != _fRAT.end () && "The arch. reg was not found in Fetch-RAT!");
	return _fRAT[a_reg]->_reg;
}

void o3_registerRename::update_fRAT (AR a_reg, PR p_reg) {
	Assert (_RF.find (p_reg) != _RF.end ());
	Assert (_RF[p_reg]->_reg_state == AVAILABLE);
	if (_fRAT.find (a_reg) != _fRAT.end ()) {
		_fRAT.erase (a_reg);
	}
	_fRAT.insert (pair<AR, o3_regElem*> (a_reg, _RF[p_reg]));
}

void o3_registerRename::update_cRAT (AR a_reg, PR p_reg) {
	Assert (_cRAT.find (a_reg) != _cRAT.end ());
	Assert (_cRAT[a_reg]->_reg_state == AVAILABLE);
	Assert (_RF.find (p_reg) != _RF.end ());
	Assert (_RF[p_reg]->_reg_state == ARCH_REG);

	_cRAT.erase (a_reg);
	_cRAT.insert (pair<AR, o3_regElem*> (a_reg, _RF[p_reg]));
}

bool o3_registerRename::isAnyPRavailable () {
    return (_availablePRset.size () == 0) ? false : true;
}

PR o3_registerRename::getAvailablePR () {
	Assert (_availablePRset.size () > 0 && "Invalid use-set size.");
	o3_regElem* p_reg = _availablePRset.back ();
	Assert (p_reg->_reg_state == AVAILABLE && "Register State is Invalid - Register must be in Available State");
	_availablePRset.pop_back ();
	return p_reg->_reg;
}

void o3_registerRename::setAsAvailablePR (PR p_reg) {
	Assert (_RF.find (p_reg) != _RF.end () && "Physical reg not found!");
	_availablePRset.push_back (_RF[p_reg]);
	_RF[p_reg]->_prev_pr = NULL;
	Assert (_availablePRset.size () <= GRRF_SIZE && "Rename table has grown too large (size violation).");
}

int o3_registerRename::getNumAvailablePR () {
	return _availablePRset.size ();
}

PR o3_registerRename::getPrevPR (PR p_reg) {
	Assert (_RF.find (p_reg) != _RF.end () && "Reg not found in ARST!");
	Assert (_RF[p_reg]->_prev_pr != NULL && "The previous PR not found.");
	return _RF[p_reg]->_prev_pr->_reg;
}

void o3_registerRename::squashRenameReg () {
	_fRAT.clear ();
	_availablePRset.clear ();
	map<PR, o3_regElem*> rf = _RF;

    map<AR, o3_regElem*>::iterator it;
    for (it = _cRAT.begin (); it != _cRAT.end (); it++) {
        AR a_reg = it->first;
        PR p_reg = it->second->_reg;
        o3_regElem* p_obj = it->second;
        p_obj->_reg_state = ARCH_REG;
        p_obj->_prev_pr = NULL;
        _fRAT.insert (pair<AR, o3_regElem*> (a_reg, p_obj));
        rf.erase (p_reg);
    }

	Assert (_fRAT.size () == _cRAT.size ());
    Assert (rf.size () == _RF.size () - _cRAT.size ());

    map<PR, o3_regElem*>::iterator it_pr;
    for (it_pr = rf.begin (); it_pr != rf.end (); it_pr++) {
        o3_regElem* p_obj = it_pr->second;
        PR p_reg = p_obj->_reg;
        p_obj->_reg_state = AVAILABLE;
        p_obj->_prev_pr = NULL;
        _availablePRset.push_back (_RF[p_reg]);
    }
    Assert (_availablePRset.size () <= GRRF_SIZE && "Rename table has grown too large (size violation).");
}

void o3_registerRename::updatePR (PR new_pr, PR prev_pr, REG_REN_STATE state) {
	Assert (_RF.find (new_pr) != _RF.end () && "Physical reg not found!");
	Assert (_RF.find (prev_pr) != _RF.end () && "Physical reg not found!");
	Assert ((_RF[new_pr]->_reg_state == AVAILABLE && state == RENAMED_INVALID) &&
		     "Phys Reg. State Transition Violation.");
    _RF[new_pr]->_prev_pr = _RF[prev_pr];
	_RF[new_pr]->_reg_state = state;
}

void o3_registerRename::updatePRstate (PR p_reg, REG_REN_STATE state) {
	Assert (_RF.find (p_reg) != _RF.end () && "Physical register value was not found!");
	Assert (((_RF[p_reg]->_reg_state == AVAILABLE && state == RENAMED_INVALID) ||
             (_RF[p_reg]->_reg_state == RENAMED_INVALID && state == RENAMED_VALID) ||
		     (_RF[p_reg]->_reg_state == RENAMED_VALID && state == ARCH_REG) ||
		     (_RF[p_reg]->_reg_state == ARCH_REG && state == AVAILABLE)) &&
		     "Phys Reg. State Transition Violation.");
	_RF[p_reg]->_reg_state = state;
}

void o3_registerRename::squashPRstate (PR p_reg) {
	Assert (_RF.find (p_reg) != _RF.end () && "Physical register was not found!");
	Assert (_RF[p_reg]->_reg_state == RENAMED_INVALID || _RF[p_reg]->_reg_state == RENAMED_VALID);
	_RF[p_reg]->_reg_state = AVAILABLE;
}

REG_REN_STATE o3_registerRename::getPRstate (PR p_reg) {
	Assert (_RF.find (p_reg) != _RF.end () && "Physical register was not found!");
	return _RF[p_reg]->_reg_state;
}

/* FIND IF REG DATA IS AVAILABLE FOR READ */
bool o3_registerRename::isPRvalid (PR p_reg) {
#ifdef ASSERTION
    Assert (p_reg >= _p_rf_lo && p_reg <= _p_rf_hi);
    Assert (_RF[p_reg]->_reg_state != AVAILABLE && 
            "Register must hold data to read from - call updateReg () first");
#endif
    REG_REN_STATE state = _RF[p_reg]->_reg_state;
    return (state == ARCH_REG || state == RENAMED_VALID) ? true : false;
}

bool o3_registerRename::hasFreeRdPort (CYCLE now, WIDTH numRegRdPorts) {
    Assert (_rd_port_cnt >= numRegRdPorts);
    if (_cycle < now) {
        _num_free_rd_port = _rd_port_cnt - numRegRdPorts;
        _num_free_wr_port = _wr_port_cnt;
        _cycle = now;
        return true;
    } else if (_cycle == now) {
        _num_free_rd_port -= numRegRdPorts;
        if (_num_free_rd_port >= 0) return true;
        else return false;
    }
    Assert (true == false && "should not have gotten here");
    return false;
}

bool o3_registerRename::hasFreeWrPort (CYCLE now) {
    if (_cycle < now) {
        _num_free_rd_port = _rd_port_cnt;
        _num_free_wr_port = _wr_port_cnt;
        _cycle = now;
        return true;
    } else if (_cycle == now) {
        _num_free_wr_port--;
        if (_num_free_wr_port > 0) {
            return true;
        } else {
            //Assert (true == false && "this feature may not work -look into it"); (TODO)
            return false;
        }
    }
    Assert (true == false && "should not have gotten here");
    return false;
}
