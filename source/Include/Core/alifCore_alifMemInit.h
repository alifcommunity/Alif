#pragma once










#include "alifCore_alifMem.h"




extern void* _alifMem_rawMalloc(void* , size_t);
extern void* _alifMem_rawCalloc(void* , size_t , size_t );
extern void* _alifMem_rawRealloc(void* , void* , size_t );
extern void _alifMem_rawFree(void* , void* );
#define ALIFRAW_ALLOC {nullptr, _alifMem_rawMalloc, _alifMem_rawCalloc, _alifMem_rawRealloc, _alifMem_rawFree}

#ifdef WITH_ALIFMALLOC
extern void* alifObject_malloc(void* , size_t );
extern void* alifObject_calloc(void* , size_t , size_t );
extern void* alifObject_realloc(void* , void* , size_t );
extern void  alifObject_free(void* , void* );
#  define ALIFOBJ_ALLOC {nullptr, alifObject_malloc, alifObject_calloc, alifObject_realloc, alifObject_free}
#else
#define ALIFOBJ_ALLOC ALIFRAW_ALLOC
#endif

#define ALIFMEM_ALLOC ALIFOBJ_ALLOC

extern void* alifMem_debugRawMalloc(void* , size_t );
extern void* alifMem_debugRawCalloc(void* , size_t , size_t );
extern void* alifMem_debugRawRealloc(void* , void* , size_t );
extern void  alifMem_debugRawFree(void* , void* );

extern void* alifMem_debugMalloc(void* , size_t );
extern void* alifMem_debugCalloc(void* , size_t , size_t );
extern void* alifMem_debugRealloc(void* , void* , size_t );
extern void alifMem_debugFree(void* , void* );

#define ALIFDEBUGRAW_ALLOC(runtime) {&(runtime).allocators.debug.raw, alifMem_debugRawMalloc,  alifMem_debugRawCalloc, alifMem_debugRawRealloc, alifMem_debugRawFree, }


#define ALIFDEBUGMEM_ALLOC(runtime) {&(runtime).allocators.debug.mem, alifMem_debugMalloc, alifMem_debugCalloc, alifMem_debugRealloc, alifMem_debugFree, }


#define ALIFDEBUGOBJ_ALLOC(runtime) {&(runtime).allocators.debug.obj, alifMem_debugMalloc, alifMem_debugCalloc, alifMem_debugRealloc, alifMem_debugFree, }

extern void* alifMem_arenaAlloc(void*, size_t);
extern void alifMem_arenaFree(void*, void*, size_t);

#ifdef ALIF_DEBUG
#define ALIFMEM_ALLOCATORSSTANDERD_INIT(runtime) { ALIFDEBUGRAW_ALLOC(runtime), ALIFDEBUGMEM_ALLOC(runtime), ALIFDEBUGOBJ_ALLOC(runtime), }
#else
#define ALIFMEM_ALLOCATORSSTANDERD_INIT(runtime)\
{ \
	ALIFRAW_ALLOC, \
	ALIFMEM_ALLOC, \
	ALIFOBJ_ALLOC, \
}
#endif

#define ALIFMEM_ALLOCATORSDEBUG_INIT {	{'r', ALIFRAW_ALLOC}, {'m', ALIFMEM_ALLOC}, {'o', ALIFOBJ_ALLOC}, }


#define ALIFMEM_ALLOCATORSOBJ_ARENAINIT { nullptr, alifMem_arenaAlloc, alifMem_arenaFree }
