#pragma once
#include "alif.h"

// سيتم نقلها الى ملف alifThread عند الانتهاء من runtime كامل

#define NATIVE_TSS_KEY_T unsigned long

struct AlifTSST {
	int _is_initialized;
	NATIVE_TSS_KEY_T _key;
};

///////////////////////////////////////////////////////////////////////////

class AlifRuntimeState
{
public:

	int _initialized;

	/* Is running Py_PreInitialize()? */
	int preinitializing;

	/* Is Python preinitialized? Set to 1 by Py_PreInitialize() */
	int preinitialized;

	/* Is Python core initialized? Set to 1 by _Py_InitializeCore() */
	int core_initialized;

	/* Is Python fully initialized? Set to 1 by Py_Initialize() */
	int initialized;

	volatile uintptr_t finalizing;

	class AlifInterpreters
	{
	public:

		void* mutex;

		int64_t nextID;

	};

	unsigned long main_thread;

	/* Used for the thread state bound to the current thread. */
	AlifTSST autoTSSkey;

	/* Used instead of PyThreadState.trash when there is not current tstate. */
	AlifTSST trashTSSkey;

};

