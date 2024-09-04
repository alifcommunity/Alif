#pragma once




// 19
#define ALIF_CRITICAL_SECTION_INACTIVE       0x1
#define ALIF_CRITICAL_SECTION_TWO_MUTEXES    0x2
#define ALIF_CRITICAL_SECTION_MASK           0x3






void alifCriticalSection_resume(AlifThread*); // 89



void alifCriticalSection_suspendAll(AlifThread*); // 99


#ifdef ALIF_GIL_DISABLED

static inline AlifIntT alifCriticalSection_isActive(uintptr_t tag)
{
	return tag != 0 and (tag & ALIF_CRITICAL_SECTION_INACTIVE) == 0;
}










#endif
