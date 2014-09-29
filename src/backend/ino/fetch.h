/*******************************************************************************
 * fetch.h
 ******************************************************************************/
#ifndef _FETCH_H
#define _FETCH_H

#include "../unit/stage.h"

class fetch : protected stage {
	public:
		fetch (port<dynInstruction*>& bp_to_fetch_port,
			   port<dynInstruction*>& fetch_to_decode_port,
			   port<dynInstruction*>& fetch_to_bp_port,
               CAMtable<dynInstruction*>* iQUE,
               CAMtable<dynInstruction*>* iROB,
			   WIDTH fetch_width,
               sysClock* clk,
			   string stage_name
              );
		~fetch ();

		SIM_MODE doFETCH (FRONTEND_STATUS);

    private:
        PIPE_ACTIVITY fetchImpl (FRONTEND_STATUS);
        void squash ();
        void regStat ();
        bool isGoToFrontend (FRONTEND_STATUS);

	private:
		port<dynInstruction*>* _bp_to_fetch_port;
		port<dynInstruction*>* _fetch_to_decode_port;
		port<dynInstruction*>* _fetch_to_bp_port;
        CAMtable<dynInstruction*>* _iQUE;
        CAMtable<dynInstruction*>* _iROB;
        int _insListIndx;
        bool _switch_to_frontend;
};

#endif
