#include "alif.h"

#include "AlifCore_Lock.h"
#include "AlifCore_ParkingLot.h"
#include "AlifCore_Time.h"
#include "AlifCore_Semaphore.h"



static const AlifTimeT _timeToBeFairNS_ = 1000 * 1000; // 20


static const AlifIntT _maxSpinCount_ = 40; // 26


class MutexEntry { // 31
public:
	AlifTimeT timeToBeFair{};
	AlifIntT handedOff{};
};


static void alif_yield(void) { // 40
#ifdef _WINDOWS
	SwitchToThread();
#elif defined(HAVE_SCHED_H)
	sched_yield();
#endif
}


AlifLockStatus_ alifMutex_lockTimed(AlifMutex* _m,
	AlifTimeT _timeout, AlifLockFlags_ _flags) { // 50
	uint8_t v = alifAtomic_loadUint8Relaxed(&_m->bits);
	if ((v & ALIF_LOCKED) == 0) {
		if (alifAtomic_compareExchangeUint8(&_m->bits, &v, v | ALIF_LOCKED)) {
			return AlifLockStatus_::Alif_Lock_Acquired;
		}
	}
	else if (_timeout == 0) {
		return AlifLockStatus_::Alif_Lock_Failure;
	}

	AlifTimeT now{};
	(void)alifTime_monotonicRaw(&now);
	AlifTimeT endtime = 0;
	if (_timeout > 0) {
		endtime = _alifTime_add(now, _timeout);
	}

	MutexEntry entry = {
		.timeToBeFair = now + _timeToBeFairNS_,
		.handedOff = 0,
	};

	AlifSizeT spin_count = 0;
	for (;;) {
		if ((v & ALIF_LOCKED) == 0) {
			if (alifAtomic_compareExchangeUint8(&_m->bits, &v, v | ALIF_LOCKED)) {
				return AlifLockStatus_::Alif_Lock_Acquired;
			}
			continue;
		}

		if (!(v & ALIF_HAS_PARKED) and spin_count < _maxSpinCount_) {
			alif_yield();
			spin_count++;
			continue;
		}

		if (_timeout == 0) {
			return AlifLockStatus_::Alif_Lock_Failure;
		}

		uint8_t newv = v;
		if (!(v & ALIF_HAS_PARKED)) {
			newv = v | ALIF_HAS_PARKED;
			if (!alifAtomic_compareExchangeUint8(&_m->bits, &v, newv)) {
				continue;
			}
		}

		int ret = alifParkingLot_park(&_m->bits, &newv, sizeof(newv), _timeout,
			&entry, (_flags & AlifLockFlags_::Alif_Lock_Detach) != 0);

		if (ret == Alif_Park_Ok) {
			if (entry.handedOff) {
				return AlifLockStatus_::Alif_Lock_Acquired;
			}
		}
		else if (ret == Alif_Park_Intr
			and (_flags & AlifLockFlags_::Alif_Lock_Handle_Signals)) {
			if (alif_makePendingCalls() < 0) {
				return Alif_Lock_Intr;
			}
		}
		else if (ret == Alif_Park_Timeout) {
			return AlifLockStatus_::Alif_Lock_Failure;
		}

		if (_timeout > 0) {
			_timeout = alifDeadline_get(endtime);
			if (_timeout <= 0) {
				_timeout = 0;
			}
		}

		v = alifAtomic_loadUint8Relaxed(&_m->bits);
	}
}

static void mutex_unpark(AlifMutex* _m, MutexEntry* _entry, AlifIntT _hasMoreWaiters) { // 137
	uint8_t v = 0;
	if (_entry) {
		AlifTimeT now{};
		(void)alifTime_monotonicRaw(&now);
		int should_be_fair = now > _entry->timeToBeFair;

		_entry->handedOff = should_be_fair;
		if (should_be_fair) {
			v |= ALIF_LOCKED;
		}
		if (_hasMoreWaiters) {
			v |= ALIF_HAS_PARKED;
		}
	}
	alifAtomic_storeUint8(&_m->bits, v);
}

