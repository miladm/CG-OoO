/*******************************************************************************
 * wires.cpp
 *******************************************************************************/

#include "wires.h"

wires::wires (AXES_TYPE axes_type, sysClock* clk, const YAML::Node& root, string wire_name) 
	: unit (wire_name, clk),
      _axes_type (axes_type),
      _wire_len (10),
      _e_wire (wire_name, root),
      s_unavailable_cnt (g_stats.newScalarStat (wire_name, "unavailable_cnt", "Number of unavailable wire accesses", 0, NO_PRINT_ZERO))
{
    if (_axes_type == READ) {
        root["rd_wire_cnt"] >> _wire_cnt;
    } else if (_axes_type == WRITE) {
        root["wr_wire_cnt"] >> _wire_cnt;
    } else {
        Assert (0 && "Invalid wire type");
    }
    Assert (_wire_cnt > 0);

    _cycle = _clk->now ();
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
        _num_free_wire = _wire_cnt - 1; /*-- 1 for having used the wire once already --*/
        _cycle = now;
        Assert (_num_free_wire >= 0);
    } else if (_cycle == now) {
        _num_free_wire--;
    }
    _e_wire.wireAccess ();
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
