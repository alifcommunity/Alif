#pragma once










typedef unsigned int alifMemUint;  /* assuming >= 16 bits */

#undef  uint
#define uint alifMemUint



















































































































#if SIZEOF_VOID_P > 4
#define ALIGNMENT              16               /* must be 2^N */
#define ALIGNMENT_SHIFT         4
#else
#define ALIGNMENT               8               /* must be 2^N */
#define ALIGNMENT_SHIFT         3
#endif


#define INDEX2SIZE(I) (((alifMemUint)(I) + 1) << ALIGNMENT_SHIFT)
















#define SMALL_REQUEST_THRESHOLD 512
#define NBSMALL_SIZE_CLASSES   (SMALL_REQUEST_THRESHOLD / ALIGNMENT)











#define SYSTEM_PAGE_SIZE        (4 * 1024)




#ifdef WITH_MEMORY_LIMITS
#ifndef SMALL_MEMORY_LIMIT
#define SMALL_MEMORY_LIMIT      (64 * 1024 * 1024)      /* 64 MB -- more? */
#endif
#endif

#if !defined(WITH_ALIFMALLOC_RADIXTREE)



#define WITH_ALIFMALLOC_RADIXTREE 1
#endif

#if SIZEOF_VOID_P > 4

#define USE_LARGE_ARENAS
#if WITH_ALIFMALLOC_RADIXTREE

#define USE_LARGE_POOLS
#endif
#endif














#ifdef USE_LARGE_ARENAS
#define ARENA_BITS              20                    /* 1 MiB */
#else
#define ARENA_BITS              18                    /* 256 KiB */
#endif
#define ARENA_SIZE              (1 << ARENA_BITS)
#define ARENA_SIZE_MASK         (ARENA_SIZE - 1)

#ifdef WITH_MEMORY_LIMITS
#define MAX_ARENAS              (SMALL_MEMORY_LIMIT / ARENA_SIZE)
#endif




#ifdef USE_LARGE_POOLS
#define POOL_BITS               14                  /* 16 KiB */
#else
#define POOL_BITS               12                  /* 4 KiB */
#endif
#define POOL_SIZE               (1 << POOL_BITS)
#define POOL_SIZE_MASK          (POOL_SIZE - 1)







#define MAX_POOLS_INARENA  (ARENA_SIZE / POOL_SIZE)











typedef uint8_t AlifMemBlock;


class poolHeader {
public:
	union {
		AlifMemBlock* padding;
		uint count;
	} ref;          /* number of allocated blocks    */
	AlifMemBlock* freeBlock;             /* pool's free list head         */
	poolHeader* nextPool;       /* next pool of this size class  */
	poolHeader* prevPool;       /* previous pool       ""        */
	uint arenaIndex;                    /* index into arenas of base adr */
	uint szidx;                         /* block size class index        */
	uint nextOffset;                    /* bytes to virgin block         */
	uint maxNextOffset;                 /* largest valid nextoffset      */
};

typedef poolHeader* PoolP;

class ArenaObject{
public:




	uintptr_t address;


	AlifMemBlock* poolAddress;




	uint nFreePools;


	uint nTotalPools;


	poolHeader* freePools;













	ArenaObject* nextArena;
	ArenaObject* prevArena;
};

#define POOL_OVERHEAD   ALIFSIZE_ROUND_UP(sizeof(poolHeader), ALIGNMENT)

#define DUMMY_SIZE_IDX          0xffff  /* size class of newly cached pools */

/* Round pointer P down to the closest pool-aligned address <= P, as a PoolP */
#define POOL_ADDR(P) ((PoolP)ALIFALIGN_DOWN((P), POOL_SIZE))

/* Return total number of blocks in pool of size index I, as a uint. */
#define NUMBLOCKS(I) ((alifMemUint)(POOL_SIZE - POOL_OVERHEAD) / INDEX2SIZE(I))



































































































#define OBMALLOC_USEDPOOLS_SIZE (2 * ((NBSMALL_SIZE_CLASSES + 7) / 8) * 8)

class ObmallocPools {
public:
	PoolP used[OBMALLOC_USEDPOOLS_SIZE];
};




















































