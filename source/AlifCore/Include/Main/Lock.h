#pragma once


#define ALIF_UNLOCKED    0
#define ALIF_LOCKED      1







class AlifMutex { // 29
public:
	uint8_t bits;  // (private)
};


void alifMutex_lock(AlifMutex*); // 34
void alifMutex_unlock(AlifMutex*); // 37

static inline void _alifMutex_lock(AlifMutex* m) { // 44
	uint8_t expected = ALIF_UNLOCKED;
	if (!alifAtomic_compareExchangeUint8(&m->bits, &expected, ALIF_LOCKED)) {
		alifMutex_lock(m);
	}
}
#define ALIFMUTEX_LOCK _alifMutex_lock


static inline void _alifMutex_unlock(AlifMutex* m) { // 55
	uint8_t expected = ALIF_LOCKED;
	if (!alifAtomic_compareExchangeUint8(&m->bits, &expected, ALIF_UNLOCKED)) {
		alifMutex_unlock(m);
	}
}
#define ALIFMUTEX_UNLOCK _alifMutex_unlock
