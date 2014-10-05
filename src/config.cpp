/*******************************************************************************
 * config.cpp
 ******************************************************************************/

#include "config.h"

using namespace std;

config::config () {
	Assert (false && "This constructor is unsupported - config ()");
	_core_type = OUT_OF_ORDER;
	_branch_mode = dynPredBr;
	_mem_model = TOTAL_ORDER;
	//rrMode _reg_ren_mode = RR_ACTIVE; TODO put this line back
}

// PRE:
//		cfgFile to exist
//		PinPointFile to exist
config::config (string bench_path, string config_path) {
//    YAML::Node config;
//    config = YAML::LoadFile ("/home/milad/esc_project/svn/PARS/src/config/base.yaml");
//    std::ifstream fin ("/home/milad/esc_project/svn/PARS/src/config/base.yaml");
//    YAML::Parser parser (fin);
//    YAML::Node doc;
//    for (YAML::iterator it=doc.begin ();it!=doc.end ();++it) {
//        std::string scalar;
//        *it >> scalar;
//        std::cout << "Found scalar: " << scalar << std::endl;
//    }

//    cout << conf["processor"] << endl;


    /* =========== OLD CODE ============ */
	g_msg.simStep ("PARSING SIMULATION CONFIGURATIONS");
    string pars_cfg = "/home/milad/esc_project/svn/PARS/src/config/";
    _config_path  = pars_cfg + config_path;
    _bench_path   = pars_cfg + bench_path + ".cfg";
	_f_sim_cfg    = fopen (_config_path.c_str (), "r");
	_f_bench_cfg  = fopen (_bench_path.c_str (), "r");

    /*-- BENCHMARK PARAMS --*/
	#ifdef ASSERTION
	Assert (_f_bench_cfg != NULL);
	Assert (_f_sim_cfg != NULL);
	#endif
	fscanf (_f_bench_cfg, "%s = \" %s \"\n", _param, _program_name);
	#ifdef ASSERTION
	Assert (strcmp (_param,"APPLICATION") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_bench_cfg, "%s = \" %s \"", _param, pinPoint_s_file);
	#ifdef ASSERTION
	Assert (strcmp (_param,"PINPOINT_SIM_FILE") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_bench_cfg, "%s = \" %s \"", _param, pinPoint_w_file);
	#ifdef ASSERTION
	Assert (strcmp (_param,"PINPOINT_WEIGHT_FILE") == 0 && "Wrong Parameter parsed.");
	#endif
	Assert (parsePinPointFiles () == true && "No simpoint data parsed.");

    /*-- SIMULATION PARAMS --*/
	int t_use_simpoint = -1;
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &t_use_simpoint);
	_use_simpoint =  (bool)t_use_simpoint;
	#ifdef ASSERTION
	Assert (strcmp (_param,"USE_SIMPOINT") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, (int*)&_enable_log_stat);
	#ifdef ASSERTION
	Assert (strcmp (_param,"ENABLE_LOG_STAT") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, (int*)&_sch_mode);
	#ifdef ASSERTION
	Assert (strcmp (_param,"SCH_MODE") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, (int*)&_reg_alloc_mode);
	#ifdef ASSERTION
	Assert (strcmp (_param,"REG_ALLOC_MODE") == 0 && "Wrong Parameter parsed.");
	#endif

	/*-- U-ARCH PARAMS --*/
	fscanf (_f_sim_cfg, "%s = %d\n", _param, (int*)&_core_type);
	#ifdef ASSERTION
	Assert (strcmp (_param,"CORE_TYPE") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, (int*)&_branch_mode);
	#ifdef ASSERTION
	Assert (strcmp (_param,"BRANCH_PRED_MODE") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, (int*)&_mem_model);
	#ifdef ASSERTION
	Assert (strcmp (_param,"MEM_MODEL") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, (int*)&_enable_squash);
	#ifdef ASSERTION
	Assert (strcmp (_param,"ENABLE_SQUASH") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, (int*)&_enable_eu_fwd);
	#ifdef ASSERTION
	Assert (strcmp (_param,"ENABLE_EU_FWD") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, (int*)&_enable_mem_fwd);
	#ifdef ASSERTION
	Assert (strcmp (_param,"ENABLE_MEM_FWD") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_num_eu);
	#ifdef ASSERTION
	Assert (strcmp (_param,"NUM_EU") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_num_mem_levels);
	#ifdef ASSERTION
	Assert (strcmp (_param,"NUM_MEM_LEVELS") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_cache_lat[0]);
	#ifdef ASSERTION
	Assert (strcmp (_param,"L1_LATENCY") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_cache_lat[1]);
	#ifdef ASSERTION
	Assert (strcmp (_param,"L2_LATENCY") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_cache_lat[2]);
	#ifdef ASSERTION
	Assert (strcmp (_param,"L3_LATENCY") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_cache_lat[3]);
	#ifdef ASSERTION
	Assert (strcmp (_param,"DRAM_LATENCY") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_st_lat);
	#ifdef ASSERTION
	Assert (strcmp (_param,"STORE_LATENCY") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_alu_cnt);
	#ifdef ASSERTION
	Assert (strcmp (_param,"ALU_COUNT") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_rob_size);
	#ifdef ASSERTION
	Assert (strcmp (_param,"ROB_SIZE") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_iwin_size);
	#ifdef ASSERTION
	Assert (strcmp (_param,"iWIN_SIZE") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_pb_win_cnt);
	#ifdef ASSERTION
	Assert (strcmp (_param,"PB_WIN_COUNT") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_pb_win_size);
	#ifdef ASSERTION
	Assert (strcmp (_param,"PB_WIN_SIZE") == 0 && "Wrong Parameter parsed.");
	#endif

	/*-- DELAYS --*/
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_fetch_delay);
	#ifdef ASSERTION
	Assert (strcmp (_param,"FETCH_DELAY") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_bp_delay);
	#ifdef ASSERTION
	Assert (strcmp (_param,"BP_DELAY") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_decode_delay);
	#ifdef ASSERTION
	Assert (strcmp (_param,"DECODE_DELAY") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_regren_delay);
	#ifdef ASSERTION
	Assert (strcmp (_param,"REGREN_DELAY") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_issue_delay);
	#ifdef ASSERTION
	Assert (strcmp (_param,"ISSUE_DELAY") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_alu_delay);
	#ifdef ASSERTION
	Assert (strcmp (_param,"ALU_DELAY") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_complete_delay);
	#ifdef ASSERTION
	Assert (strcmp (_param,"COMPLETE_DELAY") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_wb_delay);
	#ifdef ASSERTION
	Assert (strcmp (_param,"WRITEBACK_DELAY") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_fwd_delay);
	#ifdef ASSERTION
	Assert (strcmp (_param,"FORWARDING_DELAY") == 0 && "Wrong Parameter parsed.");
	#endif

	/*-- UNIT WIDTHS --*/
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_bp_width);
	#ifdef ASSERTION
	Assert (strcmp (_param,"BP_WIDTH") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_decode_width);
	#ifdef ASSERTION
	Assert (strcmp (_param,"DECODE_WIDTH") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_issue_width);
	#ifdef ASSERTION
	Assert (strcmp (_param,"ISSUE_WIDTH") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_dispatch_width);
	#ifdef ASSERTION
	Assert (strcmp (_param,"DISPATCH_WIDTH") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_commit_width);
	#ifdef ASSERTION
	Assert (strcmp (_param,"COMMIT_WIDTH") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_squash_width);
	#ifdef ASSERTION
	Assert (strcmp (_param,"SQUASH_WIDTH") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_wb_width);
	#ifdef ASSERTION
	Assert (strcmp (_param,"WB_WIDTH") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_pb_win_width);
	#ifdef ASSERTION
	Assert (strcmp (_param,"PB_WIN_WIDTH") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_regren_width);
	#ifdef ASSERTION
	Assert (strcmp (_param,"REGREN_WIDTH") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = %d\n", _param, &_fetch_width);
	#ifdef ASSERTION
	Assert (strcmp (_param,"FETCH_WIDTH") == 0 && "Wrong Parameter parsed.");
	#endif


	fscanf (_f_sim_cfg, "%s = \" %s \"", _param, output_w_file);
	#ifdef ASSERTION
	Assert (strcmp (_param,"OUTPUT_FILE") == 0 && "Wrong Parameter parsed.");
	#endif
	fscanf (_f_sim_cfg, "%s = \" %s \"", _param, obj_r_file);
	#ifdef ASSERTION
	Assert (strcmp (_param,"OBJ_FILE") == 0 && "Wrong Parameter parsed.");
	#endif

	verifyConfig ();
}

