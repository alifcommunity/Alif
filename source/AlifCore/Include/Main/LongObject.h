#pragma once









 // 12
#define ALIFLONG_CHECK(_op) \
        ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(_op), ALIF_TPFLAGS_LONG_SUBCLASS)
#define ALIFLONG_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifLongType_)
