/*******************************************************************************
 * registerFile.cpp
 ******************************************************************************/

#include "registerFile.h"

registerFile::registerFile (PR rf_begin_num, 
                            PR rf_size, 
                            WIDTH rd_port_cnt, 
                            WIDTH wr_port_cnt,
                            sysClock* clk,
                            string rf_name)
    : unit (rf_name, clk),
      _rf_size (rf_size),
      _rf_begin_num (rf_begin_num),
      _rf_end_num (rf_begin_num+rf_size-1),
      _wr_port_cnt (wr_port_cnt),
      _rd_port_cnt (rd_port_cnt)
{
    Assert (_rf_size > 0);
    for (PR i = _rf_begin_num; i < _rf_end_num; i++) {
        registerElement* reg = new registerElement (i);
        _RF.insert (pair<PR, registerElement*> (i, reg));
    }
    _cycle = START_CYCLE;
    _num_free_wr_port = _wr_port_cnt;
    _num_free_rd_port = _rd_port_cnt;
}

registerFile::~registerFile () {
    map <PR, registerElement*>::iterator it;
    for (it = _RF.begin (); it != _RF.end (); it++) {
        delete it->second;
    }
}

/* REG UPDATE WHEN DATA IS AVAILAABLE TO WRITE TO IT */
void registerFile::updateReg (PR reg) {
#ifdef ASSERTION
    Assert (reg >= _rf_begin_num && reg <= _rf_end_num);
    Assert (_RF[reg]->_reg_state == WAIT_ON_WRITE_REG);
#endif
    _RF[reg]->_reg_state = DONE_WRITE_REG;
}

/* REG RESERVE WHEN THE OPERATION IS ISSUED FOR EXECUTION */
void registerFile::reserveReg (PR reg) {
#ifdef ASSERTION
    Assert (reg >= _rf_begin_num && reg <= _rf_end_num);
    Assert (_RF[reg]->_reg_state == DONE_WRITE_REG || _RF[reg]->_reg_state == NO_VAL_REG);
#endif
    _RF[reg]->_reg_state = WAIT_ON_WRITE_REG;
}

/* FIND IF REG DATA IS AVAILABLE FOR READ */
bool registerFile::isRegValid (PR reg) {
#ifdef ASSERTION
    Assert (reg >= _rf_begin_num && reg <= _rf_end_num);
#endif
    if (_RF[reg]->_axesedBefore == true) {
#ifdef ASSERTION
        Assert (_RF[reg]->_reg_state != NO_VAL_REG && 
                "Register must hold data to read from - call updateReg () first");
#endif
    } else {
        _RF[reg]->_axesedBefore = true;
        if (_RF[reg]->_reg_state == NO_VAL_REG)
            _RF[reg]->_reg_state = DONE_WRITE_REG; /// NOTE: only works for INO
    }
    return _RF[reg]->_reg_state == DONE_WRITE_REG ? true : false;
}

/* FIND IF REG IS AVAILABLE FOR WRITE */
bool registerFile::isRegBusy (PR reg) {
#ifdef ASSERTION
	Assert (reg >= _rf_begin_num && reg <= _rf_end_num);
#endif

    return (_RF[reg]->_reg_state == WAIT_ON_WRITE_REG ? true : false);
}

void registerFile::resetRF () {
    map<PR, registerElement*>::iterator it;
    for (it = _RF.begin(); it != _RF.end(); it++) {
        (*it).second->resetReg ();
    }
}

bool registerFile::hasFreeRdPort (CYCLE now, WIDTH numRegRdPorts) {
    Assert (_rd_port_cnt >= numRegRdPorts);
    if (_cycle < now) {
        _num_free_rd_port = _rd_port_cnt - numRegRdPorts;
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

bool registerFile::hasFreeWrPort (CYCLE now) {
    if (_cycle < now) {
        _num_free_wr_port = _wr_port_cnt;
        _cycle = now;
        return true;
    } else if (_cycle == now) {
        _num_free_wr_port--;
        if (_num_free_wr_port > 0) {
            return true;
        } else {
            //Assert (true == false && "this feature may not work -look into it");
            return false;
        }
    }
    Assert (true == false && "should not have gotten here");
    return false;
}
