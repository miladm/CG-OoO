/*******************************************************************************
 * oooLD_lsq_ctrl.h
 *
 * Load-Store Queuee Controller Unit
 * The API for interfacing with LSQ is provided here
 * This is Naive Implementation of a OOO-LD INO-ST LSQ Management Model
 * (i.e. no store set speculation)
 ******************************************************************************/

#ifndef _OOOLD_LSQ_CTRL_H
#define _OOOLD_LSQ_CTRL_H

#include "instruction.h"
#include "../global/global.h"
#include "lsq.h"

void oooLD_lsqEnque (lsq *oooLd_inoSt_LSQ, instruction *ins);
bool oooLD_lsqHazard (lsq *oooLd_inoSt_LSQ, instruction *ins);

void oooLD_insertSQ_addrNdata(lsq *oooLd_inoSt_LSQ, instruction *ins);
bool oooLD_insertLQ_addr(lsq *oooLd_inoSt_LSQ, instruction *ins);
void oooLD_insertLQ_data(lsq *oooLd_inoSt_LSQ, instruction *ins); //place holder

INS_ID oooLD_findLQviolation(lsq *oooLd_inoSt_LSQ, instruction *ins);
void oooLD_updateSQcommitSet(lsq *oooLd_inoSt_LSQ, instruction *ins);

void oooLD_lqDequ (lsq *oooLd_inoSt_LSQ, instruction *ins);
void oooLD_sqDequ (lsq *oooLd_inoSt_LSQ, int cycle);

int oooLD_squashSQ (lsq *oooLd_inoSt_LSQ, INS_ADDR insId);
int oooLD_squashLQ (lsq *oooLd_inoSt_LSQ, INS_ADDR insId);

#endif

