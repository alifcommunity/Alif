#pragma once

#include "AlifCore_FreeListState.h"
#include "AlifCore_Object.h"
#include "AlifCore_State.h"




static inline AlifFreeLists* alifFreeLists_get(void) { // 16 
	AlifThread* thread = _alifThread_get();
	
	return &((AlifThreadImpl*)thread)->freeLists;
}

#define ALIF_FREELIST_FREE(_name , _nameMcro, _op, _freeFunc) \
    alifFreeList_free(&alifFreeLists_get()->_name, ALIFOBJECT_CAST(_op),	\
		ALIF ## _nameMcro ## _MAXFREELIST, _freeFunc) // 39

#define ALIF_FREELIST_POP(_type, _name) ALIF_CAST(_type*, alifFreeList_pop(&alifFreeLists_get()->_name)) // 46

#define ALIF_FREELIST_POP_MEM(_name) \
    alifFreeList_popMem(&alifFreeLists_get()->_name) // 52

static inline AlifIntT alifFreeList_push(AlifFreeList* _fl,
	void* _obj, AlifSizeT _maxSize) {  // 57
	if (_fl->size < _maxSize and _fl->size >= 0) {
		*(void**)_obj = _fl->freeList;
		_fl->freeList = _obj;
		_fl->size++;
		return 1;
	}
	return 0;
}

static inline void alifFreeList_free(AlifFreeList* _fl, void* _obj,
	AlifSizeT _maxSize, FreeFunc _doFree) { // 71
	if (!alifFreeList_push(_fl, _obj, _maxSize)) {
		_doFree(_obj);
	}
}

static inline void* alifFreeList_popNoStats(AlifFreeList* _fl)  { // 80
	void* obj_ = _fl->freeList;
	if (obj_ != nullptr) {
		_fl->freeList = *(void**)obj_;
		_fl->size--;
	}
	return obj_;
}

static inline AlifObject* alifFreeList_pop(AlifFreeList* _fl)  {// 91
	AlifObject* op_ = (AlifObject*)alifFreeList_popNoStats(_fl);
	if (op_ != nullptr) {
		alif_newReference(op_);
	}
	return op_;
}

static inline void* alifFreeList_popMem(AlifFreeList* _fl) { // 102
	void* op = alifFreeList_popNoStats(_fl);
	if (op != nullptr) {
		//OBJECT_STAT_INC(_fromFreeList_); // not needed
	}
	return op;
}
