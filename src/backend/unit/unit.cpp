/*******************************************************************************
 * unit.cpp
 *******************************************************************************/

#include "unit.h"

unit::unit (string class_name, sysClock* clk, PJ energy_per_access) 
    : _c_name (class_name),
      _clk (clk),
      s_energy (g_stats.newEnergyStat (energy_per_access, class_name, "energy", "Energy consumped (pJ)", 0, NO_PRINT_ZERO))
{ }

unit::~unit () {}

/*
unit::unit() {
		_read_port_cnt = 1;
		_write_port_cnt = 1;
		_num_rows = 0;
		_num_columns = 0;
		_row_size = 0;
		_unit_type = RAM_ARRAY;
		_latency = 0;
}

unit::unit(int read_port_cnt,
		   int write_port_cnt,
		   int num_rows,
		   int num_columns,
		   int row_size,
		   CYCLE latency,
		   unitType unit_type) {
		_read_port_cnt = read_port_cnt;
		_write_port_cnt = write_port_cnt;
		_num_rows = num_rows;
		_num_columns = num_columns;
		_row_size = row_size;
		_latency = latency;
		_unit_type = unit_type;
}

unit::~unit() {}
*/
