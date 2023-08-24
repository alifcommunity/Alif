#pragma once




















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
