/*******************************************************************************
 *  redundancy_elim.cpp
 ******************************************************************************/

#include "redundancy_elim.h"

/* MOV OPS WITH THEIR WRITE OPERAND OVERWRITTEN IN THE SAME BLK */
static void overwrittenMovOpElim (List<basicblock*>* bbList, SCH_MODE sch_mode) {
    long int num_dead_mov_op = 0;
	for (int i = 0; i < bbList->NumElements (); i++) {
		basicblock* bb = bbList->Nth (i);
        num_dead_mov_op += bb->overwrittenMovOpElim (sch_mode);
	}
	printf ("\tNumber of overwritten MOV ops eliminated: %ld\n", num_dead_mov_op);
}



/* MOV OPS WITHOUT USERS */
static void deadMovOpElim (List<basicblock*>* bbList, SCH_MODE sch_mode) {
    long int num_dead_mov_op = 0;
	for (int i = 0; i < bbList->NumElements (); i++) {
		basicblock* bb = bbList->Nth (i);
        num_dead_mov_op += bb->deadMovOpElim (sch_mode);
	}
	printf ("\tNumber of dead MOV ops eliminated: %ld\n", num_dead_mov_op);
}

/* MOV OPS WITH IDENTICAL SORUCE AND DESTINOATION REGISTERS */
static void redundantMovOpElim (List<basicblock*>* bbList, SCH_MODE sch_mode) {
    long int num_redundant_mov_op = 0;
	for (int i = 0; i < bbList->NumElements (); i++) {
		basicblock* bb = bbList->Nth (i);
        num_redundant_mov_op += bb->redundantMovOpElim (sch_mode);
	}
	printf ("\tNumber of redundant MOV ops eliminated: %ld\n", num_redundant_mov_op);
}

void movOpElimination (List<basicblock*>* bbList, SCH_MODE sch_mode) {
    redundantMovOpElim (bbList, sch_mode);
    deadMovOpElim (bbList, sch_mode);
    overwrittenMovOpElim (bbList, sch_mode);
}
