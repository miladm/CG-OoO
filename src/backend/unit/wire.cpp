/*******************************************************************************
 * wire.cpp
 *******************************************************************************/

#include "wire.h"

wire::wire (WIDTH wire_cnt, sysClock& clk, AXES_TYPE axes_type, string wire_name) 
	: unit (wire_name),
      _axes_type (axes_type),
      _wire_cnt (wire_cnt)
{
    _cycle = clk.now ();
	Assert (_wire_cnt > 0 && _wire_delay > 0);
}

wire::~wire () {}

bool wire::hasFreeWire (sysClock& clk) {
    CYCLE now = clk.now ();
    if (_cycle < now) {
        return true;
    } else if (_cycle == now) {
        return (_num_free_rd_port > 0) ? true : false;
    }
    Assert (true == false && "should not have gotten here");
    return false;
}

void wire::updateWireState (sysClock& clk) {
    CYCLE now = clk.now ();
    if (_cycle < now) {
        _num_free_rd_port = _rd_port_cnt;
        _num_free_wr_port = _wr_port_cnt;
        _cycle = now;
    } else if (_cycle == now) {
        table<tableType_T>::_num_free_rd_port--;
    }
}
