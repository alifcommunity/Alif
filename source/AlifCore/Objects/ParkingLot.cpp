#include "alif.h"

#include "AlifCore_LList.h"
#include "AlifCore_Lock.h"
#include "AlifCore_ParkingLot.h"
#include "AlifCore_State.h"
#include "AlifCore_Semaphore.h"
#include "AlifCore_Time.h"



class Bucket { // 14
public:
	LListNode root{};
	AlifRawMutex mutex{};
	AlifUSizeT numWaiters{};
};

class WaitEntry { // 23
public:
	void* parkArg{};
	uintptr_t addr{};
	AlifSemaphore sema{};
	LListNode node{};
	bool isUnparking{};
};


// 35
#define NUM_BUCKETS 257

//#define BUCKET_INIT(b, i) [i] = { .root = LLIST_INIT(b[i].root) }
#define BUCKET_INIT(b, i) { .root = LLIST_INIT(b[i].root) }
#define BUCKET_INIT_2(b, i)   BUCKET_INIT(b, i),     BUCKET_INIT(b, i+1)
#define BUCKET_INIT_4(b, i)   BUCKET_INIT_2(b, i),   BUCKET_INIT_2(b, i+2)
#define BUCKET_INIT_8(b, i)   BUCKET_INIT_4(b, i),   BUCKET_INIT_4(b, i+4)
#define BUCKET_INIT_16(b, i)  BUCKET_INIT_8(b, i),   BUCKET_INIT_8(b, i+8)
#define BUCKET_INIT_32(b, i)  BUCKET_INIT_16(b, i),  BUCKET_INIT_16(b, i+16)
#define BUCKET_INIT_64(b, i)  BUCKET_INIT_32(b, i),  BUCKET_INIT_32(b, i+32)
#define BUCKET_INIT_128(b, i) BUCKET_INIT_64(b, i),  BUCKET_INIT_64(b, i+64)
#define BUCKET_INIT_256(b, i) BUCKET_INIT_128(b, i), BUCKET_INIT_128(b, i+128)

static Bucket buckets[NUM_BUCKETS] = { // 48
	BUCKET_INIT_256(buckets, 0),
	BUCKET_INIT(buckets, 256),
};


void alifSemaphore_init(AlifSemaphore* _sema) { // 53
#if defined(_WINDOWS)
	_sema->platformSem = CreateSemaphore(
		nullptr,	//  attributes
		0,			//  initial count
		10,			//  maximum count
		nullptr		//  unnamed
	);
	if (!_sema->platformSem) {
		//alif_fatalError("ParkingLot: CreateSemaphore failed");
		exit(-1);
	}
#elif defined(ALIF_USE_SEMAPHORES)
	if (sem_init(&_sema->platformSem, /*pshared=*/0, /*value=*/0) < 0) {
		//alif_fatalError("ParkingLot: sem_init failed");
		exit(-1);
	}
#else
	if (pthread_mutex_init(&_sema->mutex, nullptr) != 0) {
		//alif_fatalError("ParkingLot: pthread_mutex_init failed");
		exit(-1);
	}
	if (pthread_cond_init(&_sema->cond, nullptr)) {
		//alif_fatalError("ParkingLot: pthread_cond_init failed");
		exit(-1);
	}
	_sema->counter = 0;
#endif
}

void alifSemaphore_destroy(AlifSemaphore* _sema) { // 81
#if defined(_WINDOWS)
	CloseHandle(_sema->platformSem);
#elif defined(ALIF_USE_SEMAPHORES)
	sem_destroy(&_sema->platformSem);
#else
	pthread_mutex_destroy(&_sema->mutex);
	pthread_cond_destroy(&_sema->cond);
#endif
}

