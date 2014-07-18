/*******************************************************************************
 *  registerRename.h
 ******************************************************************************/

#ifndef _REGISTERZ_RENAME_H
#define _REGISTERZ_RENAME_H

#include "global.h"
#include "list.h"

void renameWriteReg (long int reg);
void renameReadReg (int indx, long int renReg);
long int getRenamedReg (long int reg);

#endif