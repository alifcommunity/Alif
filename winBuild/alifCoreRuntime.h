#pragma once
#include "alif.h"

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

};

