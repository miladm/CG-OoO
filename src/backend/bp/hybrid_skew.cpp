/*
 * Copyright (c) 2004-2006 The Regents of The University of Michigan
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

#include <bp_lib/intmath.hh>
#include "hybrid_skew.h"

HybridBPskew::HybridBPskew(unsigned _localPredictorSize,
                           unsigned _localCtrBits,
                           unsigned _globalPredictorSize,
                           unsigned _globalHistoryBits,
                           unsigned _globalCtrBits,
                           unsigned _choicePredictorSize,
                           unsigned _choiceCtrBits,
                           unsigned _instShiftAmt,
						   unsigned _fetchWidth)
    : localPredictorSize(_localPredictorSize),
      localCtrBits(_localCtrBits),
      globalPredictorSize(_globalPredictorSize),
      globalCtrBits(_globalCtrBits),
      globalHistoryBits(_globalHistoryBits),
      choicePredictorSize(_globalPredictorSize),
      choiceCtrBits(_choiceCtrBits),
      instShiftAmt(_instShiftAmt),
      fetchWidth(_fetchWidth)
{
    if (!isPowerOf2(localPredictorSize)) {
        Assert (0 && "Invalid local predictor size!\n");
    }

    //Setup the array of counters for the local predictor
    localCtrs.resize(localPredictorSize);

    for (int i = 0; i < localPredictorSize; ++i)
        localCtrs[i].setBits(localCtrBits);

    //localPredictorMask = floorPow2(localPredictorSize) - 1;

    if (!isPowerOf2(globalPredictorSize)) {
        Assert (0 && "Invalid global predictor size!\n");
    }

    //Setup the array of counters for the global predictor
    globalCtrs0.resize(globalPredictorSize);
    globalCtrs1.resize(globalPredictorSize);

    for (int i = 0; i < globalPredictorSize; ++i) {
        globalCtrs0[i].setBits(globalCtrBits);
        globalCtrs1[i].setBits(globalCtrBits);
	}

    //Clear the global history
    globalHistory = 0;
    // Setup the global history mask
    globalHistoryMask = (1 << globalHistoryBits) - 1;

    if (!isPowerOf2(choicePredictorSize)) {
        Assert (0 && "Invalid choice predictor size!\n");
    }

    //Setup the array of counters for the choice predictor
    choiceCtrs.resize(choicePredictorSize);

    for (int i = 0; i < choicePredictorSize; ++i)
        choiceCtrs[i].setBits(choiceCtrBits);

    // @todo: Allow for different thresholds between the predictors.
    threshold = (1 << (localCtrBits - 1)) - 1;

	//Shift amount is dependent on the fetch width and the instruction size in bytes
	totalInstShiftAmt = instShiftAmt + (unsigned)log2(fetchWidth);

	assert(HIST_SHFT_1 > HIST_SHFT_0);
	assert((unsigned)pow(2,HIST_SHFT_1) < globalPredictorSize);
}

void
HybridBPskew::updateGlobalHistTaken()
{
    globalHistory = (globalHistory << 1) | 1;
    globalHistory = globalHistory & globalHistoryMask;
}

void
HybridBPskew::updateGlobalHistNotTaken()
{
    globalHistory = (globalHistory << 1);
    globalHistory = globalHistory & globalHistoryMask;
}

unsigned
HybridBPskew::computeLocalIndex(Addr branch_addr, unsigned positionInFetchGroup)
{
	unsigned indx = (((branch_addr>>totalInstShiftAmt)<<instShiftAmt)+(Addr)positionInFetchGroup) 
					& (localPredictorSize-1);
	assert(indx < localPredictorSize);
	return indx;
}

unsigned
HybridBPskew::computeChoiceIndex(Addr branch_addr, unsigned positionInFetchGroup)
{
	unsigned indx = ((((branch_addr>>totalInstShiftAmt)^globalHistory)<<instShiftAmt)+(Addr)positionInFetchGroup) 
					& (choicePredictorSize-1);
	assert(indx < choicePredictorSize);
	return indx;
}

unsigned
HybridBPskew::computeGlobalIndex0(Addr branch_addr, unsigned positionInFetchGroup)
{
	unsigned mask = (0x1 << HIST_SHFT_0) - 1;
	unsigned globalHistory_loBits = (globalHistory & mask) << (globalHistoryBits - HIST_SHFT_0);
	unsigned globalHistory_hiBits = globalHistory >> HIST_SHFT_0;
	unsigned globalHistory_shifted = globalHistory_loBits | globalHistory_hiBits;
	unsigned indx = ((((branch_addr>>totalInstShiftAmt)^globalHistory_shifted)<<instShiftAmt)+(Addr)positionInFetchGroup) 
					& (globalPredictorSize-1);
	assert(indx < globalPredictorSize);
	return indx;
}

unsigned
HybridBPskew::computeGlobalIndex1(Addr branch_addr, unsigned positionInFetchGroup)
{
	unsigned mask = (0x1 << HIST_SHFT_1) - 1;
	unsigned globalHistory_loBits = (globalHistory & mask) << (globalHistoryBits - HIST_SHFT_1);
	unsigned globalHistory_hiBits = globalHistory >> HIST_SHFT_1;
	unsigned globalHistory_shifted = globalHistory_loBits | globalHistory_hiBits;
	unsigned indx = ((((branch_addr>>totalInstShiftAmt)^globalHistory_shifted)<<instShiftAmt)+(Addr)positionInFetchGroup) 
					& (globalPredictorSize-1);
	assert(indx < globalPredictorSize);
	return indx;
}

void
HybridBPskew::BTBUpdate(Addr &branch_addr, void * &bp_history)
{
    //Update Global History to Not Taken
    globalHistory = globalHistory & (globalHistoryMask - 1);
}

bool
HybridBPskew::gSkewPredict(Addr &branch_addr, void * &bp_history, unsigned positionInFetchGroup, bool onDemand, bool &choice, BPHistory *history)
{
    unsigned local_predictor_idx;
    unsigned global0_predictor_idx, global1_predictor_idx;
    bool local_prediction;
    bool global0_prediction, global1_prediction;
    //Lookup in the bimodal predictor to get its branch prediction
    local_predictor_idx = computeLocalIndex(branch_addr, positionInFetchGroup);
    local_prediction = localCtrs[local_predictor_idx].read() > threshold;
	
    //Lookup in the global0 predictor to get its branch prediction
    global0_predictor_idx = computeGlobalIndex0(branch_addr, positionInFetchGroup);
    global0_prediction = globalCtrs0[global0_predictor_idx].read() > threshold;
    history->globalPredictorIdx0 = global0_predictor_idx;

    //Lookup in the global1 predictor to get its branch prediction
    global1_predictor_idx = computeGlobalIndex1(branch_addr, positionInFetchGroup);
    global1_prediction = globalCtrs1[global1_predictor_idx].read() > threshold;
    history->globalPredictorIdx1 = global1_predictor_idx;

	int predictionSum = (int)local_prediction + (int)global0_prediction + (int)global1_prediction;

    assert(local_predictor_idx < localPredictorSize);
    assert(global0_predictor_idx < globalPredictorSize);
    assert(global1_predictor_idx < globalPredictorSize);

	if (predictionSum >= MAJORITY_TAKEN) {
		history->localused   = (local_prediction == true) ? true : false;
		history->global0used = (global0_prediction == true) ? true : false;
		history->global1used = (global1_prediction  == true) ? true : false;
		return true;
	} else {
		history->localused   = (local_prediction == false) ? true : false;
		history->global0used = (global0_prediction == false) ? true : false;
		history->global1used = (global1_prediction  == false) ? true : false;
		return false;
	}
}

bool
HybridBPskew::lookup(Addr &branch_addr, void * &bp_history, unsigned positionInFetchGroup, bool onDemand, bool &choice, bool onlyLP)
{
    unsigned local_predictor_idx;
    unsigned choice_predictor_idx;

    bool local_prediction;
    bool global_prediction;
    bool choice_prediction;

    BPHistory *history = new BPHistory;

    //Lookup in the local predictor to get its branch prediction
    local_predictor_idx = computeLocalIndex(branch_addr, positionInFetchGroup);
    local_prediction = localCtrs[local_predictor_idx].read() > threshold;

    //Lookup in the global predictor to get its branch prediction
    global_prediction = gSkewPredict(branch_addr, bp_history, positionInFetchGroup, onDemand, choice, history);

    //Lookup in the choice predictor to see which one to use
    choice_predictor_idx = computeChoiceIndex(branch_addr, positionInFetchGroup);
    if (onlyLP) {
        choiceCtrs[choice_predictor_idx].decrement();
        choiceCtrs[choice_predictor_idx].decrement();
    }
    choice_prediction = choiceCtrs[choice_predictor_idx].read() > threshold;

    // Create BPHistory and pass it back to be recorded.
    history->choicePredictorIdx = choice_predictor_idx;
    history->localPredictorIdx  = local_predictor_idx;
    history->globalHistory = globalHistory;
    history->localPredTaken = local_prediction;
    history->globalPredTaken = global_prediction;
    history->globalUsed = choice_prediction;
    bp_history = (void *)history;

    assert(globalHistory < globalPredictorSize);
    assert(positionInFetchGroup >= 0);
    assert(positionInFetchGroup < fetchWidth);
    assert(local_predictor_idx < localPredictorSize);

    // Commented code is for doing speculative update of counters and
    // all histories.
	bool prediction;
    if (choice_prediction) {
        if (global_prediction) {
			updateGlobalHistTaken();
			prediction = true;
        } else {
			updateGlobalHistNotTaken();
			prediction = false;
        }
    } else {
        if (local_prediction) {
			updateGlobalHistTaken();
			prediction = true;
        } else {
			updateGlobalHistNotTaken();
			prediction = false;
        }
    }
  //Addr tempPC = ((branch_addr>>totalInstShiftAmt)<<totalInstShiftAmt)+((Addr)pow(instShiftAmt,2))*(fetchWidth-1); //Create the addr of last ins in cacheline
  //if (prediction == true) { //TAKEN
  //	updateGlobalHistTaken();
  //} else if (branch_addr == tempPC && prediction == false) {
  //    updateGlobalHistNotTaken();
  //}
    /* testing if local predictor functions right */
	choice = choice_prediction;
    return prediction;
}

