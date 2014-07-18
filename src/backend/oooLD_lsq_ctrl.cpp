/*******************************************************************************
 * oooLD_lsq_ctrl.cpp
 *
 * Load-Store Queuee Controller Unit
 * The API for interfacing with LSQ is provided here
 * This is Naive Implementation of a OOO-LD INO-ST LSQ Management Model
 * (i.e. no store set speculation)
 ******************************************************************************/

#include "oooLD_lsq_ctrl.h"

/* Enque into both LQ and SQ */
void oooLD_lsqEnque (lsq *oooLd_inoSt_LSQ, instruction *ins) {
	if (ins->getMemType() == READ) {
		Assert (!oooLd_inoSt_LSQ->isLQfull());
		oooLd_inoSt_LSQ->pushBackLQ(ins);
	} else if (ins->getMemType() == WRITE) {
		Assert(!oooLd_inoSt_LSQ->isSQfull());
		oooLd_inoSt_LSQ->pushBackSQ(ins);
	}
}

/* Test if LQ/SQ is full */
bool oooLD_lsqHazard (lsq *oooLd_inoSt_LSQ, instruction *ins) {
	if (ins->getMemType() == READ) {
		if (oooLd_inoSt_LSQ->isLQfull()) {
			//printf("full LQ\n");
			return true; //STALL FETCH
		}
	} else if (ins->getMemType() == WRITE) {
		if (oooLd_inoSt_LSQ->isSQfull()) {
			//printf("full SQ\n");
			return true; //STALL FETCH
		}
	}
	return false;
}

/* SQ Data Forwarding to LD op */
bool oooLD_MemForwarding (lsq* oooLd_inoSt_LSQ, instruction *ins) {
	Assert(ins->getMemType() == READ);
	bool isMemAddrInSQ = oooLd_inoSt_LSQ->lookupSQ(ins);
	if (isMemAddrInSQ) {
		//Forward value
		oooLd_inoSt_LSQ->setForwardData(ins);
		return true;
		//TODO make sure cache is no longer accessed + latency is properly set
	} else {
		return false;
		//lookup cache
		//TODO
	}
}

/* Insert Data (not modeled) & Address into SQ */
void oooLD_insertSQ_addrNdata(lsq *oooLd_inoSt_LSQ, instruction *ins) {
	Assert(ins->getMemType() == WRITE);
	oooLd_inoSt_LSQ->insertSQ_addrNdata(ins);
}

/* Insert Address into LQ */
bool oooLD_insertLQ_addr(lsq *oooLd_inoSt_LSQ, instruction *ins) {
	Assert(ins->getMemType() == READ);
	oooLd_inoSt_LSQ->insertLQ_addr(ins);
	return oooLD_MemForwarding (oooLd_inoSt_LSQ, ins);
}

/* Insert Data into LQ */
void oooLD_insertLQ_data(lsq *oooLd_inoSt_LSQ, instruction *ins) {
	Assert(ins->getMemType() == READ);
	//not modeled - placeholder
}

/* Squash both SQ Instructions */
int oooLD_squashSQ (lsq *oooLd_inoSt_LSQ, INS_ADDR insId) {
	return oooLd_inoSt_LSQ->squashSQ(insId);
}

/* Squash both LQ Instructions */
int oooLD_squashLQ (lsq *oooLd_inoSt_LSQ, INS_ADDR insId) {
	return oooLd_inoSt_LSQ->squashLQ(insId);
}

/* Check LQ to detect Address Aliasigin Violation */
//Finds the LD instruction that has failed.
//Squash the pipeline from that instruction onwards
INS_ID oooLD_findLQviolation(lsq *oooLd_inoSt_LSQ, instruction *ins) {
	Assert(ins->getMemType() == WRITE);
	Assert(ins->getStatus() == complete);
	INS_ID id = oooLd_inoSt_LSQ->lookupLQ(ins);
	return id;
}

/* Advance the commit pointer to the next ST op
 * NOTE: here I 'search' to find the ST op ID to avoid queue walk bugs
 */
void oooLD_updateSQcommitSet(lsq *oooLd_inoSt_LSQ, instruction *ins) {
	Assert(ins->getMemType() == WRITE);
	oooLd_inoSt_LSQ->updateSQcommitSet(ins);
}

/* Deque Load op from LQ */
void oooLD_lqDequ (lsq *oooLd_inoSt_LSQ, instruction *ins) {
	Assert(ins->getStatus() == complete);
	if (oooLd_inoSt_LSQ->isLQempty()) {
		printf("ERROR: LQ element not found!\n"); //Assert(oooLd_inoSt_LSQ->isLQempty() == false);
		return;
	}
	Assert(oooLd_inoSt_LSQ->getLQheadId() == ins->getInsID());
	oooLd_inoSt_LSQ->popFrontLQ();
}

/* Deque Store op from SQ */
void oooLD_sqDequ (lsq *oooLd_inoSt_LSQ, int cycle) {
	//corresponding ins is commited already and non-existant in ROB
	//Don't access ins element of SQ struct here (the obj is deleted already)
	if (oooLd_inoSt_LSQ->isSQempty()) return;
	//DeQueue as many ST operations as possible (TODO makes sense in hardware?)
	while (!oooLd_inoSt_LSQ->isSQempty() && oooLd_inoSt_LSQ->isFrontSQdoneWritingCache(cycle)) {
		oooLd_inoSt_LSQ->popFrontSQ();
	}
}
