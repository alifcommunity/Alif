#pragma once



AlifSizeT alifThread_getThreadID() { // 260

    //if (!initialized) alifThread_initThread();

    return GetCurrentThreadId();
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
