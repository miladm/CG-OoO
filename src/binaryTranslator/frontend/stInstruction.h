/*******************************************************************************
 * stInstruction.h
 ******************************************************************************/
#ifndef _STINSTRUCTION_H
#define _STINSTRUCTION_H

#include "list.h"
#include <string.h>
#include <string>

typedef enum {NO_REG, REG_READ, REG_WRITE} TYPE;
typedef long long unsigned int ADDRS;

class stInstruction {
    public: 
        stInstruction (FILE*);
        ~stInstruction () {}
        void dump ();

        char _type;
        string _mnemonic;
        string _disassemble;
        ADDRS _addr;
        ADDRS _fall_through;
        ADDRS _destination;
        bool _has_fall_through;
        bool _has_destination;
        List<string> _regs;
        List<TYPE> _reg_types;
        int _mem_w_size;
        int _mem_r_size;
        FILE* _trace_static;
};

#endif
