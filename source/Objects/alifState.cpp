#include "alif.h"
#include "alifCore_alifCycle.h"
#include "alifCore_initConfig.h"
#include "alifCore_alifMem.h"
#include "alifCore_runtime_init.h"


#ifdef HAVE_DLOPEN
	#ifdef HAVE_DLFCN_H
	#include <dlfcn.h>
	#endif
	#if !HAVE_DECL_RTLD_LAZY
	#define RTLD_LAZY 1
	#endif
#endif


static const AlifRuntimeState initial = ALIFRUNTIMESTATE_INIT(alifRuntime);

#define NUMLOCKS 9


static int alloc_forRuntime(AlifThreadTypeLock locks[NUMLOCKS]) {

	AlifMemAllocatorExternal oldAlloc{};

	alifMem_setDefaultAllocator(ALIFMEM_DOMAIN_RAW, &oldAlloc);

	for (int i = 0; i < NUMLOCKS; i++) {
		AlifThreadTypeLock lock = alifThread_allocateLock();
		if (lock == nullptr) {
			for (int j = 0; j < i; j++) {
				alifThread_freeLock(locks[j]);
				locks[j] = nullptr;
			}
			break;
		}
		locks[i] = lock;
	}

	alifMem_setAllocator(ALIFMEM_DOMAIN_RAW, &oldAlloc);
	return 0;
}

AlifStatus alifRuntimeState_init(AlifRuntimeState* _runtime)
{
	/* We preserve the hook across init, because there is
	   currently no public API to set it between runtime
	   initialization and interpreter initialization. */
	void* openCodeHook = _runtime->openCodeHook;
	void* openCodeUserData = _runtime->openCodeUserData;
	AlifAuditHookEntry* auditHookHead = _runtime->auditHooks.head;
	// Preserve nextIndex value if alif_initialize()/alif_finalize()
	// is called multiple times.
	AlifSizeT unicode_next_index = _runtime->unicodeState.ids.nextIndex;

	AlifThreadTypeLock locks[NUMLOCKS];
	if (alloc_forRuntime(locks) != 0) {
		return ALIFSTATUS_NO_MEMORY();
	}

	if (_runtime->initialized) {
		// alif_initialize() must be running again.
		// Reset to AlifRuntimeState_INIT.
		memcpy(_runtime, &initial, sizeof(*_runtime));
	}

	if (gilStateTss_init(_runtime) != 0) {
		alifRuntimeState_fini(_runtime);
		return ALIFSTATUS_NO_MEMORY();
	}

	if (alifThreadTss_create(&_runtime->trashTSSkey) != 0) {
		alifRuntimeState_fini(_runtime);
		return ALIFSTATUS_NO_MEMORY();
	}

	init_runtime(_runtime, openCodeHook, openCodeUserData, auditHookHead, unicode_next_index, locks);

	return ALIFSTATUS_OK();
}
