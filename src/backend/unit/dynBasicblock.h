/*******************************************************************************
 * dynBasicblock.h
 ******************************************************************************/

#ifndef _DYNBASICBLOCK_H
#define _DYNBASICBLOCK_H

#include "unit.h"
#include "bbInstruction.h"
#include "bbStat.h"

class bbInstruction;

struct bbHead {
    bbHead (LENGTH max_ins_cnt)
        : _max_rd_g_reg_cnt (2 * max_ins_cnt),
          _max_wr_g_reg_cnt (max_ins_cnt)
    {
        _avail_rd_g_reg_cnt = _max_rd_g_reg_cnt;
        _avail_wr_g_reg_cnt = _max_wr_g_reg_cnt;
        Assert (_max_rd_g_reg_cnt == 2 * _max_wr_g_reg_cnt);
    }
    bool hasAvailReg (LENGTH wr_g_reg_cnt, LENGTH rd_g_reg_cnt) {
        if (_avail_wr_g_reg_cnt - wr_g_reg_cnt < 0) return false;
        if (_avail_rd_g_reg_cnt - rd_g_reg_cnt < 0) return false;
        return true;
    }
    void updateAvailReg (LENGTH wr_g_reg_cnt, LENGTH rd_g_reg_cnt) {
        _avail_wr_g_reg_cnt -= wr_g_reg_cnt;
        _avail_rd_g_reg_cnt -= rd_g_reg_cnt;

        Assert (_avail_wr_g_reg_cnt - wr_g_reg_cnt >= 0);
        Assert (_avail_rd_g_reg_cnt - rd_g_reg_cnt >= 0);
    }


    /* BBHEAD PARAMETERS */
    bool _bb_has_br;
    ADDRS _bb_br_ins_addr;
    BB_ID _bb_seq_num;

    /* ARCH & PHYS GLOBAL REGISTERS */
    set<AR> _a_rd_g_reg;
    set<AR> _a_wr_g_reg;
    map<AR, PR> _a2p_rd_g_reg;
    map<AR, PR> _a2p_wr_g_reg;

    /* bbHEAD HW CONSTRAINTS */
    const LENGTH _max_rd_g_reg_cnt;
    const LENGTH _max_wr_g_reg_cnt;
    LENGTH _avail_rd_g_reg_cnt;
    LENGTH _avail_wr_g_reg_cnt;
};

class dynBasicblock : public unit {
    public:
        dynBasicblock (SCHED_MODE, string class_name = "dynBasicblock");
        ~dynBasicblock ();

        void setBBbrAddr (bool, ADDRS);
        void setBBID (BB_ID);
        void setGPR (AR, PR, AXES_TYPE);
        bool insertIns (bbInstruction*);
        void rescheduleInsList (INS_ID*);
        bool setupAR (bbInstruction*);
        void buildInsSchedule ();
        void incCompletedInsCntr ();
        void setWrongPath ();
        void setMemViolation ();
        void setBBstaticInsList (list<ADDRS>&);

        bool bbHasBr ();
        ADDRS getBBbrAddr ();
        BB_ID getBBID ();
        INS_ID getBBheadID ();
        set<AR>* getGARrdList ();
        set<AR>* getGARwrList ();
        bbInstruction* popFront ();
        LENGTH getBBsize ();
        LENGTH getBBorigSize ();
        BUFF_STATE getBBstate ();
        bool isOnWrongPath ();
        PR getGPR (AR, AXES_TYPE);
        bool isMemOrBrViolation ();
        List<bbInstruction*>* getBBinsList ();
        bool isBBcomplete ();

        void squash ();
        void reset ();
        void commit ();

        //BB CONTROL
        void resetStates ();

        SCALAR getNumWasteIns ();
        void setNumWasteIns (INS_ID);

    private:
        void setBBheadID ();

    private:
        bbHead _head;
        map <ADDRS, bbInstruction*> _bbInsMap;
        list<ADDRS> _staticBBinsList;
        List<bbInstruction*> _insList;
        List<bbInstruction*> _schedInsList;
        List<bbInstruction*> _schedInsList_waitList;

        const LENGTH _max_bb_size;
        LENGTH _num_completed_ins;

        bool _bb_on_wrong_path;
        bool _bb_has_mem_violation;

        INS_ID _head_ins_seq_num;

        SCALAR _wasted_ins_cnt;
        
        SCHED_MODE _scheduling_mode;
};


#endif
