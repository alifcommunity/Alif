#pragma once

#include <iostream>
#include <vector>

class MemoryBlock {
private:
    std::vector<char*> blocks_;
    char* currentSegment;
    size_t segmentSize;
    size_t currentIndex;

public:
    MemoryBlock(size_t _segmentSize = 8192);

    void* allocate(size_t);
};