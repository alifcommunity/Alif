#pragma once










typedef unsigned int AlifMemUint;  /* assuming >= 16 bits */

#undef  uint
#define uint AlifMemUint



















































































































#if SIZEOF_VOID_P > 4
#define ALIGNMENT              16               /* must be 2^N */
#define ALIGNMENT_SHIFT         4
#else
#define ALIGNMENT               8               /* must be 2^N */
#define ALIGNMENT_SHIFT         3
#endif



















#define SMALL_REQUEST_THRESHOLD 512
#define NB_SMALL_SIZE_CLASSES   (SMALL_REQUEST_THRESHOLD / ALIGNMENT)



















































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

/*
 * Size of the pools used for small blocks.  Must be a power of 2.
 */
#ifdef USE_LARGE_POOLS
#define POOL_BITS               14                  /* 16 KiB */
#else
#define POOL_BITS               12                  /* 4 KiB */
#endif
#define POOL_SIZE               (1 << POOL_BITS)
#define POOL_SIZE_MASK          (POOL_SIZE - 1)







#define MAX_POOLS_IN_ARENA  (ARENA_SIZE / POOL_SIZE)
#if MAX_POOLS_IN_ARENA * POOL_SIZE != ARENA_SIZE
#   error "arena size not an exact multiple of pool size"
#endif





/* When you say memory, my mind reasons in terms of (pointers to) blocks */
typedef uint8_t AlifMemBlock;

/* Pool for small blocks. */
struct PoolHeader {
	union {
		AlifMemBlock* padding;
		uint count;
	} ref;          /* number of allocated blocks    */
	AlifMemBlock* freeBlock;             /* pool's free list head         */
	PoolHeader* nextPool;       /* next pool of this size class  */
	PoolHeader* prevPool;       /* previous pool       ""        */
	uint arenaindex;                    /* index into arenas of base adr */
	uint szidx;                         /* block size class index        */
	uint nextoffset;                    /* bytes to virgin block         */
	uint maxnextoffset;                 /* largest valid nextoffset      */
};

typedef class PoolHeader* PoolP;


class ArenaObject {
public:
	uintptr_t address;

	AlifMemBlock* poolAddress;

	uint nfreePools;
	uint ntotalPools;
	PoolHeader* freePools;

	ArenaObject* nextArena;
	ArenaObject* prevArena;
};









































































































































#define OBMALLOC_USED_POOLS_SIZE (2 * ((NB_SMALL_SIZE_CLASSES + 7) / 8) * 8)

struct ObmallocPools {
	PoolP used[OBMALLOC_USED_POOLS_SIZE];
};






















































class ObmallocMgmt {
public:
	/* Array of objects used to track chunks of memory (arenas). */
	ArenaObject* arenas;
	/* Number of slots currently allocated in the `arenas` vector. */
	uint maxArenas;

	/* The head of the singly-linked, NULL-terminated list of available
	 * arena_objects.
	 */
	ArenaObject* unusedArenaObjects;

	/* The head of the doubly-linked, NULL-terminated at each end, list of
	 * arena_objects associated with arenas that have pools available.
	 */
	ArenaObject* usableArenas;

	/* nfp2lasta[nfp] is the last arena in usable_arenas with nfp free pools */
	ArenaObject* nfp2Lasta[MAX_POOLS_IN_ARENA + 1];

	/* Number of arenas allocated that haven't been free()'d. */
	size_t narenasCurrentlyAllocated;

	/* Total number of times malloc() called to allocate an arena. */
	size_t ntimesArenaAllocated;
	/* High water mark (max value ever seen) for narenas_currently_allocated. */
	size_t narenasHighwater;

	AlifSizeT rawAllocatedBlocks;
};



































#define POINTER_BITS 64










#define IGNORE_BITS 0


#define USE_INTERIOR_NODES














#if ARENA_BITS >= 32
#   error "حجم الاكتلة يجب ان يكون < 2^32"
#endif


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













class ArenaCoverageT {
	int32_t tailHi;
	int32_t tailLo;
};

class ArenaMapBot {
public:
	ArenaCoverageT arenas[MAP_BOT_LENGTH];
};




#ifdef USE_INTERIOR_NODES
class ArenaMapMidT {
public:
	ArenaMapBot* ptrs[MAP_MID_LENGTH];
};

class ArenaMapTopT {
public:
	ArenaMapMidT* ptrs[MAP_TOP_LENGTH];
};
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












class ObmallocState {
public:
	ObmallocPools pools;
	ObmallocMgmt mgmt;
	ObmallocUsage usage;
};