AlifIntT alifMutex_tryUnlock(AlifMutex* _m) { // 158
	uint8_t v = alifAtomic_loadUint8(&_m->bits);
	for (;;) {
		if ((v & ALIF_LOCKED) == 0) {
			return -1;
		}
		else if ((v & ALIF_HAS_PARKED)) {
			alifParkingLot_unpark(&_m->bits, (AlifUnparkFnT*)mutex_unpark, _m);
			return 0;
		}
		else if (alifAtomic_compareExchangeUint8(&_m->bits, &v, ALIF_UNLOCKED)) {
			return 0;
		}
	}
}








class RawMutexEntry { // 181
public:
	RawMutexEntry* next{};
	AlifSemaphore sema{};
};

void alifRawMutex_lockSlow(AlifRawMutex* m) { // 186
	RawMutexEntry waiter;
	alifSemaphore_init(&waiter.sema);

	uintptr_t v = alifAtomic_loadUintptr(&m->v);
	for (;;) {
		if ((v & ALIF_LOCKED) == 0) {
			if (alifAtomic_compareExchangeUintptr(&m->v, &v, v | ALIF_LOCKED)) {
				break;
			}
			continue;
		}

		waiter.next = (RawMutexEntry*)(v & ~1);
		uintptr_t desired = ((uintptr_t)&waiter) | ALIF_LOCKED;
		if (!alifAtomic_compareExchangeUintptr(&m->v, &v, desired)) {
			continue;
		}

		alifSemaphore_wait(&waiter.sema, -1, /*detach=*/0);
	}

	alifSemaphore_destroy(&waiter.sema);
}




void alifRawMutex_unlockSlow(AlifRawMutex* _m) { // 217
	uintptr_t v = alifAtomic_loadUintptr(&_m->v);
	for (;;) {
		if ((v & ALIF_LOCKED) == 0) {
			//alif_fatalError("unlocking mutex that is not locked");
			return; // temp
		}

		RawMutexEntry* waiter = (RawMutexEntry*)(v & ~1);
		if (waiter) {
			uintptr_t next_waiter = (uintptr_t)waiter->next;
			if (alifAtomic_compareExchangeUintptr(&_m->v, &v, next_waiter)) {
				alifSemaphore_wakeup(&waiter->sema);
				return;
			}
		}
		else {
			if (alifAtomic_compareExchangeUintptr(&_m->v, &v, ALIF_UNLOCKED)) {
				return;
			}
		}
	}
}



AlifIntT alifEvent_waitTimed(AlifEvent* _evt, AlifTimeT _timeoutNS, AlifIntT _detach) { // 274
	for (;;) {
		uint8_t v = alifAtomic_loadUint8(&_evt->v_);
		if (v == ALIF_LOCKED) {
			// event already set
			return 1;
		}
		if (v == ALIF_UNLOCKED) {
			if (!alifAtomic_compareExchangeUint8(&_evt->v_, &v, ALIF_HAS_PARKED)) {
				continue;
			}
		}

		uint8_t expected = ALIF_HAS_PARKED;
		(void)alifParkingLot_park(&_evt->v_, &expected, sizeof(_evt->v_),
			_timeoutNS, nullptr, _detach);

		return alifAtomic_loadUint8(&_evt->v_) == ALIF_LOCKED;
	}
}









 // 397
#define ALIF_WRITE_LOCKED 1
#define ALIFRWMUTEX_READER_SHIFT 2
#define ALIF_RWMUTEX_MAX_READERS (UINTPTR_MAX >> ALIFRWMUTEX_READER_SHIFT)

