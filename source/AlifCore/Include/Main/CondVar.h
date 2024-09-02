#pragma once




















ALIF_LOCAL_INLINE(AlifIntT) alifMutex_INIT(AlifMutexT* _cs) { // 236
	InitializeSRWLock(_cs);
	return 0;
}


ALIF_LOCAL_INLINE(AlifIntT) alifMutex_FINI(AlifMutexT* _cs) { // 243
	return 0;
}

ALIF_LOCAL_INLINE(AlifIntT) alifMutex_LOCK(AlifMutexT* _cs) { // 249
	AcquireSRWLockExclusive(_cs);
	return 0;
}

ALIF_LOCAL_INLINE(AlifIntT) alifMutex_UNLOCK(AlifMutexT* _cs) { // 256
	ReleaseSRWLockExclusive(_cs);
	return 0;
}

ALIF_LOCAL_INLINE(AlifIntT) alifCond_INIT(AlifCondT* _cv) { // 263
	InitializeConditionVariable(_cv);
	return 0;
}

ALIF_LOCAL_INLINE(AlifIntT) alifCond_FINI(AlifCondT* _cv) { // 276
	return 0;
}
