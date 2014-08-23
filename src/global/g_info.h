/*******************************************************************************
 * g_info.h
 *******************************************************************************/

#ifdef G_I_INFO_EN

#include "pin.H"
#include "pin_isa.H"

struct i_info {
    i_info() { v=false; }
    i_info( ADDRINT p, OPCODE o, string d, bool c, bool r, bool f ) { v=true; pc=p; opcode=o; diss=d; is_call=c; is_ret=r; has_ft=f; }

	unsigned v;
	ADDRINT pc;
    OPCODE opcode;
	string diss;
	bool is_call;
	bool is_ret;
	bool has_ft;
};

#endif