static uintptr_t rwMutexSet_parkedAndWait(AlifRWMutex* _rwMutex, uintptr_t _bits) { // 401
	if ((_bits & ALIF_HAS_PARKED) == 0) {
		uintptr_t newval = _bits | ALIF_HAS_PARKED;
		if (!alifAtomic_compareExchangeUintptr(&_rwMutex->bits, &_bits, newval)) {
			return _bits;
		}
		_bits = newval;
	}

	alifParkingLot_park(&_rwMutex->bits, &_bits, sizeof(_bits), -1, nullptr, 1);
	return alifAtomic_loadUintptrRelaxed(&_rwMutex->bits);
}

static uintptr_t rwMutex_readerCount(uintptr_t _bits) { // 419
	return _bits >> ALIFRWMUTEX_READER_SHIFT;
}

void alifRWMutex_rLock(AlifRWMutex* _rwMutex) { // 425
	uintptr_t bits = alifAtomic_loadUintptrRelaxed(&_rwMutex->bits);
	for (;;) {
		if ((bits & ALIF_WRITE_LOCKED)) {
			// A writer already holds the lock.
			bits = rwMutexSet_parkedAndWait(_rwMutex, bits);
			continue;
		}
		else if ((bits & ALIF_HAS_PARKED)) {
			bits = rwMutexSet_parkedAndWait(_rwMutex, bits);
			continue;
		}
		else {
			// The lock is unlocked or read-locked. Try to grab it.
			uintptr_t newval = bits + (1 << ALIFRWMUTEX_READER_SHIFT);
			if (!alifAtomic_compareExchangeUintptr(&_rwMutex->bits,
				&bits, newval)) {
				continue;
			}
			return;
		}
	}
}



void alifRWMutex_rUnlock(AlifRWMutex* _rwMutex) { // 456
	uintptr_t bits = alifAtomic_addUintptr(&_rwMutex->bits, -(1 << ALIFRWMUTEX_READER_SHIFT));
	bits -= (1 << ALIFRWMUTEX_READER_SHIFT);

	if (rwMutex_readerCount(bits) == 0 and (bits & ALIF_HAS_PARKED)) {
		alifParkingLot_unparkAll(&_rwMutex->bits);
		return;
	}
}


void alifRWMutex_lock(AlifRWMutex* _rwMutex) { // 469
	uintptr_t bits = alifAtomic_loadUintptrRelaxed(&_rwMutex->bits);
	for (;;) {
		if ((bits & ~ALIF_HAS_PARKED) == 0) {
			if (!alifAtomic_compareExchangeUintptr(&_rwMutex->bits,
				&bits,
				bits | ALIF_WRITE_LOCKED)) {
				continue;
			}
			return;
		}

		// Otherwise, we have to wait.
		bits = rwMutexSet_parkedAndWait(_rwMutex, bits);
	}
}




void alifRWMutex_unlock(AlifRWMutex* _rwMutex) { // 490
	uintptr_t oldBits = alifAtomic_exchangeUintptr(&_rwMutex->bits, 0);

	if ((oldBits & ALIF_HAS_PARKED) != 0) {
		alifParkingLot_unparkAll(&_rwMutex->bits);
	}
}


#define SEQLOCK_IS_UPDATING(_sequence) (_sequence & 0x01) // 503



uint32_t alifSeqLock_beginRead(AlifSeqLock* _seqLock) { // 540
	uint32_t sequence = alifAtomic_loadUint32Acquire(&_seqLock->sequence);
	while (SEQLOCK_IS_UPDATING(sequence)) {
		alif_yield();
		sequence = alifAtomic_loadUint32Acquire(&_seqLock->sequence);
	}

	return sequence;
}


void alifMutex_lock(AlifMutex* _m) { // 579
	alifMutex_lockTimed(_m, -1, AlifLockFlags_::Alif_Lock_Detach);
}



void alifMutex_unlock(AlifMutex* _m) { // 586
	if (alifMutex_tryUnlock(_m) < 0) {
		//alif_fatalError("unlocking mutex that is not locked");
		return; // temp
	}
}
