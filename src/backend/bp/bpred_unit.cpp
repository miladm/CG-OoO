/*
 * Copyright (c) 2004-2005 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Kevin Lim
 */

#include <algorithm>

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "arch/utility.hh"
#include "base/trace.hh"
#include "config/the_isa.hh"
#include "cpu/o3/bpred_unit.hh"
#include "debug/Fetch.hh"
#include "params/DerivO3CPU.hh"
#include <stdio.h>

template<class Impl>
BPredUnit<Impl>::BPredUnit (DerivO3CPUParams *params)
    : _name (params->name + ".BPredUnit"),
      BTB (params->BTBEntries,
          params->BTBTagSize,
          params->instShiftAmt)
{
    // Setup the selected predictor.
    if (params->predType == "local") {
        localBP = new LocalBP (params->localPredictorSize,
                              params->localCtrBits,
                              params->instShiftAmt);
        predictor = Local;
    } else if (params->predType == "hybrid") {
        hybridbp = new HybridBP (params->localPredictorSize,
                                        params->localCtrBits,
                                        params->globalPredictorSize,
                                        params->globalHistoryBits,
                                        params->globalCtrBits,
                                        params->choicePredictorSize,
                                        params->choiceCtrBits,
                                        params->instShiftAmt,
										params->fetchWidth);
        predictor = Hybrid;
    } else if (params->predType == "hybrid_skew") {
        hybridSkewbp = new HybridBPskew (params->localPredictorSize,
                                        params->localCtrBits,
                                        params->globalPredictorSize,
                                        params->globalHistoryBits,
                                        params->globalCtrBits,
                                        params->choicePredictorSize,
                                        params->choiceCtrBits,
                                        params->instShiftAmt,
										params->fetchWidth);
        predictor = HybridSkew;
    } else {
        fatal ("Invalid BP selected!");
    }

    for (int i=0; i < Impl::MaxThreads; i++)
        RAS[i].init (params->RASSize);
	
	fetchWidth = params->fetchWidth;
	onDemand = params->onDemand;
	std::cout << "ODBP flag = " << params->onDemand << "\n";

	instShiftAmt = params->instShiftAmt;
	totalInstShiftAmt = instShiftAmt + (unsigned)log2 (fetchWidth);
}

template <class Impl>
void
BPredUnit<Impl>::regStats ()
{
    lookups
        .name (name () + ".lookups")
        .desc ("Number of BP lookups only")
        ;

    lookup_and_updates
        .name (name () + ".lookup_and_updates")
        .desc ("Number of BP lookups (read and update when \"squash\" or \"update\")")
        ;

    condPredicted
        .name (name () + ".condPredicted")
        .desc ("Number of conditional branches predicted")
        ;

    condIncorrect
        .name (name () + ".condIncorrect")
        .desc ("Number of conditional branches incorrect")
        ;

    BTBLookups
        .name (name () + ".BTBLookups")
        .desc ("Number of BTB lookups (read and update when squash)")
        ;

    BTBHits
        .name (name () + ".BTBHits")
        .desc ("Number of BTB hits")
        ;

    BTBCorrect
        .name (name () + ".BTBCorrect")
        .desc ("Number of correct BTB predictions (this stat may not "
              "work properly.")
        ;

    usedRAS
        .name (name () + ".usedRAS")
        .desc ("Number of times the RAS was used to get a target.")
        ;

    RASIncorrect
        .name (name () + ".RASInCorrect")
        .desc ("Number of incorrect RAS predictions.")
        ;

    DY_treated_SN
        .name (name () + ".DY_treated_SN")
        .desc ("Number of DY annotations turned into SN - ODBP")
        ;

    ST_treated_SN
        .name (name () + ".ST_treated_SN")
        .desc ("Number of DY annotations turned into SN - ODBP")
        ;

    DY_ST_treated_SN
        .name (name () + ".DY_ST_treated_SN")
        .desc ("Number of DY annotations turned into SN - ODBP")
        ;
	DY_ST_treated_SN = ST_treated_SN + DY_treated_SN;

  //localBrPredCnt
  //    .name (name () + ".localBrPredCnt")
  //    .desc ("Local BP Lookup")
  //    ;

  //globalBrPredCnt
  //    .name (name () + ".globalBrPredCnt")
  //    .desc ("Global BP Lookup")
  //    ;
  //bpIndxHist
	//	.init (0)
	//	.name (name () + ".bpIndxHist")
	//	.desc ("bpIndxHist")
	//	.prereq (bpIndxHist);
}

