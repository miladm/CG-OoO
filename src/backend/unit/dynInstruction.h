/*******************************************************************************
 * dynInstruction.h
 ******************************************************************************/
#ifndef _DYNINSTRUCTION_H
#define _DYNINSTRUCTION_H

#include "unit.h"
//#include "../ino/registerFile.h"
//#include "../o3/registerFile.h"

class dynInstruction : public unit {
    public:
        dynInstruction ();
        ~dynInstruction ();

        // SET INS ATRIBUTES
        void setInsAddr (ADDRS insAddr);
        void setInsID (INS_ID seqNum);
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
        WIDTH getNumRdAR ();
        WIDTH getNumRdPR ();
        List<AR>* getARrdList ();
        List<AR>* getARwrList ();
        List<PR>* getPRrdList ();
        List<PR>* getPRwrList ();

        // INS CONTROL
        void copyRegsTo (dynInstruction* ins);
        void resetStates ();

    private:
        //INS
        ADDRS _insAddr;
        INS_ID _seqNum;
        INS_TYPE _insType;

        //REGS
        List<AR> _a_rdReg;
        List<AR> _a_wrReg;
        List<AR> _a_rdReg_waitList;
        List<AR> _p_rdReg;
        List<AR> _p_wrReg;
        List<AR> _p_rdReg_waitList;

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
};

#endif
