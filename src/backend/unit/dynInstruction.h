/*******************************************************************************
 * dynInstruction.h
 ******************************************************************************/
#ifndef _DYNINSTRUCTION_H
#define _DYNINSTRUCTION_H

#include "unit.h"

class dynInstruction : public unit {
    public:
        dynInstruction (string class_name = "dynInstruction");
        ~dynInstruction ();

        // SET INS ATRIBUTES
        void setInsAddr (ADDRS ins_addr);
        void setInsID (INS_ID seq_num);
        void setInsType (INS_TYPE insType);
        void setAR (AR ar, AXES_TYPE type);
        void setPR (PR pr, AXES_TYPE type);
        void setMemAtr (MEM_TYPE memType, ADDRS memAddr, BYTES memAxesSize, bool stackRd, bool stackWr);
        void setMemAtr (MEM_TYPE memType, BYTES memAxesSize);
        void setBrAtr (ADDRS brTarget, ADDRS brFalThru, bool hasFalThru, bool brTaken, bool isCall, bool isRet, bool isJump, bool isDirBrOrCallOrJmp);
        void setBrAtr (ADDRS brTarget, bool isCall, bool isRet, bool isJump);
        void setPipeStage (PIPE_STAGE insStage);
        void setSQstate (SQ_STATE state);
        void setLQstate (LQ_STATE state);
        void setWrongPath (bool is_on_wrong_path);
        void setMemViolation ();

        // GET INS ATRIBUTES
        ADDRS getInsAddr ();
        INS_ID getInsID ();
        INS_TYPE getInsType ();
        ADDRS getMemAddr ();
        BYTES getMemAxesSize ();
        MEM_TYPE getMemType ();
        ADDRS getBrTarget ();
        BR_TYPE getBrType ();
        bool isBrTaken ();
        PIPE_STAGE getPipeStage ();
        SQ_STATE getSQstate ();
        LQ_STATE getLQstate ();
        bool isOnWrongPath ();
        bool isMemViolation ();
        bool isMemOrBrViolation ();
        WIDTH getNumRdAR ();
        WIDTH getTotNumRdAR ();
        WIDTH getNumRdPR ();
        List<AR>* getARrdList ();
        List<AR>* getARwrList ();
        List<PR>* getPRrdList ();
        List<PR>* getPRwrList ();

        // INS CONTROL
        void resetStates ();

    protected:
        //INS
        ADDRS _ins_addr;
        INS_ID _seq_num;
        INS_TYPE _ins_type;

        //BR
        ADDRS _brTarget;
        bool _brTaken;
        BR_TYPE _brType;

        //MEM
        BYTES _memAxesSize;
        ADDRS _memAddr;
        bool _stackRd;
        bool _stackWr;
        MEM_TYPE _memType;

        //MISC
        PIPE_STAGE _insStage;
        SQ_STATE _sq_state;
        LQ_STATE _lq_state;
        char _command[INS_STRING_SIZE];
        int _hitLevel;
        bool _is_on_wrong_path;
        bool _is_mem_violation;

    private:
        //REGS
        List<AR> _a_rdReg;
        List<AR> _a_wrReg;
        List<AR> _a_rdReg_waitList;
        List<AR> _p_rdReg;
        List<AR> _p_wrReg;
        List<AR> _p_rdReg_waitList;

};

#endif
