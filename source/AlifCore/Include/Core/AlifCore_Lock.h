#pragma once





// 17
#define ALIF_HAS_PARKED  2
#define ALIF_ONCE_INITIALIZED 4






enum AlifLockFlags_ { // 41
	Alif_Lock_Dont_Detach = 0,
	Alif_Lock_Detach = 1,
	Alif_Lock_Handle_Signals = 2,
};




extern AlifLockStatus_ alifMutex_lockTimed(AlifMutex*, AlifTimeT, AlifLockFlags_); // 54


extern AlifIntT alifMutex_tryUnlock(AlifMutex*); // 69





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
