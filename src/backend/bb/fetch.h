/*******************************************************************************
 * fetch::.h
 ******************************************************************************/
#ifndef _BB_FETCH_H
#define _BB_FETCH_H

#include "../unit/stage.h"

//typedef enum {FRONT_END, BACK_END} SIM_MODE;

class bb_fetch : protected stage {
	public:
		bb_fetch (port<dynInstruction*>& bp_to_fetch_port,
			   port<dynInstruction*>& fetch_to_decode_port,
			   port<dynInstruction*>& fetch_to_bp_port,
			   WIDTH fetch_width,
               sysClock* clk,
			   string stage_name
              );
		~bb_fetch ();

		SIM_MODE doFETCH ();

    private:
        PIPE_ACTIVITY fetchImpl ();
        void squash ();
        void regStat ();

	private:
		port<dynInstruction*>* _bp_to_fetch_port;
		port<dynInstruction*>* _fetch_to_decode_port;
		port<dynInstruction*>* _fetch_to_bp_port;
        int _insListIndx;
        bool _switch_to_frontend;
};

#endif
