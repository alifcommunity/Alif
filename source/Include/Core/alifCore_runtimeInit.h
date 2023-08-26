#pragma once












#include "alifCore_alifMemInit.h"
#include "alifCore_obmallocInit.h"








#define ALIFRUNTIMESTATE_INIT(runtime) \
	{\
	.allocators = { \
		.standard = ALIFMEM_ALLOCATORSSTANDERD_INIT(runtime), \
		.debug = ALIFMEM_ALLOCATORSDEBUG_INIT, \
		.objArena = ALIFMEM_ALLOCATORSOBJ_ARENAINIT, \
		}, \
	.obmalloc = ALIFMEMORY_GLOBALSTATE_INIT, \
	\
	}
