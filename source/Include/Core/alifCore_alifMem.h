#pragma once
























class DebugAllocApiT {
public:
	char apiID;
	AlifMemAllocatorEx alloc;
} ;

class AlifMemAllocators {
public:
	AlifThreadTypeLock mutex;
	class {
	public:
		AlifMemAllocatorEx raw;
		AlifMemAllocatorEx mem;
		AlifMemAllocatorEx obj;
	} standard;
	class {
	public:
		DebugAllocApiT raw;
		DebugAllocApiT mem;
		DebugAllocApiT obj;
	} debug;
	AlifObjectArenaAllocator objArena;
};


extern int alifMem_setDefaultAllocator(
	AlifMemAllocatorDomain domain,
	AlifMemAllocatorEx* old_alloc);












#define ALIFMEM_CLEANBYTE      0xCD
#define ALIFMEM_DEADBYTE       0xDD
#define ALIFMEM_FORBIDDENBYTE  0xFD
