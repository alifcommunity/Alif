#pragma once


#include "AlifCore_Thread.h"

























AlifUIntT alifThread_getThreadID() { // 365
	volatile pthread_t threadID{};

	if (!INITIALIZED) INITIALIZED = 1;

	threadID = pthread_self();
	return (AlifUSizeT)threadID;
}




void ALIF_NO_RETURN alifThread_exitThread(void) { // 406
	if (!INITIALIZED)
		exit(0);
#if defined(__wasi__)
	abort();
#else
	pthread_exit(0);
#endif
}




AlifIntT alifThread_acquireLock(AlifThreadTypeLock lock, AlifIntT waitflag) { // 799
	//return alifThread_acquireLockTimed(lock, waitflag ? -1 : 0, /*intr_flag=*/0);
	return 0; //
}




AlifIntT alifThreadTSS_create(AlifTssT* _key) { // 935
	/* If the key has been created, function is silently skipped. */
	if (_key->isInitialized) {
		return 0;
	}

	AlifIntT fail = pthread_key_create(&(_key->key), nullptr);
	if (fail) {
		return -1;
	}
	_key->isInitialized = 1;
	return 0;
}





void alifThreadTSS_delete(AlifTssT* _key) { // 952
	/* If the key has not been created, function is silently skipped. */
	if (!_key->isInitialized) {
		return;
	}

	pthread_key_delete(_key->key);
	/* pthread has not provided the defined invalid value for the key. */
	_key->isInitialized = 0;
}


AlifIntT alifThreadTSS_set(AlifTSST* _key, void* _value) { // 966
	AlifIntT fail = pthread_setspecific(_key->key, _value);
	return fail ? -1 : 0;
}

void* alifThreadTSS_get(AlifTssT* _key) { // 974
	return pthread_getspecific(_key->key);
}
