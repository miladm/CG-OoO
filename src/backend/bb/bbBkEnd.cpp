/*******************************************************************************
 * bbBkEnd.cpp
 *******************************************************************************/

#include "bbBkEnd.h"

bb_sysCore* _bb_core;
sysClock* g_bb_clk;

void bbBkEndRun (FRONTEND_STATUS frontend_status) {
    _bb_core->runCore(frontend_status);
}

void bbBkEnd_init () {
    dbg.print ((DBG_LEVEL)0x1, "Initializing Backend"); //TODO fix this

    WIDTH width, num_bbW;
    CYCLE bpu_lat, fch_lat, dcd_lat, sch_lat, exe_lat, mem_lat, cmt_lat;

    const YAML::Node& root = g_cfg->_root["cpu"]["backend"];
    const YAML::Node& bbWin = root["table"]["bbWindow"];
    const YAML::Node& pipe = root["pipe"];
    const YAML::Node& bpu = pipe["bp"];
    const YAML::Node& fch = pipe["fetch"];
    const YAML::Node& dcd = pipe["decode"];
    const YAML::Node& sch = pipe["schedule"];
    const YAML::Node& exe = pipe["execution"];
    const YAML::Node& mem = pipe["memory"];
    const YAML::Node& cmt = pipe["commit"];

    bpu["latency"] >> bpu_lat; fch["latency"] >> fch_lat;
    dcd["latency"] >> dcd_lat; sch["latency"] >> sch_lat;
    exe["latency"] >> exe_lat; mem["latency"] >> mem_lat;
    cmt["latency"] >> cmt_lat;
    root["width"] >> width; bbWin["count"] >> num_bbW;
    WIDTH eu_width;
    root["eu"]["alu"]["count"] >> eu_width;

	g_bb_clk = new sysClock (1);
    _bb_core = new bb_sysCore (g_bb_clk, 
            8*width, 8*width, 8*width, width, eu_width, width, width, num_bbW,
            fch_lat, 500, 
            1, 500, 
            bpu_lat, 500, 
            dcd_lat - 1, 500, 
            sch_lat, 500, 
            1, 500, 
            exe_lat, 500, 
            1, 500, 
            1, 500, 
            1, 500);
}

void bbBkEnd_fini () {
    delete _bb_core;
    delete g_bb_clk;
}
