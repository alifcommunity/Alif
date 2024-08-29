#include "alif.h"

#include "AlifCore_Lock.h"
#include "AlifCore_CriticalSection.h"





















void alifCriticalSection_suspendAll(AlifThread* _thread) { // 55
	uintptr_t* tagptr = &_thread->criticalSection;
	while (alifCriticalSection_isActive(*tagptr)) {
		AlifCriticalSection* c = untag_criticalSection(*tagptr);

		if (c->csMutex) {
			ALIFMUTEX_UNLOCK(c->csMutex);
			if ((*tagptr & ALIF_CRITICALSECTION_TWO_MUTEXES)) {
				AlifCriticalSection2* c2 = (AlifCriticalSection2*)c;
				if (c2->_cs_mutex2) {
					ALIFMUTEX_UNLOCK(c2->csMutex2);
				}
			}
		}

		*tagptr |= ALIF_CRITICAL_SECTION_INACTIVE;
		tagptr = &c->csPrev;
	}
}




void alifCriticalSection_resume(AlifThread* _thread) { // 79
	uintptr_t p_ = _thread->criticalSection;
	AlifCriticalSection* c_ = untag_criticalSection(p_);

	AlifMutex* m1_ = c_->csMutex;
	c_->csMutex = nullptr;

	AlifMutex* m2_ = nullptr;
	AlifCriticalSection2* c2_ = nullptr;
	if ((p_ & ALIF_CRITICALSECTION_TWO_MUTEXES)) {
		c2_ = (AlifCriticalSection2*)c_;
		m2_ = c2_->csMutex2;
		c2_->csMutex2 = nullptr;
	}

	if (m1_) {
		ALIFMUTEX_LOCK(m1_);
	}
	if (m2_) {
		ALIFMUTEX_LOCK(m2_);
	}

	c_->csMutex = m1_;
	if (m2_) {
		c2_->csMutex2 = m2_;
	}

	_thread->criticalSection &= ~ALIF_CRITICAL_SECTION_INACTIVE;
}
