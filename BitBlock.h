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

#pragma once

#include <string>
#include <stdint.h>

using namespace std;

namespace rmf
{

//------------------------------------------------------------------------|
// Container class for managing a contiguous block of bits within an
// unsigned integer.
class BitBlock
{  
public:

    // Some important size constants.
    // Assumption is 8 bits per byte
    static const int8_t BITS_PER_BYTE = 8;
    static const int8_t MAX_NUM_BITS = BITS_PER_BYTE * sizeof(uint64_t);

    // Constructors, Destructor
    BitBlock();
    BitBlock(const BitBlock & block);
    BitBlock(const uint64_t data, const int8_t size);
    virtual ~BitBlock();

    // These just overwrite the whole block, with
    // exception of the base address, starting at
    // offset zero (LSB)
    void SetBlock(const BitBlock & block);
    void SetBlock(const uint64_t data, const int8_t size);
    void Clear() { SetBlock(0, 0); }

    // Get bits at bit offset. This will be used to
    // support random access  read operations in
    // larger scope (BitDeque).
    BitBlock GetBits(const int8_t offset) const;

    // Replace (set) bits at bit offset.  This will
    // not alter the size of the block, but only
    // overwrite  bits currently allocated.
    BitBlock SetBits(const BitBlock & block, const int8_t offset);
    BitBlock SetBits(const uint64_t data, const int8_t size,
    		const int8_t offset);

    // Get raw data and base address
    uint64_t GetData() const;
    uint64_t GetAddr() const;

    // Capacity related getters
    int8_t GetSize() const;
    int8_t GetSpare() const;
    bool IsEmpty() const;
    bool IsFull() const;

    // LSB right-most bits are 'Back'.  This pushes
    // the caller's bits into the right end of the
    // block, shifting existing bits to the left.
    // Bits that overflow off the left end are
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

    // Note insert-at-offset methods won't be
    // necessary in this class b/c we can
    // rely on the larger deque's insertion
    // capabilities combined with push/pop
    // here to come up with creative ways to
    // insert and remove bits at arbitrary
    // addresses.... Remove() Insert()

    // Chained addr updater function and 
    // setter for pointer to next block.
    // This will be the fastest way to
    // support auomatically maintaining each
    // block's base addr so they can be used
    // for binary search in random access.
    void SetAddr(const uint64_t addr);
    //void SetNext(BitBlock* next);

    // Generate a human readable report
    // string representing current state.
    //string Report();

    BitBlock & operator=(const BitBlock & block);
    BitBlock & operator=(const uint64_t data);
    BitBlock & operator=(const uint32_t data);

    friend ostream & operator<<(ostream & os, const BitBlock & block);

private:
              
    // Large primitive for bit storage and
    // manipulation. Could be templatized
    // for platforms with less bits.
    uint64_t _data;

    // Number of bits stored in this block.
    // LSB is zero index. MSB is 63 or
    // sizeof(data)*8 - 1. This is signed to
    // account for underflow condition.
    int8_t _size;

    // This is the base bit address within a
    // larger set of blocks.  A BitDeque
    // class could operate without this, but
    // would be forced to iterate on every
    // random access.  Having this and
    // keeping it up to date will allow
    // things like binary search to be
    // implemented at a higher level.
    uint64_t _addr;      

    // Pointer to the next highest block.
    // This will be NULL for the last block
    // in a series, or for temporary blocks
    // where address is N/A.
    //BitBlock* _next;
};

}

