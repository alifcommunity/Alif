#include "MemoryBlock.h"


MemoryBlock::MemoryBlock(size_t _segmentSize)
    : segmentSize(_segmentSize), currentIndex(0)
{
    currentSegment = new char[segmentSize];
    blocks_.push_back(currentSegment);
}

void* MemoryBlock::allocate(size_t _size)
{
    _size = (_size + 7) & ~7;
    if (currentIndex + _size > segmentSize)
    {
        currentSegment = new char[segmentSize];
        blocks_.push_back(currentSegment);
        currentIndex = 0;
    }

    void* ptr_ = currentSegment + currentIndex;
    currentIndex += _size;
    return ptr_;
}