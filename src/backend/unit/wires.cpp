/*******************************************************************************
 * wires.cpp
 *******************************************************************************/

#include "wires.h"

wires::wires (WIDTH wire_cnt, AXES_TYPE axes_type, sysClock* clk, string wire_name) 
	: unit (wire_name, clk),
      _axes_type (axes_type),
      _wire_cnt (wire_cnt),
      _wire_len (10),
      s_unavailable_cnt (g_stats.newScalarStat (wire_name, "unavailable_cnt", "Number of unavailable wire accesses", 0, NO_PRINT_ZERO))
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
        bool result = (_num_free_wire > 0) ? true : false;
        if (!result) s_unavailable_cnt++;
        return result;
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

WIDTH wires::getNumFreeWires () {
    CYCLE now = _clk->now ();
    if (_cycle < now) {
        return _wire_cnt;
    } else if (_cycle == now) {
        return _num_free_wire;
    }
    Assert (true == false && "should not have gotten here");
    return _wire_cnt;
}
