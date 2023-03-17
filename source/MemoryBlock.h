#pragma once

#include <iostream>
#include <vector>

class MemoryBlock {
private:
    std::vector<char*> segments_;
    char* currentSegment;
    size_t segmentSize;
    size_t currentIndex;
    int* segCounts;

public:
    MemoryBlock(size_t _segmentSize = 8192);

    void* allocate(size_t);
    void deallocate(void*);
};
