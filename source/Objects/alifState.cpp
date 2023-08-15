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

inline int tState_tss_initialized(_alifTSST* key)
{
	return alifThread_tss_isCreated(key);
}

inline int tState_tss_init(_alifTSST* key)
{
	return alifThread_tss_create(key);
}

inline void tState_tss_fini(_alifTSST* key)
{
	alifThread_tss_delete(key);
}

inline AlifThreadState* tstate_tss_get(_alifTSST* key)
{
	return (AlifThreadState*)alifThread_tss_get(key);
}

inline int tState_tss_set(_alifTSST* key, AlifThreadState* tstate)
{
	return alifThread_tss_set(key, (void*)tstate);
}

inline int tState_tss_clear(_alifTSST* key)
{
	return alifThread_tss_set(key, (void*)nullptr);
}

#ifdef HAVE_FORK

AlifStatus tState_tss_reinit(_alifTSST* key)
{
	if (!tState_tss_initialized(key)) {
		return ALIFStatus_OK();
	}
	alifThreadState* tState = tState_tss_get(key);

	tState_tss_fini(key);
	if (tState_tss_init(key) != 0) {
		return ALIFStatus_NO_MEMORY();
	}

	if (tState && tState_tss_set(key, tState) != 0) {
		return ALIFStatus_ERR("failed to re-set autoTSSkey");
	}
	return ALIFStatus_OK();
}
#endif

#define GITSTATETSS_INITIALIZEF(runtime) \
    tState_tss_initialized(&(runtime)->autoTSSKey)
#define GITSTATETSS_INIT(runtime) \
    tState_tss_init(&(runtime)->autoTSSKey)
#define GITSTATETSS_FINI(runtime) \
    tState_tss_fini(&(runtime)->autoTSSKey)
#define GITSTATETSS_GET(runtime) \
    tstate_tss_get(&(runtime)->autoTSSKey)
#define GITSTATETSS_SET(runtime, tstate) \
    tState_tss_set(&(runtime)->autoTSSKey, tstate)
#define _gilstate_tss_clear(runtime) \
    tState_tss_clear(&(runtime)->autoTSSKey)
#define gilstate_tss_reinit(runtime) \
    tState_tss_reinit(&(runtime)->autoTSSKey)

static const AlifRuntimeState initial = ALIFRUNTIMESTATE_INIT(alifRuntime);

#define NUMLOCKS 9
#define LOCKS_INIT(runtime) \
    { \
        &(runtime)->AlifInterpreters.mutex, \
        &(runtime)->auditHooks.mutex, \
        &(runtime)->allocators.mutex, \
    }

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
	AlifSizeT unicodeNextIndex = _runtime->unicodeState.ids.nextIndex;

	AlifThreadTypeLock locks[NUMLOCKS];
	if (alloc_forRuntime(locks) != 0) {
		return ALIFSTATUS_NO_MEMORY();
	}

	if (_runtime->initialized) {
		// alif_initialize() must be running again.
		// Reset to AlifRuntimeState_INIT.
		memcpy(_runtime, &initial, sizeof(*_runtime));
	}

	if (GITSTATETSS_INIT(_runtime) != 0) {
		alifRuntimeState_fini(_runtime);
		return ALIFSTATUS_NO_MEMORY();
	}

	if (alifThread_tss_create(&_runtime->trashTSSKey) != 0) {
		alifRuntimeState_fini(_runtime);
		return ALIFSTATUS_NO_MEMORY();
	}

	init_runtime(_runtime, openCodeHook, openCodeUserData, auditHookHead, unicodeNextIndex, locks);

	return ALIFSTATUS_OK();
}

void alifRuntimeState_fini(AlifRuntimeState* runtime)
{
	if (GITSTATETSS_INITIALIZEF(runtime)) {
		GITSTATETSS_FINI(runtime);
	}

	if (alifThread_tss_isCreated(&runtime->trashTSSKey)) {
		alifThread_tss_delete(&runtime->trashTSSKey);
	}

	AlifMemAllocatorExternal old_alloc;
	alifMem_setDefaultAllocator(ALIFMEM_DOMAIN_RAW, &old_alloc);
#define FREE_LOCK(LOCK) \
    if (LOCK != NULL) { \
        alifThread_freeLock(LOCK); \
        LOCK = NULL; \
    }

	void* lockptrs[NUMLOCKS] = LOCKS_INIT(runtime);
	for (int i = 0; i < NUMLOCKS; i++) {
		FREE_LOCK(*lockptrs[i]);
	}

#undef FREE_LOCK
	alifMem_setAllocator(ALIFMEM_DOMAIN_RAW, &old_alloc);
}
