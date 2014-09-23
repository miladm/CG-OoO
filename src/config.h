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
#include "lib/utility.h"
#include "global/global.h"
#include "global/g_variable.h"
#include "lib/yaml-cpp/yaml.h"

using namespace std;

class config {
	public:
		config();
		config(char*,g_variable*);
		~config();
		bool parsePinPointFiles(char*, char*);
		char* getProgName();
        SCH_MODE getSchMode ();
        REG_ALLOC_MODE getRegAllocMode ();

		std::map<SIMP,SIMW> _simpoint;
		bool _use_simpoint;

	public:
		void verifyConfig();

		char param[PARSE_LEN]; 
		char program_name[PARSE_LEN];
		FILE* f_config;
		g_variable * _g_var;
        SCH_MODE _sch_mode;
        REG_ALLOC_MODE _reg_alloc_mode;

		//PinPoint Config
		char pinPoint_s_file[PARSE_LEN];
		char pinPoint_w_file[PARSE_LEN];
		char output_w_file[PARSE_LEN];
		char obj_r_file[PARSE_LEN];

		//CPU Config
		core coreType;
		brMode branchMode;
		memModel memoryModel;
		rrMode regRenMode;
		int num_mem_levels;
		int cache_lat[MEM_HIGHERARCHY];
		int st_lat;
		int alu_count;
		int rob_size;
		int iwin_size;
		int pb_win_count;
		int pb_win_size;

		//DELAYS
		int fetch_delay;
		int bp_delay;
		int decode_delay;
		int regren_delay;
		int issue_delay;
		int alu_delay;
		int complete_delay;
		int wb_delay;
		int fwd_delay;

		//UNIT WIDTHS
		int bp_width;
		int decode_width;
		int issue_width;
		int dispatch_width;
		int commit_width;
		int squash_width;
		int wb_width;
		int pb_win_width;
		int regren_width;
		int fetch_width;
};

#endif
