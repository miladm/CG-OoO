/*******************************************************************************
 * decode.h
 ******************************************************************************/

#ifndef _DECODE_H
#define _DECODE_H

#include "../unit/stage.h"

class decode : protected stage {
	public:
		decode (port<dynInstruction*>& fetch_to_decode_port, 
			    port<dynInstruction*>& decode_to_schedule_port, 
			    WIDTH decode_width,
                sysClock* clk,
			    string stage_name);
		~decode ();

		void doDECODE ();

    private:
		PIPE_ACTIVITY decodeImpl ();
        void squash ();
        void regStat ();

	private:
		port<dynInstruction*>* _fetch_to_decode_port;
		port<dynInstruction*>* _decode_to_schedule_port;
};

#endif
