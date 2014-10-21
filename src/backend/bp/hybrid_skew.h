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

#ifndef __CPU_O3_HYBRID_SKEW_PRED_HH__
#define __CPU_O3_HYBRID_SKEW_PRED_HH__

#include <vector>
#include <math.h>

#include <bp_lib/types.hh>
#include <bp_lib/sat_counter.h>
#include <utility.h>

#define MAJORITY_TAKEN 2
#define HIST_SHFT_0 4
#define HIST_SHFT_1 8

/**
 * Implements a tournament branch predictor, hopefully identical to the one
 * used in the 21264.  It has a local predictor, which uses a local history
 * table to index into a table of counters, and a global predictor, which
 * uses a global history to index into a table of counters.  A choice
 * predictor chooses between the two.  Only the global history register
 * is speculatively updated, the rest are updated upon branches committing
 * or misspeculating.
 */
class HybridBPskew
{
  public:
    /**
     * Default branch predictor constructor.
     */
    HybridBPskew(unsigned localPredictorSize,
                 unsigned localCtrBits,
                 unsigned globalPredictorSize,
                 unsigned globalHistoryBits,
                 unsigned globalCtrBits,
                 unsigned choicePredictorSize,
                 unsigned choiceCtrBits,
                 unsigned instShiftAmt,
				 unsigned fetchWidth);

	/** 
	 * this unit computes the address index to the BP based on the EV8 predictor
	 * Milad
	 */
	unsigned computeLocalIndex(Addr branch_addr, unsigned positionInFetchGroup);

	/** 
	 * this unit computes the address index to the BP based on the EV8 predictor
	 * Milad
	 */
	unsigned computeGlobalIndex0(Addr branch_addr, unsigned positionInFetchGroup);
	unsigned computeGlobalIndex1(Addr branch_addr, unsigned positionInFetchGroup);

	/** 
	 * this unit computes the address index to the BP based on the EV8 predictor
	 * Milad
	 */
	unsigned computeChoiceIndex(Addr branch_addr, unsigned positionInFetchGroup);

    /**
     * Looks up the given address in the branch predictor and returns
     * a true/false value as to whether it is taken.  Also creates a
     * BPHistory object to store any state it will need on squash/update.
     * @param branch_addr The address of the branch to look up.
     * @param bp_history Pointer that will be set to the BPHistory object.
     * @return Whether or not the branch is taken.
     */
    bool lookup(Addr &branch_addr, void * &bp_history, unsigned positionInFetchGroup, bool onDemand, bool &choice, bool onlyLP);

    /**
     * Records that there was an unconditional branch, and modifies
     * the bp history to point to an object that has the previous
     * global history stored in it.
     * @param bp_history Pointer that will be set to the BPHistory object.
     */
    void uncondBr(void * &bp_history);

    /* Records that there was a branch that was ondemand predicted
       create a bp history for it */
    void onDemandBr(Addr &branch_addr, void * &bp_history, unsigned positionInFetchGroup);

    /**
     * Updates the branch predictor to Not Taken if a BTB entry is
     * invalid or not found.
     * @param branch_addr The address of the branch to look up.
     * @param bp_history Pointer to any bp history state.
     * @return Whether or not the branch is taken.
     */
    void BTBUpdate(Addr &branch_addr, void * &bp_history);
    /**
     * Updates the branch predictor with the actual result of a branch.
     * @param branch_addr The address of the branch to update.
     * @param taken Whether or not the branch was taken.
     * @param bp_history Pointer to the BPHistory object that was created
     * when the branch was predicted.
     * @param squashed is set when this function is called during a squash
     * operation.
     */
    void update(Addr &branch_addr, bool taken, void *bp_history, bool squashed);
	void histUpdate(Addr &branch_addr, bool taken, void *bp_history, bool squashed);
	void histGarbageCollect(void *bp_history);

    /**
     * Restores the global branch history on a squash.
     * @param bp_history Pointer to the BPHistory object that has the
     * previous global branch history in it.
     */
    void squash(void *bp_history);

    /** Returns the global history. */
    inline unsigned readGlobalHist() { return globalHistory; }

  private:
    /**
     * Returns if the branch should be taken or not, given a counter
     * value.
     * @param count The counter value.
     */
    inline bool getPrediction(uint8_t &count);

    /**
     * Returns the local history index, given a branch address.
     * @param branch_addr The branch's PC address.
     */
    inline unsigned calcLocHistIdx(Addr &branch_addr);

  public:
    /** Updates global history as taken. */
    void updateGlobalHistTaken();

    /** Updates global history as not taken. */
    void updateGlobalHistNotTaken();

  private:

    /**
     * The branch history information that is created upon predicting
     * a branch.  It will be passed back upon updating and squashing,
     * when the BP can use this information to update/restore its
     * state properly.
     */
    struct BPHistory {
#ifdef DEBUG
        BPHistory()
        { newCount++; }
        ~BPHistory()
        { newCount--; }

        static int newCount;
#endif
        unsigned globalHistory;
        unsigned globalPredictorIdx0;
        unsigned globalPredictorIdx1;
        unsigned choicePredictorIdx;
        unsigned localPredictorIdx;
        bool localPredTaken;
        bool globalPredTaken;
        bool globalUsed;

		//for gskew only
        bool localused;
        bool global0used;
        bool global1used;
    };

	/**
	 * the gskre branch prediction unit
	 */
	bool gSkewPredict(Addr&, void*&, unsigned int, bool, bool&, BPHistory *history);

    /** Flag for invalid predictor index */
    static const int invalidPredictorIndex = -1;
    /** Local counters. */
    std::vector<SatCounter> localCtrs;

    /** Size of the local predictor. */
    unsigned localPredictorSize;

    /** Mask to get the proper index bits into the predictor. */
    unsigned localPredictorMask;

    /** Number of bits of the local predictor's counters. */
    unsigned localCtrBits;

    /** Array of counters that make up the global predictor. */
    std::vector<SatCounter> globalCtrs0;
    std::vector<SatCounter> globalCtrs1;

    /** Size of the global predictor. */
    unsigned globalPredictorSize;

    /** Number of bits of the global predictor's counters. */
    unsigned globalCtrBits;

    /** Global history register. */
    unsigned globalHistory;

    /** Number of bits for the global history. */
    unsigned globalHistoryBits;

    /** Mask to get the proper global history. */
    unsigned globalHistoryMask;

    /** Array of counters that make up the choice predictor. */
    std::vector<SatCounter> choiceCtrs;

    /** Size of the choice predictor (identical to the global predictor). */
    unsigned choicePredictorSize;

    /** Number of bits of the choice predictor's counters. */
    unsigned choiceCtrBits;

    /** Number of bits to shift the instruction over to get rid of the word
     *  offset.
     */
    unsigned instShiftAmt;

    /** Number of bits to shift the instruction over to get rid of the fetchwidth
     *  offset AND word offset.
     */
    unsigned totalInstShiftAmt;

    /** Threshold for the counter value; above the threshold is taken,
     *  equal to or below the threshold is not taken.
     */
    unsigned threshold;

	unsigned fetchWidth;
};

#endif // __CPU_O3_HYBRID_SKEW_PRED_HH__
