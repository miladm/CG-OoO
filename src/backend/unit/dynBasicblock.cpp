/*******************************************************************************
 * dynBasicblock.cpp
 *******************************************************************************/

#include "dynBasicblock.h"

dynBasicblock::dynBasicblock (SCHED_MODE scheduling_mode, string class_name)
    : unit (class_name),
      _head (10),
      _max_bb_size (100) //TODO configurable and small
{ 
    _num_completed_ins = 0;
    _bb_on_wrong_path = false;
    _bb_has_mem_violation = false;
    _head_ins_seq_num = 0;
    _wasted_ins_cnt = 0;
    _scheduling_mode = scheduling_mode;
    _done_fetch = false;
}

dynBasicblock::~dynBasicblock () {
    map<ADDRS, bbInstruction*>::iterator it = _bbInsMap.begin ();
    while (it != _bbInsMap.end ()) {
        delete it->second;
        _bbInsMap.erase (it++);
    }
}

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
    /* IF NO STATIC SCHEDULE EXISTS, ASSUME DYN SCHEDULING */
    if (_scheduling_mode == DYNAMIC_SCH) {
        _schedInsList.Append (ins);
        _schedInsList_waitList.Append (ins);
        setBBheadID ();
    } else if (_scheduling_mode == STATIC_SCH && _staticBBinsList.size () == 0) {
        _insList.Append (ins);
    } else { /*-- STATIC_SCH --*/
        ADDRS ins_addr = ins->getInsAddr ();
        Assert (_bbInsMap.find (ins_addr) == _bbInsMap.end ());
        _bbInsMap.insert (pair<ADDRS, bbInstruction*> (ins_addr, ins));
    }
    if (setupAR (ins)) return true;
    else return false;
}

void dynBasicblock::rescheduleInsList (INS_ID* seq_num) {
    Assert (_scheduling_mode == STATIC_SCH);
    //    if (_schedInsList.NumElements () == (int)_staticBBinsList.size ()) return;
    Assert (_schedInsList.NumElements () == 0);// return;

    if (_staticBBinsList.size () == 0) { /*-- MISSING STATIC INS SCHEDULE --*/
        for (int i = 0; i < _insList.NumElements (); i++) {
            bbInstruction* ins = _insList.Nth(i);
            ins->setInsID ((*seq_num)++);
            _schedInsList.Append (ins);
            _schedInsList_waitList.Append (ins);
            setBBheadID ();
        }
        if (_insList.NumElements () > 0)
            g_bbStat->s_dynBB_without_stBB_cnt++;
    } else {
        list<ADDRS>::iterator it;
        for (it = _staticBBinsList.begin (); it != _staticBBinsList.end (); it++) {
            ADDRS ins_addr = *it;
            if (_bbInsMap.find (ins_addr) == _bbInsMap.end()) {
                g_bbStat->s_missing_stIns_in_dynList_cnt++;
                continue;
            }
            bbInstruction* ins = _bbInsMap[ins_addr];
            ins->setInsID ((*seq_num)++);
            _schedInsList.Append (ins);
            _schedInsList_waitList.Append (ins);
            setBBheadID ();
        }
        for (it = _staticBBinsList.begin (); it != _staticBBinsList.end (); it++) {
            ADDRS ins_addr = *it;
            if (_bbInsMap.find (ins_addr) == _bbInsMap.end()) continue;
            _bbInsMap.erase (ins_addr);
        }
        g_bbStat->s_missing_dynIns_in_stList_cnt += _bbInsMap.size ();
//        map<ADDRS, bbInstruction*>::iterator itt = _bbInsMap.begin ();
//        while (itt != _bbInsMap.end ()) {
//            ADDRS ins_addr = itt->first;
//            cout << ins_addr << " in BB " <<  _head._bb_br_ins_addr<< endl;
//            itt++;
//        }
//        cout << endl;
    }
}

void dynBasicblock::setBBstaticInsList (list<ADDRS>& staticBBinsList) {
    Assert (_scheduling_mode == STATIC_SCH);
    _staticBBinsList = staticBBinsList;
}

bool dynBasicblock::isInInsMap (ADDRS ins_addr) {
    Assert (_scheduling_mode == STATIC_SCH);
    return (_bbInsMap.find (ins_addr) != _bbInsMap.end ());
    //TODO: repeated instructions can still happen to cases where _insList is used - fix
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

/*-- THIS FUNCTION ENABLES DISTROYING THE ONLY LASTING COPY OF INS-LIST - USE IT WITH CAUTION --*/
List<bbInstruction*>* dynBasicblock::getBBinsList () {return &_schedInsList;}

bool dynBasicblock::isBBcomplete () {return (_schedInsList.NumElements () == _num_completed_ins) ? true : false;}

// ***********************
// ** INS CONTROL       **
// ***********************
LENGTH dynBasicblock::getBBsize () { return _schedInsList_waitList.NumElements (); }

LENGTH dynBasicblock::getBBorigSize () { return _schedInsList.NumElements (); }

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
    while (_schedInsList_waitList.NumElements () > 0) {
        _schedInsList_waitList.RemoveAt (0);
    }
    for (int i = 0; i < _schedInsList.NumElements (); i++) {
        bbInstruction* ins = _schedInsList.Nth (i);
        _schedInsList_waitList.Append (ins);
        ins->resetStates ();
        ins->resetWrongPath ();
    }
    _num_completed_ins = 0;
    _bb_has_mem_violation = false;
    _bb_on_wrong_path = false;
    _wasted_ins_cnt = 0;
    _done_fetch = false;
}

SCALAR dynBasicblock::getNumWasteIns () {
    return _wasted_ins_cnt;
}

void dynBasicblock::setNumWasteIns (INS_ID faulty_ins_sn) {
    int i;
    for (i = 0; i < _schedInsList.NumElements (); i++) {
        if (_schedInsList.Nth(i)->getInsID () == faulty_ins_sn) {
            _wasted_ins_cnt = i;
            break;
        }
    }
    Assert (i < _schedInsList.NumElements () && "The faulty instruction was not found! Invalid behavior.");
}

void dynBasicblock::setDoneFetch () {
    Assert (_done_fetch == false);
    _done_fetch = true;
}

bool dynBasicblock::isDoneFetch () {
    return _done_fetch;
}
