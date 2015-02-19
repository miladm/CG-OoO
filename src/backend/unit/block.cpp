/*******************************************************************************
 * block.cpp
 *******************************************************************************/

#include "block.h"

block::block (SCHED_MODE scheduling_mode, string class_name)
    : unit (class_name)
{ 
    _scheduling_mode = scheduling_mode;
}

block::~block () { 
    for (int i = 0; i < _insList.NumElements (); i++) {
        _insList.RemoveAt (i);
    }
    map<ADDRS, dynInstruction*>::iterator it;
    for (it = _blkInsMap.begin (); it != _blkInsMap.end (); it++) {
        delete it->second;
    }
}

void block::insertIns (dynInstruction* ins) {
    /* IF NO STATIC SCHEDULE EXISTS, ASSUME DYN SCHEDULING */
    if (_scheduling_mode == DYNAMIC_SCH) {
        _schedInsList.Append (ins);
    } else if (_scheduling_mode == STATIC_SCH && _staticBBinsList.size () == 0) {
        _insList.Append (ins);
    } else { /*-- STATIC_SCH --*/
        ADDRS ins_addr = ins->getInsAddr ();
        Assert (_blkInsMap.find (ins_addr) == _blkInsMap.end ());
        _blkInsMap.insert (pair<ADDRS, dynInstruction*> (ins_addr, ins));
    }
}

void block::rescheduleInsList (INS_ID* seq_num) {
    Assert (_scheduling_mode == STATIC_SCH);
    Assert (_schedInsList.NumElements () == 0);

    if (_staticBBinsList.size () == 0) { /*-- MISSING STATIC INS SCHEDULE --*/
        for (int i = 0; i < _insList.NumElements (); i++) {
            dynInstruction* ins = _insList.Nth(i);
            ins->setInsID ((*seq_num)++);
            _schedInsList.Append (ins);
        }
        if (_insList.NumElements () > 0)
            g_bbStat->s_block_without_stBB_cnt++;
    } else {
        list<ADDRS>::iterator it;
        for (it = _staticBBinsList.begin (); it != _staticBBinsList.end (); it++) {
            ADDRS ins_addr = *it;
            if (_blkInsMap.find (ins_addr) == _blkInsMap.end()) {
                g_bbStat->s_missing_stIns_in_dynList_cnt++;
                continue;
            }
            dynInstruction* ins = _blkInsMap[ins_addr];
            _blkInsMap.erase (ins_addr);
            ins->setInsID ((*seq_num)++);
            _schedInsList.Append (ins);
        }

        /* REMOVE INSTRUCTIONS THAT ARE ALREADY SCHEDULED */
        for (it = _staticBBinsList.begin (); it != _staticBBinsList.end (); it++) {
            ADDRS ins_addr = *it;
            if (_blkInsMap.find (ins_addr) == _blkInsMap.end()) continue;
            _blkInsMap.erase (ins_addr);
        }
        g_bbStat->s_missing_dynIns_in_stList_cnt += _blkInsMap.size ();
    }
}

list<ADDRS> block::getUnInstrumentedIns () {
    list<ADDRS>::iterator it;
    for (it = _staticBBinsList.begin (); it != _staticBBinsList.end (); it++) {
        ADDRS ins_addr = *it;
        if (_blkInsMap.find (ins_addr) == _blkInsMap.end()) {
            _unInstrumentedList.push_back (ins_addr);
        }
    }
    return _unInstrumentedList;
}

void block::setBBstaticInsList (list<ADDRS>& staticBBinsList) {
    Assert (_scheduling_mode == STATIC_SCH);
    _staticBBinsList = staticBBinsList;
}

dynInstruction* block::popFront () {
#ifdef ASSERTION
    Assert (_schedInsList.NumElements () > 0);
#endif
    dynInstruction* ins = _schedInsList.Nth (0);
    _schedInsList.RemoveAt (0);
    return ins;
}

LENGTH block::getBlockSize () { return _schedInsList.NumElements (); }

bool block::isInInsMap (ADDRS ins_addr) {
    Assert (_scheduling_mode == STATIC_SCH);
    return (_blkInsMap.find (ins_addr) != _blkInsMap.end ());
}
