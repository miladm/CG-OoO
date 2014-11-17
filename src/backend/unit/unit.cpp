/*******************************************************************************
 * unit.cpp
 *******************************************************************************/

#include "unit.h"

unit::unit (string class_name, sysClock* clk, PJ cam_epa, PJ ram_epa) 
    : energy (class_name, cam_epa, ram_epa),
      _c_name (class_name),
      _clk (clk)
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
