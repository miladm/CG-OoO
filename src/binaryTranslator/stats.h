/*******************************************************************************
 *  stats.h
 ******************************************************************************/

#ifndef _STATS_H
#define _STATS_H

struct block_stats {
    public:
        int length;
        int width;
        int upld_cnt;
        int upld_dep_cnt;
        int upld_n_dep_cnt;
//        int wbb_cnt;
};


#endif