static AlifIntT alifSemaphore_platformWait(AlifSemaphore* sema, AlifTimeT timeout) { // 94
	AlifIntT res;
#if defined(_WINDOWS)
	DWORD wait{};
	DWORD millis = 0;
	if (timeout < 0) {
		millis = INFINITE;
	}
	else {
		AlifTimeT div = _alifTime_asMilliseconds(timeout, AlifTimeRoundT::AlifTime_Round_TIMEOUT);
		if ((AlifTimeT)ALIF_DWORD_MAX < div) {
			millis = ALIF_DWORD_MAX;
		}
		else {
			millis = (DWORD)div;
		}
	}
	wait = WaitForSingleObjectEx(sema->platformSem, millis, FALSE);
	if (wait == WAIT_OBJECT_0) {
		res = Alif_Park_Ok;
	}
	else if (wait == WAIT_TIMEOUT) {
		res = Alif_Park_Timeout;
	}
	else {
		res = Alif_Park_Intr;
	}
#elif defined(ALIF_USE_SEMAPHORES)
	AlifIntT err{};
	if (timeout >= 0) {
		struct timespec ts;

#if defined(CLOCK_MONOTONIC) and defined(HAVE_SEM_CLOCKWAIT)and !defined(ALIF_THREAD_SANITIZER)
		AlifTimeT now{};
		(void)alifTime_monotonicRaw(&now);
		AlifTimeT deadline = _alifTime_add(now, timeout);
		alifTime_asTimeSpecClamp(deadline, &ts);

		err = sem_clockwait(&sema->platformSem, CLOCK_MONOTONIC, &ts);
#else
		AlifTimeT now{};
		(void)alifTime_timeRaw(&now);
		AlifTimeT deadline = _alifTime_add(now, timeout);

		alifTime_asTimeSpecClamp(deadline, &ts);

		err = sem_timedwait(&sema->platformSem, &ts);
#endif
	}
	else {
		err = sem_wait(&sema->platformSem);
	}
	if (err == -1) {
		err = errno;
		if (err == EINTR) {
			res = Alif_Park_Intr;
		}
		else if (err == ETIMEDOUT) {
			res = Alif_Park_Timeout;
		}
		else {
			//alif_fatalErrorFormat(__func__,
			//	"unexpected error from semaphore: %d",
			//	err);
			return -1; // temp
		}
	}
	else {
		res = Alif_Park_Ok;
	}
//#else
	pthread_mutex_lock(&sema->mutex);
	AlifIntT err = 0;
	if (sema->counter == 0) {
		if (timeout >= 0) {
			struct timespec ts;
#if defined(HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE_NP)
			alifTime_asTimeSpecClamp(timeout, &ts);
			err = pthread_cond_timedwait_relative_np(&sema->cond, &sema->mutex, &ts);
#else
			AlifTimeT now{};
			(void)alifTime_timeRaw(&now);
			AlifTimeT deadline = _alifTime_add(now, timeout);
			alifTime_asTimeSpecClamp(deadline, &ts);

			err = pthread_cond_timedwait(&sema->cond, &sema->mutex, &ts);
#endif // HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE_NP
		}
		else {
			err = pthread_cond_wait(&sema->cond, &sema->mutex);
		}
	}
	if (sema->counter > 0) {
		sema->counter--;
		res = Alif_Park_Ok;
	}
	else if (err) {
		res = Alif_Park_Timeout;
	}
	else {
		res = Alif_Park_Intr;
	}
	pthread_mutex_unlock(&sema->mutex);
#endif
	return res;
}

static inline void alifRawMutex_lock(AlifRawMutex* m) { // 111
	uintptr_t unlocked = ALIF_UNLOCKED;
	if (alifAtomic_compareExchangeUintptr(&m->v, &unlocked, ALIF_LOCKED)) {
		return;
	}
	alifRawMutex_lockSlow(m);
}

AlifIntT alifSemaphore_wait(AlifSemaphore* _sema,
	AlifTimeT _timeout, AlifIntT _detach) { // 198
	AlifThread* thread = nullptr;
	if (_detach) {
		thread = _alifThread_get();
		if (thread and alifAtomic_loadIntRelaxed(&thread->state) ==
			ALIF_THREAD_ATTACHED) {
			// Only detach if we are attached
			alifEval_releaseThread(thread);
		}
		else {
			thread = nullptr;
		}
	}
	AlifIntT res = alifSemaphore_platformWait(_sema, _timeout);
	if (thread) {
		alifEval_acquireThread(thread);
	}
	return res;
}

void alifSemaphore_wakeup(AlifSemaphore* _sema) { // 220
#if defined(_WINDOWS)
	if (!ReleaseSemaphore(_sema->platformSem, 1, nullptr)) {
		//alif_fatalError("ParkingLot: ReleaseSemaphore failed");
		return; // temp
	}
#elif defined(ALIF_USE_SEMAPHORES)
	AlifIntT err = sem_post(&_sema->platform_sem);
	if (err != 0) {
		//alif_fatalError("ParkingLot: sem_post failed");
		return; // temp
	}
#else
	pthread_mutex_lock(&_sema->mutex);
	_sema->counter++;
	pthread_cond_signal(&_sema->cond);
	pthread_mutex_unlock(&_sema->mutex);
#endif
}

static void enqueue(Bucket* bucket, const void* address, WaitEntry* wait) { // 240
	llist_insertTail(&bucket->root, &wait->node);
	++bucket->numWaiters;
}

