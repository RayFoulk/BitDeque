//------------------------------------------------------------------------|
// Copyright (c) 2016 through 2018 by Raymond M. Foulk IV
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

#include "Log.h"
#include "BitBlock.h"

#include <stdlib.h>

namespace rmf
{

//------------------------------------------------------------------------|
BitBlock::BitBlock()
: _data(0)
, _size(0)
, _addr(0)
//, _next(NULL)
{
}

//------------------------------------------------------------------------|
BitBlock::BitBlock(const BitBlock & block)
: BitBlock()
{
    SetBlock(block);
}

//------------------------------------------------------------------------|
BitBlock::BitBlock(const uint64_t data, const int8_t size)
: BitBlock()
{
    SetBlock(data, size);
}

//------------------------------------------------------------------------|
BitBlock::~BitBlock()
{
}

//------------------------------------------------------------------------|
void BitBlock::SetBlock(const BitBlock & block)
{
    SetBlock(block.GetData(), block.GetSize());
}

//------------------------------------------------------------------------|
// Cannot just assume caller's data is clean (zero) above (left of) the
// size argument. so need to mask higher bits to zero to facilitate later
// bitwise operations.
void BitBlock::SetBlock(const uint64_t data, const int8_t size)
{
    // Coerce the caller's size into range which will allow block to be
	// initialized easily and some higher level operations to be simpler.
	//Also account for special case of 64
    if (size >= MAX_NUM_BITS)
    {
        _data = data;
        _size = MAX_NUM_BITS;
    }
    else if (size <= 0)
    {
        _data = 0;
        _size = 0;
    }
    else
    {
        // Make an AND mask from size and use it to
        // ensure unused part of data is zeroed out.
        uint64_t mask = ((uint64_t) 1 << size) - 1;
        _data = data & mask;
        _size = size;
    }
}

//------------------------------------------------------------------------|
BitBlock BitBlock::GetBits(const int8_t offset) const
{
    // TODO: Need ctor with addr initializer?
    // TODO: Since we may have ptr to next block, we could make more of an
	// effort right here to attempt to return a specified number of bits.
	// TBD
    return BitBlock(_data >> offset, _size - offset); // _addr + offset
}

//------------------------------------------------------------------------|
BitBlock BitBlock::SetBits(const BitBlock & block, const int8_t offset)
{
    return SetBits(block.GetData(), block.GetSize(), offset);
}

//------------------------------------------------------------------------|
// Overwrite data bits at offset with those in the caller's block.  Do not
// alter the size of this current block, but return any remainder higher
// order bits.  Zero size return block indicates all bits were written.
//
// Before:
//     data:   oTUVWXYZ    <- offset 3 shown intentionally
//     _data:     ooABCDEF
//
// After:
//     _data:     ooXYZDEF
//     return:    ooooTUVW
//
//------------------------------------------------------------------------|
BitBlock BitBlock::SetBits(const uint64_t data, const int8_t size,
		const int8_t offset)
{
    // only true if caller data is larger
    int8_t setBits = _size - offset;
    int8_t keepSize = _size; 

    // Discard and clear high bits first.
    // _size gets altered so need to remember.
    // could also add new _size + setBits.
    //PopHigh(setBits);

    // Mask in caller's bits. keepSize causes
    // higher bits in caller's buffer to not get
    // used.
    SetBlock((data << offset) | _data, keepSize);

    // return the part of caller's data that did not
    // get used.
    return BitBlock(data >> setBits,
                    size - setBits);
}
   
//------------------------------------------------------------------------|
uint64_t BitBlock::GetData() const
{
    return _data;
}

//------------------------------------------------------------------------|
int8_t BitBlock::GetSize() const
{ 
    return _size;
}

//------------------------------------------------------------------------|
uint64_t BitBlock::GetAddr() const
{ 
    return _addr;
}

//------------------------------------------------------------------------|
int8_t BitBlock::GetSpare() const
{
    return (MAX_NUM_BITS - _size);
}

//------------------------------------------------------------------------|
bool BitBlock::IsEmpty() const
{
    return (_size <= 0);
}

//------------------------------------------------------------------------|
bool BitBlock::IsFull() const
{
    return (_size >= MAX_NUM_BITS);
}

//------------------------------------------------------------------------|
//              _block      block
// Before       +-+-+-+-+-+  +-+-+-+-+-+
// Call         | | |A|B|C|  | |W|X|Y|Z|
//              +-+-+-+-+-+  +-+-+-+-+-+
//
//  return       _block
//  +-+-+-+-+-+  +-+-+-+-+-+      After
//  | | | |A|B|  |C|W|X|Y|Z|      Call
//  +-+-+-+-+-+  +-+-+-+-+-+
//    
//------------------------------------------------------------------------|
BitBlock BitBlock::PushLow(const BitBlock & block)
{
    // The number of bits we are able to keep
    // is the same as caller's spare bits.
    int8_t keepSize = block.GetSpare();
    
    // What we are forced to shift out is our
    // size minus keepSize. Store any MSBs
    // that would have gotten shifted out in
    // the overflow return block.
    BitBlock overFlow(_data >> keepSize, _size - keepSize);

    // Make some room for incoming bits
    // and adjust our size down by the
    // amount shifted out.  Data is
    // temporarily in an invalid state.
    _data <<= block.GetSize();
    _size -= overFlow.GetSize();

    // Apply the new data bits
    _data |= block.GetData();
    _size += block.GetSize();

    // TODO: Update base address chain
    return overFlow;
}

//------------------------------------------------------------------------|
BitBlock BitBlock::PushLow(const uint64_t data, const int8_t size)
{
    return PushLow(BitBlock(data, size));
}

//------------------------------------------------------------------------|
BitBlock BitBlock::PopLow(const int8_t size)
{
    // Use our own data but the caller's size
    // to construct the return block, but
    // only if there are enough bits
    // remaining. Otherwise return a short
    // block size.
    BitBlock lowBits(_data, (_size > size) ? size : _size);

    // Now alter our own data by the caller's
    // size.  This is an easy way.
    SetBlock(GetBits(size));

    // TODO: Update base address chain
    // only when size changes

    return lowBits;
}

//------------------------------------------------------------------------|
//              _block      block
// Before       +-+-+-+-+-+  +-+-+-+-+-+
// Call         | | |A|B|C|  | |W|X|Y|Z|
//              +-+-+-+-+-+  +-+-+-+-+-+
//
//  return       _block
//  +-+-+-+-+-+  +-+-+-+-+-+      After
//  | | | |B|C|  |W|X|Y|Z|A|      Call
//  +-+-+-+-+-+  +-+-+-+-+-+
//
//------------------------------------------------------------------------|
BitBlock BitBlock::PushHigh(const BitBlock & block)
{
    // The number of bits we are able to keep
    // is the same as caller's spare bits.
    int8_t keepSize = block.GetSpare();
    
    // What we are forced to shift out is our
    // size minus keepSize. Store any LSBs
    // that would have gotten shifted out in
    // the overflow return block.
    BitBlock overFlow(_data, _size - keepSize);

    // Make some room for incoming bits
    // and adjust our size down by the
    // amount shifted out.  Data is
    _data >>= overFlow.GetSize();
    _size -= overFlow.GetSize();

    // Apply the new data bits
    _data |= (block.GetData() << _size);
    _size += block.GetSize();

    // TODO: Update base address chain
    return overFlow;
}

//------------------------------------------------------------------------|
BitBlock BitBlock::PushHigh(const uint64_t data, const int8_t size)
{
    return PushHigh(BitBlock(data, size));
}

//------------------------------------------------------------------------|
BitBlock BitBlock::PopHigh(const int8_t size)
{
    // Use our own data but the caller's size
    // to construct the return block, but
    // only if there are enough bits
    // remaining. Otherwise return a short
    // block size.
    int8_t remaining = (_size > size) ? (_size - size) : 0;
    BitBlock highBits(_data >> remaining, _size - remaining);

    // Now alter our own data by the caller's
    // size.  This is an easy way.
    SetBlock(_data, remaining);

    // TODO: Update base address chain
    // only when size changes

    return highBits;
}

//------------------------------------------------------------------------|
void BitBlock::SetAddr(uint64_t addr)
{
    // Go ahead and set my own address.
    _addr = addr;

    // Set my neighbor's address if I have
    // one.  This will continue until the the
    // last block with no neighbor.  My first
    // inclination was to use a typedefed
    // function pointer, which might have
    // been marginally faster, but this seems
    // simpler.
    //if (_next != NULL)
    //{
    //    _next->SetAddr(_addr + _size);
    //}
}

//------------------------------------------------------------------------|
//void BitBlock::SetNext(BitBlock* next)
//{
//    // pointer to next block
//    _next = next;
//}

//------------------------------------------------------------------------|
BitBlock & BitBlock::operator=(const BitBlock & block)
{
    if (this != &block)
    {
        this->SetBlock(block);
        this->SetAddr(block.GetAddr());
    }

    return *this;
}

//------------------------------------------------------------------------|
BitBlock & BitBlock::operator=(const uint64_t data)
{
    this->SetBlock(data, BitBlock::MAX_NUM_BITS);
    this->SetAddr(0);
    return *this;
}

//------------------------------------------------------------------------|
BitBlock & BitBlock::operator=(const uint32_t data)
{
    this->SetBlock((uint64_t) data, sizeof(uint32_t) *
    		BitBlock::BITS_PER_BYTE);
    this->SetAddr(0);
    return *this;
}

//------------------------------------------------------------------------|
ostream & operator<<(ostream & os, const BitBlock & block)
{
    // Show all 4-bit hex nibbles as ASCII
    // chars atop their respective LSB.
    os << endl;

    string line("hex: ");
    static const uint8_t NIBBLE_BITS = 4;
    int8_t shift = BitBlock::MAX_NUM_BITS -
        NIBBLE_BITS;
    int8_t nibble;

    while (shift >= 0)
    {
        nibble = (0x0F & (block.GetData()
                          >> shift));
        line += "   ";
        line += (char) (nibble > 9) ?
            nibble + 55 : nibble + 48;
        shift -= NIBBLE_BITS;
    }
    os << line << endl;

    // Show all individual bits, using lower
    // case 'o' as a special zero bit that is
    // unused (spare).  This gives a good
    // visual representation of how full the
    // bit buffer is.
    line = "bin: ";
    int8_t spare = block.GetSpare();
    char zero = 'o';
    uint64_t mask = 0x8000000000000000;
    while (mask != 0)
    {
        if (spare == 0) { zero = '0'; }
        line += (block.GetData() & mask) ? '1' : zero;
        mask >>= 1;
        spare--;
    }
    os << line << endl;

    // lastly show the size
    os << "size: " << (int16_t) block.GetSize();

    return os;
}

}









