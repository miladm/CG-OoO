/*******************************************************************************
 *  registerRename.h
 ******************************************************************************/

#ifndef _REGISTER_RENAME_H
#define _REGISTER_RENAME_H

#include <map>
#include <vector>
#include "../global/global.h"
#include "../lib/list.h"

class instruction;

class registerRename {
	public:
		registerRename();
		registerRename(int arf_lo, int arf_hi);
		~registerRename();
		
		PR getRenamedReg(AR a_reg);
		void update_fRAT(AR a_reg, PR p_reg);
		void update_cRAT(AR a_reg, PR p_reg);
		void squashRAT(AR a_reg);
		void flush_fRAT();

		bool isAnyPRavailable();
		PR getAvailablePR();
		void setAsAvailablePR(PR p_reg);
		int getNumAvailablePR();

		void setARST(PR new_pr,PR old_pr);
		PR getARST(PR p_reg);
		void eraseARST(PR p_reg);
		void squashARST(PR p_reg);

		void updatePRFSM(PR p_reg, REG_REN_STATE state, instruction* writerIns);
		void updatePRFSM(PR p_reg, REG_REN_STATE state);
		REG_REN_STATE getPRFSM(PR p_reg);
		instruction* getWriterIns(PR p_reg);
		void squashPRFSM(PR p_reg); //squash unit
		//TODO: when a register is in invalid state, the instruction needing it must becoem dependent on it somehow. my framework requires that instructions becoem dependent on each other. what should I do?

	private:
		map<AR,PR> _fRAT, _cRAT;
		map<PR,VALID> _PRvalid;  //TODO think about using this for making sure the initial state of register is valid. do I need it?
		map<PR,REG_REN_STATE> _PRFSM;
		map<PR,PR> _ARST;
		vector<PR> _availablePRset; //This is a FILO (stack)
		map<PR,int> _PR; //we don't use this map in simulation. just a place holder
		map<PR,instruction*> _writeInstructions;
};

#endif