template <class Impl>
void
BPredUnit<Impl>::switchOut ()
{
    // Clear any state upon switch out.
    for (int i = 0; i < Impl::MaxThreads; ++i) {
        squash (0, i);
    }
}

template <class Impl>
void
BPredUnit<Impl>::takeOverFrom ()
{
    // Can reset all predictor state, but it's not necessarily better
    // than leaving it be.
/*
    for (int i = 0; i < Impl::MaxThreads; ++i)
        RAS[i].reset ();

    BP.reset ();
    BTB.reset ();
*/
}
template <class Impl>
void
BPredUnit<Impl>::increase_BP_lookup (int lookupCnt)
{
  lookups += lookupCnt;
}
template <class Impl>
void
BPredUnit<Impl>::increase_BP_lookup_and_update (int lookupCnt)
{
  lookup_and_updates += lookupCnt;
}
template <class Impl>
void
BPredUnit<Impl>::increase_BTB_lookup ()
{
  BTBLookups++;
  usedRAS++;
}

template <class Impl>
bool
BPredUnit<Impl>::predict (DynInstPtr &inst, TheISA::PCState &pc, ThreadID tid, bool odbpStatic, bool predTaken, int positionInFetchGroup, int positionInLine, bool &allPredStatic, int oracleBP, bool onlyLP)
{
    // See if branch predictor predicts taken.
    // If so, get its target addr either from the BTB or the RAS.
    // Save off record of branch stuff so the RAS can be fixed
    // up once it's done.

    bool pred_taken = false;
    TheISA::PCState target = pc;

    void *bp_history = NULL;
	static bool firstDY = true;
	if (onDemand && positionInFetchGroup == 0)
		firstDY = true;

    //if (inst->isUncondCtrl ())
    //{
    //    DPRINTF (Fetch, "BranchPred: [tid:%i]: Unconditional control.\n", tid);
    //    pred_taken = true;
    //    // Tell the BP there was an unconditional branch.
    //    BPUncond (bp_history);
    //}
    //else
    //{
      ++condPredicted;
		bool choicePred;
        if (!onDemand) { //BASEBP
			//Addr indx = ((pc.instAddr ()>>4)<<2)+positionInLine;
			//if (bpIndxHist.find (indx) != bpIndxHist.end ()) { bpIndxHist[indx]++;
			//} else { bpIndxHist[indx] = 1; }
			pred_taken = BPLookup (pc.instAddr (), bp_history, positionInLine, onDemand, choicePred, onlyLP);
			DPRINTF (Fetch, "BranchPred: [tid:%i]: Branch predictor predicted %i for PC %s\n", tid, pred_taken, inst->pcState ());
			if (positionInFetchGroup == 0) {
				increase_BP_lookup (fetchWidth);
				increase_BP_lookup_and_update (fetchWidth);
				BTBLookups+=fetchWidth;
			}
			//inst->setWasBPlookup (true); TODO put this back
			inst->setDyPredUsed (choicePred);
        } else { //ODBP
			if (odbpStatic)
			{
			    pred_taken = predTaken;
				if (pred_taken) {
					//hybridbp->updateGlobalHistTaken ();
					BTBLookups++;
				//} else {
					//Addr tempPC = ((pc.instAddr ()>>totalInstShiftAmt)<<totalInstShiftAmt)+ ((Addr)pow (instShiftAmt,2))* (fetchWidth-1); //Create the addr of last ins in cacheline
					//if (pc.instAddr () == tempPC)
					//hybridbp->updateGlobalHistNotTaken ();
				}
				noBPLookup (pc.instAddr (), bp_history, positionInLine);
				inst->setWasBPlookup (false);
			}
			else if (!odbpStatic)
			{
				//Addr indx = ((pc.instAddr ()>>4)<<2)+positionInLine;
				//if (bpIndxHist.find (indx) != bpIndxHist.end ()) { bpIndxHist[indx]++;
				//} else { bpIndxHist[indx] = 1; }
			    pred_taken = BPLookup (pc.instAddr (), bp_history, positionInLine, onDemand, choicePred, onlyLP);
				allPredStatic = false; //had at least one DY in fetch group
			    DPRINTF (Fetch, "BranchPred: [tid:%i]: Branch predictor predicted %i for PC %s\n", tid, pred_taken, inst->pcState ());
				if (firstDY) {
					increase_BP_lookup (fetchWidth);
					increase_BP_lookup_and_update (fetchWidth);
					firstDY=false;
				}
				BTBLookups++;
				inst->setWasBPlookup (true);
				inst->setDyPredUsed (choicePred);
			} else {
				assert (true && "this should have never run");
			    pred_taken = false; //Turn DY or ST into SN - no update of PHT here
				noBPLookup (pc.instAddr (), bp_history, positionInLine);
				if (odbpStatic && predTaken)	ST_treated_SN++;
				else if (!odbpStatic)			DY_treated_SN++;
				inst->setWasBPlookup (false);
			}
        }
    //}

    DPRINTF (Fetch, "BranchPred: [tid:%i]: [sn:%i] Creating prediction history "
                "for PC %s\n",
            tid, inst->seqNum, inst->pcState ());

    PredictorHistory predict_record (inst->seqNum, pc.instAddr (),
                                    pred_taken, bp_history, tid);
	if (onDemand) {
		predict_record.wasControl = inst->isControl ();
		predict_record.wasStatic = odbpStatic;
		predict_record.wasST = predTaken; //This is the "original" static prediction
	}

    // Now lookup in the BTB or RAS.
    if (pred_taken)
    {
        ++usedRAS; //RAS read or write
        if (inst->isReturn ()) //look up RAS
        {
            predict_record.wasReturn = true;
            // If it's a function return call, then look up the address
            // in the RAS.
            TheISA::PCState rasTop = RAS[tid].top ();
            target = TheISA::buildRetPC (pc, rasTop);

            // Record the top entry of the RAS, and its index.
            predict_record.usedRAS = true;
            predict_record.RASIndex = RAS[tid].topIdx ();
            predict_record.RASTarget = rasTop;

            assert (predict_record.RASIndex < 16);

            RAS[tid].pop ();

            DPRINTF (Fetch, "BranchPred: [tid:%i]: Instruction %s is a return, "
                    "RAS predicted target: %s, RAS index: %i.\n",
                    tid, inst->pcState (), target, predict_record.RASIndex);
        }
        else //Look up BTB
        {
            if (inst->isCall ()) {
                RAS[tid].push (pc);

                // Record that it was a call so that the top RAS entry can
                // be popped off if the speculation is incorrect.
                predict_record.wasCall = true;

                DPRINTF (Fetch, "BranchPred: [tid:%i]: Instruction %s was a "
                        "call, adding %s to the RAS index: %i.\n",
                        tid, inst->pcState (), pc, RAS[tid].topIdx ());
            }

            if (BTB.valid (pc.instAddr (), tid)) {
                ++BTBHits;
                predict_record.validBTB = true;

                // If it's not a return, use the BTB to get the target addr.
                target = BTB.lookup (pc.instAddr (), tid);

                DPRINTF (Fetch, "BranchPred: [tid:%i]: Instruction %s predicted"
                        " target is %s.\n", tid, inst->pcState (), target);

            } else { //missed in RAS and BTB
                DPRINTF (Fetch, "BranchPred: [tid:%i]: BTB doesn't have a "
                        "valid entry.\n",tid);
                pred_taken = false;
                // The Direction of the branch predictor is altered because the
                // BTB did not have an entry
                // The predictor needs to be updated accordingly
              //if (!inst->isCall () && !inst->isReturn ()) {
                      BPBTBUpdate (pc.instAddr (), bp_history);
                      DPRINTF (Fetch, "BranchPred: [tid:%i]:[sn:%i] BPBTBUpdate"
                              " called for %s\n",
                              tid, inst->seqNum, inst->pcState ());
              //} else if (inst->isCall () && !inst->isUncondCtrl ()) {
              //      RAS[tid].pop ();
              //}
                TheISA::advancePC (target, inst->staticInst);
            }

        }
    } else { //predict not taken
        if (inst->isReturn ()) {
           predict_record.wasReturn = true;
        }
        TheISA::advancePC (target, inst->staticInst);
    }

	predict_record.predTaken = pred_taken;
    pc = target;

    predHist[tid].push_front (predict_record);

    DPRINTF (Fetch, "BranchPred: [tid:%i]: [sn:%i]: History entry added."
            "predHist.size (): %i\n", tid, inst->seqNum, predHist[tid].size ());

    return pred_taken;
}

