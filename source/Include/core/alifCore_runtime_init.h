#pragma once

#include "alifCore_alifMem_init.h"
#include "alifCore_alifMemory_init.h"
#include "alifThread.h"

#define ALLOCATORS(runtime) { .standard = ALIFMEM_ALLOCATORS_STANDERD_INIT(alifRuntime) , .debug = ALIFMEM_ALLOCATORS_DEBUG_INIT , .objArena = ALIFMEM_ALLOCATORS_OBJ_ARENA_INIT }

#define ALIFRUNTIMESTATE_INIT(runtime) { .allocators = ALLOCATORS(0) , .memory = ALIFMEMORY_GLOBAL_STATE_INIT , .autoTSSKey = ALIF_TSS_NEEDS_INIT , };
