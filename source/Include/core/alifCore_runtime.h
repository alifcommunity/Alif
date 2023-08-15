#pragma once

#include "alifCore_alifMem.h"
#include "alifCore_alifThread.h"
#include "alifCore_unicodeObject.h"

///////////////////////////////////////////////////////////////////////////


/* Runtime audit hook state */

class AlifAuditHookEntry {
public:
	AlifAuditHookEntry* next;
	AlifAuditHookFunction hookCFunction;
	void* userData;
};


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

	}alifInterpreters; // هنا اضطررت لادراج اسم له ليكون متوافق مع ال macro 

	unsigned long main_thread;

	AlifMemAllocators allocators;
	MemoryGlobalState memory;
	AlifThreadRuntimeState threads;

	AlifTSST autoTSSKey;

	AlifTSST trashTSSKey;

	AlifPreConfig preConfig;

	AlifOpenCodeHookFunction openCodeHook;
	void* openCodeUserData;
	class {
	public:
		AlifThreadTypeLock mutex;
		AlifAuditHookEntry* head;
	} auditHooks;

	struct AlifUnicodeRuntimeState unicodeState;
};


extern AlifStatus alifRuntimeState_init(AlifRuntimeState*);

/* Initialize alifRuntimeState.
   Return nullptr on success, or return an error message on failure. */
extern AlifStatus alifRuntime_initialize();
