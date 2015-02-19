/*******************************************************************************
 * block.h
 ******************************************************************************/

#ifndef _BLOCK_H
#define _BLOCK_H

#include "unit.h"
#include "dynInstruction.h"
#include "bbStat.h"

class block : public unit {
    public:
        block (SCHED_MODE, string class_name = "block");
        ~block ();

        void insertIns (dynInstruction*);
        void rescheduleInsList (INS_ID*);
        void setBBstaticInsList (list<ADDRS>&);
        dynInstruction* popFront ();
        list<ADDRS> getUnInstrumentedIns ();
        LENGTH getBlockSize ();
        bool isInInsMap (ADDRS);

    private:
        list<ADDRS> _staticBBinsList;
        list<ADDRS> _unInstrumentedList;
        map <ADDRS, dynInstruction*> _blkInsMap;
        List<dynInstruction*> _insList;
        List<dynInstruction*> _schedInsList;
        SCHED_MODE _scheduling_mode;
};

#endif
