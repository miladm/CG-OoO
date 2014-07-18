/*******************************************************************************
 * memlog.h
 *******************************************************************************/

#ifndef _MEMLOG_H
#define _MEMLOG_H

#include <map>
#include "pin.H"
#include "pin_isa.H"

class memory_buffer {
public:
	memory_buffer() {}
	void save( ADDRINT addr, unsigned len, unsigned char *buf );
	void recover();

private:
	map<ADDRINT,unsigned char> m_buf;

};

#endif
