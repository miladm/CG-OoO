/*******************************************************************************
 * bbWindow.h
 *******************************************************************************
 * This is the basic-block window hardware unit.
 ******************************************************************************/

#ifndef _BB_WINDOW_H
#define _BB_WINDOW_H

#include "../unit/unit.h"
#include "../unit/table.h"
#include "lrfManager.h"

class bbWindow : public unit {
    public:
        bbWindow (string bbWin_id, sysClock* clk);
        FIFOtable<dynInstruction*> _win;
        bb_lrfManager _LRF_MGR;
        const WIDTH _id;
};

#endif
