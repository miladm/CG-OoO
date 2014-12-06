/*******************************************************************************
 *  logGen.h
 ******************************************************************************/

#ifndef _LOG_GEN_H
#define _LOG_GEN_H

#include <string>
#include "basicblock.h"

//Write basicbloc/phraseblock instructions in file
void writeToFile (List<basicblock*> *bbList, string *program_name, SCH_MODE sch_mode, REG_ALLOC_MODE reg_alloc_mode, CLUSTER_MODE cluster_mode);

#endif
