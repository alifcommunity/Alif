#pragma once



// This contains code for allocating unique indices in an array. It is used by
// the free-threaded build to assign each thread a globally unique index into
// each code object's thread-local bytecode array.

// A min-heap of indices
class AlifIndexHeap {
public:
	int32_t* values{};

	// Number of items stored in values
	AlifSizeT size{};

	// Maximum number of items that can be stored in values
	AlifSizeT capacity{};
};

// An unbounded pool of indices. Indices are allocated starting from 0. They
// may be released back to the pool once they are no longer in use.
class AlifIndexPool {
public:
	AlifMutex mutex{};

	// Min heap of indices available for allocation
	AlifIndexHeap freeIndices{};

	// Next index to allocate if no free indices are available
	int32_t nextIndex{};
};

// Allocate the smallest available index. Returns -1 on error.
extern int32_t _alifIndexPool_allocIndex(AlifIndexPool*);

// Release `index` back to the pool
extern void _alifIndexPool_freeIndex(AlifIndexPool*, int32_t);

extern void _alifIndexPool_fini(AlifIndexPool*);
