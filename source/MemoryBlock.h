#pragma once

#include <iostream>
#include <vector>
#include <map>

//class MemorySegment {
//public:
//    char* value_;
//    uint16_t size_;
//};

class MemoryBlock {
private:
    std::vector<char*> blocks_;
    char* currentSegment;
    std::map<void*, size_t> freeSegments;
    size_t segmentSize;
    size_t currentIndex;

public:
    MemoryBlock(size_t _segmentSize = 8192);

    void* allocate(size_t);
    void deallocate(void*, size_t);
};
