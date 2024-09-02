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


ALIF_LOCAL_INLINE(AlifIntT) alifCond_WAIT(AlifCondT* _cv, AlifMutexT* _cs) { // 276
	return SleepConditionVariableSRW(_cv, _cs, INFINITE, 0) ? 0 : -1;
}

ALIF_LOCAL_INLINE(AlifIntT) alifCond_TIMEDWAIT(AlifCondT* _cv,
	AlifMutexT* _cs, long long _us) { // 283
	BOOL success = SleepConditionVariableSRW(_cv, _cs, (DWORD)(_us / 1000), 0);
	if (!success) {
		if (GetLastError() == ERROR_TIMEOUT) {
			return 1;
		}
		return -1;
	}
	return 0;
}

ALIF_LOCAL_INLINE(AlifIntT) alifCond_SIGNAL(AlifCondT* _cv) { // 296
	WakeConditionVariable(_cv);
	return 0;
}
