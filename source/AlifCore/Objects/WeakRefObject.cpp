#include "alif.h"

#include "AlifCore_CriticalSection.h"
#include "AlifCore_Lock.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"
#include "AlifCore_State.h"
#include "AlifCore_WeakRef.h"




 // 37
#define GET_WEAKREFS_LISTPTR(_o) \
        ((AlifWeakReference **) _alifObject_getWeakRefsListPtr(_o))




AlifSizeT _alifWeakref_getWeakrefCount(AlifObject* _obj) { // 41
	if (!_alifType_supportsWeakRefs(ALIF_TYPE(_obj))) {
		return 0;
	}

	LOCK_WEAKREFS(_obj);
	AlifSizeT count = 0;
	AlifWeakReference* head = *GET_WEAKREFS_LISTPTR(_obj);
	while (head != NULL) {
		++count;
		head = head->next;
	}
	UNLOCK_WEAKREFS(_obj);
	return count;
}




static void clearWeakRef_lockHeld(AlifWeakReference* _self, AlifObject** _callback) { // 78
	if (_self->object != ALIF_NONE) {
		AlifWeakReference** list = GET_WEAKREFS_LISTPTR(_self->object);
		if (*list == _self) {
			alifAtomic_storePtr(&*list, _self->next);
		}
		alifAtomic_storePtr(&_self->object, ALIF_NONE);
		if (_self->prev != nullptr) {
			_self->prev->next = _self->next;
		}
		if (_self->next != nullptr) {
			_self->next->prev = _self->prev;
		}
		_self->prev = nullptr;
		_self->next = nullptr;
	}
	if (_callback != nullptr) {
		*_callback = _self->callback;
		_self->callback = nullptr;
	}
}









static AlifIntT is_basicRef(AlifWeakReference* _ref) { // 350
	return (_ref->callback == nullptr) and ALIFWEAKREF_CHECKREFEXACT(_ref);
}

static AlifIntT is_basicProxy(AlifWeakReference* _proxy) { // 356
	return (_proxy->callback == nullptr) and ALIFWEAKREF_CHECKPROXY(_proxy);
}

static AlifIntT isBasicRef_orProxy(AlifWeakReference* _wr) { // 362
	return is_basicRef(_wr) or is_basicProxy(_wr);
}


static AlifIntT parseWeakRef_initArgs(const char* funcname,
	AlifObject* args, AlifObject* kwargs,
	AlifObject** obp, AlifObject** callbackp) { // 450
	return alifArg_unpackTuple(args, funcname, 1, 2, obp, callbackp);
}



static AlifIntT weakref___init__(AlifObject* _self,
	AlifObject* _args, AlifObject* _kwargs) { // 467
	AlifObject* tmp{};

	if (!_ALIFARG_NOKEYWORDS("ref", _kwargs))
		return -1;

	if (parseWeakRef_initArgs("__init__", _args, _kwargs, &tmp, &tmp))
		return 0;
	else
		return -1;
}

AlifTypeObject _alifWeakrefRefType_ = { // 493
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مرجع_ضعيف.نوع_مرجعي",
	.basicSize = sizeof(AlifWeakReference),
	//.dealloc = weakref_dealloc,
	.vectorCallOffset = offsetof(AlifWeakReference, vectorCall),
	//.call = alifVectorCall_call,
	//.repr = weakref_repr,
	//.hash = weakref_hash,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
				ALIF_TPFLAGS_HAVE_VECTORCALL | ALIF_TPFLAGS_BASETYPE,
	//.traverse = gc_traverse,
	//.clear = gc_clear,
	//.richCompare = weakref_richcompare,
	//.methods = weakref_methods,
	//.members = weakref_members,
	.init = weakref___init__,
	.alloc = alifType_genericAlloc,
	//.new_ = weakref___new__,
	.free = alifObject_gcDel,
};















AlifTypeObject _alifWeakrefProxyType_ = { // 846
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مرجع_ضعيف.نوع_وكيل",
	.basicSize = sizeof(AlifWeakReference),
	.itemSize = 0,
	/* methods */
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
};


AlifTypeObject _alifWeakrefCallableProxyType_ = { // 881
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مرجع_ضعيف.نوع_وكيل_قابل_للاستدعاء",
	.basicSize = sizeof(AlifWeakReference),
	.itemSize = 0,
	/* methods */
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
};






static void handle_callback(AlifWeakReference* ref, AlifObject* callback) { // 967
	AlifObject* cbResult = alifObject_callOneArg(callback, (AlifObject*)ref);

	if (cbResult == nullptr) {
		//alifErr_writeUnraisable(callback);
	}
	else ALIF_DECREF(cbResult);
}




void alifObject_clearWeakRefs(AlifObject* _object) { // 984
	AlifWeakReference** list{};

	if (_object == nullptr
		or !_alifType_supportsWeakRefs(ALIF_TYPE(_object))
		or ALIF_REFCNT(_object) != 0)
	{
		//ALIFERR_BADINTERNALCALL();
		return;
	}

	list = GET_WEAKREFS_LISTPTR(_object);
	if (alifAtomic_loadPtr(&*list) == nullptr) {
		return;
	}

	for (int done = 0; !done;) {
		LOCK_WEAKREFS(_object);
		if (*list != nullptr and isBasicRef_orProxy(*list)) {
			AlifObject* callback;
			clearWeakRef_lockHeld(*list, &callback);
		}
		done = (*list == nullptr) or !isBasicRef_orProxy(*list);
		UNLOCK_WEAKREFS(_object);
	}

	AlifSizeT numWeakrefs = _alifWeakref_getWeakrefCount(_object);
	if (numWeakrefs == 0) {
		return;
	}

	//AlifObject* exc = alifErr_getRaisedException();
	AlifObject* tuple = alifTuple_new(numWeakrefs * 2);
	if (tuple == nullptr) {
		//_alifWeakref_clearWeakRefsNoCallbacks(_object);
		//alifErr_writeUnraisable(nullptr);
		//alifErr_setRaisedException(exc);
		return;
	}

	AlifSizeT num_items = 0;
	for (AlifIntT done = 0; !done;) {
		AlifObject* callback = nullptr;
		LOCK_WEAKREFS(_object);
		AlifWeakReference* cur = *list;
		if (cur != nullptr) {
			clearWeakRef_lockHeld(cur, &callback);
			if (alif_tryIncRef((AlifObject*)cur)) {
				ALIFTUPLE_SET_ITEM(tuple, num_items, (AlifObject*)cur);
				ALIFTUPLE_SET_ITEM(tuple, num_items + 1, callback);
				num_items += 2;
				callback = nullptr;
			}
		}
		done = (*list == nullptr);
		UNLOCK_WEAKREFS(_object);

		ALIF_XDECREF(callback);
	}

	for (AlifSizeT i = 0; i < num_items; i += 2) {
		AlifObject* callback = ALIFTUPLE_GET_ITEM(tuple, i + 1);
		if (callback != nullptr) {
			AlifObject* weakref = ALIFTUPLE_GET_ITEM(tuple, i);
			handle_callback((AlifWeakReference*)weakref, callback);
		}
	}

	ALIF_DECREF(tuple);

	//alifErr_setRaisedException(exc);
}
