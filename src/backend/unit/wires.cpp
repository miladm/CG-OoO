/*******************************************************************************
 * wires.cpp
 *******************************************************************************/

#include "wires.h"

wires::wires (WIDTH wire_cnt, AXES_TYPE axes_type, sysClock* clk, string wire_name) 
	: unit (wire_name, clk),
      _axes_type (axes_type),
      _wire_cnt (wire_cnt)
{
    _cycle = _clk->now ();
	Assert (_wire_cnt > 0);
}

wires::~wires () {}

bool wires::hasFreeWire () {
    CYCLE now = _clk->now ();
    if (_cycle < now) {
        return true;
    } else if (_cycle == now) {
        return (_num_free_wire > 0) ? true : false;
    }
    Assert (true == false && "should not have gotten here");
    return false;
}

void wires::updateWireState () {
    CYCLE now = _clk->now ();
    if (_cycle < now) {
        _num_free_wire = _wire_cnt;
        _cycle = now;
    } else if (_cycle == now) {
        _num_free_wire--;
    }
}

WIDTH wires::getNumFreeWire () {
    CYCLE now = _clk->now ();
    if (_cycle < now) {
        return _wire_cnt;
    } else if (_cycle == now) {
        return _num_free_wire;
    }
    Assert (true == false && "should not have gotten here");
    return _wire_cnt;
}
