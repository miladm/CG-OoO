/*******************************************************************************
 * fetch::.h
 ******************************************************************************/
#ifndef _BB_FETCH_H
#define _BB_FETCH_H

#include "../unit/stage.h"

//typedef enum {FRONT_END, BACK_END} SIM_MODE;
typedef enum {FETCH_IN_PROGRESS, FETCH_COMPLETE} BB_FETCH_STATE;

class bb_fetch : protected stage {
	public:
		bb_fetch (port<bbInstruction*>& bp_to_fetch_port,
			   port<bbInstruction*>& fetch_to_decode_port,
			   port<bbInstruction*>& fetch_to_bp_port,
               CAMtable<dynBasicblock*>* bbROB,
			   WIDTH fetch_width,
               sysClock* clk,
			   string stage_name
              );
		~bb_fetch ();
        void squashCurrentBB ();

		SIM_MODE doFETCH ();

    private:
        PIPE_ACTIVITY fetchImpl ();
        void squash ();
        void regStat ();
        void getNewBB ();
        void updateBBfetchState ();
        void delBB (dynBasicblock*);
        void delIns (bbInstruction*);

	private:
		port<bbInstruction*>* _bp_to_fetch_port;
		port<bbInstruction*>* _fetch_to_decode_port;
		port<bbInstruction*>* _fetch_to_bp_port;
        CAMtable<dynBasicblock*>* _bbROB;
        int _insListIndx;
        bool _switch_to_frontend;
        BB_FETCH_STATE _fetch_state;
        dynBasicblock* _current_bb;
};

#endif