template <class Impl>
void
BPredUnit<Impl>::update (const InstSeqNum &done_sn, ThreadID tid)
{
    DPRINTF (Fetch, "BranchPred: [tid:%i]: Committing branches until "
            "[sn:%lli].\n", tid, done_sn);

    while (!predHist[tid].empty () &&
           predHist[tid].back ().seqNum <= done_sn) {
        // Update the branch predictor with the correct results.
		DPRINTF (Fetch, "BranchPred: [tid:%i]: Committing branch "
		        "[sn:%lli].\n", tid, predHist[tid].back ().seqNum);
        if (!onDemand || 
	        (onDemand && 
	    	 !predHist[tid].back ().wasStatic &&
	    	 predHist[tid].back ().wasControl)) {
	    	BPUpdate (predHist[tid].back ().pc,
        	         predHist[tid].back ().predTaken,
        	         predHist[tid].back ().bpHistory, false);
	    }
		BPHistUpdate (predHist[tid].back ().pc, //should not do anything
    	         predHist[tid].back ().predTaken,
    	         predHist[tid].back ().bpHistory, false);

        predHist[tid].pop_back ();
    }
    DPRINTF (Fetch, "BranchPred: [tid:%i]: Done Committing branches\n", tid);

}

template <class Impl>
void
BPredUnit<Impl>::squash (const InstSeqNum &squashed_sn, ThreadID tid)
{
    History &pred_hist = predHist[tid];

    while (!pred_hist.empty () &&
           pred_hist.front ().seqNum > squashed_sn) {
        if (pred_hist.front ().usedRAS) {
            DPRINTF (Fetch, "BranchPred: [tid:%i]: Restoring top of RAS to: %i,"
                    " target: %s.\n", tid,
                    pred_hist.front ().RASIndex, pred_hist.front ().RASTarget);

            RAS[tid].restore (pred_hist.front ().RASIndex,
                             pred_hist.front ().RASTarget);
        } else if (pred_hist.front ().wasCall && pred_hist.front ().validBTB) {
            // Was a call but predicated false. Pop RAS here
            DPRINTF (Fetch, "BranchPred: [tid: %i] Squashing"
                    "  Call [sn:%i] PC: %s Popping RAS\n", tid,
                    pred_hist.front ().seqNum, pred_hist.front ().pc);
            RAS[tid].pop ();
        }

        // This call should delete the bpHistory.
        BPSquash (pred_hist.front ().bpHistory);

        DPRINTF (Fetch, "BranchPred: [tid:%i]: Removing history for [sn:%i] "
                "PC %s.\n", tid, pred_hist.front ().seqNum,
                pred_hist.front ().pc);

        pred_hist.pop_front ();

        DPRINTF (Fetch, "[tid:%i]: predHist.size (): %i\n",
                tid, predHist[tid].size ());
    }

}

