/*******************************************************************************
 * dynBasicblock.h
 ******************************************************************************/

#ifndef _DYNBASICBLOCK_H
#define _DYNBASICBLOCK_H

#include "unit.h"
#include "dynInstruction.h"

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
        dynBasicblock (string class_name = "dynBasicblock");
        ~dynBasicblock ();

        void setBBbrAddr (bool, ADDRS);
        void setBBID (BB_ID);
        void setWrongPath ();
        void setGPR (AR, PR, AXES_TYPE);
        bool insertIns (dynInstruction*);
        bool setupAR (dynInstruction*);
        void buildInsSchedule ();

        bool bbHasBr ();
        ADDRS getBBbrAddr ();
        BB_ID getBBID ();
        set<AR>* getGARrdList ();
        set<AR>* getGARwrList ();
        dynInstruction* popFront ();
        LENGTH getBBsize ();
        BUFF_STATE getBBstate ();
        bool isOnWrongPath ();
        PR getGPR (AR, AXES_TYPE);

        void squash ();
        void reset ();
        void commit ();

    private:
        bbHead _head;
        map <ADDRS, dynInstruction*> _bbInsMap;
        List<dynInstruction*> _schedInsList;
        List<dynInstruction*> _schedInsList_waitList;

        const LENGTH _max_bb_size;

        bool _is_on_wrong_path;
        bool _is_mem_violation;
};

#endif
