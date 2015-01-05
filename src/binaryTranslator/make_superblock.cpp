/*******************************************************************************
 *  make_superblock.cpp
 ******************************************************************************/

#include "make_superblock.h"

basicblock* form_superblock (basicblock* bb, basicblock* dst_bb) {
    List<instruction*>* insList = bb->getInsList ();
    List<instruction*>* dst_insList = dst_bb->getInsList ();
    for (int i = 0; i < dst_insList->NumElements (); i++) {
        instruction* ins = dst_insList->Nth(i);
        ins->resetMy_BBorPB_id ();
        bb->addIns (ins, NO_BR_DST);
    }

    bb->resetFallThrough (); /* THIS MUST BE UNNECESSARY */
    bb->resetTakenTarget ();
    bb->resetDescendents ();
    if (dst_bb->hasFallThrough ()) {
        basicblock* fall_thru_bb = dst_bb->getFallThrough ();
        bb->setFallThrough (fall_thru_bb);
        bb->setDescendent (fall_thru_bb);
    } 
    if (dst_bb->hasTakenTarget ()) {
        basicblock* target_bb = dst_bb->getTakenTarget ();
        bb->setTakenTarget (target_bb);
        bb->setDescendent (target_bb);
    }

    if (dst_bb->hasHeader ()) bb->setBBbrHeader (dst_bb->getBBbrHeader ());
    else bb->resetBBbrHeader ();
}

void make_superblock (List<instruction*> *insList,
		      		  List<basicblock*> *bbList,
					  map<int,variable*> &varList, 
		      		  set<ADDR> *brDstSet,
		      		  map<ADDR, basicblock*> *bbMap,
					  map<ADDR,instruction*> *insAddrMap,
                      LENGTH cluster_size,
                      SCH_MODE sch_mode) {
    set<ADDR> toRemoveBB;
    for (int i = 0; i < bbList->NumElements (); i++) {
        basicblock* bb = bbList->Nth(i);
        instruction* last_ins = bb->getLastIns();
        set<ADDR> superblockSet;
        superblockSet.insert (bb->getID ());
        LABEL1:
        if (toRemoveBB.find (bb->getID ()) != toRemoveBB.end ()) continue;
        if (last_ins->getType () == 'j' && last_ins->hasDst ()) {
            instruction* dst_ins = last_ins->getInsDst ();
            ADDR dst_bb_id = dst_ins->getMy_BB_id ();
            if (bbMap->find (dst_bb_id) == bbMap->end ()) { cout << "DST BB not found" << endl; continue; }
            if (superblockSet.find (dst_bb_id) != superblockSet.end ()) { cout << "avliding loop" << endl; continue; }
            if (toRemoveBB.find (bb->getID ()) != toRemoveBB.end ()) { cout << "to remove" << endl; continue; }
            if (dst_bb_id == bb->getID ()) { cout << "self loop" << endl; continue; }
            basicblock* dst_bb = (*bbMap)[dst_bb_id];
            form_superblock (bb, dst_bb);
            superblockSet.insert (dst_bb_id);
            toRemoveBB.insert(dst_bb_id);
            goto LABEL1;
        }
    }

    /* REMOVE THE BB'S THAT ARE MERGED WITH OTHERS - TEMEPRARY CODE */
    for (int i = bbList->NumElements () - 1; i >= 0; i--) {
        basicblock* bb = bbList->Nth(i);
        ADDR bb_addr = bb->getID ();
        if (toRemoveBB.find (bb_addr) != toRemoveBB.end ())
            bbList->RemoveAt (i);
    }

	/* PERFORM LIST SCHEDULING ON BB */
    if (sch_mode == LIST_SCH) {
        printf ("\tSetup Dependency Table\n");
        dependencySetup (bbList);
        printf ("\tRun List Scheduling\n");
        for  (int i = 0; i < bbList->NumElements (); i++) {
            basicblock* bb = bbList->Nth (i);
            listSchedule (bb); //why does this affect BB structure?
        }
    }
}

//WHAT TO DO WITH MULTIPLE JUMPS TO THE SAME CODE?
