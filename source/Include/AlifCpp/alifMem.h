#pragma once




















enum AlifMemAllocatorName {
	AlifMem_Allocator_Not_Set = 0,
	AlifMem_Allocator_Default = 1,
	AlifMem_Allocator_Debug = 2,
	AlifMem_Allocator_Malloc = 3,
	AlifMem_Allocator_Malloc_Debug = 4,
#ifdef WITH_PYMALLOC
	AlifMem_Allocator_AlifMalloc = 5,
	AlifMem_Allocator_AlifMalloc_Debug = 6,
#endif
};
