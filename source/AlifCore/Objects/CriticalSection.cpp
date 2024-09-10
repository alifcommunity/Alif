#include "alif.h"

#include "AlifCore_Lock.h"
#include "AlifCore_CriticalSection.h"






void alifCriticalSection_beginSlow(AlifCriticalSection* _c, AlifMutex* _m) { // 11
#ifdef ALIF_GIL_DISABLED
	AlifThread* thread = alifThread_get();
	_c->mutex = nullptr;
	_c->prev = (uintptr_t)thread->criticalSection;
	thread->criticalSection = (uintptr_t)_c;

	ALIFMUTEX_LOCK(_m);
	_c->mutex = _m;
#endif
}






#ifdef ALIF_GIL_DISABLED
static AlifCriticalSection* untag_criticalSection(uintptr_t tag) { // 45
	return (AlifCriticalSection*)(tag & ~ALIF_CRITICAL_SECTION_MASK);
}
#endif







void alifCriticalSection_suspendAll(AlifThread* _thread) { // 55
	uintptr_t* tagptr = &_thread->criticalSection;
	while (alifCriticalSection_isActive(*tagptr)) {
		AlifCriticalSection* c = untag_criticalSection(*tagptr);

		if (c->mutex) {
			ALIFMUTEX_UNLOCK(c->mutex);
			if ((*tagptr & ALIF_CRITICAL_SECTION_TWO_MUTEXES)) {
				AlifCriticalSection2* c2 = (AlifCriticalSection2*)c;
				if (c2->mutex2) {
					ALIFMUTEX_UNLOCK(c2->mutex2);
				}
			}
		}

		*tagptr |= ALIF_CRITICAL_SECTION_INACTIVE;
		tagptr = &c->prev;
	}
}




void alifCriticalSection_resume(AlifThread* _thread) { // 79
	uintptr_t p_ = _thread->criticalSection;
	AlifCriticalSection* c_ = untag_criticalSection(p_);

	AlifMutex* m1_ = c_->mutex;
	c_->mutex = nullptr;

	AlifMutex* m2_ = nullptr;
	AlifCriticalSection2* c2_ = nullptr;
	if ((p_ & ALIF_CRITICAL_SECTION_TWO_MUTEXES)) {
		c2_ = (AlifCriticalSection2*)c_;
		m2_ = c2_->mutex2;
		c2_->mutex2 = nullptr;
	}

	if (m1_) {
		ALIFMUTEX_LOCK(m1_);
	}
	if (m2_) {
		ALIFMUTEX_LOCK(m2_);
	}

	c_->mutex = m1_;
	if (m2_) {
		c2_->mutex2 = m2_;
	}

	_thread->criticalSection &= ~ALIF_CRITICAL_SECTION_INACTIVE;
}






#undef ALIFCRITICALSECTION_BEGIN
void alifCriticalSection_begin(AlifCriticalSection* _c, AlifObject* _op) { // 114
#ifdef ALIF_GIL_DISABLED
	_alifCriticalSection_begin(_c, _op);
#endif
}

#undef ALIFCRITICALSECTION_END
void alifCriticalSection_end(AlifCriticalSection* _c) { // 124
#ifdef ALIF_GIL_DISABLED
	_alifCriticalSection_end(_c);
#endif
}
