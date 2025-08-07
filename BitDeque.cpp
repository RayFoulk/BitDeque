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
// Get a chunk of bits from arbitrary bit offset
BitBlock BitDeque::GetBits(const uint64_t addr)
{
    if (addr >= _size || _blocks.empty())
    {
        return BitBlock(); // empty block
    }
    
    // FIXME: Concerned about how this will behave if GetBits() is
    //   requested at an address that crosses BitBlock boundaries.
    //   The returned BitBlock should be a contiguous composite of
    //   whatever chunks of other internal BitBlocks is necessary. 
    // Find the block containing this address
    uint64_t currentAddr = 0;
    for (auto& block : _blocks)
    {
        if (addr >= currentAddr && addr < currentAddr + block.GetSize())
        {
            // Address is within this block
            int8_t offset = static_cast<int8_t>(addr - currentAddr);
            return block.GetBits(offset);
        }
        currentAddr += block.GetSize();
    }
    
    return BitBlock(); // address not found
}

//------------------------------------------------------------------------|
//BitBlock BitDeque::GetBits(const uint64_t addr, const uint64_t size)
// TODO: Considering the usefulness of a GetBits() overload
//   that allows requesting up to a certain number of bits.
//   May not be that useful since the user can do whatever they want
//   with what they get from normal GetBits(), which should just
//   be a copied chunk of bits.

//------------------------------------------------------------------------|
BitBlock BitDeque::SetBits(const BitBlock & block, const uint64_t addr)
{
    return SetBits(block.GetData(), block.GetSize(), addr);
}

//------------------------------------------------------------------------|
BitBlock BitDeque::SetBits(const uint64_t data, const int8_t size,
		const uint64_t addr)
{
    if (addr >= _size || _blocks.empty() || size <= 0)
    {
        return BitBlock(data, size); // return unchanged data
    }
    
    // Find the block containing this address
    uint64_t currentAddr = 0;
    for (auto& block : _blocks)
    {
        if (addr >= currentAddr && addr < currentAddr + block.GetSize())
        {
            // Address is within this block
            int8_t offset = static_cast<int8_t>(addr - currentAddr);
            return block.SetBits(data, size, offset);
        }
        currentAddr += block.GetSize();
    }
    
    return BitBlock(data, size); // address not found, return unchanged
}

//------------------------------------------------------------------------|
BitBlock BitDeque::PushLow(const BitBlock & block)
{
    return PushLow(block.GetData(), block.GetSize());
}

//------------------------------------------------------------------------|
BitBlock BitDeque::PushLow(const uint64_t data, const int8_t size)
{
    if (size <= 0)
    {
        return BitBlock();
    }
    
    BitBlock newBits(data, size);
    
    if (_blocks.empty())
    {
        // First block - just add it
        _blocks.push_back(newBits);
        _size += size;
        return BitBlock(); // no overflow
    }
    
    // Try to add to the last (rightmost) block first
    BitBlock& lastBlock = _blocks.back();
    if (!lastBlock.IsFull())
    {
        BitBlock overflow = lastBlock.PushLow(newBits);
        _size += (size - overflow.GetSize());
        
        // If there's overflow, create a new block for it
        if (!overflow.IsEmpty())
        {
            _blocks.push_back(overflow);
            _size += overflow.GetSize();
        }
    }
    else
    {
        // Last block is full, add new block
        _blocks.push_back(newBits);
        _size += size;
    }
    
    return BitBlock(); // BitDeque doesn't overflow
}

//------------------------------------------------------------------------|
BitBlock BitDeque::PopLow(const int8_t size)
{
    if (size <= 0 || _blocks.empty() || _size == 0)
    {
        return BitBlock();
    }
    
    int8_t remaining = size;
    BitBlock result;
    
    while (remaining > 0 && !_blocks.empty())
    {
        BitBlock& lastBlock = _blocks.back();
        int8_t blockSize = lastBlock.GetSize();
        
        if (remaining >= blockSize)
        {
            // Take the entire last block
            if (result.IsEmpty())
            {
                result = lastBlock;
            }
            else
            {
                // Combine with existing result
                BitBlock overflow = result.PushHigh(lastBlock);
                // Note: In a full implementation, we'd need to handle
                // overflow when combining multiple blocks
            }
            
            remaining -= blockSize;
            _size -= blockSize;
            _blocks.pop_back();
        }
        else
        {
            // Take part of the last block
            BitBlock popped = lastBlock.PopLow(remaining);
            
            if (result.IsEmpty())
            {
                result = popped;
            }
            else
            {
                result.PushHigh(popped);
            }
            
            _size -= remaining;
            remaining = 0;
            
            // Remove block if it became empty
            if (lastBlock.IsEmpty())
            {
                _blocks.pop_back();
            }
        }
    }
    
    return result;
}

//------------------------------------------------------------------------|
BitBlock BitDeque::PushHigh(const BitBlock & block)
{
    return PushHigh(block.GetData(), block.GetSize());
}