void
HybridBPskew::uncondBr(void * &bp_history)
{
    // Create BPHistory and pass it back to be recorded.
    BPHistory *history = new BPHistory;
    history->globalHistory = globalHistory;
    history->localPredTaken = true;
    history->globalPredTaken = true;
    history->globalUsed = true;
    history->choicePredictorIdx = invalidPredictorIndex; //computeChoiceIndex(branch_addr, positionInFetchGroup);
    history->globalPredictorIdx0 = invalidPredictorIndex; //computeGlobalIndex0(branch_addr, positionInFetchGroup);
    history->globalPredictorIdx1 = invalidPredictorIndex; //computeGlobalIndex1(branch_addr, positionInFetchGroup);
    history->localPredictorIdx  = invalidPredictorIndex; //computeLocalIndex(branch_addr, positionInFetchGroup);
    bp_history = static_cast<void *>(history);

    updateGlobalHistTaken();
}

void
HybridBPskew::onDemandBr(Addr &branch_addr, void * &bp_history, unsigned positionInFetchGroup)
{
  // Create BPHistory and pass it back to be recorded.
  BPHistory *history = new BPHistory;
  history->globalHistory = globalHistory;
  history->choicePredictorIdx = invalidPredictorIndex; //computeChoiceIndex(branch_addr, positionInFetchGroup);
  history->globalPredictorIdx0 = invalidPredictorIndex; //computeGlobalIndex0(branch_addr, positionInFetchGroup);
  history->globalPredictorIdx1 = invalidPredictorIndex; //computeGlobalIndex1(branch_addr, positionInFetchGroup);
  history->localPredictorIdx  = invalidPredictorIndex; //computeLocalIndex(branch_addr, positionInFetchGroup);
  bp_history = static_cast<void *>(history);
}

