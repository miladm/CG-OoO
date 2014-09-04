/*******************************************************************************
 * fetch::.h
 ******************************************************************************/
#ifndef _O3_FETCH_H
#define _O3_FETCH_H

#include "../unit/stage.h"

//typedef enum {FRONT_END, BACK_END} SIM_MODE;

class o3_fetch : protected stage {
	public:
		o3_fetch (port<dynInstruction*>& bp_to_fetch_port,
			   port<dynInstruction*>& fetch_to_decode_port,
			   port<dynInstruction*>& fetch_to_bp_port,
               CAMtable<dynInstruction*>* iQUE,
			   WIDTH fetch_width,
               sysClock* clk,
			   string stage_name
              );
		~o3_fetch ();

		SIM_MODE doFETCH ();

    private:
        PIPE_ACTIVITY fetchImpl ();
        void squash ();
        void regStat ();

	private:
		port<dynInstruction*>* _bp_to_fetch_port;
		port<dynInstruction*>* _fetch_to_decode_port;
		port<dynInstruction*>* _fetch_to_bp_port;
        CAMtable<dynInstruction*>* _iQUE;
        int _insListIndx;
        bool _switch_to_frontend;
};

#endif
