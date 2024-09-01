#pragma once


























ALIF_LOCAL_INLINE(AlifIntT) alifMutex_LOCK(AlifMutexT* cs) { // 249
	AcquireSRWLockExclusive(cs);
	return 0;
}

ALIF_LOCAL_INLINE(AlifIntT) alifMutex_UNLOCK(AlifMutexT* cs) { // 256
	ReleaseSRWLockExclusive(cs);
	return 0;
}
