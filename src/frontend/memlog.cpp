/*******************************************************************************
 * memlog.cpp
 *******************************************************************************/

#include "memlog.h"

void memory_buffer::save( ADDRINT addr, unsigned len, unsigned char *buf )
{
	for( unsigned i=0; i<len; i++ ) {
		ADDRINT eaddr = addr+i;
		if( m_buf.find(eaddr) == m_buf.end() ) // save only the first "original" value
			m_buf[eaddr]=buf[i];
	}
}

void memory_buffer::recover()
{
	map<ADDRINT,unsigned char>::iterator i;
	for( i=m_buf.begin(); i != m_buf.end(); i++ ) {
		ADDRINT eaddr = i->first;
		unsigned char value = i->second;
		PIN_SafeCopy((ADDRINT*)(eaddr), &value, sizeof(unsigned char));
	}	
	//cout << "m_buf: " << m_buf.size() << endl;
	m_buf.clear();
}
