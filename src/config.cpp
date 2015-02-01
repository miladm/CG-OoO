/*******************************************************************************
 * config.cpp
 ******************************************************************************/

#include "config.h"

using namespace std;

config::config () {
	Assert (0 && "This constructor is unsupported - config ()");
	_core_type = OUT_OF_ORDER;
	_bp_type = PERFECT_BP;
	_mem_model = TOTAL_ORDER;
	//rrMode _reg_ren_mode = RR_ACTIVE; TODO put this line back
}

// PRE:
//		cfgFile TO EXIST
//		PinPointFile TO EXIST
config::config (string bench_path, string config_path, string out_dir) {
    string yaml_file = out_dir + "/base.yaml";
    ifstream fin (yaml_file);
    YAML::Parser parser (fin);
    parser.GetNextDocument (_root);


    int prof_temp, wp_temp;
    _root["cpu"]["frontend"]["profiling"] >> prof_temp;
    _root["cpu"]["backend"]["rf"]["lrf"]["rf_lo"] >> _larf_lo;
    _root["cpu"]["backend"]["rf"]["lrf"]["rf_hi"] >> _larf_hi;
    _root["cpu"]["backend"]["rf"]["grf"]["a_lo"]  >> _garf_lo;
    _root["cpu"]["backend"]["rf"]["grf"]["a_hi"]  >> _garf_hi;
    _enable_profile = (bool)prof_temp;
    _root["cpu"]["frontend"]["wrong_path"] >> wp_temp;
    _enable_wp = (bool)wp_temp;
    _root["cpu"]["frontend"]["br_misspred_delay"] >> _br_misspred_delay;


    /* =========== ============ */
    /* organzie this code  - hack */
    _warmed_up = false;
    _enable_warm_up = true;

    /* =========== OLD CODE ============ */
	g_msg.simStep ("PARSING SIMULATION CONFIGURATIONS");
    string cfg_file_ext = ".cfg";
    cout << config_path << endl;
    _config_path  = config_path + cfg_file_ext;
    _out_path = out_dir;
    _profile_path = "/home/milad/esc_project/svn/PARS/src/binaryTranslator/profile_files/ref"; //TODO tranfer to yaml
    cout << _config_path << endl;
    _bench_path   = bench_path + cfg_file_ext;
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
	fscanf (_f_sim_cfg, "%s = %Lf\n", _param, &_max_ins_cnt);
    _max_ins_cnt *= (float) MILLION;
	#ifdef ASSERTION
	Assert (strcmp (_param,"MAX_INS_CNT") == 0 && "Wrong Parameter parsed.");
	#endif
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
	fscanf (_f_sim_cfg, "%s = %d\n", _param, (int*)&_bp_type);
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

	fscanf (_f_sim_cfg, "%s = \" %s \"", _param, _s_file_root_path);
	#ifdef ASSERTION
	Assert (strcmp (_param,"S_FILE_PATH") == 0 && "Wrong Parameter parsed.");
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

	while  (fscanf (f_s_pinPoint,"%llu %d\n", &s_val, &s_id) != EOF && 
		   fscanf (f_w_pinPoint,"%lf %d\n", &w_val, &w_id) != EOF) {
		#ifdef ASSERTION
		Assert (s_id == w_id && "simpoint and weight files mispatch.");
		#endif
		s_val = s_val * SIMP_WINDOW_SIZE;
		_simpoint.insert (std::pair<SIMP, SIMW> (s_val,w_val));
	}

	//#ifdef DBG
    g_msg.simEvent ("SIMPOINT LOCATIONS:\n");
	map<SIMP, SIMW>::iterator it;
	for  (it = _simpoint.begin (); it != _simpoint.end (); it++) {
        g_msg.simEvent ("%llu %f\n", it->second, it->first );
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
        (*_out_file) << "* BRANCH_PRED_MODE: " << _bp_type << endl;
        (*_out_file) << "* MEM_MODEL: " << _mem_model << endl;
        (*_out_file) << "* ENABLE_SQUASH: " << _enable_squash << endl;
        (*_out_file) << "* ENABLE_EU_FWD: " << _enable_eu_fwd << endl;
        (*_out_file) << "* ENABLE_MEM_FWD: " << _enable_mem_fwd << endl;
        (*_out_file) << "* NUM_EU: " << _num_eu << endl;
        (*_out_file) << "* MAX_INS_CNT: " << _max_ins_cnt << endl;
        (*_out_file) << "* =========== =========== ===========" << endl << endl;
        //TODO add mroe config params

        cout << "* =========== =========== ===========" << endl;
        cout << sim_time << endl << endl;
        cout << "* CORE: " << _core_type << endl;
        cout << "* STAT_SCH_MODE: " << _sch_mode << endl;
        cout << "* REG_ALLOC_MODE: " << _reg_alloc_mode << endl;
        cout << "* BRANCH_PRED_MODE: " << _bp_type << endl;
        cout << "* MEM_MODEL: " << _mem_model << endl;
        cout << "* ENABLE_SQUASH: " << _enable_squash << endl;
        cout << "* ENABLE_EU_FWD: " << _enable_eu_fwd << endl;
        cout << "* ENABLE_MEM_FWD: " << _enable_mem_fwd << endl;
        cout << "* NUM_EU: " << _num_eu << endl;
        cout << "* MAX_INS_CNT: " << _max_ins_cnt << endl;
        cout << "* =========== =========== ===========" << endl << endl;
}

/* *********************** *
 * GET CONFIG ATTRIBUTES
 * *********************** */
char* config::getProgName () { return _program_name; }

char* config::getSfilePath () { return _s_file_root_path; }

string config::getOutPath () { return _out_path; }

string config::getProfilePath () { return _profile_path; }

SCH_MODE config::getSchMode  () { return _sch_mode; }

REG_ALLOC_MODE config::getRegAllocMode  () { return _reg_alloc_mode; }

CORE_TYPE config::getCoreType () {return _core_type;}

MEM_MODEL config::getMemModel () {return _mem_model;}

BP_TYPE config::getBPtype () {return _bp_type;}

long double config::getMaxInsCnt () {return _max_ins_cnt;}

bool config::isEnSquash () {return _enable_squash;}

bool config::isEnEuFwd () {return _enable_eu_fwd;}

bool config::isEnMemFwd () {return _enable_mem_fwd;}

bool config::isEnFwd () {return _enable_mem_fwd || _enable_eu_fwd;}

bool config::isEnLogStat () {return _enable_log_stat;}

bool config::isEnProfiling () {return _enable_profile;} //TODO put this to yaml

void config::setWarmedUp () {_warmed_up = true;} /* TODO this bad hack - change it*/

bool config::isWarmedUp () {return _warmed_up;}

bool config::warmUpEn () {return _enable_warm_up;}

AR config::getLARF_LO () {return _larf_lo;}

AR config::getLARF_HI () {return _larf_hi;}

AR config::getGARF_LO () {return _garf_lo;}

AR config::getGARF_HI () {return _garf_hi;}

unsigned config::brMisspredDelay () {Assert (_enable_wp); return _br_misspred_delay;}

bool config::isWrongPath () {return _enable_wp;}

config* g_cfg;
