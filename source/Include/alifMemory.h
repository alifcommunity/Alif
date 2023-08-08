#pragma once

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


#define SIZE_ROUND_UP(n,a) (((size_t)(n) + (size_t)((a) - 1) & ~(size_t)((a) - 1))

class AlignmentHeader
{
public:
	union { uint8_t* padding; unsigned int count; }ref;
	class AlignmentHeader* nextAlignment;
	class AlignmentHeader* prevAlignment;
	size_t BlockIndex;
	size_t sizeIndex;
	size_t nextOffset;
	size_t maxNextOffset;
	uint8_t* freeBlock;
};

class BlockObject
{
public:
	uintptr_t address;
	uint8_t* alignmentAddress;
	unsigned int numberFreeAlignments;
	unsigned int numberTotalAlignments;
	AlignmentHeader* freeAlignments;
	BlockObject* nextBlock;
	BlockObject* prevBlock;
};

class AlignmentsUsed
{
public:
	AlignmentHeader* used[(2 * (((512 / 16) + 7) / 8) * 8)];
};

class MemoryManager
{
public:
	BlockObject* Blocks;
	unsigned int maxBlock;
	BlockObject* unusedBlock;
	BlockObject* usableBlock;
	BlockObject* numberfreeAlignment[((1 << 20) / (1 << 14)) + 1];
	size_t numberBlockCurrentlyAllocate;
	long long rawAllocatedBlocks;
	size_t numberBlockAllocated;
	size_t numberBlockHighWater;
};

class BlockConrange {
public:
	int32_t tailHi;
	int32_t tailLo;
};

class BlockMapDown
{
public:
	BlockConrange Block[(1 << ((64 - 0) - 20 - 2 * (((64 - 0) - 20 + 2) / 3)))];
};

class BlockMapMid
{
public:
	BlockMapDown* ptrs[(1 << (((64 - 0) - 20 + 2) / 3))];
};

class BlockMapTop {
public:
	BlockMapMid* ptrs[(1 << (((64 - 0) - 20 + 2) / 3))];
};

class Use {
public:
	BlockMapTop BlockMapRoot;
	int BlockMapMidCount;
	int BlockMapDownCount;
};

class MemoryGlobalState
{
public:
	int dumpDebugState;
	AlifSizeT interpreterLeaks;
};

class MemoryState
{
public:
	AlignmentsUsed alignments;
	MemoryManager mGmt;
	Use usage;
};

