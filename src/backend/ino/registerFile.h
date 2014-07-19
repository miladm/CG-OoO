/*******************************************************************************
 * registerFile.h
 ******************************************************************************/

#ifndef _REGISTER_FILE_H
#define _REGISTER_FILE_H

#include "../unit/unit.h"
#include "../unit/wires.h"

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
		registerFile (PR rf_begin_num, PR rf_size, 
                      WIDTH rd_port_cnt, WIDTH wr_port_cnt, 
                      sysClock*, string rf_name);
		~registerFile ();
		void updateReg (PR reg);
		void reserveReg (PR reg);
		bool isRegValid (PR reg);
		bool isRegBusy (PR regNum);
        void resetRF ();

        /* WIRE CTRL */
        void updateWireState (AXES_TYPE);
        bool hasFreeWire (AXES_TYPE);
        WIDTH getNumFreeWires (AXES_TYPE);

	private:
		map<PR, registerElement*> _RF;
        CYCLE _cycle;
        const PR _rf_size;
        const PR _rf_begin_num;
        const PR _rf_end_num;
        wires _wr_port;
        wires _rd_port;
};

#endif