#define INITIAL_ARENA_OBJECTS 16

class ObmallocMGmt {
public:

	ArenaObject* arenas;

	uint maxArenas;



	ArenaObject* unusedArenaObjects;




	ArenaObject* usableArenas;


	ArenaObject* nFP2Lasta[MAX_POOLS_INARENA + 1];


	size_t nArenasCurrentlyAllocated;


	size_t nTimesArenaAllocated;

	size_t nArenasHighwater;

	AlifSizeT rawAllocatedBlocks;
};


#if WITH_ALIFMALLOC_RADIXTREE





























#if SIZEOF_VOID_P == 8


#define POINTER_BITS 64












#define IGNORE_BITS 0


#define USE_INTERIOR_NODES

#elif SIZEOF_VOID_P == 4

#define POINTER_BITS 32
#define IGNORE_BITS 0






#endif /* SIZEOF_VOID_P */







#define ADDRESS_BITS (POINTER_BITS - IGNORE_BITS)

#ifdef USE_INTERIOR_NODES

#define INTERIOR_BITS ((ADDRESS_BITS - ARENA_BITS + 2) / 3)
#else
#define INTERIOR_BITS 0
#endif

#define MAP_TOP_BITS INTERIOR_BITS
#define MAP_TOP_LENGTH (1 << MAP_TOP_BITS)
#define MAP_TOP_MASK (MAP_TOP_LENGTH - 1)

#define MAP_MID_BITS INTERIOR_BITS
#define MAP_MID_LENGTH (1 << MAP_MID_BITS)
#define MAP_MID_MASK (MAP_MID_LENGTH - 1)

#define MAP_BOT_BITS (ADDRESS_BITS - ARENA_BITS - 2*INTERIOR_BITS)
#define MAP_BOT_LENGTH (1 << MAP_BOT_BITS)
#define MAP_BOT_MASK (MAP_BOT_LENGTH - 1)

#define MAP_BOT_SHIFT ARENA_BITS
#define MAP_MID_SHIFT (MAP_BOT_BITS + MAP_BOT_SHIFT)
#define MAP_TOP_SHIFT (MAP_MID_BITS + MAP_MID_SHIFT)

#define AS_UINT(p) ((uintptr_t)(p))
#define MAP_BOT_INDEX(p) ((AS_UINT(p) >> MAP_BOT_SHIFT) & MAP_BOT_MASK)
#define MAP_MID_INDEX(p) ((AS_UINT(p) >> MAP_MID_SHIFT) & MAP_MID_MASK)
#define MAP_TOP_INDEX(p) ((AS_UINT(p) >> MAP_TOP_SHIFT) & MAP_TOP_MASK)

#if IGNORE_BITS > 0



#define HIGH_BITS(p) (AS_UINT(p) >> ADDRESS_BITS)
#else
#define HIGH_BITS(p) 0
#endif




class ArenaCoverageT {
public:
	int32_t tailHi;
	int32_t tailLo;
} ;
class ArenaMapBotT {

public:


	ArenaCoverageT arenas[MAP_BOT_LENGTH];
} ;

#ifdef USE_INTERIOR_NODES
class ArenaMapMidT {
public:
	ArenaMapBotT* ptrs[MAP_MID_LENGTH];
};
class ArenaMapTopT {
public:
	ArenaMapMidT* ptrs[MAP_TOP_LENGTH];
} ;
#endif
class ObmallocUsage {
public:



#ifdef USE_INTERIOR_NODES
	ArenaMapTopT arenaMapRoot;

	int arenaMapMidCount;
	int arenaMapBotCount;
#else
	ArenaMapBotT arenaMapRoot;
#endif
};
#endif

class ObmallocGlobalState {
public:
	int dumpDebugStats;
	AlifSizeT interpreterLeaks;
};
class ObmallocState {
public:
	ObmallocPools pools;
	ObmallocMGmt mGmt;
	ObmallocUsage usage;
};

#undef  uint




void* alifObject_virtualAlloc(size_t size);
void alifObject_virtualFree(void*, size_t size);










#ifdef WITH_ALIFMALLOC
ALIFAPI_FUNC(int) alifObject_debugMallocStats(FILE* out);
#endif
