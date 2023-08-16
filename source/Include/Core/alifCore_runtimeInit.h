#pragma once

#include "alifCore_alifMemInit.h"
#include "alifCore_alifMemoryInit.h"
#include "alifThread.h"

#define ALLOCATORS(runtime)  { .standard = ALIFMEM_ALLOCATORS_STANDERD_INIT(runtime) , .debug = ALIFMEM_ALLOCATORS_DEBUG_INIT , .objArena = ALIFMEM_ALLOCATORS_OBJ_ARENA_INIT, }

#define ALIFRUNTIMESTATE_INIT(runtime) { .allocators = ALLOCATORS(runtime) , .memory = ALIFMEMORY_GLOBAL_STATE_INIT , .autoTSSKey = ALIF_TSS_NEEDS_INIT , };