template <class Impl>
void
BPredUnit<Impl>::squash (const InstSeqNum &squashed_sn,
                        const TheISA::PCState &corrTarget,
                        bool actually_taken,
                        ThreadID tid)
{
    // Now that we know that a branch was mispredicted, we need to undo
    // all the branches that have been seen up until this branch and
    // fix up everything.
    // NOTE: This should be call conceivably in 2 scenarios:
    // (1) After an branch is executed, it updates its status in the ROB
    //     The commit stage then checks the ROB update and sends a signal to
    //     the fetch stage to squash history after the mispredict
    // (2) In the decode stage, you can find out early if a unconditional
    //     PC-relative, branch was predicted incorrectly. If so, a signal
    //     to the fetch stage is sent to squash history after the mispredict

    History &pred_hist = predHist[tid];

    ++condIncorrect;

    DPRINTF (Fetch, "BranchPred: [tid:%i]: Squashing from sequence number %i, "
            "setting target to %s.\n",
            tid, squashed_sn, corrTarget);

    // Squash All Branches AFTER this mispredicted branch
    squash (squashed_sn, tid);

    // If there's a squash due to a syscall, there may not be an entry
    // corresponding to the squash.  In that case, don't bother trying to
    // fix up the entry.
    if (!pred_hist.empty ()) {

        HistoryIt hist_it = pred_hist.begin ();
        //HistoryIt hist_it = find (pred_hist.begin (), pred_hist.end (),
        //                       squashed_sn);

        //assert (hist_it != pred_hist.end ());
        if (pred_hist.front ().seqNum != squashed_sn) {
            DPRINTF (Fetch, "Front sn %i != Squash sn %i\n",
                    pred_hist.front ().seqNum, squashed_sn);

            assert (pred_hist.front ().seqNum == squashed_sn);
        }

        if ((*hist_it).usedRAS) {
            ++RASIncorrect;
        }

		if (!onDemand || 
		    (onDemand && 
			 !pred_hist.front ().wasStatic &&
			 pred_hist.front ().wasControl)) {
			BPUpdate ((*hist_it).pc, actually_taken,
			         pred_hist.front ().bpHistory, true);
		}

		BPHistUpdate ((*hist_it).pc, actually_taken,
					pred_hist.front ().bpHistory, true);

        if (actually_taken) {
            if (hist_it->wasReturn && !hist_it->usedRAS) {
                 DPRINTF (Fetch, "BranchPred: [tid: %i] Incorrectly predicted"
                           "  return [sn:%i] PC: %s\n", tid, hist_it->seqNum,
                            hist_it->pc);
                 RAS[tid].pop ();
            }
        } else {
           //Actually not Taken
           if (hist_it->usedRAS) {
                DPRINTF (Fetch,"BranchPred: [tid: %i] Incorrectly predicted"
                           "  return [sn:%i] PC: %s Restoring RAS\n", tid,
                           hist_it->seqNum, hist_it->pc);
                DPRINTF (Fetch, "BranchPred: [tid:%i]: Restoring top of RAS"
                               " to: %i, target: %s.\n", tid,
                              hist_it->RASIndex, hist_it->RASTarget);
                RAS[tid].restore (hist_it->RASIndex, hist_it->RASTarget);

           } else if (hist_it->wasCall && hist_it->validBTB) {
                 //Was a Call but predicated false. Pop RAS here
                 DPRINTF (Fetch, "BranchPred: [tid: %i] Incorrectly predicted"
                           "  Call [sn:%i] PC: %s Popping RAS\n", tid,
                           hist_it->seqNum, hist_it->pc);
                 RAS[tid].pop ();
           }
        }
		if (onDemand && 
		    (pred_hist.front ().wasST || !pred_hist.front ().wasStatic) && 
			!pred_hist.front ().wasControl) {
			BTB.invalidate ((*hist_it).pc);
		} else  if (actually_taken) {
			DPRINTF (Fetch,"BranchPred: [tid: %i] BTB Update called for [sn:%i]"
                          " PC: %s\n", tid,hist_it->seqNum, hist_it->pc);
			BTB.update ((*hist_it).pc, corrTarget, tid);
			BTBLookups++;
            DPRINTF (Fetch,"BranchPred: [tid: %i] BTB Update called for [sn:%i]"
                            " PC: %s\n", tid,hist_it->seqNum, hist_it->pc);
		}

        DPRINTF (Fetch, "BranchPred: [tid:%i]: Removing history for [sn:%i]"
                       " PC %s  Actually Taken: %i\n", tid, hist_it->seqNum,
                       hist_it->pc, actually_taken);

        pred_hist.erase (hist_it);

        DPRINTF (Fetch, "[tid:%i]: predHist.size (): %i\n", tid, predHist[tid].size ());
    }
}

