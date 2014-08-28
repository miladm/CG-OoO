/*******************************************************************************
 * dynBasicblock.cpp
 *******************************************************************************/

#include "dynBasicblock.h"

dynBasicblock::dynBasicblock (string class_name)
    : unit (class_name),
      _head (10),
      _max_bb_size (100) //TODO configurable and small
{ 
    _num_completed_ins = 0;
    _bb_on_wrong_path = false;
    _bb_has_mem_violation = false;
    _head_ins_seq_num = 0;
}

dynBasicblock::~dynBasicblock () {}

// ***********************
// ** SET INS ATRIBUTES **
// ***********************
void dynBasicblock::setBBbrAddr (bool is_tail_br, ADDRS bb_br_ins_addr) {
    _head._bb_br_ins_addr = bb_br_ins_addr;
    _head._bb_has_br = is_tail_br;
}

void dynBasicblock::setBBID (BB_ID bb_seq_num) { _head._bb_seq_num = bb_seq_num; }

void dynBasicblock::setBBheadID () {
    if (_schedInsList.NumElements () == 1) {
        Assert (_head_ins_seq_num == 0);
        _head_ins_seq_num = _schedInsList.Nth(0)->getInsID ();
    }
}

bool dynBasicblock::insertIns (bbInstruction* ins) {
    ADDRS ins_addr = ins->getInsAddr ();
    Assert (_bbInsMap.find (ins_addr) == _bbInsMap.end ());
    //_bbInsMap.insert (pair<ADDRS, bbInstruction*> (ins_addr, ins)); // TODO - remove the hack lines below nad put this line back
    _schedInsList.Append (ins);
    _schedInsList_waitList.Append (ins);
    setBBheadID ();
    if (setupAR (ins)) return true;
    else return false;
}

void dynBasicblock::setGPR (AR a_reg, PR p_reg, AXES_TYPE axes_type) {
    if (axes_type == READ) {
        Assert (_head._a2p_rd_g_reg.find (a_reg) == _head._a2p_rd_g_reg.end ());
        _head._a2p_rd_g_reg.insert (pair<AR, PR> (a_reg, p_reg));
    } else {
        Assert (_head._a2p_wr_g_reg.find (a_reg) == _head._a2p_wr_g_reg.end ());
        _head._a2p_wr_g_reg.insert (pair<AR, PR> (a_reg, p_reg));
    }
}

void dynBasicblock::buildInsSchedule () {
    //TODO add code here and update insertIns function
}

void dynBasicblock::incCompletedInsCntr () {
    _num_completed_ins++;
    Assert (_schedInsList.NumElements () >= _num_completed_ins);
}

bool dynBasicblock::setupAR (bbInstruction* ins) {
    List<AR>* rd_list = ins->getARrdList ();
    List<AR>* wr_list = ins->getARwrList ();
    if (_head.hasAvailReg (wr_list->NumElements(), rd_list->NumElements())) return false;
    for (LENGTH i = 0; i < rd_list->NumElements (); i++) {
        AR reg = rd_list->Nth (i);
        _head._a_rd_g_reg.insert (reg);
    }
    for (LENGTH i = 0; i < wr_list->NumElements (); i++) {
        AR reg = wr_list->Nth (i);
        _head._a_wr_g_reg.insert (reg);
    }
    _head.updateAvailReg (wr_list->NumElements(), rd_list->NumElements());
    return true;
    //TODO add instruction global read/write registers to the bbHead
    //(add placeholder for applying restrictions on the number of allowable operands per BB)
    //this function should be called on insertIns
    //insertIns must check to see if it can accpet the instruction. if it can't it must start a new basicblock (future work)
}

void dynBasicblock::setWrongPath () {
    _bb_on_wrong_path = true;
}

void dynBasicblock::setMemViolation () {
    _bb_has_mem_violation = true;
}

// ***********************
// ** GET INS ATRIBUTES **
// ***********************
bool dynBasicblock::bbHasBr () { return _head._bb_has_br; }

ADDRS dynBasicblock::getBBbrAddr () {
    Assert (_head._bb_has_br == true);
    return _head._bb_br_ins_addr;
}

bbInstruction* dynBasicblock::popFront () {
#ifdef ASSERTION
    Assert (_schedInsList_waitList.NumElements () > 0);
#endif
    bbInstruction* ins = _schedInsList_waitList.Nth (0);
    _schedInsList_waitList.RemoveAt (0);
    return ins;
}

BB_ID dynBasicblock::getBBID () {return _head._bb_seq_num;}

INS_ID dynBasicblock::getBBheadID () {
    Assert (_head_ins_seq_num > 0);
    return _head_ins_seq_num;
}

set<AR>* dynBasicblock::getGARrdList () {return &_head._a_rd_g_reg;} //TODO does this fail?

set<AR>* dynBasicblock::getGARwrList () {return &_head._a_wr_g_reg;}

PR dynBasicblock::getGPR (AR a_reg, AXES_TYPE axes_type) {
    if (axes_type == READ) {
        Assert (_head._a2p_rd_g_reg.find (a_reg) != _head._a2p_rd_g_reg.end ());
        return _head._a2p_rd_g_reg[a_reg];
    } else {
        Assert (_head._a2p_wr_g_reg.find (a_reg) != _head._a2p_wr_g_reg.end ());
        return _head._a2p_wr_g_reg[a_reg];
    }
}

bool dynBasicblock::isMemOrBrViolation () {return (_bb_has_mem_violation || _bb_on_wrong_path);}

List<bbInstruction*>* dynBasicblock::getBBinsList () {return &_schedInsList;}

bool dynBasicblock::isBBcomplete () {
    return (_schedInsList.NumElements () == _num_completed_ins) ? true : false;}

// ***********************
// ** INS CONTROL       **
// ***********************
LENGTH dynBasicblock::getBBsize () {
    return _schedInsList_waitList.NumElements ();
}

bool dynBasicblock::isOnWrongPath () {return _bb_on_wrong_path;}

BUFF_STATE dynBasicblock::getBBstate () {
    LENGTH bb_size = getBBsize ();
    Assert (bb_size <= _max_bb_size);
    if (bb_size == _max_bb_size) return FULL_BUFF;
    else if (bb_size == 0) return EMPTY_BUFF;
    else return AVAILABLE_BUFF;
}

void dynBasicblock::squash () {
    //here make use of reset and do more (on indiv insturctions?)
}

void dynBasicblock::reset () {
    //restore all states nad instructino queue order
}

void dynBasicblock::commit () {
    //delete all instructions in this basicblock.
    //what happens to un-done store instructions? special handling for them
}

void dynBasicblock::resetStates () {
    for (int i = _schedInsList.NumElements () - 1; i >= 0; i--) {
        bbInstruction* ins = _schedInsList.Nth (i);
        ins->resetStates ();
        ins->resetWrongPath ();
    }
    _bb_has_mem_violation = false;
    _bb_on_wrong_path = false;
}
