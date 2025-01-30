#pragma once

#include "AlifCore_GC.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_State.h"
#include "AlifCore_TypeID.h"




#define ALIF_REF_DEFERRED (ALIF_SIZET_MAX / 8) // 28

static inline void _alif_refcntAdd(AlifObject* _op, AlifSizeT _n) { // 132 
	if (ALIF_ISIMMORTAL(_op)) {
		return;
	}


	if (alif_isOwnedByCurrentThread(_op)) {
		uint32_t local = _op->refLocal;
		AlifSizeT refcnt = (AlifSizeT)local + _n;
#  if ALIF_SIZET_MAX > UINT32_MAX
		if (refcnt > (AlifSizeT)UINT32_MAX) {

			refcnt = ALIF_IMMORTAL_REFCNT_LOCAL;
		}
#  endif
		alifAtomic_storeUint32Relaxed(&_op->refLocal, (uint32_t)refcnt);
	}
	else {
		alifAtomic_addSize(&_op->refShared, (_n << ALIF_REF_SHARED_SHIFT));
	}
}
#define ALIF_REFCNTADD(_op, _n) _alif_refcntAdd(ALIFOBJECT_CAST(_op), _n)


void alif_setImmortal(AlifObject*); // 162
void alif_setImmortalUntracked(AlifObject*); // 163 


static inline void _alif_setMortal(AlifObject* _op, AlifSizeT _refCnt) { // 167
	if (_op) {
		_op->threadID = ALIF_UNOWNED_TID;
		_op->refLocal = 0;
		_op->refShared = ALIF_REF_SHARED(_refCnt, ALIF_REF_MERGED);
	}
}

static inline void _alif_clearImmortal(AlifObject* _op) { // 182
	if (_op) {
		_alif_setMortal(_op, 1);
		ALIF_DECREF(_op);
	}
}
#define ALIF_CLEARIMMORTAL(_op) \
    do { \
        _alif_clearImmortal(ALIFOBJECT_CAST(_op)); \
        _op = nullptr; \
    } while (0)



static inline void _alif_decrefSpecialized(AlifObject* _op, const Destructor _destruct) { // 237
	ALIF_DECREF(_op);
}


AlifSizeT alif_explicitMergeRefcount(AlifObject*, AlifSizeT); // 263



static inline AlifIntT _alifType_hasFeature(AlifTypeObject* _type, unsigned long _feature) { // 281
	return ((ALIFATOMIC_LOAD_ULONG_RELAXED(&_type->flags) & _feature) != 0);
}






static inline void alif_increaseRefType(AlifTypeObject* type) { // 296
	if (!_alifType_hasFeature(type, ALIF_TPFLAGS_HEAPTYPE)) {
		return;
	}

#if defined(__GNUC__) && __GNUC__ >= 11
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Warray-bounds"
#endif

	AlifThreadImpl* thread = (AlifThreadImpl*)_alifThread_get();
	AlifHeapTypeObject* ht = (AlifHeapTypeObject*)type;

	if ((AlifUSizeT)ht->uniqueID < (AlifUSizeT)thread->types.size) {
		thread->types.refCounts[ht->uniqueID]++;
	}
	else {
		alifType_incRefSlow(ht);
	}

#if defined(__GNUC__) && __GNUC__ >= 11
#  pragma GCC diagnostic pop
#endif
}

static inline void alif_decreaseRefType(AlifTypeObject* _type) { // 336
	if (!_alifType_hasFeature(_type, ALIF_TPFLAGS_HEAPTYPE)) {
		return;
	}

	AlifThreadImpl* thread = (AlifThreadImpl*)_alifThread_get();
	AlifHeapTypeObject* ht = (AlifHeapTypeObject*)_type;

	if ((AlifUSizeT)ht->uniqueID < (AlifUSizeT)thread->types.size) {
		thread->types.refCounts[ht->uniqueID]--;
	}
	else {
		ALIF_DECREF(_type);
	}
}



