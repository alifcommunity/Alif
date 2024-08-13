#pragma once

#include "AlifCore_FreeList.h"


class AlifObjectState {
public:
#if !defined(ALIF_GIL_DISABLED)
	AlifObjectFreeLists freelists;
#endif
#ifdef ALIF_REF_DEBUG
	AlifSizeT reftotal;
#endif
#ifdef ALIF_TRACE_REFS
	AlifHashtableT* refchain;
#endif
	AlifIntT notUsed;
};
