#pragma once

#include <iostream>
#include <vector>

/*
الكتلة: هي الجزء الاكبر وهي الذاكرة الافتراضية نفسها
القطعة: هي الجزء الذي يتم حجزه وهنا يكون بحجم 8192 بايت ويتم إسناد عنوان اول بايت فيها في متغير segments_
واول اربعة بايت فيها مخصصة لعد الوحدات
الوحدة: هي الكمية المحجوزة في القطعة سواء كانت 64 بايت او اكثر او اقل
*/


class AlifMemory {
private:
    std::vector<wchar_t*> segments_;
    wchar_t* currentSegment;
    size_t segmentSize;
    size_t currentIndex;
    int* fragmentCounts;

public:
    AlifMemory(size_t _segmentSize = 128); // تتبع التعقيد الزمني لوغارتم(ت) ا

    void* allocate(size_t);
    void deallocate(void*);
};





//
//System Memory
//|
//|----System Allocator
//|    |
//|    |----Obmalloc
//|    |    |
//|    |    |----Used Blocks
//|    |    |----Free Blocks
//|    |
//|    |----Other Allocators
//|
//|----Other Memory


//#define SIZE_ROUND_UP(n,a) (((size_t)(n) + (size_t)((a) - 1) & ~(size_t)((a) - 1))
//
//class AlignmentHeader
//{
//public:
//    union { uint8_t padding; unsigned int count; }ref;
//    class AlignmentHeader* nextAlignment;
//    class AlignmentHeader* prevAlignment;
//    unsigned int SegmentIndex;
//    unsigned int sizeIndex;
//    unsigned int nextOffset;
//    unsigned int maxNextOffset;
//    uint8_t* freeBlock;
//};
//
//class SegmentObject
//{
//public:
//    uintptr_t address;
//    uint8_t* alignmentAddress;
//    unsigned int numberFreeAlignments;
//    unsigned int numberTotalAlignments;
//    class AlignmentHeader* freeAlignments;
//    class SegmentObject* nextSegment;
//    class SegmentObject* prevSegment;
//};
//
//class MemoryManager
//{
//public:
//    class SegmentObject* Segments;
//    unsigned int maxSegment;
//    class SegmentObject* unusedSegment;
//    class SegmentObject* usableSegment;
//    class SegmentObject* numberfreeAlignment[((1 << 20) / (1 << 14)) + 1];
//    size_t numberSegmentCurrentlyAllocate;
//    int rawAllocatedBlocks;
//    size_t numberSegmentAllocated;
//    size_t numberSegmentHighWater;
//};
//
//class AlignmentsUsed
//{
//public:
//    AlignmentHeader* used[(2 * (((512 / 16) + 7) / 8) * 8)];
//};
//
//class SegmentConrange {
//public:
//    int32_t tailHi;
//    int32_t tailLo;
//};
//
//class SegmentMapDown
//{
//public:
//    SegmentConrange Segment[(1 << ((64 - 0) - 20 - 2 * (((64 - 0) - 20 + 2) / 3)))];
//};
//
//class SegmentMapMid
//{
//public:
//    SegmentMapDown* ptrs[(1 << (((64 - 0) - 20 + 2) / 3))];
//};
//
//class SegmentMapTop {
//public:
//    SegmentMapMid* ptrs[(1 << (((64 - 0) - 20 + 2) / 3))];
//};
//
//class Use {
//public:
//    SegmentMapTop SegmentMapRoot;
//    int SegmentMapMidCount;
//    int SegmentMapDownCount;
//};
//
//class MemoryState
//{
//public:
//    class AlignmentsUsed alignments;
//    class MemoryManager mGmt;
//    class Use usage;
//};
//
//MemoryState state;