static WaitEntry* dequeue(Bucket* _bucket, const void* _address) { // 247
	LListNode* root = &_bucket->root;
	LListNode* node{};
	LLIST_FOR_EACH(node, root) {
		WaitEntry* wait = LLIST_DATA(node, WaitEntry, node);
		if (wait->addr == (uintptr_t)_address) {
			llist_remove(node);
			--_bucket->numWaiters;
			wait->isUnparking = true;
			return wait;
		}
	}
	return nullptr;
}

static void dequeue_all(Bucket* bucket, const void* address, LListNode* dst) { // 265
	LListNode* root = &bucket->root;
	LListNode* node{};
	LLIST_FOR_EACH_SAFE(node, root) {
		WaitEntry* wait = LLIST_DATA(node, WaitEntry, node);
		if (wait->addr == (uintptr_t)address) {
			llist_remove(node);
			llist_insertTail(dst, node);
			--bucket->numWaiters;
			wait->isUnparking = true;
		}
	}
}


static AlifIntT atomic_memcmp(const void* _addr,
	const void* _expected, AlifUSizeT _addrSize) { // 283
	switch (_addrSize) {
	case 1: return alifAtomic_loadUint8((const uint8_t*)_addr) == *(const uint8_t*)_expected;
	case 2: return alifAtomic_loadUint16((const uint16_t*)_addr) == *(const uint16_t*)_expected;
	case 4: return alifAtomic_loadUint32((const uint32_t*)_addr) == *(const uint32_t*)_expected;
	case 8: return alifAtomic_loadUint64((const uint64_t*)_addr) == *(const uint64_t*)_expected;
	default: ALIF_UNREACHABLE();
	}
}


AlifIntT alifParkingLot_park(const void* addr, const void* expected, AlifUSizeT size,
	AlifTimeT timeout_ns, void* park_arg, AlifIntT detach) { // 295
	WaitEntry wait = {
		.parkArg = park_arg,
		.addr = (uintptr_t)addr,
		.isUnparking = false,
	};

	Bucket* bucket = &buckets[((uintptr_t)addr) % NUM_BUCKETS];

	alifRawMutex_lock(&bucket->mutex);
	if (!atomic_memcmp(addr, expected, size)) {
		alifRawMutex_unlock(&bucket->mutex);
		return Alif_Park_Again;
	}
	alifSemaphore_init(&wait.sema);
	enqueue(bucket, addr, &wait);
	alifRawMutex_unlock(&bucket->mutex);

	AlifIntT res = alifSemaphore_wait(&wait.sema, timeout_ns, detach);
	if (res == Alif_Park_Ok) {
		goto done;
	}

	// timeout or interrupt
	alifRawMutex_lock(&bucket->mutex);
	if (wait.isUnparking) {
		alifRawMutex_unlock(&bucket->mutex);
		do {
			res = alifSemaphore_wait(&wait.sema, -1, detach);
		} while (res != Alif_Park_Ok);
		goto done;
	}
	else {
		llist_remove(&wait.node);
		--bucket->numWaiters;
	}
	alifRawMutex_unlock(&bucket->mutex);

done:
	alifSemaphore_destroy(&wait.sema);
	return res;

}



void alifParkingLot_unpark(const void* addr, AlifUnparkFnT* fn, void* arg) { // 344
	Bucket* bucket = &buckets[((uintptr_t)addr) % NUM_BUCKETS];

	alifRawMutex_lock(&bucket->mutex);
	WaitEntry* waiter = dequeue(bucket, addr);
	if (waiter) {
		AlifIntT has_more_waiters = (bucket->numWaiters > 0);
		fn(arg, waiter->parkArg, has_more_waiters);
	}
	else {
		fn(arg, nullptr, 0);
	}
	alifRawMutex_unlock(&bucket->mutex);

	if (waiter) {
		alifSemaphore_wakeup(&waiter->sema);
	}
}

void alifParkingLot_unparkAll(const void* _addr) { // 367
	LListNode head = LLIST_INIT(head);
	Bucket* bucket = &buckets[((uintptr_t)_addr) % NUM_BUCKETS];

	alifRawMutex_lock(&bucket->mutex);
	dequeue_all(bucket, _addr, &head);
	alifRawMutex_unlock(&bucket->mutex);

	LListNode* node{};
	LLIST_FOR_EACH_SAFE(node, &head) {
		WaitEntry* waiter = LLIST_DATA(node, WaitEntry, node);
		llist_remove(node);
		alifSemaphore_wakeup(&waiter->sema);
	}
}
