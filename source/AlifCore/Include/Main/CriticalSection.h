#pragma once


typedef class AlifCriticalSection AlifCriticalSection; // 70
typedef class AlifCriticalSection2 AlifCriticalSection2; // 71

void alifCriticalSection_begin(AlifCriticalSection*, AlifObject*); // 73

void alifCriticalSection_end(AlifCriticalSection*); // 76

//void alifCriticalSection2_begin(AlifCriticalSection2*, AlifObject*, AlifObject*); // 79

//void alifCriticalSection2_end(AlifCriticalSection2*); // 82



class AlifCriticalSection { // 98
public:
	uintptr_t prev{};
	AlifMutex* mutex{};
};



class AlifCriticalSection2 { // 110
public:
	AlifCriticalSection base{};
	AlifMutex* mutex2{};
};





#define ALIF_BEGIN_CRITICAL_SECTION(op)                                  \
    {                                                                   \
        AlifCriticalSection critSec{};                                       \
        alifCriticalSection_begin(&critSec, ALIFOBJECT_CAST(op))

#define ALIF_END_CRITICAL_SECTION()                                      \
        alifCriticalSection_end(&critSec);                                 \
    }

#define ALIF_BEGIN_CRITICAL_SECTION2(a, b)                               \
    {                                                                   \
        AlifCriticalSection2 critSec2{};                                     \
        /*alifCriticalSection2_begin(&critSec2, ALIFOBJECT_CAST(a), ALIFOBJECT_CAST(b))*/

#define ALIF_END_CRITICAL_SECTION2()                                     \
        /*alifCriticalSection2_end(&critSec2);*/                               \
    }


