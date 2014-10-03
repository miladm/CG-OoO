/*******************************************************************************
 * exeUnit.h
 ******************************************************************************/

#ifndef _EXE_UNIT_H
#define _EXE_UNIT_H

#include "unit.h"
#include "dynInstruction.h"

typedef enum {GENERIC_EU, ALU_EU, FPU_EU, MEM_EU, BRU_EU} EU_TYPE;
typedef enum {AVAILABLE_EU, RUNNING_EU, COMPLETE_EU} EU_STATE;

struct exeUnitLat {
    exeUnitLat () 
        : _alu_lat (1),
          _fpu_lat (5),
          _bru_lat (1),
          _st_buff_lat (1),
          _l1_lat (1),
          _l2_lat (1),
          _l3_lat (1),
          _mem_lat (1)
    { }

    const CYCLE _alu_lat;
    const CYCLE _fpu_lat;
    const CYCLE _bru_lat;
    const CYCLE _st_buff_lat;
    const CYCLE _l1_lat;
    const CYCLE _l2_lat;
    const CYCLE _l3_lat;
    const CYCLE _mem_lat;
};

struct exeUnit {
    exeUnit (CYCLE start, CYCLE latency, EU_TYPE eu_type) 
        : _eu_timer (start, latency),
          _eu_type (eu_type)
    {
        _eu_ins = NULL;
        _eu_state = AVAILABLE_EU;
    }
    EU_STATE getEUstate (CYCLE now, bool limit_eu_state_access) {
        if (!limit_eu_state_access) {
            return _eu_state;
        } else {
            if (_eu_state == RUNNING_EU && now >= _eu_timer.getStopTime ()) {
                _eu_state = COMPLETE_EU;
            } else if (_eu_state == RUNNING_EU) {
                _eu_state = RUNNING_EU;
            } else if (_eu_state == COMPLETE_EU) {
                _eu_state = AVAILABLE_EU;
            } else if (_eu_state == AVAILABLE_EU) {
                _eu_state = AVAILABLE_EU;
            } else {
                Assert (true == false && "invalid EU state");
            }
            return _eu_state;
        }
    }
    void runEU () {
        Assert (_eu_state == AVAILABLE_EU);
        _eu_state = RUNNING_EU;
    }
    void setEUins (dynInstruction* ins) { Assert (_eu_ins == NULL); _eu_ins = ins;}
    dynInstruction* getEUins () {return _eu_ins;}
    void resetEU () {_eu_ins = NULL; _eu_state = AVAILABLE_EU;}

    public:
        timer _eu_timer;

    private:
        const EU_TYPE _eu_type;
        EU_STATE _eu_state;
        dynInstruction* _eu_ins;
};

extern exeUnitLat g_eu_lat;

#endif
