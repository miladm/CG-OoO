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

    g_ooo_clk = new sysClock (1);
    _ooo_core = new o3_sysCore (g_ooo_clk, 
            8*width, 8*width, 8*width, width, width, width, width,
            fch_lat, 50, 
            1, 50, 
            bpu_lat, 50, 
            dcd_lat - 1, 50, 
            sch_lat, 50, 
            1, 50, 
            exe_lat, 50, 
            1, 50, 
            1, 50, 
            1, 50);
}

void oooBkEnd_fini () {
    delete _ooo_core;
    delete g_ooo_clk;
}
