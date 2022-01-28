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

#include "BitBlock.h"
#include "BitDeque.h"

namespace rmf
{
    
//------------------------------------------------------------------------|
BitDeque::BitDeque()
: _size(0)
{

}

//------------------------------------------------------------------------|
BitDeque::~BitDeque()
{

}

//------------------------------------------------------------------------|
void BitDeque::Clear()
{
    _blocks.clear();
    _size = 0;
}

//------------------------------------------------------------------------|
// Get a chunk of bits from arbirary bit offset
BitBlock BitDeque::GetBits(const uint64_t addr)
{
    // placeholder
    return BitBlock();
}

//------------------------------------------------------------------------|
//BitBlock BitDeque::GetBits(const uint64_t addr, const uint64_t size)

//------------------------------------------------------------------------|
BitBlock BitDeque::SetBits(const BitBlock & block, const uint64_t addr)
{
    // placeholder
    return BitBlock();
}

//------------------------------------------------------------------------|
BitBlock BitDeque::SetBits(const uint64_t data, const int8_t size,
		const uint64_t addr)
{
    // placeholder
    return BitBlock();
}

//------------------------------------------------------------------------|
BitBlock BitDeque::PushLow(const BitBlock & block)
{
    return BitBlock();
}

//------------------------------------------------------------------------|
BitBlock BitDeque::PushLow(const uint64_t data, const int8_t size)
{
    return BitBlock();
}

//------------------------------------------------------------------------|
BitBlock BitDeque::PopLow(const int8_t size)
{
    return BitBlock();
}

//------------------------------------------------------------------------|
BitBlock BitDeque::PushHigh(const BitBlock & block)
{
    return BitBlock();
}

//------------------------------------------------------------------------|
BitBlock BitDeque::PushHigh(const uint64_t data, const int8_t size)
{
    return BitBlock();
}

//------------------------------------------------------------------------|
BitBlock BitDeque::PopHigh(const int8_t size)
{
    return BitBlock();
}

//------------------------------------------------------------------------|
BitBlock BitDeque::Remove(const int8_t size, const uint64_t addr)
{
    return BitBlock();
}

//------------------------------------------------------------------------|
uint64_t BitDeque::Remove(const uint64_t size, const uint64_t addr)
{
    return 0;
}

//------------------------------------------------------------------------|
void BitDeque::Insert(const BitBlock & block, const uint64_t addr)
{
    return; // BitBlock();
}

}
