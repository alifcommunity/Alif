#include "alif.h"

#include "AlifCore_LList.h"
#include "AlifCore_Lock.h"
#include "AlifCore_ParkingLot.h"
#include "AlifCore_State.h"
#include "AlifCore_Semaphore.h"
#include "AlifCore_Time.h"



class Bucket { // 14
public:
	AlifRawMutex mutex{};
	LListNode root{};
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

#define BUCKET_INIT(b, i) [i] = { .root = LLIST_INIT(b[i].root) }
#define BUCKET_INIT_2(b, i)   BUCKET_INIT(b, i),     BUCKET_INIT(b, i+1)
#define BUCKET_INIT_4(b, i)   BUCKET_INIT_2(b, i),   BUCKET_INIT_2(b, i+2)
#define BUCKET_INIT_8(b, i)   BUCKET_INIT_4(b, i),   BUCKET_INIT_4(b, i+4)
#define BUCKET_INIT_16(b, i)  BUCKET_INIT_8(b, i),   BUCKET_INIT_8(b, i+8)
#define BUCKET_INIT_32(b, i)  BUCKET_INIT_16(b, i),  BUCKET_INIT_16(b, i+16)
#define BUCKET_INIT_64(b, i)  BUCKET_INIT_32(b, i),  BUCKET_INIT_32(b, i+32)
#define BUCKET_INIT_128(b, i) BUCKET_INIT_64(b, i),  BUCKET_INIT_64(b, i+64)
#define BUCKET_INIT_256(b, i) BUCKET_INIT_128(b, i), BUCKET_INIT_128(b, i+128)

static Bucket buckets[NUM_BUCKETS] = { // 48
	//BUCKET_INIT_256(buckets, 0),
	//BUCKET_INIT(buckets, 256),
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
		alif_fatalError("ParkingLot: pthread_mutex_init failed");
		exit(-1);
	}
	if (pthread_cond_init(&_sema->cond, nullptr)) {
		alif_fatalError("ParkingLot: pthread_cond_init failed");
		exit(-1);
	}
	_sema->counter = 0;
#endif
}



static inline void alifRawMutex_lock(AlifRawMutex* m) { // 111
	uintptr_t unlocked = ALIF_UNLOCKED;
	if (alifAtomic_compareExchangeUintptr(&m->v, &unlocked, ALIF_LOCKED)) {
		return;
	}
	alifRawMutex_lockSlow(m);
}

AlifIntT alifSemaphore_wait(AlifSemaphore* sema, AlifTimeT timeout, AlifIntT detach) { // 198
	AlifThread* thread = nullptr;
	if (detach) {
		thread = alifThread_get();
		if (thread and alifAtomic_loadIntRelaxed(&thread->state) ==
			ALIF_THREAD_ATTACHED) {
			// Only detach if we are attached
			alifEval_releaseThread(thread);
		}
		else {
			thread = nullptr;
		}
	}
	int res = alifSemaphore_platformWait(sema, timeout);
	if (thread) {
		alifEval_acquireThread(thread);
	}
	return res;
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

	int res = alifSemaphore_wait(&wait.sema, timeout_ns, detach);
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
