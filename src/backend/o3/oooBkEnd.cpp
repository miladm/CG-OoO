/*******************************************************************************
 * oooBkEnd.cpp
 *******************************************************************************/

#include "oooBkEnd.h"

o3_sysCore* _ooo_core;
sysClock* g_ooo_clk;

void oooBkEndRun(FRONTEND_STATUS frontend_status) {
    _ooo_core->runCore(frontend_status);
}

void oooBkEnd_init () {
    dbg.print ((DBG_LEVEL)0x1, "Initializing Backend"); //TODO fix this
    int width, bpu_lat, fch_lat, dcd_lat, sch_lat, exe_lat, mem_lat, cmt_lat;
    WIDTH bpu_width, fch_width, dcd_width, sch_width, exe_width, mem_width, cmt_width;

    const YAML::Node& root = g_cfg->_root["cpu"]["backend"];
    const YAML::Node& pipe = root["o3_pipe"];
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
    root["width"] >> width;
    WIDTH eu_width;
    root["eu"]["alu"]["count"] >> eu_width;

    g_ooo_clk = new sysClock (1);
    _ooo_core = new o3_sysCore (g_ooo_clk, 
            bpu_width, fch_width, dcd_width, sch_width, exe_width, mem_width, cmt_width,
            fch_lat, 15, 
            1, 15, 
            bpu_lat, 15, 
            dcd_lat, 15, 
            sch_lat, eu_width, 
            1, 15, 
            exe_lat, 15, 
            1, 15, 
            1, 15, 
            1, 15);
}

void oooBkEnd_fini () {
    delete _ooo_core;
    delete g_ooo_clk;
}
