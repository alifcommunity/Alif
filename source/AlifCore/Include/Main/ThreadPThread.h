#pragma once




























AlifSizeT alifThread_getThreadID() { // 365
	volatile pthread_t threadID{};

	//if (!initialized) alifThread_initThread();

	threadID = pthread_self();
	return (AlifSizeT)threadID;
}







AlifIntT alifThreadTSS_create(AlifTssT* _key) { // 935
	/* If the key has been created, function is silently skipped. */
	if (_key->isInitialized) {
		return 0;
	}

	AlifIntT fail = pthreadKey_create(&(key->_key), nullptr);
	if (fail) {
		return -1;
	}
	_key->isInitialized = 1;
	return 0;
}
