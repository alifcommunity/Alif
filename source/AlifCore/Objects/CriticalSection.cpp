#include "alif.h"

#include "AlifCore_Lock.h"
#include "AlifCore_CriticalSection.h"





















void alifCriticalSection_suspendAll(AlifThread* _thread) { // 55
	uintptr_t* tagptr = &_thread->criticalSection;
	while (alifCriticalSection_isActive(*tagptr)) {
		AlifCriticalSection* c = untag_criticalSection(*tagptr);

		if (c->csMutex) {
			ALIFMUTEX_UNLOCK(c->csMutex);
			if ((*tagptr & _Py_CRITICAL_SECTION_TWO_MUTEXES)) {
				PyCriticalSection2* c2 = (PyCriticalSection2*)c;
				if (c2->_cs_mutex2) {
					ALIFMUTEX_UNLOCK(c2->_cs_mutex2);
				}
			}
		}

		*tagptr |= _Py_CRITICAL_SECTION_INACTIVE;
		tagptr = &c->_cs_prev;
	}
}
