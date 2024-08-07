/*
 * Copyright  (c) 2004-2005 The Regents of The University of Michigan
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
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES  (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Kevin Lim
 */

#ifndef __BTB_HH__
#define __BTB_HH__

#include "../../lib/intmath.h"
#include "../../global/global.h"
#include <vector>

class BTB
{
  private:
    struct BTBEntry
    {
        BTBEntry ()
            : tag (0), target (0), valid (false)
        {}

        /** The entry's tag. */
        ADDRS tag;

        /** The entry's target. */
        ADDRS target;

        /** Whether or not the entry is valid. */
        bool valid;
    };

  public:
    /** Creates a BTB with the given number of entries, number of bits per
     *  tag, and instruction offset amount.
     *  @param numEntries Number of entries for the BTB.
     *  @param tagBits Number of bits for each tag in the BTB.
     *  @param instShiftAmt Offset amount for instructions to ignore alignment.
     */
    BTB (unsigned numEntries, unsigned tagBits,
               unsigned instShiftAmt);

    void reset ();

    /** Looks up an address in the BTB. Must call valid () first on the address.
     *  @param inst_PC The address of the branch to look up.
     *  @return Returns the target of the branch.
     */
    ADDRS lookup (ADDRS instPC);

    /** Checks if a branch is in the BTB.
     *  @param inst_PC The address of the branch to look up.
     *  @return Whether or not the branch exists in the BTB.
     */
    bool valid (ADDRS instPC);

    /** Updates the BTB with the target of a branch.
     *  @param inst_PC The address of the branch being updated.
     *  @param target_PC The target address of the branch.
     */
    void update (ADDRS instPC, const ADDRS &targetPC);
	
	/** Invalidate a BTB entry
	 */
	void invalidate (ADDRS instPC);

  private:
    /** Returns the index into the BTB, based on the branch's PC.
     *  @param inst_PC The branch to look up.
     *  @return Returns the index into the BTB.
     */
    inline unsigned getIndex (ADDRS instPC);

    /** Returns the tag bits of a given address.
     *  @param inst_PC The branch's address.
     *  @return Returns the tag bits.
     */
    inline ADDRS getTag (ADDRS instPC);

    /** The actual BTB. */
    std::vector<BTBEntry> btb;

    /** The number of entries in the BTB. */
    unsigned numEntries;

    /** The index mask. */
    unsigned idxMask;

    /** The number of tag bits per entry. */
    unsigned tagBits;

    /** The tag mask. */
    unsigned tagMask;

    /** Number of bits to shift PC when calculating index. */
    unsigned instShiftAmt;

    /** Number of bits to shift PC when calculating tag. */
    unsigned tagShiftAmt;
};

#endif // __CPU_O3_BTB_HH__
