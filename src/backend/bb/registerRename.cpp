/*********************************************************************************
 *  registerRename.cpp
 ********************************************************************************/

#include "registerRename.h"

bb_registerRename::bb_registerRename (sysClock* clk, WIDTH blk_cnt, const YAML::Node& root, string rf_name)
    : unit (rf_name, clk), 
      _wr_port (WRITE, clk, root, rf_name + ".wr_wire"),
      _rd_port (READ,  clk, root, rf_name + ".rd_wire"),
      _blk_cnt (blk_cnt)
{
    _cycle = START_CYCLE;

    /* SETUP REGISTER DIMENSIONS */
    root["a_size"] >> _a_rf_size;
    root["a_lo"] >> _a_rf_lo;
    root["a_hi"] >> _a_rf_hi;
    root["p_size"] >> _p_rf_size;
    root["p_lo"] >> _p_rf_lo;
    root["p_hi"] >> _p_rf_hi;
    root["r_size"] >> _r_rf_size;
    root["r_lo"] >> _r_rf_lo;
    root["r_hi"] >> _r_rf_hi;

    _a_rf_hi++;
    _p_rf_hi++;
    _r_rf_hi++;

    /*-- SETUP THE AVAILABLE PR SET LIST --*/
    root["segmnt_cnt"] >> _grf_segmnt_cnt;
    Assert (_grf_segmnt_cnt > 0 && 
            _grf_segmnt_cnt <= _blk_cnt && 
            _blk_cnt % _grf_segmnt_cnt == 0);
    _num_blk_per_segmnt = _blk_cnt / _grf_segmnt_cnt;
    Assert (_num_blk_per_segmnt > 0);
    _availablePRset = new List<vector<bb_regElem*>*>;
    for (int i = 0; i < _grf_segmnt_cnt; i++) {
        vector<bb_regElem*>* elem = new vector<bb_regElem*>;
        _availablePRset->Append (elem);
    }

    /*-- INITIALIZE ARCHITECTURAL REGISTER DOMAIN --*/
    Assert (_a_rf_size % _grf_segmnt_cnt == 0);
    Assert (_p_rf_size % _grf_segmnt_cnt == 0);
    _a_rf_segmnt_size = _a_rf_size / _grf_segmnt_cnt;
    _p_rf_segmnt_size = _p_rf_size / _grf_segmnt_cnt;
    Assert (_a_rf_segmnt_size < _p_rf_segmnt_size);
    PR PR_counter = 0;
    for (WIDTH i = 0; i < _grf_segmnt_cnt; i++) {
        AR a_rf_segmnt_lo = _a_rf_lo + i * _a_rf_segmnt_size;
        AR a_rf_segmnt_hi = a_rf_segmnt_lo + _a_rf_segmnt_size;
        PR_counter = _p_rf_lo + i * _p_rf_segmnt_size;
        PR p_rf_segmnt_hi = _p_rf_lo + (i + 1) * _p_rf_segmnt_size;
        cout << PR_counter << " " << p_rf_segmnt_hi << endl;
        Assert (PR_counter < p_rf_segmnt_hi);
        Assert (a_rf_segmnt_hi <= _a_rf_hi);
        for (AR a_reg = a_rf_segmnt_lo; a_reg < a_rf_segmnt_hi; a_reg++) {
            bb_regElem* p_reg = new bb_regElem (PR_counter, ARCH_REG);
            _fRAT.insert (pair<AR, bb_regElem*> (a_reg, p_reg));
            _cRAT.insert (pair<AR, bb_regElem*> (a_reg, p_reg));
            _RF.insert (pair<PR, bb_regElem*> (PR_counter, p_reg));
            PR_counter++;
        }
    }
    cout << "-" << endl;

    /*-- INITIALIZE RENAME REGISTER DOMAIN --*/
    for (WIDTH i = 0; i < _grf_segmnt_cnt; i++) {
        PR p_rf_segmnt_hi = _p_rf_lo + (i + 1) * _p_rf_segmnt_size;
        PR_counter = _p_rf_lo + i * _p_rf_segmnt_size + _a_rf_segmnt_size;
        Assert (PR_counter < p_rf_segmnt_hi);
        cout << PR_counter << " " << p_rf_segmnt_hi << endl;
        while (PR_counter < p_rf_segmnt_hi) {
            bb_regElem* p_reg = new bb_regElem (PR_counter, AVAILABLE);
            _availablePRset->Nth(i)->push_back (p_reg);
            _RF.insert (pair<PR, bb_regElem*> (PR_counter, p_reg));
            PR_counter++;
        }
    }

    for (int i = 0; i < _grf_segmnt_cnt; i++) {
        cout << _grf_segmnt_cnt << " " << _availablePRset->Nth(i)->size () << " " << _r_rf_size << " " << _r_rf_size / _grf_segmnt_cnt << endl;
        Assert (_availablePRset->Nth(i)->size () == (_r_rf_size / _grf_segmnt_cnt) && 
                "Invalid Rename table structure initialization.");
    }
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
    PR p_reg = _fRAT[a_reg]->_reg;
    Assert (p_reg >= _p_rf_lo && p_reg <= _p_rf_hi);
	return p_reg;
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

bool bb_registerRename::isAnyPRavailable (BB_ID blk_indx) {
    WIDTH grf_segmnt_indx = blkIndx2APRindx (blk_indx);
    return (_availablePRset->Nth(grf_segmnt_indx)->size () == 0) ? false : true;
}

PR bb_registerRename::getAvailablePR (BB_ID blk_indx) {
    WIDTH grf_segmnt_indx = blkIndx2APRindx (blk_indx);
	Assert (_availablePRset->Nth(grf_segmnt_indx)->size () > 0 && "Invalid use-set size.");
	bb_regElem* p_reg = _availablePRset->Nth(grf_segmnt_indx)->back ();
	Assert (p_reg->_reg_state == AVAILABLE && "Register State is Invalid - Register must be in Available State");
	_availablePRset->Nth(grf_segmnt_indx)->pop_back ();
    PR pr = p_reg->_reg;
    Assert (pr >= _p_rf_lo && pr <= _p_rf_hi);
	return pr;
}

void bb_registerRename::setAsAvailablePR (PR p_reg) {
	Assert (_RF.find (p_reg) != _RF.end () && "Physical reg not found!");
    WIDTH apr_indx = PR2APRindx (p_reg);
	_availablePRset->Nth(apr_indx)->push_back (_RF[p_reg]);
	_RF[p_reg]->_prev_pr = NULL;
	Assert (_availablePRset->Nth(apr_indx)->size () <= _r_rf_size && 
            "Rename table has grown too large (size violation).");
}

int bb_registerRename::getNumAvailablePR (BB_ID blk_indx) {
    WIDTH grf_segmnt_indx = blkIndx2APRindx (blk_indx);
	return _availablePRset->Nth(grf_segmnt_indx)->size ();
}

PR bb_registerRename::getPrevPR (PR p_reg) {
	Assert (_RF.find (p_reg) != _RF.end () && "Reg not found in ARST!");
	Assert (_RF[p_reg]->_prev_pr != NULL && "The previous PR not found.");
	return _RF[p_reg]->_prev_pr->_reg;
}

void bb_registerRename::squashRenameReg () {
	_fRAT.clear ();
    for (int i = 0; i < _grf_segmnt_cnt; i++) {
	    _availablePRset->Nth(i)->clear ();
    }
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
        WIDTH apr_indx = PR2APRindx (p_reg);
        _availablePRset->Nth(apr_indx)->push_back (_RF[p_reg]);
    }

//    for (int i = 0; i < _grf_segmnt_cnt; i++) {
//        cout << _grf_segmnt_cnt << " " << _availablePRset->Nth(i)->size () << " " << _r_rf_size << " " << _r_rf_size / _grf_segmnt_cnt << endl;
//        Assert (_availablePRset->Nth(i)->size () <= (_r_rf_size / _grf_segmnt_cnt) && 
//                "Rename table has grown too large (size violation).");
//    }
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
    if (!(p_reg >= _p_rf_lo && p_reg <= _p_rf_hi)) {cout << "shitty reg valu: " << p_reg << endl;}
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

WIDTH bb_registerRename::PR2APRindx (PR pr) {
#ifdef ASSERTION
    Assert (_p_rf_segmnt_size > 0);
#endif
    WIDTH grf_segmnt_indx = (pr - 1) / _p_rf_segmnt_size;
    return grf_segmnt_indx;
}

WIDTH bb_registerRename::blkIndx2APRindx (BB_ID blk_indx) {
    WIDTH  grf_segmnt_indx = ((WIDTH)blk_indx) / _num_blk_per_segmnt;
#ifdef ASSERTION
    Assert ((WIDTH)blk_indx < _blk_cnt);
    Assert (grf_segmnt_indx < _grf_segmnt_cnt);
#endif
    return grf_segmnt_indx;
}
