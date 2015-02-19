/*******************************************************************************
 * bbStat.h
 ******************************************************************************/

#ifndef _BBSTAT_H
#define _BBSTAT_H

#include "unit.h"

class bbStat : public unit {
    public:
        bbStat () 
            : unit ("bbStat"),
            s_missing_dynIns_in_stList_cnt (g_stats.newScalarStat ("dynBasicblock", "missing_dynIns_in_stList_cnt", "Number of dynamic instructions missing in the static ins list.", 0, NO_PRINT_ZERO)),
            s_missing_stIns_in_dynList_cnt (g_stats.newScalarStat ("dynBasicblock", "missing_stIns_in_dynList_cnt", "Number of static instructions missing in the dynamic ins list.", 0, NO_PRINT_ZERO)),
            s_dynBB_without_stBB_cnt (g_stats.newScalarStat ("dynBasicblock", "dynBB_without_stBB_cnt", "Number of dynamic BB without a static representation.", 0, NO_PRINT_ZERO)),
            s_block_without_stBB_cnt (g_stats.newScalarStat ("dynBasicblock", "block_without_stBB_cnt", "Number of dynamic blocks without a static representation.", 0, NO_PRINT_ZERO))
    { }
        ~bbStat () {}
        ScalarStat& s_missing_dynIns_in_stList_cnt;
        ScalarStat& s_missing_stIns_in_dynList_cnt;
        ScalarStat& s_dynBB_without_stBB_cnt;
        ScalarStat& s_block_without_stBB_cnt;
};

extern bbStat* g_bbStat;

#endif
