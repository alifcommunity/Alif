#pragma once

#include "alifMemory.h"
#include "alifCore_alifMem.h"
#include "alifCore_alifThread.h"

///////////////////////////////////////////////////////////////////////////

class AlifRuntimeState
{
public:

	int _initialized;

	int preinitializing;

	int preinitialized;

	int core_initialized;

	int initialized;

	volatile uintptr_t finalizing;

	class AlifInterpreters
	{
	public:

		void* mutex;

		int64_t nextID;

	};

	unsigned long main_thread;

	AlifMemAllocators allocators;
	MemoryGlobalState memory;
	AlifThreadRuntimeState threads;

	AlifTSST autoTSSKey;

	AlifTSST trashTSSKey;



};

