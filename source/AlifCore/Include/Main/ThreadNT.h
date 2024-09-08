#pragma once

#include <windows.h>
#include <limits.h>
//#ifdef HAVE_PROCESS_H
#include <process.h>
//#endif


AlifUIntT alifThread_getThreadID() { // 260

    if (!INITIALIZED) INITIALIZED = 1;

    return GetCurrentThreadId();
}




void ALIF_NO_RETURN alifThread_exitThread(void) { // 286
	if (!INITIALIZED)
		exit(0);
	_endthreadex(0);
}


AlifIntT alifThread_acquireLock(AlifThreadTypeLock aLock, AlifIntT waitflag) { // 374
	//return alifThread_acquireLockTimed(aLock, waitflag ? -1 : 0, 0);
	return 0; //
}











AlifIntT alifThreadTSS_create(AlifTssT* _key) { // 472
	/* If the key has been created, function is silently skipped. */
	if (_key->isInitialized) {
		return 0;
	}

	DWORD result = TlsAlloc();
	if (result == TLS_OUT_OF_INDEXES) {
		return -1;
	}
	/* In Windows, platform-specific key type is DWORD. */
	_key->key = result;
	_key->isInitialized = 1;
	return 0;
}



void alifThreadTSS_delete(AlifTssT* _key) { // 491
	/* If the key has not been created, function is silently skipped. */
	if (!_key->isInitialized) {
		return;
	}

	TlsFree(_key->key);
	_key->key = TLS_OUT_OF_INDEXES;
	_key->isInitialized = 0;
}


AlifIntT alifThreadTSS_set(AlifTssT* _key, void* _value) { // 505
	BOOL ok = TlsSetValue(_key->key, _value);
	return ok ? 0 : -1;
}

void* alifThreadTSS_get(AlifTssT* _key) { // 513
	int err = GetLastError();
	void* r = TlsGetValue(_key->key);
	if (r or !GetLastError()) {
		SetLastError(err);
	}
	return r;
}
