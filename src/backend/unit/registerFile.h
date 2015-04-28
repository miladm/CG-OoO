/*******************************************************************************
 * registerFile.h
 ******************************************************************************/

#ifndef _REGISTER_FILE_H
#define _REGISTER_FILE_H

#include "unit.h"
#include "wires.h"

#define RD_TO_WR_WIRE_CNT_RATIO 2

struct registerElement {
    registerElement (PR reg) 
        : _reg (reg)
    {
        _reg_state = NO_VAL_REG;
        _axesedBefore = false;
    }
    void resetReg () {
        if (_reg_state == WAIT_ON_WRITE_REG) _reg_state = NO_VAL_REG;
        _axesedBefore = false;
    }
    const PR _reg;
    REG_STATE _reg_state;
    bool _axesedBefore;
};

class registerFile : public unit {
	public:
		registerFile (sysClock*, const YAML::Node&, string rf_name);
		~registerFile ();
		void updateReg (PR reg);
		void reserveReg (PR reg);
		bool isRegValidAndReady (PR reg);
		bool isRegBusy (PR regNum);
        void resetRF ();

        /* WIRE CTRL */
        void updateWireState (AXES_TYPE, list<string> wire_name = list<string>(), bool update_wire = false);
        bool hasFreeWire (AXES_TYPE);
        WIDTH getNumFreeWires (AXES_TYPE);

	private:
		map<PR, registerElement*> _RF;
        CYCLE _cycle;
        PR _rf_size;
        PR _rf_begin_num;
        PR _rf_end_num;
        wires _wr_port;
        wires _rd_port;
};

#endif
