#pragma once

#include "AlifCore_FreeList.h"

AlifObject* alifList_extend(AlifListObject*, AlifObject*);

extern int alifSubList_appendTakeRefListResize(AlifListObject*, AlifObject*);

static inline int alifSubList_appendTakeRef(AlifListObject* _self, AlifObject* _newItem)
{
	int64_t len_ = ALIF_SIZE(_self);
	int64_t allocated_ = _self->allocate_;
	if (allocated_ > len_) {
		ALIFLIST_SETITEM(_self, len_, _newItem);
		ALIFSET_SIZE(_self, len_ + 1);
		return 0;
	}
	return alifSubList_appendTakeRefListResize(_self, _newItem);
}

static inline void alifSub_memoryRepeat(char* _dest, int64_t _lenDest, int64_t _lenSrc)
{
	int64_t copied_ = _lenSrc;
	while (copied_ < _lenDest) {
		int64_t bytesToCopy = min(copied_, _lenDest - copied_);
		memcpy(_dest + copied_, _dest, bytesToCopy);
		copied_ += bytesToCopy;
	}
}
