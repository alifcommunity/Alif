#pragma once


#include "AlifCore_Object.h"
#include "AlifCore_State.h"

#ifdef WITH_FREELISTS
#	define ALIFTUPLE_MAXSAVESIZE 20
#	define ALIFTUPLE_NFREELISTS ALIFTUPLE_MAXSAVESIZE
#	define ALIFTUPLE_MAXFREELIST 2000
#	define ALIFLIST_MAXFREELIST 80
#	define ALIFDICT_MAXFREELIST 80
#	define ALIFFLOAT_MAXFREELIST 100
#	define ALIFCONTEXT_MAXFREELIST 255
#	define ALIFASYNCGEN_MAXFREELIST 80
#	define ALIFOBJECTSTACKCHUNK_MAXFREELIST 4
#else
#  define ALIFTUPLE_MAXSAVESIZE 0
#endif

class AlifFreeList { // 30
public:
	void* freeList{};
	AlifSizeT size{};
};



class AlifFreeLists { // 40
public:
#ifdef WITH_FREELISTS
	AlifFreeList floats{};
	AlifFreeList tuples[ALIFTUPLE_MAXSAVESIZE];
	AlifFreeList lists{};
	AlifFreeList dicts{};
	AlifFreeList dictKeys{};
	AlifFreeList slices{};
	AlifFreeList contexts{};
	AlifFreeList asyncGens{};
	AlifFreeList asyncGenAsends{};
	AlifFreeList futureIters{};
	AlifFreeList objectStackChunks{};
#else
	char unused{};  // Empty structs are not allowed.
#endif
};



// ------------------------------------------- AlifCore_FreeList ------------------------------------------ //



#include "AlifCore_State.h"

static inline class AlifFreeLists* alifFreeLists_get(void) { // 16 
	AlifThread* tState = alifThread_get();
	
	return &((AlifThreadImpl*)tState)->freeLists;
}

// هنا تم وضع باراميتر اضافي خاص ب دالة alifFreeLists_get سيتم مناقشته لاحقا
#define ALIF_FREELIST_FREE(_name , _nameMcro, _op, _freeFunc) \
    alifFreeList_free(&alifFreeLists_get()->_name, ALIFOBJECT_CAST(_op), ALIF ## _nameMcro ## _MAXFREELIST, FreeFunc) // 39

#define ALIF_FREELIST_POP(_tyep, _name) ALIF_CAST(_tyep*, alifFreeList_pop(&alifFreeLists_get()->_name)) // 46


static inline AlifIntT alifFreeList_push(class AlifFreeList* _fl, void* _obj, AlifSizeT _maxSize) {  // 57
	if (_fl->size < _maxSize and _fl->size >= 0) {
		*(void**)_obj = _fl->freeList;
		_fl->freeList = _obj;
		_fl->size++;
		//OBJECT_STAT_INC(toFreeList);
		return 1;
	}
	return 0;
}

static inline void alifFreeList_free(class AlifFreeList* _fl, void* _obj, AlifSizeT _maxSize,
	FreeFunc _doFree) { // 71
	if (!alifFreeList_push(_fl, _obj, _maxSize)) {
		_doFree(_obj);
	}
}

static inline void* alifFreeList_popNoStats(class AlifFreeList* _fl)  { // 80
	void* obj_ = _fl->freeList;
	if (obj_ != nullptr) {
		_fl->freeList = *(void**)obj_;
		_fl->size--;
	}
	return obj_;
}

static inline AlifObject* alifFreeList_pop(class AlifFreeList* _fl)  {// 91
	AlifObject* op_ = (AlifObject*)alifFreeList_popNoStats(_fl);
	if (op_ != nullptr) {
		alif_newReference(op_);
	}
	return op_;
}
