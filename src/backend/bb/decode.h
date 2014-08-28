/*******************************************************************************
 * decode.h
 ******************************************************************************/

#ifndef _BB_DECODE_H
#define _BB_DECODE_H

#include "../unit/stage.h"

class bb_decode : protected stage {
	public:
		bb_decode (port<bbInstruction*>& fetch_to_decode_port, 
			       port<bbInstruction*>& decode_to_scheduler_port, 
			       WIDTH decode_width,
                   sysClock* clk,
			       string stage_name);
		~bb_decode ();
		void doDECODE ();

    private:
		PIPE_ACTIVITY decodeImpl ();
        void squash ();
        void regStat ();

	private:
		port<bbInstruction*>* _fetch_to_decode_port;
		port<bbInstruction*>* _decode_to_scheduler_port;
};

#endif