static inline void _alifObject_init(AlifObject* _op, AlifTypeObject* _typeObj) { // 370
	ALIF_SET_TYPE(_op, _typeObj);
	alif_increaseRefType(_typeObj);
	alif_newReference(_op);
}

static inline void _alifObject_initVar(AlifVarObject* _op,
	AlifTypeObject* _typeObj, AlifSizeT _size) { // 380
	alifObject_init((AlifObject*)_op, _typeObj);
	ALIF_SET_SIZE(_op, _size);
}



static inline void _alifObject_gcTrack(AlifObject* _op) { // 403
	alifObject_setGCBits(_op, ALIFGC_BITS_TRACKED);
}

static inline void alifObject_gcUntrack(AlifObject* op) { // 443
	alifObject_clearGCBits(op, ALIFGC_BITS_TRACKED);
}

// 471
#define ALIFOBJECT_GC_TRACK(_op) \
        _alifObject_gcTrack(ALIFOBJECT_CAST(_op))
#define ALIFOBJECT_GC_UNTRACK(_op) \
        alifObject_gcUntrack(ALIFOBJECT_CAST(_op))

static inline AlifIntT alif_tryIncrefFast(AlifObject* _op) { // 492
	uint32_t local = alifAtomic_loadUint32Relaxed(&_op->refLocal);
	local += 1;
	if (local == 0) {
		// immortal
		return 1;
	}
	if (alif_isOwnedByCurrentThread(_op)) {
		alifAtomic_storeUint32Relaxed(&_op->refLocal, local);
		return 1;
	}
	return 0;
}

static inline AlifIntT alif_tryIncRefShared(AlifObject* _op) { // 511
	AlifSizeT shared = alifAtomic_loadSizeRelaxed(&_op->refShared);
	for (;;) {

		if (shared == 0 or shared == ALIF_REF_MERGED) {
			return 0;
		}

		if (alifAtomic_compareExchangeSize( &_op->refShared, &shared,
			shared + (1 << ALIF_REF_SHARED_SHIFT))) {
			return 1;
		}
	}
}

static inline AlifIntT alif_tryIncRefCompare(AlifObject** _src, AlifObject* _op) { // 536
	if (alif_tryIncrefFast(_op)) {
		return 1;
	}
	if (!alif_tryIncRefShared(_op)) {
		return 0;
	}
	if (_op != alifAtomic_loadPtr(_src)) {
		ALIF_DECREF(_op);
		return 0;
	}
	return 1;
}

static inline AlifObject* alif_tryXGetRef(AlifObject** _ptr) { // 571
	AlifObject* value = (AlifObject*)alifAtomic_loadPtr(_ptr);
	if (value == nullptr) {
		return value;
	}
	if (alif_tryIncRefCompare(_ptr, value)) {
		return value;
	}
	return nullptr;
}

static inline AlifObject* _alif_newRefWithLock(AlifObject* _op) { // 586
	if (alif_tryIncrefFast(_op)) {
		return _op;
	}
	for (;;) {
		AlifSizeT shared = alifAtomic_loadSizeRelaxed(&_op->refShared);
		AlifSizeT newShared = shared + (1 << ALIF_REF_SHARED_SHIFT);
		if ((shared & ALIFREF_SHARED_FLAG_MASK) == 0) {
			newShared |= ALIF_REF_MAYBE_WEAKREF;
		}
		if (alifAtomic_compareExchangeSize(
			&_op->refShared, &shared, newShared)) {
			return _op;
		}
	}
}

static inline AlifIntT alif_tryIncRef(AlifObject* _op) { // 641
	return alif_tryIncrefFast(_op) or alif_tryIncRefShared(_op);
}


static inline AlifObject** _alifObject_getWeakRefsListPtr(AlifObject* _op) { // 677
	if (ALIFTYPE_CHECK(_op) and
		((AlifTypeObject*)_op)->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		AlifInterpreter* interp = _alifInterpreter_get();
		ManagedStaticTypeState* state = _alifStaticType_getState(
			interp, (AlifTypeObject*)_op);
		return _alifStaticType_getWeakRefsListPtr(state);
	}
	AlifSizeT offset = ALIF_TYPE(_op)->weakListOffset;
	return (AlifObject**)((char*)_op + offset);
}