template <class Impl>
void
BPredUnit<Impl>::BPUncond (void * &bp_history)
{
    // Only the hybrid predictor cares about unconditional branches.
    if (predictor == Hybrid) {
        hybridbp->uncondBr (bp_history);
    } else if (predictor == HybridSkew) {
        hybridSkewbp->uncondBr (bp_history);
    }
}

template <class Impl>
void
BPredUnit<Impl>::BPSquash (void *bp_history)
{
    if (predictor == Local) {
        localBP->squash (bp_history);
    } else if (predictor == Hybrid) {
        hybridbp->squash (bp_history);
    } else if (predictor == HybridSkew) {
        hybridSkewbp->squash (bp_history);
    } else {
        panic ("Predictor type is unexpected value!");
    }
}

template <class Impl>
bool
BPredUnit<Impl>::BPLookup (Addr instPC, void * &bp_history, int positionInLine, bool onDemand, bool &choicePred, bool onlyLP)
{
    if (predictor == Local) {
        return localBP->lookup (instPC, bp_history);
    } else if (predictor == Hybrid) {
        bool prediction = hybridbp->lookup (instPC, bp_history, (unsigned)positionInLine, onDemand, choicePred);
        return prediction;
    } else if (predictor == HybridSkew) {
        bool prediction = hybridSkewbp->lookup (instPC, bp_history, (unsigned)positionInLine, onDemand, choicePred, onlyLP);
        return prediction;
    } else {
        panic ("Predictor type is unexpected value!");
    }
}

