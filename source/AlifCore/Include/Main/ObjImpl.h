#pragma once








AlifIntT alifObject_isGC(AlifObject*); // 78











#define ALIFTYPE_IS_GC(_t) alifType_hasFeature(_t, ALIF_TPFLAGS_HAVE_GC) // 157
