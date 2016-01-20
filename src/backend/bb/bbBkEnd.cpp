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
    WIDTH bpu_width, fch_width, dcd_width, sch_width, exe_width, mem_width, cmt_width;

    const YAML::Node& root = g_cfg->_root["cpu"]["backend"];
    const YAML::Node& bbWin = root["table"]["bbWindow"];
    const YAML::Node& pipe = root["bb_pipe"];
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

    bpu["width"] >> bpu_width; fch["width"] >> fch_width;
    dcd["width"] >> dcd_width; sch["width"] >> sch_width;
    exe["width"] >> exe_width; mem["width"] >> mem_width;
    cmt["width"] >> cmt_width;
    root["width"] >> width; bbWin["count"] >> num_bbW;
    WIDTH eu_width, eu_per_blk;
    root["eu"]["alu"]["count"] >> eu_width;
    root["eu"]["alu"]["count_per_blk"] >> eu_per_blk;

	g_bb_clk = new sysClock (1);
    _bb_core = new bb_sysCore (g_bb_clk, 
            bpu_width, fch_width, dcd_width, eu_width, eu_width, mem_width, cmt_width, num_bbW,
            fch_lat, 15, 
            1, 15, 
            bpu_lat, 15, 
            dcd_lat, 15, 
            sch_lat, eu_per_blk,
            1, 15, 
            exe_lat, 15, 
            1, 15, 
            1, 15, 
            1, 15);
}

void bbBkEnd_fini () {
    delete _bb_core;
    delete g_bb_clk;
}
