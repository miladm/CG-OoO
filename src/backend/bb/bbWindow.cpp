/*******************************************************************************
 * bbWindow.cpp
 ******************************************************************************/

#include "bbWindow.h"

bbWindow::bbWindow (string bbWin_id, sysClock* clk)
    : unit ("bbWindow_" + bbWin_id, clk),
      _win (100, 8, 8, clk, "bbWindow_" + bbWin_id), //TODO configure and more real numbers
      _LRF_MGR (clk, "lrfManager_" + bbWin_id),
      _id (atoi (bbWin_id.c_str ()))
{ }
