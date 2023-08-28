#pragma once



ALIFAPI_FUNC(void*) alifMem_rawMalloc(size_t size);
ALIFAPI_FUNC(void*) alifMem_rawCalloc(size_t nelem, size_t elsize);
ALIFAPI_FUNC(void*) alifMem_rawRealloc(void* ptr, size_t new_size);
ALIFAPI_FUNC(void) alifMem_rawFree(void* ptr);


enum AlifMemAllocatorDomain {
	AlifMem_Domain_Raw,
	AlifMem_Domain_Mem,
	AlifMem_Domain_Obj
};






enum AlifMemAllocatorName {
	AlifMem_Allocator_NotSet = 0,
	AlifMem_Allocator_Default = 1,
	AlifMem_Allocator_Debug = 2,
	AlifMem_Allocator_Malloc = 3,
	AlifMem_Allocator_MallocDebug = 4,
#ifdef WITH_ALIFMALLOC
	AlifMem_Allocator_AlifMalloc = 5,
	AlifMem_Allocator_AlifMallocDebug = 6,
#endif
};


class AlifMemAllocatorEx {
public:
	void* ctx;


	void* (*malloc) (void* ctx, size_t size);


	void* (*calloc) (void* ctx, size_t nElem, size_t elSize);


	void* (*realloc) (void* ctx, void* ptr, size_t newSize);


	void (*free) (void* ctx, void* ptr);
};


ALIFAPI_FUNC(void) alifMem_getAllocator(AlifMemAllocatorDomain, AlifMemAllocatorEx*);













ALIFAPI_FUNC(void) alifMem_setAllocator(AlifMemAllocatorDomain, AlifMemAllocatorEx*);


















ALIFAPI_FUNC(void) alifMem_setupDebugHooks();








