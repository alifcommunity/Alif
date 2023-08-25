#pragma once





#include "alifCore_alifMem.h"









extern void* alifMem_rawMalloc(void* , size_t);
extern void* alifMem_rawCalloc(void* , size_t , size_t );
extern void* alifMem_rawRealloc(void* , void* , size_t );
extern void alifMem_rawFree(void* , void* );
#define ALIFRAW_ALLOC {nullptr, alifMem_rawMalloc, alifMem_rawCalloc, alifMem_rawRealloc, alifMem_rawFree}

#ifdef WITH_ALIFMALLOC
extern void* _alifObject_malloc(void* , size_t );
extern void* _alifObject_calloc(void* , size_t , size_t );
extern void* _alifObject_realloc(void* , void* , size_t );
extern void  _alifObject_free(void* , void* );
#  define ALIFOBJ_ALLOC {nullptr, _alifObject_malloc, _alifObject_calloc, _alifObject_realloc, _alifObject_free}
#else
#define ALIFOBJ_ALLOC ALIFRAW_ALLOC
#endif

#define ALIFMEM_ALLOC ALIFOBJ_ALLOC

extern void* _alifMem_debugRawMalloc(void* , size_t );
extern void* _alifMem_debugRawCalloc(void* , size_t , size_t );
extern void* _alifMem_debugRawRealloc(void* , void* , size_t );
extern void  _alifMem_debugRawFree(void* , void* );

extern void* _alifMem_debugMalloc(void* , size_t );
extern void* _alifMem_debugCalloc(void* , size_t , size_t );
extern void* _alifMem_debugRealloc(void* , void* , size_t );
extern void _alifMem_debugFree(void* , void* );

#define ALIFDEBUGRAW_ALLOC(runtime) {&(runtime).allocators.debug.raw, _alifMem_debugRawMalloc,  _alifMem_debugRawCalloc, _alifMem_debugRawRealloc, _alifMem_debugRawFree, }


#define ALIFDEBUGMEM_ALLOC(runtime) {&(runtime).allocators.debug.mem, _alifMem_debugMalloc, _alifMem_debugCalloc, _alifMem_debugRealloc, _alifMem_debugFree, }


#define ALIFDEBUGOBJ_ALLOC(runtime) {&(runtime).allocators.debug.obj, _alifMem_debugMalloc, _alifMem_debugCalloc, _alifMem_debugRealloc, _alifMem_debugFree, }

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
