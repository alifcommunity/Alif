#pragma once





AlifObject* _alifList_extend(AlifListObject*, AlifObject*); // 11

AlifIntT alifList_appendTakeRefListResize(AlifListObject*, AlifObject*); // 17


static inline AlifIntT alifList_appendTakeRef(AlifListObject* _self,
	AlifObject* _newItem) { // 20
	AlifSizeT len = ALIF_SIZE(_self);
	AlifSizeT allocated = _self->allocated;
	if (allocated > len) {
#ifdef ALIF_GIL_DISABLED
		alifAtomic_storePtrRelease(&_self->item[len], _newItem);
#else
		ALIFLIST_SET_ITEM(_self, len, _newItem);
#endif
		ALIF_SET_SIZE(_self, len + 1);
		return 0;
	}
	return alifList_appendTakeRefListResize(_self, _newItem);
}


static inline void alif_memoryRepeat(char* _dest, AlifSizeT _lenDest, AlifSizeT _lenSrc) { // 42
	AlifSizeT copied = _lenSrc;
	while (copied < _lenDest) {
		AlifSizeT bytesToCopy = ALIF_MIN(copied, _lenDest - copied);
		memcpy(_dest + copied, _dest, bytesToCopy);
		copied += bytesToCopy;
	}
}
