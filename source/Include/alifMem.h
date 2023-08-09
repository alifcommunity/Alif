#pragma once

enum AlifMemAllocateDomain{
	/* AlifMem_RawMalloc(), AlifMem_RawRealloc() and AlifMem_RawFree() */
	ALIFMEM_DOMAIN_RAW,

	/* AlifMem_Malloc(), AlifMem_Realloc() and AlifMem_Free() */
	ALIFMEM_DOMAIN_MEM,

	/* AlifObject_Malloc(), AlifObject_Realloc() and AlifObject_Free() */
	ALIFMEM_DOMAIN_OBJ
};

enum AlifMemAllocatorName {
	ALIFMEM_ALLOCATOR_NOT_SET = 0,
	ALIFMEM_ALLOCATOR_DEFAULT = 1,
	ALIFMEM_ALLOCATOR_DEBUG = 2,
	ALIFMEM_ALLOCATOR_MALLOC = 3,
	ALIFMEM_ALLOCATOR_MALLOC_DEBUG = 4,
#ifdef WITH_ALIFMALLOC
	PYMEM_ALLOCATOR_PYMALLOC = 5,
	PYMEM_ALLOCATOR_PYMALLOC_DEBUG = 6,
#endif
};

class AlifMemAllocatorExternal {

public:
	/* user context passed as the first argument to the 4 functions */
	void* ctx;

	/* allocate a memory block */
	void* (*malloc) (void* ctx, size_t size);

	/* allocate a memory block initialized by zeros */
	void* (*calloc) (void* ctx, size_t nElement, size_t elSize);

	/* allocate or resize a memory block */
	void* (*realloc) (void* ctx, void* ptr, size_t newSize);

	/* release a memory block */
	void (*free) (void* ctx, void* ptr);
};

class AlifObjectArenaAllocator {
public:
	/* user context passed as the first argument to the 2 functions */
	void* ctx;

	/* allocate an arena of size bytes */
	void* (*alloc) (void* ctx, size_t size);

	/* free an arena */
	void (*free) (void* ctx, void* ptr, size_t size);
} ;
