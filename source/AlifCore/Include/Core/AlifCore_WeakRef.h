#pragma once


#include "AlifCore_CriticalSection.h"
#include "AlifCore_Lock.h"


 // 18
#define WEAKREF_LIST_LOCK(_obj) \
    _alifInterpreter_get()  \
        ->weakrefLocks[((uintptr_t)_obj) % NUM_WEAKREF_LIST_LOCKS]

#define LOCK_WEAKREFS(_obj) \
    alifMutex_lockFlags(&WEAKREF_LIST_LOCK(_obj), AlifLockFlags_::Alif_Lock_Dont_Detach)
#define UNLOCK_WEAKREFS(_obj) ALIFMUTEX_UNLOCK(&WEAKREF_LIST_LOCK(_obj))







extern AlifSizeT _alifWeakref_getWeakrefCount(AlifObject*); // 108
