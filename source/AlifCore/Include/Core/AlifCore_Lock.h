#pragma once





// 17
#define ALIF_HAS_PARKED  2
#define ALIF_ONCE_INITIALIZED 4

static inline AlifIntT alifMutex_lockFast(uint8_t* _lockBits) { // 20
	uint8_t expected = ALIF_UNLOCKED;
	return alifAtomic_compareExchangeUint8(_lockBits, &expected, ALIF_LOCKED);
}


enum AlifLockFlags_ { // 41
	Alif_Lock_Dont_Detach = 0,
	Alif_Lock_Detach = 1,
	Alif_Lock_Handle_Signals = 2,
};




extern AlifLockStatus_ alifMutex_lockTimed(AlifMutex*, AlifTimeT, AlifLockFlags_); // 54

static inline void alifMutex_lockFlags(AlifMutex* m, AlifLockFlags_ flags) { // 58
	uint8_t expected = ALIF_UNLOCKED;
	if (!alifAtomic_compareExchangeUint8(&m->bits, &expected, ALIF_LOCKED)) {
		alifMutex_lockTimed(m, -1, flags);
	}
}


extern AlifIntT alifMutex_tryUnlock(AlifMutex*); // 69


class AlifEvent { // 73
public:
	uint8_t v_{};
};



AlifIntT alifEvent_waitTimed(AlifEvent*, AlifTimeT, AlifIntT); // 94



class AlifRawMutex { // 103
public:
	uintptr_t v{};
};



extern void alifRawMutex_lockSlow(AlifRawMutex*); // 108
extern void alifRawMutex_unlockSlow(AlifRawMutex*); // 109


static inline void alifRawMutex_unlock(AlifRawMutex* m) { // 131
	uintptr_t locked = ALIF_LOCKED;
	if (alifAtomic_compareExchangeUintptr(&m->v, &locked, ALIF_UNLOCKED)) {
		return;
	}
	alifRawMutex_unlockSlow(m);
}

typedef AlifIntT AlifOnceFnT(void* _arg); // 133


AlifIntT _alifOnceFlag_callOnceSlow(AlifOnceFlag*, AlifOnceFnT*, void*); // 137

static inline AlifIntT _alifOnceFlag_callOnce(AlifOnceFlag* _flag, AlifOnceFnT* _fn, void* _arg) { // 145
	if (alifAtomic_loadUint8(&_flag->v) == ALIF_ONCE_INITIALIZED) {
		return 0;
	}
	return _alifOnceFlag_callOnceSlow(_flag, _fn, _arg);
}





class AlifRWMutex { // 192
public:
	uintptr_t bits{};
};


void alifRWMutex_rLock(AlifRWMutex*); // 197
void alifRWMutex_rUnlock(AlifRWMutex*); // 198

void alifRWMutex_lock(AlifRWMutex*); // 201
void alifRWMutex_unlock(AlifRWMutex*);


class AlifSeqLock { // 213
public:
	uint32_t sequence{};
};




uint32_t alifSeqLock_beginRead(AlifSeqLock*); // 228


AlifIntT alifSeqLock_endRead(AlifSeqLock*, uint32_t); // 232
