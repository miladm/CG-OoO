/*******************************************************************************
 * config.h
 ******************************************************************************/

#ifndef _CONFIG_H
#define _CONFIG_H

#include <map>
#include <string>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <yaml/yaml.h>
#include "lib/utility.h"
#include "global/global.h"
#include "lib/message.h"

using namespace std;

class config {
	public:
		config ();
		config (string, string, string);
		~config ();
        void storeSimConfig (ofstream*);

        /*-- GET CONFIG ATTRIBUTES --*/
		char* getProgName ();
		char* getSfilePath ();
        string getOutPath ();
        string getProfilePath ();
        SCH_MODE getSchMode ();
        REG_ALLOC_MODE getRegAllocMode ();
        CORE_TYPE getCoreType ();
        MEM_MODEL getMemModel ();
        BP_TYPE getBPtype ();
        int getNumEu ();
        long double getMaxInsCnt ();
        bool isEnSquash ();
        bool isEnEuFwd ();
        bool isEnMemFwd ();
        bool isEnFwd ();
        bool isEnLogStat ();
        bool isEnProfiling ();
        
        /* PROGRAM WARMUP */
        void setWarmedUp ();
        bool isWarmedUp ();
        bool warmUpEn ();

        /* REGISTRE FILE TYPES AND SIZE */
        AR getLARF_LO ();
        AR getLARF_HI ();
        AR getGARF_LO ();
        AR getGARF_HI ();

        /* WRONG PATH */
        unsigned brMisspredDelay ();
        bool isWrongPath ();

    private:
		bool parsePinPointFiles ();
		void verifyConfig ();
        void findLargestSimpoint ();

    public:
		std::map<SIMP,SIMW> _simpoint;
		bool _use_simpoint;
        SIMP fast_fwd_ins_cnt;

	private:
		char _param[PARSE_LEN]; 
		char _program_name[PARSE_LEN];
		FILE* _f_bench_cfg;
		FILE* _f_sim_cfg;
        SCH_MODE _sch_mode;
        REG_ALLOC_MODE _reg_alloc_mode;
        string _config_path;
        string _out_path;
        string _profile_path;
        string _bench_path;
        bool _enable_log_stat;
        long double _max_ins_cnt;

		//PinPoint Config
		char pinPoint_s_file[PARSE_LEN];
		char pinPoint_w_file[PARSE_LEN];
		char output_w_file[PARSE_LEN];
		char obj_r_file[PARSE_LEN];
		char _s_file_root_path[PARSE_LEN];

		//CPU Config
		CORE_TYPE _core_type;
		BP_TYPE _bp_type;
		MEM_MODEL _mem_model;
		rrMode _reg_ren_mode;
		int _num_mem_levels;
		int _cache_lat[MEM_HIGHERARCHY];
		int _st_lat;
		int _alu_cnt;
		int _rob_size;
		int _iwin_size;
		int _pb_win_cnt;
		int _pb_win_size;
        int _num_eu;
        bool _enable_squash;
        bool _enable_eu_fwd;
        bool _enable_mem_fwd;

		//DELAYS
		int _fetch_delay;
		int _bp_delay;
		int _decode_delay;
		int _regren_delay;
		int _issue_delay;
		int _alu_delay;
		int _complete_delay;
		int _wb_delay;
		int _fwd_delay;

        // WRONG PATH (WP)
        bool _enable_wp;
        unsigned _br_misspred_delay;

		//UNIT WIDTHS
		int _bp_width;
		int _decode_width;
		int _issue_width;
		int _dispatch_width;
		int _commit_width;
		int _squash_width;
		int _wb_width;
		int _pb_win_width;
		int _regren_width;
		int _fetch_width;

        //PROGRAM WARMUP
        bool _enable_warm_up;
        bool _warmed_up;

        //REGISTER FILE PARAMS
        AR _larf_lo, _larf_hi, _garf_lo, _garf_hi;

        //PROFILING
        bool _enable_profile;
    public:
        YAML::Node _root;
};

extern config* g_cfg;

#endif
