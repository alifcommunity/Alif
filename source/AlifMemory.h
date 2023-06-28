#pragma once

#include <iostream>
#include <vector>

/*
الكتلة: هي الجزء الاكبر وهي الذاكرة الافتراضية نفسها
القطعة: هي الجزء الذي يتم حجزه وهنا يكون بحجم 8192 بايت ويتم إسناد عنوان اول بايت فيها في متغير segments_
واول اربعة بايت فيها مخصصة لعد الوحدات
الوحدة: هي الكمية المحجوزة في القطعة سواء كانت 64 بايت او اكثر او اقل
*/


//class AlifMemory {
//private:
//    std::vector<wchar_t*> segments_;
//    wchar_t* currentSegment;
//    size_t segmentSize;
//    size_t currentIndex;
//    int* fragmentCounts;
//
//public:
//    AlifMemory(size_t _segmentSize = 128); // تتبع التعقيد الزمني لوغارتم(ت) ا
//
//    void* allocate(size_t);
//    void deallocate(void*);
//};


#define SIZE_ROUND_UP(n,a) (((size_t)(n) + (size_t)((a) - 1) & ~(size_t)((a) - 1))

class PoolHeader
{
public:
    union { uint8_t padding; unsigned int count; }ref;
    class PoolHeader* nextPool;
    class PoolHeader* prevPool;
    unsigned int arenaIndex;
    unsigned int sizeIndex;
    unsigned int nextOffset;
    unsigned int maxNextOffset;
    uint8_t* freeBlock;
};

class ArenaObject
{
public:
    uintptr_t address;
    uint8_t* poolAddress;
    unsigned int numberFreePools;
    unsigned int numberTotalPools;
    class PoolHeader* freePools;
    class ArenaObject* nextArena;
    class ArenaObject* prevArena;
};

class MemoryManager
{
public:
    class ArenaObject* arenas;
    unsigned int maxArena;
    class ArenaObject* unusedArena;
    class ArenaObject* usableArena;
    class ArenaObject* numberfreePool[((1 << 20) / (1 << 14)) + 1];
    size_t numberArenaCurrentlyAllocate;
    int rawAllocatedBlocks;
    size_t numberArenaAllocated;
    size_t numberArenaHighWater;
};

class PoolsUsed
{
public:
    PoolHeader* used[(2 * (((512 / 16) + 7) / 8) * 8)];
};

class ArenaConrange {
public:
    int32_t tailHi;
    int32_t tailLo;
};

class ArenaMapBot
{
public:
    ArenaConrange arena[(1 << ((64 - 0) - 20 - 2 * (((64 - 0) - 20 + 2) / 3)))];
};

class ArenaMapMid
{
public:
    ArenaMapBot* ptrs[(1 << (((64 - 0) - 20 + 2) / 3))];
};

class ArenaMapTop {
public:
    ArenaMapMid* ptrs[(1 << (((64 - 0) - 20 + 2) / 3))];
};

class Usage {
public:
    ArenaMapTop arenaMapRoot;
    int arenaMapMidCount;
    int arenaMapBotCount;
};

class MemoryState
{
public:
    class PoolsUsed pools;
    class MemoryManager mGmt;
    class Usage usage;
};

MemoryState state;