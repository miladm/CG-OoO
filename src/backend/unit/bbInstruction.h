/*******************************************************************************
 * bbInstruction.h
 ******************************************************************************/
#ifndef _BBINSTRUCTION_H
#define _BBINSTRUCTION_H

#include "dynBasicblock.h"
#include "dynInstruction.h"

class dynBasicblock;

class bbInstruction : public dynInstruction {
    public:
        bbInstruction (string class_name = "bbInstruction");
        ~bbInstruction ();

        // SET INS ATRIBUTES
        void setbbWinID (WIDTH);
        void setBB (dynBasicblock* bb);
        void setAR (AR ar, AXES_TYPE type);
        void setPR (PR pr, AXES_TYPE type);

        // GET INS ATRIBUTES
        dynBasicblock* getBB ();
        WIDTH getbbWinID ();
        WIDTH getNumRdAR ();
        WIDTH getTotNumRdAR ();
        WIDTH getNumRdPR ();
        List<AR>* getARrdList ();
        List<AR>* getARwrList ();
        List<PR>* getPRrdList ();
        List<PR>* getPRwrList ();

        // INS CONTROL
        void resetWrongPath ();
        void resetStates ();

    private:
        //BB
        dynBasicblock* _bb;
        WIDTH _bbWin_id;

        //REGS
        List<AR> _a_rdReg;
        List<AR> _a_wrReg;
        List<AR> _a_rdReg_waitList;
        List<AR> _p_rdReg;
        List<AR> _p_wrReg;
        List<AR> _p_rdReg_waitList;
};

#endif
