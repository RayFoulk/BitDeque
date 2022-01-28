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
	
#include "Log.h"
#include "BitBlock.h"
#include "BitDeque.h"

//------------------------------------------------------------------------|
int main(int argc, char *argv[])
{
    // Test BitBlock
    rmf::BitBlock block;

    /*
    while (!block.IsFull())
    {
        block.PushLow(1, 1);
        block.Log();
    }

    block.Clear();
    block.Log();

    while (!block.IsFull())
    {
        block.PushLow(2, 2);
        block.Log();
    }

    block.Clear();
    block.Log();

    while (!block.IsFull())
    {
        block.PushLow(1, 2);
        block.Log();
    }

    block.Clear();
    block.Log();

    rmf::BitBlock overFlow;
    int i = 0;
    while (!block.IsFull())
    {
        overFlow = block.PushLow(++i, 6);
        block.Log();
        overFlow.Log();
    }

    rmf::BitBlock lowBits;
    while (!block.IsEmpty())
    {
        lowBits = block.PopLow(5);
        block.Log("block");
        lowBits.Log("lowBits");
    }
    */

    /*
    block.PushLow(0xF, 4);
    block.Log("block");

    rmf::BitBlock lowBits = block.PopLow(5);
    block.Log("block");
    lowBits.Log("lowBits");

    block.PushLow(0x55, 5);
    block.Log("block");

    lowBits = block.PopLow(5);
    block.Log("block");
    lowBits.Log("lowBits");
    */

    /*
    // PUSHHIGH TEST
    rmf::BitBlock overFlow;
    while (!block.IsFull())
    {
        //overFlow = block.PushHigh(0xAAAA, 15);
        overFlow = block.PushHigh(0x00FF, 15);
        block.Log("block");
        overFlow.Log("overflow");
    }

    rmf::BitBlock highBits;
    while (!block.IsEmpty())
    {
        highBits = block.PopHigh(5);
        block.Log("block");
        highBits.Log("highBits");
    }
    */

    // POPHIGH CORRECTNESS TEST
    /*
    block = 0xFF00FF00FF00FF00;
    LOG("block: " << block);

    rmf::BitBlock highBits = block.PopHigh(5);
    LOG("block: " << block);
    LOG("highBits: " << highBits);

    block.SetBlock(7, 3);
    LOG("block: " << block);

    highBits = block.PopHigh(5);
    LOG("block: " << block);
    LOG("highBits: " << highBits);
    */

    //rmf::BitBlock lowBits;    
    //rmf::BitDeque bits;

    // MASK ALGORITHM TEST 32/64 BITS

    /* for (int8_t size = 0; size < 64; size ++)
    {
        // Make an AND mask from size and use
        // it to ensure unused part of data
        // is zeroed out.
        LOG("size: " << (int16_t) size);

        uint64_t shifted = ((uint64_t) 1 <<
               (uint64_t) size);
        LOG("shifted: " << std::hex <<
               shifted << std::dec);


        uint64_t mask = ((uint64_t) 1 << size) - 1;
        LOG("mask: " << std::hex <<
               mask << std::dec);
    } */

    // SETBITS TEST
    
    block.PushLow(0x55, 5);
    LOG("block: " << block);

    block.SetBits(0xAA, 4, 0);
    LOG("block: " << block);

        
    return 0;

}






