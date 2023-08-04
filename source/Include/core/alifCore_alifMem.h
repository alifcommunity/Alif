#pragma once

class AlifMemAllocatorExternal {

public:
	/* user context passed as the first argument to the 4 functions */
	void* ctx;

	/* allocate a memory block */
	void* (*malloc) (void* ctx, size_t size);

	/* allocate a memory block initialized by zeros */
	void* (*calloc) (void* ctx, size_t nelem, size_t elsize);

	/* allocate or resize a memory block */
	void* (*realloc) (void* ctx, void* ptr, size_t new_size);

	/* release a memory block */
	void (*free) (void* ctx, void* ptr);
};

class DebugAllocateAPI {
public:
	/* We tag each block with an API ID in order to tag API violations */
	char api_id;
	AlifMemAllocatorExternal alloc;
};

class AlifMemAllocators {
public:
	void* mutex;
	struct {
		AlifMemAllocatorExternal raw;
		AlifMemAllocatorExternal mem;
		AlifMemAllocatorExternal obj;
	} standard;
	struct {
		DebugAllocateAPI raw;
		DebugAllocateAPI mem;
		DebugAllocateAPI obj;
	} debug;
	AlifMemAllocatorExternal obj_arena;
};

#define ALIFMEM_CLEANBYTE      0xCD
#define ALIFMEM_DEADBYTE       0xDD
#define ALIFMEM_FORBIDDENBYTE  0xFD
