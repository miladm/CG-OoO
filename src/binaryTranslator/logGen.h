/*******************************************************************************
 *  logGen.h
 ******************************************************************************/

#ifndef _LOG_GEN_H
#define _LOG_GEN_H

#include <string>
#include "basicblock.h"

//Write basicbloc/phraseblock instructions in file
void writeToFile (List<basicblock*>*, string*, SCH_MODE, REG_ALLOC_MODE, CLUSTER_MODE, LENGTH);

#endif