void
HybridBPskew::update(Addr &branch_addr, bool taken, void *bp_history,
                     bool squashed)
{
    // Update the choice predictor to tell it which one was correct if
    // there was a prediction.
    if (bp_history) {
        BPHistory *history = static_cast<BPHistory *>(bp_history);
        bool historyPred = false;
        if (history->globalUsed) {
           historyPred = history->globalPredTaken;
        } else {
           historyPred = history->localPredTaken;
        }
		// Update BP upon commit
		if (!squashed) {
			// Update the choice predictor to tell it which one was correct if
			// there was a prediction.
			if (history->localPredTaken != history->globalPredTaken) {
			    // If the local prediction matches the actual outcome,
			    // decerement the counter.  Otherwise increment the counter
			    if (!history->globalUsed && history->choicePredictorIdx != invalidPredictorIndex) {
			        choiceCtrs[history->choicePredictorIdx].decrement();
			    } else if (history->globalUsed && history->choicePredictorIdx != invalidPredictorIndex) {
			        choiceCtrs[history->choicePredictorIdx].increment();
			    }
			}
			if (!history->globalUsed) {
			   //Strengthen bimodal
				if (taken) {
					if (history->localPredictorIdx != invalidPredictorIndex)
						localCtrs[history->localPredictorIdx].increment();
				} else {
					if (history->localPredictorIdx != invalidPredictorIndex)
					   localCtrs[history->localPredictorIdx].decrement();
				}
			} else {
			   //Strengthen gskew
				if (taken) {
					if (history->globalPredictorIdx0 != invalidPredictorIndex && history->global0used)
						globalCtrs0[history->globalPredictorIdx0].increment();
					if (history->globalPredictorIdx1 != invalidPredictorIndex && history->global1used)
						globalCtrs1[history->globalPredictorIdx1].increment();
					if (history->localPredictorIdx != invalidPredictorIndex && history->localused)
						localCtrs[history->localPredictorIdx].increment();
				} else {
					if (history->globalPredictorIdx0 != invalidPredictorIndex && history->global0used)
						globalCtrs0[history->globalPredictorIdx0].decrement();
					if (history->globalPredictorIdx1 != invalidPredictorIndex && history->global1used)
						globalCtrs1[history->globalPredictorIdx1].decrement();
					if (history->localPredictorIdx != invalidPredictorIndex && history->localused)
						localCtrs[history->localPredictorIdx].decrement();
				}
			}
		} else if (squashed && historyPred != taken) { //upon a branch lookup mis-pred - not btb mis-pred
			if (history->localPredTaken == history->globalPredTaken) {
				//strengthen all predictors
				if (taken) {
					if (history->globalPredictorIdx0 != invalidPredictorIndex)
						globalCtrs0[history->globalPredictorIdx0].increment();
					if (history->globalPredictorIdx1 != invalidPredictorIndex)
						globalCtrs1[history->globalPredictorIdx1].increment();
					if (history->localPredictorIdx != invalidPredictorIndex)
						localCtrs[history->localPredictorIdx].increment();
				} else {
					if (history->globalPredictorIdx0 != invalidPredictorIndex)
						globalCtrs0[history->globalPredictorIdx0].decrement();
					if (history->globalPredictorIdx1 != invalidPredictorIndex)
						globalCtrs1[history->globalPredictorIdx1].decrement();
					if (history->localPredictorIdx != invalidPredictorIndex)
						localCtrs[history->localPredictorIdx].decrement();
				}
			} else {
				// Meta predictor update:
				// If the local prediction matches the actual outcome,
				// decerement the counter.  Otherwise increment the counter
				if (!history->globalUsed && history->choicePredictorIdx != invalidPredictorIndex) {
				    choiceCtrs[history->choicePredictorIdx].increment();
				} else if (history->globalUsed && history->choicePredictorIdx != invalidPredictorIndex) {
				    choiceCtrs[history->choicePredictorIdx].decrement();
				}
				if (history->globalUsed) {
				   //Strengthen bimodal when gskew fails
					if (taken) {
						if (history->localPredictorIdx != invalidPredictorIndex)
							localCtrs[history->localPredictorIdx].increment();
					} else {
						if (history->localPredictorIdx != invalidPredictorIndex)
						   localCtrs[history->localPredictorIdx].decrement();
					}
				} else {
				   //Strengthen gskew when bimodal fails
					if (taken) {
						if (history->globalPredictorIdx0 != invalidPredictorIndex && history->global0used)
							globalCtrs0[history->globalPredictorIdx0].increment();
						if (history->globalPredictorIdx1 != invalidPredictorIndex && history->global1used)
							globalCtrs1[history->globalPredictorIdx1].increment();
						if (history->localPredictorIdx != invalidPredictorIndex && history->localused)
							localCtrs[history->localPredictorIdx].increment();
					} else {
						if (history->globalPredictorIdx0 != invalidPredictorIndex && history->global0used)
							globalCtrs0[history->globalPredictorIdx0].decrement();
						if (history->globalPredictorIdx1 != invalidPredictorIndex && history->global1used)
							globalCtrs1[history->globalPredictorIdx1].decrement();
						if (history->localPredictorIdx != invalidPredictorIndex && history->localused)
							localCtrs[history->localPredictorIdx].decrement();
					}
				}
			}
		}
		assert(history->globalPredictorIdx0 < globalPredictorSize &&
			   history->globalPredictorIdx1 < globalPredictorSize &&
		       history->localPredictorIdx < localPredictorSize);
    }
}

void
HybridBPskew::histUpdate(Addr &branch_addr, bool taken, void *bp_history,
                     bool squashed)
{
    if (bp_history) {
       BPHistory *history = static_cast<BPHistory *>(bp_history);
		if (squashed) {
			if (taken) {
				globalHistory = (history->globalHistory << 1) | 1;
				globalHistory = globalHistory & globalHistoryMask;
			} else {
				globalHistory = (history->globalHistory << 1);
				globalHistory = globalHistory & globalHistoryMask;
			}
		}
       // We're done with this history, now delete it.
       delete history;
    }
}

/* use it when update() is called, but not histUpdate() */
void
HybridBPskew::histGarbageCollect(void *bp_history)
{
    if (bp_history) {
       BPHistory *history = static_cast<BPHistory *>(bp_history);
       // We're done with this history, now delete it.
       delete history;
    }
}

void
HybridBPskew::squash(void *bp_history)
{
    BPHistory *history = static_cast<BPHistory *>(bp_history);

    // Restore global history to state prior to this branch.
    globalHistory = history->globalHistory;

    // Delete this BPHistory now that we're done with it.
    delete history;
}

#ifdef DEBUG
int
HybridBPskew::BPHistory::newCount = 0;
#endif
