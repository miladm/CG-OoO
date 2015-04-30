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
                  CAMtable<dynBasicblock*>* bbQUE,
			      WIDTH fetch_width,
                  sysClock* clk,
			      string stage_name
                 );
		~bb_fetch ();
        void squashCurrentBB ();

		SIM_MODE doFETCH (FRONTEND_STATUS);

    private:
        PIPE_ACTIVITY fetchImpl (FRONTEND_STATUS);
        void squash ();
        void regStat ();
        void updateBBfetchState ();
        bool isGoToFrontend (FRONTEND_STATUS);
        BUFF_STATE getNewBB ();
        bool fetchBB (FRONTEND_STATUS);

	private:
		port<bbInstruction*>* _bp_to_fetch_port;
		port<bbInstruction*>* _fetch_to_decode_port;
		port<bbInstruction*>* _fetch_to_bp_port;
        CAMtable<dynBasicblock*>* _bbROB;
        CAMtable<dynBasicblock*>* _bbQUE;
        int _insListIndx;
        bool _switch_to_frontend;
        BB_FETCH_STATE _fetch_state;
        dynBasicblock* _current_bb;

        /*-- STAT --*/
        ScalarStat& s_bb_cnt;
        RatioStat& s_bb_size_avg;

        /*-- ENERGY --*/
        table_energy _e_icache;
};

#endif
