/*******************************************************************************
 * inoBkEnd.cpp
 *******************************************************************************/

#include "inoBkEnd.h"

sysCore* _core;
sysClock* g_ino_clk;

void inoBkEndRun(FRONTEND_STATUS frontend_status) {
    _core->runCore(frontend_status);
}

void inoBkEnd_init () {
    dbg.print ((DBG_LEVEL)0x1, "Initializing Backend"); //TODO fix this
    int width, bpu_lat, fch_lat, dcd_lat, sch_lat, exe_lat, mem_lat, cmt_lat;

    const YAML::Node& root = g_cfg->_root["cpu"]["backend"];
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
    root["width"] >> width;
    WIDTH eu_width;
    root["eu"]["alu"]["count"] >> eu_width;

    g_ino_clk = new sysClock (1);
    _core = new sysCore (g_ino_clk,
            width, width, width, eu_width, eu_width, width, width,
            fch_lat, 50, 
            1, 50, 
            bpu_lat, 50, 
            dcd_lat - 1, 50, 
            1, eu_width,  //TODO sch_lat is replaced here - take care of it.
            1, 50, 
            exe_lat, 50, 
            1, 50, 
            1, 50, 
            1, 50);
}

void inoBkEnd_fini () {
    delete _core;
    delete g_ino_clk;
}