//------------------------------------------------------------------------|
BitBlock BitDeque::PushHigh(const uint64_t data, const int8_t size)
{
    if (size <= 0)
    {
        return BitBlock();
    }
    
    BitBlock newBits(data, size);
    
    if (_blocks.empty())
    {
        // First block - just add it
        _blocks.push_front(newBits);
        _size += size;
        return BitBlock(); // no overflow
    }
    
    // Try to add to the first (leftmost) block first
    BitBlock& firstBlock = _blocks.front();
    if (!firstBlock.IsFull())
    {
        BitBlock overflow = firstBlock.PushHigh(newBits);
        _size += (size - overflow.GetSize());
        
        // If there's overflow, create a new block for it
        if (!overflow.IsEmpty())
        {
            _blocks.push_front(overflow);
            _size += overflow.GetSize();
        }
    }
    else
    {
        // First block is full, add new block at front
        _blocks.push_front(newBits);
        _size += size;
    }
    
    return BitBlock(); // BitDeque doesn't overflow
}

//------------------------------------------------------------------------|
BitBlock BitDeque::PopHigh(const int8_t size)
{
    if (size <= 0 || _blocks.empty() || _size == 0)
    {
        return BitBlock();
    }
    
    int8_t remaining = size;
    BitBlock result;
    
    while (remaining > 0 && !_blocks.empty())
    {
        BitBlock& firstBlock = _blocks.front();
        int8_t blockSize = firstBlock.GetSize();
        
        if (remaining >= blockSize)
        {
            // Take the entire first block
            if (result.IsEmpty())
            {
                result = firstBlock;
            }
            else
            {
                // Combine with existing result
                BitBlock overflow = result.PushLow(firstBlock);
                // Note: In a full implementation, we'd need to handle
                // overflow when combining multiple blocks
            }
            
            remaining -= blockSize;
            _size -= blockSize;
            _blocks.pop_front();
        }
        else
        {
            // Take part of the first block
            BitBlock popped = firstBlock.PopHigh(remaining);
            
            if (result.IsEmpty())
            {
                result = popped;
            }
            else
            {
                result.PushLow(popped);
            }
            
            _size -= remaining;
            remaining = 0;
            
            // Remove block if it became empty
            if (firstBlock.IsEmpty())
            {
                _blocks.pop_front();
            }
        }
    }
    
    return result;
}

//------------------------------------------------------------------------|
BitBlock BitDeque::Remove(const int8_t size, const uint64_t addr)
{
    if (size <= 0 || addr >= _size || _blocks.empty())
    {
        return BitBlock();
    }
    
    // Find the block containing the start address
    uint64_t currentAddr = 0;
    size_t blockIndex = 0;
    
    for (blockIndex = 0; blockIndex < _blocks.size(); ++blockIndex)
    {
        uint64_t blockStart = currentAddr;
        uint64_t blockEnd = currentAddr + _blocks[blockIndex].GetSize();
        
        if (addr >= blockStart && addr < blockEnd)
        {
            break;
        }
        currentAddr = blockEnd;
    }
    
    if (blockIndex >= _blocks.size())
    {
        return BitBlock(); // address not found
    }
    
    BitBlock result;
    int8_t remaining = size;
    int8_t offsetInBlock = static_cast<int8_t>(addr - currentAddr);
    
    // This is a simplified implementation - a complete version would
    // handle removal spanning multiple blocks and defragmentation
    BitBlock& targetBlock = _blocks[blockIndex];
    
    if (offsetInBlock + remaining <= targetBlock.GetSize())
    {
        // Simple case: removal is within one block
        // For now, this is a placeholder - proper implementation would
        // need to split the block, remove the middle part, and rejoin
        result = BitBlock(0, remaining); // placeholder
        _size -= remaining;
    }
    
    // TODO: Implement defragmentation logic here
    Defragment();
    
    return result;
}

//------------------------------------------------------------------------|
uint64_t BitDeque::Remove(const uint64_t size, const uint64_t addr)
{
    // For large removals, we don't return the actual bits
    if (size > BitBlock::MAX_NUM_BITS)
    {
        // TODO: Implement large removal logic
        return 0; // placeholder
    }
    
    // For smaller removals, delegate to the BitBlock version
    BitBlock removed = Remove(static_cast<int8_t>(size), addr);
    return removed.GetSize();
}

//------------------------------------------------------------------------|
void BitDeque::Insert(const BitBlock & block, const uint64_t addr)
{
    if (block.IsEmpty())
    {
        return;
    }
    
    if (addr >= _size)
    {
        // Insert at end (append)
        PushLow(block);
        return;
    }
    
    if (addr == 0)
    {
        // Insert at beginning (prepend)
        PushHigh(block);
        return;
    }
    
    // FIXME: Implement insertion at arbitrary position
    // This would involve:
    // 1. Finding the target block
    // 2. Potentially splitting it at the insertion point
    // 3. Inserting the new bits
    // 4. Defragmenting if necessary
    
    _size += block.GetSize();
    Defragment();
}

//------------------------------------------------------------------------|
void BitDeque::Defragment()
{
    if (_blocks.size() <= 1)
    {
        return; // Nothing to defragment
    }
    
    // Combine adjacent blocks that have room
    for (size_t i = 0; i < _blocks.size() - 1; )
    {
        BitBlock& current = _blocks[i];
        BitBlock& next = _blocks[i + 1];
        
        if (current.GetSpare() >= next.GetSize())
        {
            // Current block has room for the next block
            BitBlock overflow = current.PushLow(next);
            
            // Remove the next block
            _blocks.erase(_blocks.begin() + i + 1);
            
            // If there was overflow, create a new block for it
            if (!overflow.IsEmpty())
            {
                _blocks.insert(_blocks.begin() + i + 1, overflow);
                ++i; // Skip the newly inserted block
            }
            // Don't increment i, check the same position again
        }
        else
        {
            ++i;
        }
    }
}

}
