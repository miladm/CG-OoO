/*******************************************************************************
 * bbInstruction.h
 ******************************************************************************/
#ifndef _BBINSTRUCTION_H
#define _BBINSTRUCTION_H

#include "dynBasicblock.h"
#include "dynInstruction.h"

typedef enum {LOCAL_REG, GLOBAL_REG} REG_TYPE;

class dynBasicblock;

class bbInstruction : public dynInstruction {
    public:
        bbInstruction (string class_name = "bbInstruction");
        ~bbInstruction ();

        // SET INS ATRIBUTES
        void setBBWinID (WIDTH);
        void setBB (dynBasicblock* bb);
        void setAR (AR ar, AXES_TYPE type);
        void setPR (PR pr, AXES_TYPE type);

        // GET INS ATRIBUTES
        dynBasicblock* getBB ();
        WIDTH getBBWinID ();
        WIDTH getNumRdAR ();
        WIDTH getNumRdLAR ();
        WIDTH getTotNumRdAR ();
        WIDTH getTotNumRdLAR ();
        WIDTH getNumRdPR ();
        List<AR>* getARrdList ();
        List<AR>* getARwrList ();
        List<PR>* getPRrdList ();
        List<PR>* getPRwrList ();
        List<AR>* getLARrdList ();
        List<AR>* getLARwrList ();

        // INS CONTROL
        void copyRegsTo (bbInstruction*);
        void resetWrongPath ();
        void resetStates ();

    private:
        REG_TYPE getARtype (AR);

    private:
        //BB
        dynBasicblock* _bb;
        WIDTH _bbWin_id;

        //LOCAL REGS
        List<AR> _l_rdReg;
        List<AR> _l_wrReg;
        List<AR> _l_rdReg_waitList;

        //GLOBAL REGS
        List<AR> _a_rdReg;
        List<AR> _a_wrReg;
        List<AR> _a_rdReg_waitList;
        List<AR> _p_rdReg;
        List<AR> _p_wrReg;
        List<AR> _p_rdReg_waitList;
};

#endif
