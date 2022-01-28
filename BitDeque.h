//------------------------------------------------------------------------|
// Copyright (c) 2016-2018, 2022 by Raymond M. Foulk IV
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//------------------------------------------------------------------------|

//------------------------------------------------------------------------|
// Example uses for this class include: Cryptography, Compression,
//Communication (With possible runtime ICD definitions), mapping BitFields
// within data structures, Arbitrary Precision math libraries for integer
// and floating point (eg DynInt and DynFloat) where exponent and mantissa
// size grow as needed by operations.

#pragma once

#include <stdint.h>
#include <deque>

using namespace std;

namespace rmf
{
class BitBlock;

class BitDeque
{
public:
    
    BitDeque();
    virtual ~BitDeque();

    // Empties all bits from the bit deque
    void Clear();

    // Returns the size in bits of the deque
    uint64_t GetSize() const { return _size; }

    // Get a chunk of bits from arbirary bit offset
    BitBlock GetBits(const uint64_t addr);
    //BitBlock GetBits(const uint64_t addr, const uint64_t size);

    // Replace (set) bits at bit offset.  This will
    // not alter the size of the block, but only
    // overwrite  bits currently allocated.
    BitBlock SetBits(const BitBlock & block, const uint64_t addr);
    BitBlock SetBits(const uint64_t data, const int8_t size,
    		const uint64_t addr);

    // LSB right-most bits are 'Back'.  This pushes
    // the caller's bits into the right end of the
    // block, shifting existing bits to the left.
    // Bits that  overflow off the left end are
    // returned
    BitBlock PushLow(const BitBlock & block);
    BitBlock PushLow(const uint64_t data, const int8_t size);

    // This pops LSBs off the right, shifting
    // MSBs 'down' and decreasing _size
    BitBlock PopLow(const int8_t size);

    // MSB left-most bits are 'Front'.  This
    // pushes the caller's bits into the left
    // end of the block, shifting existing
    // bits to the right (if necessary since
    // buffer is LSB-justified).  Bits that
    // 'overflow' off the right end are
    // returned.
    BitBlock PushHigh(const BitBlock & block);
    BitBlock PushHigh(const uint64_t data, const int8_t size);

    // This pops MSBs off the left.  No
    // shifting to accommodate should ever
    // be necessary here.
    BitBlock PopHigh(const int8_t size);

//------------------------------------------------------------------------|
    // Remove (delete) a chunk of bits at an
    // arbitrary bit offset.  The bits deleted are
    // returned in a block if small enough,
    // otherwise if a larger number are to be
    // removed then return the amount removed
    BitBlock Remove(const int8_t size, const uint64_t addr);
    uint64_t Remove(const uint64_t size, const uint64_t addr);

    void Insert(const BitBlock & block, const uint64_t addr);


protected:

    //void ReBase();
 

private:

    // Containers for storing bits
    std::deque<BitBlock> _blocks;

    // Total number of bits
    uint64_t _size;
};

}

