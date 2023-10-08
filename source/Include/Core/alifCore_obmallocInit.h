#pragma once













#define PTA(pools, x) \
    ((PoolP)((uint8_t *)&(pools.used[2*(x)]) - 2*sizeof(AlifMemBlock *)))
#define PT(p, x)   PTA(p, x), PTA(p, x)

#define PT_8(p, start) \
    PT(p, start), \
    PT(p, start+1), \
    PT(p, start+2), \
    PT(p, start+3), \
    PT(p, start+4), \
    PT(p, start+5), \
    PT(p, start+6), \
    PT(p, start+7)

#if (512 / 16) <= 8
#  define ALIFMEMORY_ALIGNMENTS_INIT(p) \
    { PT_8(p, 0) }
#elif (512 / 16) <= 16
#  define ALIFMEMORY_ALIGNMENTS_INIT(p) \
    { PT_8(p, 0), PT_8(p, 8) }
#elif (512 / 16) <= 24
#  define ALIFMEMORY_ALIGNMENTS_INIT(p) \
    { PT_8(p, 0), PT_8(p, 8), PT_8(p, 16) }
#elif (512 / 16) <= 32
#  define ALIFMEMORY_ALIGNMENTS_INIT(p) \
    { PT_8(p, 0), PT_8(p, 8), PT_8(p, 16), PT_8(p, 24) }
#elif (512 / 16) <= 40
#  define ALIFMEMORY_ALIGNMENTS_INIT(p) \
    { PT_8(p, 0), PT_8(p, 8), PT_8(p, 16), PT_8(p, 24), PT_8(p, 32) }
#elif (512 / 16) <= 48
#  define ALIFMEMORY_ALIGNMENTS_INIT(p) \
    { PT_8(p, 0), PT_8(p, 8), PT_8(p, 16), PT_8(p, 24), PT_8(p, 32), PT_8(p, 40) }
#elif (512 / 16) <= 56
#  define ALIFMEMORY_ALIGNMENTS_INIT(p) \
    { PT_8(p, 0), PT_8(p, 8), PT_8(p, 16), PT_8(p, 24), PT_8(p, 32), PT_8(p, 40), PT_8(p, 48) }
#elif (512 / 16) <= 64
#  define ALIFMEMORY_ALIGNMENTS_INIT(p) \
    { PT_8(p, 0), PT_8(p, 8), PT_8(p, 16), PT_8(p, 24), PT_8(p, 32), PT_8(p, 40), PT_8(p, 48), PT_8(p, 56) }
#endif

#define ALIFMEMORY_GLOBALSTATE_INIT {.dumpDebugStats = -1}

#define ALIFMEMORY_STATE_INIT(obmalloc){ .pools = {.used = ALIFMEMORY_ALIGNMENTS_INIT(obmalloc.pools)}}