void config::verifyConfig () {
	g_msg.simStep ("VERIFYING CONFIGURATIONS");
	Assert (_num_mem_levels == MEM_HIGHERARCHY);
	for  (int i = 0; i < MEM_HIGHERARCHY; i++) {
		Assert (_cache_lat[i] > 0);}
	Assert (_st_lat > 0 && _st_lat <= _cache_lat[0]);
	Assert (_alu_cnt > 0);
	Assert (_rob_size > 0);
	Assert (_iwin_size > 0 && _iwin_size <= _rob_size);
	Assert (_pb_win_cnt > 0);
	Assert (_pb_win_size > 0);

	Assert (_fetch_delay > 0);
	Assert (_bp_delay > 0);
	Assert (_decode_delay > 0);
	Assert (_regren_delay > 0);
	Assert (_issue_delay > 0);
	Assert (_alu_delay > 0);
	Assert (_complete_delay > 0);
	Assert (_wb_delay > 0);
	Assert (_fwd_delay > 0);

	Assert (_bp_width > 0);
	Assert (_decode_width > 0);
	Assert (_issue_width > 0);
	Assert (_dispatch_width > 0);
	Assert (_commit_width > 0);
	Assert (_squash_width > 0);
	Assert (_wb_width > 0);
	Assert (_pb_win_width > 0);
	Assert (_regren_width > 0);
	Assert (_fetch_width > 0);

    Assert (_num_eu > 0);
}

