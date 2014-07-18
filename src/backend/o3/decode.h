/*******************************************************************************
 * decode.h
 ******************************************************************************/

#ifndef _O3_DECODE_H
#define _O3_DECODE_H

#include "../unit/stage.h"

class o3_decode : protected stage {
	public:
		o3_decode (port<dynInstruction*>& fetch_to_decode_port, 
			    port<dynInstruction*>& decode_to_schedule_port, 
			    WIDTH decode_width,
			    string stage_name);
		~o3_decode ();

		void doDECODE (sysClock& clk);
		PIPE_ACTIVITY decodeImpl (sysClock& clk);
        void squash (sysClock& clk);
        void regStat (sysClock& clk);

	private:
		port<dynInstruction*>* _fetch_to_decode_port;
		port<dynInstruction*>* _decode_to_schedule_port;
};

#endif
