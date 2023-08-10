#include "alif.h"
//#include "alifCore_alifCycle.h"
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

#define NUMLOCKS 9


int alloc_for_runtime(void* locks[NUMLOCKS]) {

	AlifMemAllocatorExternal oldAlloc{};

	alifMem_setDefaultAllocator(ALIFMEM_DOMAIN_RAW, &oldAlloc);

	for (int i = 0; i < NUMLOCKS; i++) {
		void* lock = alifThread_allocate_lock();
		if (lock == NULL) {
			for (int j = 0; j < i; j++) {
				alifThread_free_lock(locks[j]);
				locks[j] = NULL;
			}
			break;
		}
		locks[i] = lock;
	}

	alifMem_setAllocator(ALIFMEM_DOMAIN_RAW, &oldAlloc);
	return 0;
}

AlifStatus alifRuntimeState_init(AlifRuntimeState* runtime) {

	void* locks[NUMLOCKS];

	if (alloc_for_runtime(locks) != 0) {
		return ALIFSTATUS_NO_MEMORY();
	}

	return (AlifStatus)0;
}
