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
      _rf_end_num (rf_begin_num + rf_size - 1),
      _wr_port (wr_port_cnt, WRITE, clk, rf_name + ".wr_wire"),
      _rd_port (rd_port_cnt, READ,  clk, rf_name + ".rd_wire")
{
    Assert (_rf_size > 0);
    Assert (rd_port_cnt == RD_TO_WR_WIRE_CNT_RATIO * wr_port_cnt && 
            "Must have twice as many read ports than write ports.");
    for (PR i = _rf_begin_num; i < _rf_end_num; i++) {
        registerElement* reg = new registerElement (i);
        _RF.insert (pair<PR, registerElement*> (i, reg));
    }
    _cycle = START_CYCLE;
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
bool registerFile::isRegValidAndReady (PR reg) {
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

bool registerFile::hasFreeWire (AXES_TYPE axes_type) {
    if (axes_type == READ)
        return _rd_port.hasFreeWire ();
    else
        return _wr_port.hasFreeWire ();
}

void registerFile::updateWireState (AXES_TYPE axes_type) {
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

WIDTH registerFile::getNumFreeWires (AXES_TYPE axes_type) {
    if (axes_type == READ)
        return _rd_port.getNumFreeWires ();
    else
        return _wr_port.getNumFreeWires ();
}
