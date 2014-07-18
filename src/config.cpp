/*******************************************************************************
 * config.cpp
 ******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <iostream>
#include "lib/utility.h"
#include "config.h"

using namespace std;

config::config() {
	Assert(false && "This constructor is unsupported - config()");
	coreType = OUT_OF_ORDER;
	branchMode = dynPredBr;
	memoryModel = TOTAL_ORDER;
	//rrMode regRenMode = RR_ACTIVE; TODO put this line back
}

// PRE:
//		cfgFile to exist
//		PinPointFile to exist
config::config(char* cfg_file, g_variable * g_var) {
	_g_var = g_var;
	_g_var->msg.simStep("PARSING SIMULATION CONFIGURATIONS");
	f_config = fopen (cfg_file,"r");
	#ifdef ASSERTION
	Assert(f_config != NULL);
	#endif
	fscanf(f_config, "%s = \" %s \"\n", param, program_name);
	#ifdef ASSERTION
	Assert(strcmp(param,"APPLICATION")==0 && "Wrong Parameter parsed.");
	#endif
	int t_use_simpoint = -1;
	fscanf(f_config, "%s = %d\n", param, &t_use_simpoint);
	_use_simpoint = (bool)t_use_simpoint;
	#ifdef ASSERTION
	Assert(strcmp(param,"USE_SIMPOINT")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = \" %s \"", param, pinPoint_s_file);
	#ifdef ASSERTION
	Assert(strcmp(param,"PINPOINT_SIM_FILE")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = \" %s \"", param, pinPoint_w_file);
	#ifdef ASSERTION
	Assert(strcmp(param,"PINPOINT_WEIGHT_FILE")==0 && "Wrong Parameter parsed.");
	#endif

	//U-ARCH PARAMS
	fscanf(f_config, "%s = %d\n", param, (int*)&coreType);
	#ifdef ASSERTION
	Assert(strcmp(param,"CORE_TYPE")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, (int*)&branchMode);
	#ifdef ASSERTION
	Assert(strcmp(param,"BRANCH_PRED_MODE")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, (int*)&memoryModel);
	#ifdef ASSERTION
	Assert(strcmp(param,"MEM_MODEL")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &num_mem_levels);
	#ifdef ASSERTION
	Assert(strcmp(param,"NUM_MEM_LEVELS")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &cache_lat[0]);
	#ifdef ASSERTION
	Assert(strcmp(param,"L1_LATENCY")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &cache_lat[1]);
	#ifdef ASSERTION
	Assert(strcmp(param,"L2_LATENCY")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &cache_lat[2]);
	#ifdef ASSERTION
	Assert(strcmp(param,"L3_LATENCY")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &cache_lat[3]);
	#ifdef ASSERTION
	Assert(strcmp(param,"DRAM_LATENCY")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &st_lat);
	#ifdef ASSERTION
	Assert(strcmp(param,"STORE_LATENCY")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &alu_count);
	#ifdef ASSERTION
	Assert(strcmp(param,"ALU_COUNT")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &rob_size);
	#ifdef ASSERTION
	Assert(strcmp(param,"ROB_SIZE")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &iwin_size);
	#ifdef ASSERTION
	Assert(strcmp(param,"iWIN_SIZE")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &pb_win_count);
	#ifdef ASSERTION
	Assert(strcmp(param,"PB_WIN_COUNT")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &pb_win_size);
	#ifdef ASSERTION
	Assert(strcmp(param,"PB_WIN_SIZE")==0 && "Wrong Parameter parsed.");
	#endif

	//DELAYS
	fscanf(f_config, "%s = %d\n", param, &fetch_delay);
	#ifdef ASSERTION
	Assert(strcmp(param,"FETCH_DELAY")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &bp_delay);
	#ifdef ASSERTION
	Assert(strcmp(param,"BP_DELAY")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &decode_delay);
	#ifdef ASSERTION
	Assert(strcmp(param,"DECODE_DELAY")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &regren_delay);
	#ifdef ASSERTION
	Assert(strcmp(param,"REGREN_DELAY")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &issue_delay);
	#ifdef ASSERTION
	Assert(strcmp(param,"ISSUE_DELAY")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &alu_delay);
	#ifdef ASSERTION
	Assert(strcmp(param,"ALU_DELAY")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &complete_delay);
	#ifdef ASSERTION
	Assert(strcmp(param,"COMPLETE_DELAY")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &wb_delay);
	#ifdef ASSERTION
	Assert(strcmp(param,"WRITEBACK_DELAY")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &fwd_delay);
	#ifdef ASSERTION
	Assert(strcmp(param,"FORWARDING_DELAY")==0 && "Wrong Parameter parsed.");
	#endif

	//UNIT WIDTHS
	fscanf(f_config, "%s = %d\n", param, &bp_width);
	#ifdef ASSERTION
	Assert(strcmp(param,"BP_WIDTH")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &decode_width);
	#ifdef ASSERTION
	Assert(strcmp(param,"DECODE_WIDTH")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &issue_width);
	#ifdef ASSERTION
	Assert(strcmp(param,"ISSUE_WIDTH")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &dispatch_width);
	#ifdef ASSERTION
	Assert(strcmp(param,"DISPATCH_WIDTH")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &commit_width);
	#ifdef ASSERTION
	Assert(strcmp(param,"COMMIT_WIDTH")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &squash_width);
	#ifdef ASSERTION
	Assert(strcmp(param,"SQUASH_WIDTH")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &wb_width);
	#ifdef ASSERTION
	Assert(strcmp(param,"WB_WIDTH")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &pb_win_width);
	#ifdef ASSERTION
	Assert(strcmp(param,"PB_WIN_WIDTH")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &regren_width);
	#ifdef ASSERTION
	Assert(strcmp(param,"REGREN_WIDTH")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = %d\n", param, &fetch_width);
	#ifdef ASSERTION
	Assert(strcmp(param,"FETCH_WIDTH")==0 && "Wrong Parameter parsed.");
	#endif


	fscanf(f_config, "%s = \" %s \"", param, output_w_file);
	#ifdef ASSERTION
	Assert(strcmp(param,"OUTPUT_FILE")==0 && "Wrong Parameter parsed.");
	#endif
	fscanf(f_config, "%s = \" %s \"", param, obj_r_file);
	#ifdef ASSERTION
	Assert(strcmp(param,"OBJ_FILE")==0 && "Wrong Parameter parsed.");
	#endif

	Assert(parsePinPointFiles(pinPoint_s_file, pinPoint_w_file) == true && "No simpoint data parsed.");
	verifyConfig();
}

void config::verifyConfig() {
	_g_var->msg.simStep("VERIFYING CONFIGURATIONS");
	Assert(num_mem_levels == MEM_HIGHERARCHY);
	for (int i = 0; i < MEM_HIGHERARCHY; i++) {
		Assert(cache_lat[i] > 0);}
	Assert(st_lat > 0 && st_lat <= cache_lat[0]);
	Assert(alu_count > 0);
	Assert(rob_size > 0);
	Assert(iwin_size > 0 && iwin_size <= rob_size);
	Assert(pb_win_count > 0);
	Assert(pb_win_size > 0);

	Assert(fetch_delay > 0);
	Assert(bp_delay > 0);
	Assert(decode_delay > 0);
	Assert(regren_delay > 0);
	Assert(issue_delay > 0);
	Assert(alu_delay > 0);
	Assert(complete_delay > 0);
	Assert(wb_delay > 0);
	Assert(fwd_delay > 0);

	Assert(bp_width > 0);
	Assert(decode_width > 0);
	Assert(issue_width > 0);
	Assert(dispatch_width > 0);
	Assert(commit_width > 0);
	Assert(squash_width > 0);
	Assert(wb_width > 0);
	Assert(pb_win_width > 0);
	Assert(regren_width > 0);
	Assert(fetch_width > 0);
}

// PRE:
//		pinPoint_file to exist
bool config::parsePinPointFiles(char* pinPoint_s_file, char* pinPoint_w_file) {
	_g_var->msg.simStep("PARSING PINPOINTS CONFIGURATIONS");
	//TODO check if the file exists - assert if not
	FILE* f_s_pinPoint = fopen(pinPoint_s_file, "r");
	FILE* f_w_pinPoint = fopen(pinPoint_w_file, "r");
	#ifdef ASSERTION
	Assert(f_s_pinPoint != NULL);
	Assert(f_w_pinPoint != NULL);
	#endif
	int s_id, w_id;
	SIMP s_val; 
	SIMW w_val;
	while (fscanf(f_s_pinPoint,"%lu %d\n", &s_val, &s_id) != EOF && 
		   fscanf(f_w_pinPoint,"%lf %d\n", &w_val, &w_id) != EOF) {
		#ifdef ASSERTION
		Assert(s_id == w_id && "simpoint and weight files mispatch.");
		#endif
		s_val = s_val * SIMP_WINDOW_SIZE;
		_simpoint.insert(std::pair<SIMP,SIMW>(s_val,w_val));
	}
	//#ifdef DBG
	cout << "** PinPoint Locations:\n";
	map<SIMP,SIMW>::iterator it;
	for (it = _simpoint.begin(); it != _simpoint.end(); it++) {
		cout << it->first << " " << it->second << endl;
	}
	//#endif
	return ((_simpoint.size() > 0) ? true : false);
}

char* config::getProgName() {
	return program_name;
}
