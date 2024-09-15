#pragma once

#include "AlifCore_Lock.h"
#include "AlifCore_State.h"


// 19
#define ALIF_CRITICAL_SECTION_INACTIVE       0x1
#define ALIF_CRITICAL_SECTION_TWO_MUTEXES    0x2
#define ALIF_CRITICAL_SECTION_MASK           0x3


// 23
#ifdef ALIF_GIL_DISABLED
#define ALIF_BEGIN_CRITICAL_SECTION_MUT(mutex)                           \
    {                                                                   \
        AlifCriticalSection critSec{};                                       \
        _alifCriticalSection_beginMutex(&critSec, mutex)

#define ALIF_BEGIN_CRITICAL_SECTION2_MUT(m1, m2)                         \
    {                                                                   \
        AlifCriticalSection2 critSec2{};                                     \
        _alifCriticalSection2_beginMutex(&critSec2, m1, m2)


#else 
#define ALIF_BEGIN_CRITICAL_SECTION_MUT(mut) {
#define ALIF_BEGIN_CRITICAL_SECTION2_MUT(m1, m2) {
#define ALIF_BEGIN_CRITICAL_SECTION_SEQUENCE_FAST(original) {
#define ALIF_END_CRITICAL_SECTION_SEQUENCE_FAST() }
#define ALIF_CRITICAL_SECTION_ASSERT_MUTEX_LOCKED(mutex)
#define ALIF_CRITICAL_SECTION_ASSERT_OBJECT_LOCKED(op)
#endif


void alifCriticalSection_resume(AlifThread*); // 89

void alifCriticalSection_beginSlow(AlifCriticalSection*, AlifMutex*); // 92
void alifCriticalSection2_beginSlow(AlifCriticalSection2*, AlifMutex*, AlifMutex*, AlifIntT); // 95
void alifCriticalSection_suspendAll(AlifThread*); // 99


#ifdef ALIF_GIL_DISABLED

static inline AlifIntT alifCriticalSection_isActive(uintptr_t _tag) { // 103
	return _tag != 0 and (_tag & ALIF_CRITICAL_SECTION_INACTIVE) == 0;
}


static inline void _alifCriticalSection_beginMutex(AlifCriticalSection* _c,
	AlifMutex* _m) { // 109
	if (alifMutex_lockFast(&_m->bits)) {
		AlifThread* thread = alifThread_get();
		_c->mutex = _m;
		_c->prev = thread->criticalSection;
		thread->criticalSection = (uintptr_t)_c;
	}
	else {
		alifCriticalSection_beginSlow(_c, _m);
	}
}


static inline void _alifCriticalSection_begin(AlifCriticalSection* _c,
	AlifObject* _op) { // 123
	_alifCriticalSection_beginMutex(_c, &_op->mutex);
}
#define ALIFCRITICALSECTION_BEGIN _alifCriticalSection_begin


static inline void _alifCriticalSection_pop(AlifCriticalSection* _c) { // 133
	AlifThread* thread = alifThread_get();
	uintptr_t prev = _c->prev;
	thread->criticalSection = prev;

	if ((prev & ALIF_CRITICAL_SECTION_INACTIVE) != 0) {
		alifCriticalSection_resume(thread);
	}
}


static inline void _alifCriticalSection_end(AlifCriticalSection* _c) { // 145
	ALIFMUTEX_UNLOCK(_c->mutex);
	_alifCriticalSection_pop(_c);
}
#define ALIFCRITICALSECTION_END _alifCriticalSection_end


static inline void _alifCriticalSection2_beginMutex(AlifCriticalSection2* _c,
	AlifMutex* _m1, AlifMutex* _m2) { // 153
	if (_m1 == _m2) {
		_c->mutex2 = nullptr;
		_alifCriticalSection_beginMutex(&_c->base, _m1);
		return;
	}

	if ((uintptr_t)_m2 < (uintptr_t)_m1) {
		AlifMutex* tmp = _m1;
		_m1 = _m2;
		_m2 = tmp;
	}

	if (alifMutex_lockFast(&_m1->bits)) {
		if (alifMutex_lockFast(&_m2->bits)) {
			AlifThread* tstate = alifThread_get();
			_c->base.mutex = _m1;
			_c->mutex2 = _m2;
			_c->base.prev = tstate->criticalSection;

			uintptr_t p = (uintptr_t)_c | ALIF_CRITICAL_SECTION_TWO_MUTEXES;
			tstate->criticalSection = p;
		}
		else {
			alifCriticalSection2_beginSlow(_c, _m1, _m2, 1);
		}
	}
	else {
		alifCriticalSection2_beginSlow(_c, _m1, _m2, 0);
	}
}
















#endif
