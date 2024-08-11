#pragma once

// سيتم اضافة هذا الملف لاحقا للذاكرة

//#ifndef ALIF_GIL_DISABLED
//class AlifSubMutex {public: uint8_t v_; };
//#endif
//typedef struct AlifSubMutex AlifMutex;
//
//
//static inline void alifMutex_lock(AlifMutex* _m)
//{
//    uint8_t expected = ALIFSUB_UNLOCKED;
//    if (!alifSub_atomic_compare_exchange_uint8(&m->v, &expected, ALIFSUB_LOCKED)) {
//        alifSubMutex_lockSlow(m);
//    }
//}
//
//// Unlocks the mutex.
//static inline void alifMutex_unlock(AlifMutex* _m)
//{
//    uint8_t expected = ALIFSUB_LOCKED;
//    if (!alifSub_atomic_compare_exchange_uint8(&m->v, &expected, ALIFSUB_LOCKED)) {
//        alifSubMutex_unlockSlow(m);
//    }
//}