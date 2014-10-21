/*******************************************************************************
 * stInstruction.cpp
 ******************************************************************************/

#include "stInstruction.h"

stInstruction::stInstruction (FILE* trace_static) {
    _has_fall_through = false;
    _has_destination = false;
    _trace_static = trace_static;
    _mem_w_size = -1;
    _mem_r_size = -1;
}

void stInstruction::dump () {
    fprintf(_trace_static, "%c\n", _type);
    if (_type == 'R') fprintf(_trace_static, "%d\n", _mem_r_size); //SIZE IN BYTES
    else if (_type == 'W') fprintf(_trace_static, "%d\n", _mem_w_size); //SIZE IN BYTES
    fprintf(_trace_static, "#%s\n", _mnemonic.c_str());
    fprintf(_trace_static, "%s\n", _disassemble.c_str());
    fprintf(_trace_static, "%lx\n", _addr);

    if (_has_fall_through) fprintf(_trace_static, "1\n%lx\n", _fall_through);
    else fprintf(_trace_static, "0\n");
    if (_has_destination) fprintf(_trace_static, "1\n%lx\n", _destination);
    else fprintf(_trace_static, "0\n");

    // UINT32 operandCount = INS_MaxNumRRegs(ins)+INS_MaxNumWRegs(ins);
    Assert (_regs.NumElements () == _reg_types.NumElements ());
    for (int i = 0; i < _regs.NumElements (); i++) {
        fprintf(_trace_static, "%s\n%d\n", _regs.Nth (i).c_str (), (int)_reg_types.Nth (i));
    }
    fprintf(_trace_static, "---\n");
}
