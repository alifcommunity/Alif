#pragma once

#include "AlifCore_Lock.h"
#include "AlifCore_State.h"


// 19
#define ALIF_CRITICAL_SECTION_INACTIVE       0x1
#define ALIF_CRITICAL_SECTION_TWO_MUTEXES    0x2
#define ALIF_CRITICAL_SECTION_MASK           0x3






void alifCriticalSection_resume(AlifThread*); // 89

void alifCriticalSection_beginSlow(AlifCriticalSection*, AlifMutex*); // 92

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




#endif