template <class Impl>
void
BPredUnit<Impl>::BPBTBUpdate (Addr instPC, void * &bp_history)
{
    if (predictor == Local) {
        return localBP->BTBUpdate (instPC, bp_history);
    } else if (predictor == Hybrid) {
        return hybridbp->BTBUpdate (instPC, bp_history);
    } else if (predictor == HybridSkew) {
        return hybridSkewbp->BTBUpdate (instPC, bp_history);
    } else {
        panic ("Predictor type is unexpected value!");
    }
}

template <class Impl>
void
BPredUnit<Impl>::noBPLookup (Addr instPC, void * &bp_history, int positionInLine)
{
	assert (onDemand);
    if (predictor == Local) {
        assert (false && "this function is not supported for loacl prediction - (see Milad!)");
    } else if (predictor == Hybrid) {
        return hybridbp->onDemandBr (instPC, bp_history, (unsigned)positionInLine);
    } else if (predictor == HybridSkew) {
        return hybridSkewbp->onDemandBr (instPC, bp_history, (unsigned)positionInLine);
    } else {
        panic ("Predictor type is unexpected value!");
    }
}

template <class Impl>
void
BPredUnit<Impl>::BPUpdate (Addr instPC, bool taken, void *bp_history,
                 bool squashed)
{
    if (predictor == Local) {
        localBP->update (instPC, taken, bp_history);
    } else if (predictor == Hybrid) {
        hybridbp->update (instPC, taken, bp_history, squashed);
    } else if (predictor == HybridSkew) {
        hybridSkewbp->update (instPC, taken, bp_history, squashed);
    } else {
        panic ("Predictor type is unexpected value!");
    }
	increase_BP_lookup_and_update (1);
}

template <class Impl>
void
BPredUnit<Impl>::BPHistUpdate (Addr instPC, bool taken, void *bp_history,
                 bool squashed)
{
    if (predictor == Local) {
        assert (false && "this function is not supported for loacl prediction - (see Milad!)");
    } else if (predictor == Hybrid) {
        hybridbp->histUpdate (instPC, taken, bp_history, squashed);
    } else if (predictor == HybridSkew) {
        hybridSkewbp->histUpdate (instPC, taken, bp_history, squashed);
    } else {
        panic ("Predictor type is unexpected value!");
    }
}

template <class Impl>
void
BPredUnit<Impl>::BPHistGarbageCollect (void *bp_history)
{
    if (predictor == Local) {
        assert (false && "this function is not supported for loacl prediction - (see Milad!)");
    } else if (predictor == Hybrid) {
        hybridbp->histGarbageCollect (bp_history);
    } else if (predictor == HybridSkew) {
        hybridSkewbp->histGarbageCollect (bp_history);
    } else {
        panic ("Predictor type is unexpected value!");
    }
}

template <class Impl>
void
BPredUnit<Impl>::dump ()
{
    HistoryIt pred_hist_it;

    for (int i = 0; i < Impl::MaxThreads; ++i) {
        if (!predHist[i].empty ()) {
            pred_hist_it = predHist[i].begin ();

            cprintf ("predHist[%i].size (): %i\n", i, predHist[i].size ());

            while (pred_hist_it != predHist[i].end ()) {
                cprintf ("[sn:%lli], PC:%#x, tid:%i, predTaken:%i, "
                        "bpHistory:%#x\n",
                        pred_hist_it->seqNum, pred_hist_it->pc,
                        pred_hist_it->tid, pred_hist_it->predTaken,
                        pred_hist_it->bpHistory);
                pred_hist_it++;
            }
            cprintf ("\n");
        }
    }
}
