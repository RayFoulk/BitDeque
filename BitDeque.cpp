//------------------------------------------------------------------------|
// Copyright (c) 2016 through 2025 by Raymond M. Foulk IV
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
BitBlock BitDeque::GetBits(const uint64_t addr)
{
    return GetBits(addr, BitBlock::MAX_NUM_BITS);
}

//------------------------------------------------------------------------|
// Get a chunk of bits from arbitrary bit offset
// This version handles cross-block boundaries by assembling bits
// from multiple internal BitBlocks if necessary
BitBlock BitDeque::GetBits(const uint64_t addr, const int8_t size)
{
    if (addr >= _size || _blocks.empty() || size <= 0)
    {
        return BitBlock(); // empty block
    }
    
    // Calculate actual size to retrieve (don't go beyond end of deque)
    int8_t actualSize = size;
    if (addr + size > _size)
    {
        actualSize = static_cast<int8_t>(_size - addr);
    }
    
    // Find starting block and offset within that block
    uint64_t currentAddr = 0;
    size_t blockIndex = 0;
    
    for (blockIndex = 0; blockIndex < _blocks.size(); ++blockIndex)
    {
        uint64_t blockEnd = currentAddr + _blocks[blockIndex].GetSize();
        if (addr >= currentAddr && addr < blockEnd)
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
    int8_t remaining = actualSize;
    int8_t offset = static_cast<int8_t>(addr - currentAddr);
    
    // Extract bits across blocks if necessary
    while (remaining > 0 && blockIndex < _blocks.size())
    {
        const BitBlock& currentBlock = _blocks[blockIndex];
        int8_t availableInBlock = currentBlock.GetSize() - offset;
        int8_t toTake = (remaining < availableInBlock) ? remaining : availableInBlock;
        
        // Get the bits we need from this block
        BitBlock blockBits = currentBlock.GetBits(offset);
        if (blockBits.GetSize() > toTake)
        {
            // Trim to exactly what we need
            blockBits.SetBlock(blockBits.GetData(), toTake);
        }
        
        if (result.IsEmpty())
        {
            result = blockBits;
        }
        else
        {
            // Combine with existing result - push into higher bits
            if (result.GetSpare() >= blockBits.GetSize())
            {
                // Can fit the entire block bits
                result.PushLow(blockBits);
            }
            else
            {
                // Result is getting full, take only what fits
                int8_t canTake = result.GetSpare();
                if (canTake > 0)
                {
                    BitBlock partial(blockBits.GetData(), canTake);
                    result.PushLow(partial);
                }
                break;
            }
        }
        
        remaining -= toTake;
        offset = 0; // For subsequent blocks, start at offset 0
        ++blockIndex;
    }
    
    return result;
}

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
    
    int8_t remaining = (size > _size) ? static_cast<int8_t>(_size) : size;
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
            else if (result.GetSpare() >= blockSize)
            {
                result.PushHigh(lastBlock);
            }
            else
            {
                // Can't fit more, we're done
                break;
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
            else if (result.GetSpare() >= popped.GetSize())
            {
                result.PushHigh(popped);
            }
            
            _size -= remaining;
            remaining = 0;
            
            // Remove empty blocks naturally
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
    
    int8_t remaining = (size > _size) ? static_cast<int8_t>(_size) : size;
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
            else if (result.GetSpare() >= blockSize)
            {
                result.PushLow(firstBlock);
            }
            else
            {
                // Can't fit more, we're done
                break;
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
            else if (result.GetSpare() >= popped.GetSize())
            {
                result.PushLow(popped);
            }
            
            _size -= remaining;
            remaining = 0;
            
            // Remove empty blocks naturally
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
    
    // Calculate actual removal size
    int8_t actualSize = size;
    if (addr + size > _size)
    {
        actualSize = static_cast<int8_t>(_size - addr);
    }
    
    // Find the starting block and offset
    uint64_t currentAddr = 0;
    size_t startBlockIndex = 0;
    
    for (startBlockIndex = 0; startBlockIndex < _blocks.size(); ++startBlockIndex)
    {
        uint64_t blockEnd = currentAddr + _blocks[startBlockIndex].GetSize();
        if (addr >= currentAddr && addr < blockEnd)
        {
            break;
        }
        currentAddr = blockEnd;
    }
    
    if (startBlockIndex >= _blocks.size())
    {
        return BitBlock(); // address not found
    }
    
    // First, collect the bits we're removing for return value
    BitBlock removedBits = GetBits(addr, actualSize);
    
    // Now perform the removal
    int8_t offsetInStartBlock = static_cast<int8_t>(addr - currentAddr);
    int8_t remaining = actualSize;
    size_t blockIndex = startBlockIndex;
    
    while (remaining > 0 && blockIndex < _blocks.size())
    {
        BitBlock& currentBlock = _blocks[blockIndex];
        int8_t availableInBlock = currentBlock.GetSize() - offsetInStartBlock;
        int8_t toRemove = (remaining < availableInBlock) ? remaining : availableInBlock;
        
        if (offsetInStartBlock == 0 && toRemove == currentBlock.GetSize())
        {
            // Remove entire block
            _blocks.erase(_blocks.begin() + blockIndex);
            _size -= toRemove;
            remaining -= toRemove;
            // Don't increment blockIndex since we removed a block
        }
        else if (offsetInStartBlock == 0)
        {
            // Remove from the beginning of the block
            currentBlock.PopHigh(toRemove);
            _size -= toRemove;
            remaining -= toRemove;
            ++blockIndex;
        }
        else if (offsetInStartBlock + toRemove == currentBlock.GetSize())
        {
            // Remove from the end of the block
            currentBlock.PopLow(toRemove);
            _size -= toRemove;
            remaining -= toRemove;
            ++blockIndex;
        }
        else
        {
            // Remove from the middle - need to split the block
            BitBlock leftPart = currentBlock.GetBits(0);
            leftPart.SetBlock(leftPart.GetData(), offsetInStartBlock);
            
            BitBlock rightPart = currentBlock.GetBits(offsetInStartBlock + toRemove);
            
            // Replace current block with left part
            currentBlock = leftPart;
            
            // Insert right part after current block if it's not empty
            if (!rightPart.IsEmpty())
            {
                _blocks.insert(_blocks.begin() + blockIndex + 1, rightPart);
            }
            
            _size -= toRemove;
            remaining -= toRemove;
            ++blockIndex;
        }
        
        offsetInStartBlock = 0; // For subsequent blocks, start at offset 0
    }
    
    // Only do lazy defragmentation around the affected area
    LazyDefragment(startBlockIndex);
    
    return removedBits;
}

//------------------------------------------------------------------------|
uint64_t BitDeque::Remove(const uint64_t size, const uint64_t addr)
{
    if (size <= BitBlock::MAX_NUM_BITS)
    {
        // For smaller removals, use the BitBlock version
        BitBlock removed = Remove(static_cast<int8_t>(size), addr);
        return removed.GetSize();
    }
    
    // For large removals, remove in chunks
    uint64_t totalRemoved = 0;
    uint64_t remaining = size;
    uint64_t currentAddr = addr;
    
    while (remaining > 0 && currentAddr < _size)
    {
        int8_t chunkSize = (remaining > BitBlock::MAX_NUM_BITS) ? 
                           BitBlock::MAX_NUM_BITS : 
                           static_cast<int8_t>(remaining);
        
        BitBlock removed = Remove(chunkSize, currentAddr);
        uint64_t removedSize = removed.GetSize();
        
        if (removedSize == 0)
        {
            break; // Nothing more to remove
        }
        
        totalRemoved += removedSize;
        remaining -= removedSize;
        // Note: currentAddr stays the same since we're removing bits at that position
    }
    
    return totalRemoved;
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
    
    // Find the target block and position
    uint64_t currentAddr = 0;
    size_t blockIndex = 0;
    
    for (blockIndex = 0; blockIndex < _blocks.size(); ++blockIndex)
    {
        uint64_t blockEnd = currentAddr + _blocks[blockIndex].GetSize();
        if (addr >= currentAddr && addr <= blockEnd)
        {
            break;
        }
        currentAddr = blockEnd;
    }
    
    if (blockIndex >= _blocks.size())
    {
        // Insert at the very end
        PushLow(block);
        return;
    }
    
    int8_t offsetInBlock = static_cast<int8_t>(addr - currentAddr);
    BitBlock& targetBlock = _blocks[blockIndex];
    
    if (offsetInBlock == 0)
    {
        // Insert at the beginning of this block
        _blocks.insert(_blocks.begin() + blockIndex, block);
    }
    else if (offsetInBlock == targetBlock.GetSize())
    {
        // Insert at the end of this block
        _blocks.insert(_blocks.begin() + blockIndex + 1, block);
    }
    else
    {
        // Insert in the middle - split the block
        BitBlock leftPart = targetBlock.GetBits(0);
        leftPart.SetBlock(leftPart.GetData(), offsetInBlock);
        
        BitBlock rightPart = targetBlock.GetBits(offsetInBlock);
        
        // Replace current block with left part
        targetBlock = leftPart;
        
        // Insert the new block and right part
        _blocks.insert(_blocks.begin() + blockIndex + 1, block);
        _blocks.insert(_blocks.begin() + blockIndex + 2, rightPart);
    }
    
    _size += block.GetSize();
    
    // Only do lazy defragmentation around the insertion point
    LazyDefragment(blockIndex);
}


//------------------------------------------------------------------------|
void BitDeque::Append(const BitDeque& other)
{
    if (other.GetSize() == 0)
    {
        return; // Nothing to append
    }

    if (_size == 0)
    {
        // If this deque is empty, just copy all blocks from other
        _blocks = other._blocks;
        _size = other._size;
        return;
    }

    // Try to merge the first block of other with our last block
    bool merged = false;
    if (!_blocks.empty() && !other._blocks.empty())
    {
        BitBlock& ourLastBlock = _blocks.back();
        const BitBlock& theirFirstBlock = other._blocks.front();

        if (ourLastBlock.GetSpare() >= theirFirstBlock.GetSize())
        {
            // Our last block can absorb their first block completely
            ourLastBlock.PushLow(theirFirstBlock);
            _size += theirFirstBlock.GetSize();
            merged = true;
        }
    }

    // Add remaining blocks from other
    size_t startIndex = merged ? 1 : 0;
    for (size_t i = startIndex; i < other._blocks.size(); ++i)
    {
        _blocks.push_back(other._blocks[i]);
        _size += other._blocks[i].GetSize();
    }

    // Lazy defragment around the junction point
    if (!_blocks.empty() && _blocks.size() > 1)
    {
        // Find the junction point (where we started appending)
        size_t junctionIndex = _blocks.size() - (other._blocks.size() - startIndex);
        if (junctionIndex > 0)
        {
            LazyDefragment(junctionIndex - 1);
        }
    }
}

//------------------------------------------------------------------------|
BitDeque BitDeque::Split(const uint64_t addr)
{
    BitDeque rightPart;

    if (addr == 0)
    {
        // Split at beginning - right part gets everything, this becomes empty
        rightPart._blocks = std::move(_blocks);
        rightPart._size = _size;
        _blocks.clear();
        _size = 0;
        return rightPart;
    }

    if (addr >= _size)
    {
        // Split beyond end - this unchanged, right part is empty
        return rightPart; // Already empty
    }

    // Find the block containing the split address
    uint64_t currentAddr = 0;
    size_t splitBlockIndex = 0;

    for (splitBlockIndex = 0; splitBlockIndex < _blocks.size(); ++splitBlockIndex)
    {
        uint64_t blockEnd = currentAddr + _blocks[splitBlockIndex].GetSize();
        if (addr >= currentAddr && addr < blockEnd)
        {
            break; // Split address is within this block
        }
        if (addr == blockEnd)
        {
            // Split exactly at block boundary
            splitBlockIndex++; // Split after this block
            break;
        }
        currentAddr = blockEnd;
    }

    if (splitBlockIndex >= _blocks.size())
    {
        // This shouldn't happen given our bounds check above
        return rightPart; // Return empty right part
    }

    // Handle the split
    if (addr == currentAddr)
    {
        // Split exactly at block boundary - no block splitting needed
        // Move blocks [splitBlockIndex, end) to right part
        for (size_t i = splitBlockIndex; i < _blocks.size(); ++i)
        {
            rightPart._blocks.push_back(_blocks[i]);
            rightPart._size += _blocks[i].GetSize();
        }

        // Remove those blocks from this deque
        _blocks.erase(_blocks.begin() + splitBlockIndex, _blocks.end());
        _size = currentAddr; // Size is now up to the split point
    }
    else
    {
        // Split is within a block - need to split the block
        BitBlock& splitBlock = _blocks[splitBlockIndex];
        int8_t offsetInBlock = static_cast<int8_t>(addr - currentAddr);

        // Create left and right parts of the split block
        BitBlock leftPart = splitBlock.GetBits(0);
        leftPart.SetBlock(leftPart.GetData(), offsetInBlock);

        BitBlock rightBlockPart = splitBlock.GetBits(offsetInBlock);

        // Replace the split block with its left part in this deque
        _blocks[splitBlockIndex] = leftPart;

        // Add the right part of the split block to the right deque (if not empty)
        if (!rightBlockPart.IsEmpty())
        {
            rightPart._blocks.push_back(rightBlockPart);
            rightPart._size += rightBlockPart.GetSize();
        }

        // Move all blocks after the split block to the right deque
        for (size_t i = splitBlockIndex + 1; i < _blocks.size(); ++i)
        {
            rightPart._blocks.push_back(_blocks[i]);
            rightPart._size += _blocks[i].GetSize();
        }

        // Remove those blocks from this deque
        _blocks.erase(_blocks.begin() + splitBlockIndex + 1, _blocks.end());

        // Update this deque's size
        _size = addr;
    }

    // Clean up any empty blocks and do light defragmentation
    RemoveEmptyBlocks();
    rightPart.RemoveEmptyBlocks();

    // Light defragmentation around the split points
    if (!_blocks.empty() && splitBlockIndex > 0 && splitBlockIndex < _blocks.size())
    {
        LazyDefragment(splitBlockIndex - 1);
    }

    if (!rightPart._blocks.empty() && rightPart._blocks.size() > 1)
    {
        rightPart.LazyDefragment(0);
    }

    return rightPart;
}

//------------------------------------------------------------------------|
void BitDeque::LazyDefragment(size_t aroundIndex)
{
    if (_blocks.size() <= 1)
    {
        return;
    }
    
    // Only defragment adjacent blocks around the specified index
    // This keeps the operation local and predictable
    
    // Check if we can combine with the previous block
    if (aroundIndex > 0)
    {
        BitBlock& prev = _blocks[aroundIndex - 1];
        BitBlock& current = _blocks[aroundIndex];
        
        if (prev.GetSpare() >= current.GetSize())
        {
            // Previous block can absorb the current block
            prev.PushLow(current);
            _blocks.erase(_blocks.begin() + aroundIndex);
            aroundIndex--; // Adjust index since we removed a block
        }
        else if (current.GetSpare() >= prev.GetSize() && !prev.IsEmpty())
        {
            // Current block can absorb the previous block
            current.PushHigh(prev);
            _blocks.erase(_blocks.begin() + aroundIndex - 1);
            aroundIndex--; // Adjust index since we removed a block
        }
    }
    
    // Check if we can combine with the next block
    if (aroundIndex < _blocks.size() - 1)
    {
        BitBlock& current = _blocks[aroundIndex];
        BitBlock& next = _blocks[aroundIndex + 1];
        
        if (current.GetSpare() >= next.GetSize())
        {
            // Current block can absorb the next block
            current.PushLow(next);
            _blocks.erase(_blocks.begin() + aroundIndex + 1);
        }
        else if (next.GetSpare() >= current.GetSize() && !current.IsEmpty())
        {
            // Next block can absorb the current block
            next.PushHigh(current);
            _blocks.erase(_blocks.begin() + aroundIndex);
        }
    }
    
    // Clean up any empty blocks that might have been created
    RemoveEmptyBlocks();
}

//------------------------------------------------------------------------|
void BitDeque::Defragment()
{
    if (_blocks.size() <= 1)
    {
        return; // Nothing to defragment
    }
    
    // Full defragmentation pass - combine blocks optimally
    // This is more thorough but potentially expensive
    bool changed = true;
    
    while (changed && _blocks.size() > 1)
    {
        changed = false;
        
        for (size_t i = 0; i < _blocks.size() - 1; )
        {
            BitBlock& current = _blocks[i];
            BitBlock& next = _blocks[i + 1];
            
            if (current.GetSpare() >= next.GetSize())
            {
                // Current block has room for the entire next block
                current.PushLow(next);
                _blocks.erase(_blocks.begin() + i + 1);
                changed = true;
                // Don't increment i, check the same position again
            }
            else if (next.GetSpare() >= current.GetSize())
            {
                // Next block has room for the entire current block
                next.PushHigh(current);
                _blocks.erase(_blocks.begin() + i);
                changed = true;
                // Don't increment i, check the same position again
            }
            else if (current.GetSpare() > 0 && next.GetSize() > 0)
            {
                // Partial merge: move some bits from next to current
                int8_t canMove = current.GetSpare();
                BitBlock moved = next.PopHigh(canMove);
                current.PushLow(moved);
                changed = true;
                
                // If next block became empty, it will be cleaned up below
                ++i;
            }
            else
            {
                ++i;
            }
        }
    }
    
    // Remove any empty blocks
    RemoveEmptyBlocks();
}

//------------------------------------------------------------------------|
void BitDeque::RemoveEmptyBlocks()
{
    // Remove any empty blocks that might remain
    for (auto it = _blocks.begin(); it != _blocks.end(); )
    {
        if (it->IsEmpty())
        {
            it = _blocks.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

}