// Fast inlined version of ALIFTYPE_IS_GC()
#define _ALIFTYPE_IS_GC(_t) _alifType_hasFeature(_t, ALIF_TPFLAGS_HAVE_GC) // 735

static inline AlifIntT _alifObject_isGC(AlifObject* obj) { // 739
	AlifTypeObject* type = ALIF_TYPE(obj);
	return (_ALIFTYPE_IS_GC(type) and (type->isGC == nullptr or type->isGC(obj)));
}


static inline AlifHashT alifObject_hashFast(AlifObject* _op) { // 784 
	if (ALIFUSTR_CHECKEXACT(_op)) {
		AlifHashT hash = alifAtomic_loadSizeRelaxed(&ALIFASCIIOBJECT_CAST(_op)->hash);
		if (hash != -1) {
			return hash;
		}
	}
	return alifObject_hash(_op);
}


static inline AlifUSizeT alifType_preHeaderSize(AlifTypeObject* _tp) { // 761 
	return (AlifUSizeT)(alifType_hasFeature(_tp, ALIF_TPFLAGS_PREHEADER) * 2 * sizeof(AlifObject*));
}

void alifObject_gcLink(AlifObject*); // 750


static inline AlifIntT _alifType_supportsWeakRefs(AlifTypeObject* _type) { // 759
	return (_type->weakListOffset != 0);
}


extern AlifObject* alifType_allocNoTrack(AlifTypeObject*, AlifSizeT); // 763
extern AlifTypeObject* _alifType_calculateMetaclass(AlifTypeObject*, AlifObject*); // 766

void alifObject_initInlineValues(AlifObject*, AlifTypeObject*); // 771
extern AlifIntT alifObject_storeInstanceAttribute(AlifObject*, AlifObject*, AlifObject*);
extern bool alifObject_tryGetInstanceAttribute(AlifObject* , AlifObject* , AlifObject** ); // 774 

#define MANAGED_DICT_OFFSET    (((AlifSizeT)sizeof(AlifObject *))*-1) // 778 
#define MANAGED_WEAKREF_OFFSET (((AlifSizeT)sizeof(AlifObject *))*-2)

union AlifManagedDictPointer{ // 785
	AlifDictObject* dict{};
};

static inline AlifManagedDictPointer* alifObject_managedDictPointer(AlifObject* obj) { // 790
	return (AlifManagedDictPointer*)((char*)obj + MANAGED_DICT_OFFSET);
}

static inline AlifDictObject* alifObject_getManagedDict(AlifObject* _obj) { // 797
	AlifManagedDictPointer* dorv = alifObject_managedDictPointer(_obj);
	return (AlifDictObject*)alifAtomic_loadPtrAcquire(&dorv->dict);
}


static inline AlifDictValues* alifObject_inlineValues(AlifObject* _obj) { // 803
	AlifTypeObject* tp_ = ALIF_TYPE(_obj);
	return (AlifDictValues*)((char*)_obj + tp_->basicSize);
}


extern AlifObject** alifObject_computedDictPointer(AlifObject*); // 813


AlifObject* alifObject_lookupSpecial(AlifObject*, AlifObject*); // 817

AlifIntT _alifObject_getMethod(AlifObject*, AlifObject*, AlifObject**); // 822
extern AlifObject* _alifObject_nextNotImplemented(AlifObject*); // 823




#define ALIFCPPFUNCTION_TRAMPOLINECALL(_meth, _self, _args) \
    (_meth)((_self), (_args)) // 844
#define ALIFCPPFUNCTIONWITHKEYWORDS_TRAMPOLINECALL(_meth, _self, _args, _kw) \
    (_meth)((_self), (_args), (_kw)) // 846

extern AlifTypeObject _alifNoneType_; // 851


extern AlifIntT _alifSwappedOp_[]; // 856
