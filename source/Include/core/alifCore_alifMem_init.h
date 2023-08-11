#pragma once

#include "alifCore_alifMem.h"
#include "alifMemory.h"
#include "alifConfig.h"

void* alifMem_raw_malloc(void* ctx, size_t size);
void* alifMem_raw_calloc(void* ctx, size_t nElement, size_t elSize);
void* alifMem_raw_realloc(void* ctx, void* ptr, size_t size);
void alifMem_raw_free(void* ctx, void* ptr);
#define MALLOC_ALLOC {nullptr, alifMem_raw_malloc, alifMem_raw_calloc, alifMem_raw_realloc, alifMem_raw_free}
#define ALIFRAW_ALLOC MALLOC_ALLOC

#ifdef WITH_ALIFMALLOC
extern void* object_malloc(void* ctx, size_t numberByte);
extern void* object_calloc(void* ctx, size_t nElement, size_t elSize);
extern void* object_realloc(void* ctx, void* ptr, size_t numberByte);
extern void  object_free(void* ctx, void* ptr);
#  define ALIFOBJ_ALLOC {nullptr, object_malloc, object_calloc, object_realloc, object_free}
#else
#define ALIFOBJ_ALLOC ALIFRAW_ALLOC
#endif

#define ALIFMEM_ALLOC ALIFOBJ_ALLOC

extern void* alifMem_debug_raw_malloc(void* ctx, size_t numberByte);
extern void* alifMem_debug_raw_calloc(void* ctx, size_t nElement, size_t elSize);
extern void* alifMem_debug_raw_realloc(void* ctx, void* ptr, size_t nByte);
extern void  alifMem_debug_raw_free(void* ctx, void* ptr);

extern void* alifMem_malloc(void* ctx, size_t size);
extern void* alifMem_calloc(void* ctx, size_t nElement, size_t elSize);
extern void* alifMem_realloc(void* ctx, void* ptr, size_t newSize);
extern void alifMem_free(void* ctx, void* ptr);

#define ALIFDEBUGRAW_ALLOC(runtime) {&(runtime).allocators.debug.raw, alifMem_debug_raw_malloc,  alifMem_debug_raw_calloc, alifMem_debug_raw_realloc, alifMem_debug_raw_free, }

#define ALIFDEBUGMEM_ALLOC(runtime) {&(runtime).allocators.debug.mem, alifMem_malloc, alifMem_calloc, alifMem_realloc, alifMem_free, }

#define ALIFDEBUGOBJ_ALLOC(runtime) {&(runtime).allocators.debug.obj, alifMem_malloc, alifMem_calloc, alifMem_realloc, alifMem_free, }

extern void* alifMem_arenaAlloc(void*, size_t);
extern void alifMem_arenaFree(void*, void*, size_t);

#ifdef ALIF_DEBUG
#define ALIFMEM_ALLOCATORS_STANDERD_INIT(runtime) { ALIFDEBUGRAW_ALLOC(runtime), ALIFDEBUGMEM_ALLOC(runtime), ALIFDEBUGOBJ_ALLOC(runtime), }
#else
#define ALIFMEM_ALLOCATORS_STANDERD_INIT(runtime)\
{ \
	ALIFRAW_ALLOC, \
	ALIFMEM_ALLOC, \
	ALIFOBJ_ALLOC, \
}
#endif

#define ALIFMEM_ALLOCATORS_DEBUG_INIT {	{'r', ALIFRAW_ALLOC}, {'m', ALIFMEM_ALLOC}, {'o', ALIFOBJ_ALLOC}, }

#define ALIFMEM_ALLOCATORS_OBJ_ARENA_INIT { nullptr, alifMem_arenaAlloc, alifMem_arenaFree }
