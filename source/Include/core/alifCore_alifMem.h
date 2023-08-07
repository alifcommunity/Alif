#pragma once

class DebugAllocateAPI {
public:
	/* We tag each block with an API ID in order to tag API violations */
	char apiID;
	AlifMemAllocatorExternal alloc;
};

class AlifMemAllocators {
public:
	void* mutex;
	class Standard {
	public:
		AlifMemAllocatorExternal raw;
		AlifMemAllocatorExternal mem;
		AlifMemAllocatorExternal obj;
	}standard;
	class Debug {
	public:
		DebugAllocateAPI raw;
		DebugAllocateAPI mem;
		DebugAllocateAPI obj;
	}debug;
	AlifObjectArenaAllocator objArena;
};

#define ALIFMEM_CLEANBYTE      0xCD
#define ALIFMEM_DEADBYTE       0xDD
#define ALIFMEM_FORBIDDENBYTE  0xFD