// PRE:
//		pinPoint_file to exist
bool config::parsePinPointFiles () {
    cout << pinPoint_s_file << endl;
    cout << pinPoint_w_file << endl;
	g_msg.simStep ("PARSING PINPOINTS CONFIGURATIONS");
	//TODO check if the file exists - assert if not
	FILE* f_s_pinPoint = fopen (pinPoint_s_file, "r");
	FILE* f_w_pinPoint = fopen (pinPoint_w_file, "r");
	#ifdef ASSERTION
	Assert (f_s_pinPoint != NULL);
	Assert (f_w_pinPoint != NULL);
	#endif
	int s_id, w_id;
	SIMP s_val; 
	SIMW w_val;
	while  (fscanf (f_s_pinPoint,"%lu %d\n", &s_val, &s_id) != EOF && 
		   fscanf (f_w_pinPoint,"%lf %d\n", &w_val, &w_id) != EOF) {
		#ifdef ASSERTION
		Assert (s_id == w_id && "simpoint and weight files mispatch.");
		#endif
		s_val = s_val * SIMP_WINDOW_SIZE;
		_simpoint.insert (std::pair<SIMP,SIMW> (s_val,w_val));
	}
	//#ifdef DBG
	cout << "** PinPoint Locations:\n";
	map<SIMP,SIMW>::iterator it;
	for  (it = _simpoint.begin (); it != _simpoint.end (); it++) {
		cout << it->first << " " << it->second << endl;
	}
	//#endif
	return  ( (_simpoint.size () > 0) ? true : false);
}

void config::storeSimConfig (ofstream* _out_file) {
        time_t rawtime;
        struct tm * timeinfo;
        char sim_time [80];
        time (&rawtime);
        timeinfo = localtime (&rawtime);
        strftime (sim_time, 80,"* DONE SIM @ %F %I:%M%p",timeinfo);
        (*_out_file) << "* =========== =========== ===========" << endl;
        (*_out_file) << sim_time << endl << endl;
        (*_out_file) << "* CORE: " << _core_type << endl;
        (*_out_file) << "* STAT_SCH_MODE: " << _sch_mode << endl;
        (*_out_file) << "* REG_ALLOC_MODE: " << _reg_alloc_mode << endl;
        (*_out_file) << "* MEM_MODEL: " << _mem_model << endl;
        (*_out_file) << "* ENABLE_SQUASH: " << _enable_squash << endl;
        (*_out_file) << "* ENABLE_EU_FWD: " << _enable_eu_fwd << endl;
        (*_out_file) << "* ENABLE_MEM_FWD: " << _enable_mem_fwd << endl;
        (*_out_file) << "* NUM_EU: " << _num_eu << endl;
        (*_out_file) << "* =========== =========== ===========" << endl << endl;
        //TODO add mroe config params
}

/* *********************** *
 * GET CONFIG ATTRIBUTES
 * *********************** */
char* config::getProgName () { return _program_name; }

SCH_MODE config::getSchMode  () { return _sch_mode; }

REG_ALLOC_MODE config::getRegAllocMode  () { return _reg_alloc_mode; }

CORE_TYPE config::getCoreType () {return _core_type;}

MEM_MODEL config::getMemModel () {return _mem_model;}

int config::getNumEu () {return _num_eu;}

bool config::isEnSquash () {return _enable_squash;}

bool config::isEnEuFwd () {return _enable_eu_fwd;}

bool config::isEnMemFwd () {return _enable_mem_fwd;}

bool config::isEnFwd () {return _enable_mem_fwd || _enable_eu_fwd;}

bool config::isEnLogStat () {return _enable_log_stat;}

config* g_cfg;
