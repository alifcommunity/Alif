#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Call.h"
#include "AlifCore_Dict.h"
#include "AlifCore_Lock.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"
#include "AlifCore_ObjectAlloc.h"
#include "AlifCore_State.h"
#include "AlifCore_SymTable.h"
#include "AlifCore_TypeObject.h"
#include "AlifCore_UStrObject.h"
#include "AlifCore_WeakRef.h"
#include "AlifCore_CriticalSection.h" // ربما يمكن حذفه بعد الإنتهاء من تطوير اللغة


#include "clinic/TypeObject.cpp.h"


// 40
#define MCACHE_MAX_ATTR_SIZE    100
#define MCACHE_HASH(_version, _nameHash)                                 \
        (((AlifUIntT)(_version) ^ (AlifUIntT)(_nameHash))          \
         & ((1 << MCACHE_SIZE_EXP) - 1))
#define MCACHE_HASH_METHOD(_type, _name)                                  \
    MCACHE_HASH(alifAtomic_loadUint32Relaxed(&(_type)->versionTag),   \
                ((AlifSizeT)(_name)) >> 3)
#define MCACHE_CACHEABLE_NAME(_name)                             \
        ALIFUSTR_CHECKEXACT(_name) and                           \
        (ALIFUSTR_GET_LENGTH(_name) <= MCACHE_MAX_ATTR_SIZE)

#define NEXT_GLOBAL_VERSION_TAG _alifDureRun_.types.nextVersionTag
#define NEXT_VERSION_TAG(_interp) (_interp)->types.nextVersionTag



// 64
#define TYPE_LOCK &alifInterpreter_get()->types.mutex
#define BEGIN_TYPE_LOCK() ALIF_BEGIN_CRITICAL_SECTION_MUT(TYPE_LOCK)
#define END_TYPE_LOCK() ALIF_END_CRITICAL_SECTION()

#define BEGIN_TYPE_DICT_LOCK(d) \
    ALIF_BEGIN_CRITICAL_SECTION2_MUT(TYPE_LOCK, &ALIFOBJECT_CAST(d)->mutex)

#define END_TYPE_DICT_LOCK() ALIF_END_CRITICAL_SECTION2()



static AlifObject* slot_tpNew(AlifTypeObject*, AlifObject*, AlifObject*); // 99


static AlifObject* lookup_maybeMethod(AlifObject*, AlifObject*, AlifIntT*); // 102




static inline AlifTypeObject* type_fromRef(AlifObject* ref) { // 108
	AlifObject* obj = _alifWeakRef_getRef(ref);
	if (obj == nullptr) {
		return nullptr;
	}
	return ALIFTYPE_CAST(obj);
}



static inline AlifUSizeT managedStatic_typeIndexGet(AlifTypeObject* _self) { // 130
	return (AlifUSizeT)_self->subclasses - 1;
}

static inline void managedStatic_typeIndexSet(AlifTypeObject* _self,
	AlifUSizeT _index) { // 137
	_self->subclasses = (AlifObject*)(_index + 1);
}

static ManagedStaticTypeState* managedStatic_typeStateGet(AlifInterpreter* _interp,
	AlifTypeObject* _self) { // 176
	AlifUSizeT index = managedStatic_typeIndexGet(_self);
	ManagedStaticTypeState* state =
		&(_interp->types.builtins.initialized[index]);
	if (state->type == _self) {
		return state;
	}
	if (index > ALIFMAX_MANAGED_STATIC_EXT_TYPES) {
		return state;
	}
	return &(_interp->types.forExtensions.initialized[index]);
}



ManagedStaticTypeState* _alifStaticType_getState(AlifInterpreter* _interp,
	AlifTypeObject* _self) { // 193
	return managedStatic_typeStateGet(_interp, _self);
}


static void managedStatic_typeStateInit(AlifInterpreter* _interp,
	AlifTypeObject* _self, AlifIntT _isBuiltin, AlifIntT _initial) { // 200

	AlifUSizeT index{};
	if (_initial) {
		if (_isBuiltin) {
			index = _interp->types.builtins.numInitialized;
		}
		else {
			ALIFMUTEX_LOCK(&_interp->types.mutex);
			index = _interp->types.forExtensions.nextIndex;
			_interp->types.forExtensions.nextIndex++;
			ALIFMUTEX_UNLOCK(&_interp->types.mutex);
		}
		managedStatic_typeIndexSet(_self, index);
	}
	else {
		index = managedStatic_typeIndexGet(_self);
	}
	AlifUSizeT full_index = _isBuiltin
		? index
		: index + ALIFMAX_MANAGED_STATIC_BUILTIN_TYPES;

	(void)alifAtomic_addInt64(
		&_alifDureRun_.types.managedStatic.types[full_index].interpCount, 1);

	if (_initial) {
		_alifDureRun_.types.managedStatic.types[full_index].type = _self;
	}

	ManagedStaticTypeState* state = _isBuiltin
		? &(_interp->types.builtins.initialized[index])
		: &(_interp->types.forExtensions.initialized[index]);

	state->type = _self;
	state->isBuiltin = _isBuiltin;

	if (_isBuiltin) {
		_interp->types.builtins.numInitialized++;
	}
	else {
		_interp->types.forExtensions.numInitialized++;
	}
}




static inline void start_readying(AlifTypeObject* _type) { // 356
	if (_type->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		AlifInterpreter* interp = _alifInterpreter_get();
		ManagedStaticTypeState* state = managedStatic_typeStateGet(interp, _type);
		state->readying = 1;
		return;
	}
	_type->flags |= ALIF_TPFLAGS_READYING;
}

static inline void stop_readying(AlifTypeObject* _type) { // 371 
	if (_type->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		AlifInterpreter* interp = _alifInterpreter_get();
		ManagedStaticTypeState* state = managedStatic_typeStateGet(interp, _type);
		state->readying = 0;
		return;
	}
	_type->flags &= ~ALIF_TPFLAGS_READYING;
}

static inline AlifIntT is_readying(AlifTypeObject* _type) { // 385
	if (_type->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		AlifInterpreter* interp = _alifInterpreter_get();
		ManagedStaticTypeState* state = managedStatic_typeStateGet(interp, _type);
		return state->readying;
	}
	return (_type->flags & ALIF_TPFLAGS_READYING) != 0;
}

static inline AlifObject* lookup_tpDict(AlifTypeObject* _self) { // 401
	if (_self->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		AlifInterpreter* interp = _alifInterpreter_get();
		ManagedStaticTypeState* state = _alifStaticType_getState(interp, _self);
		return state->dict;
	}
	return _self->dict;
}

AlifObject* alifType_getDict(AlifTypeObject* _self) { // 413
	return lookup_tpDict(_self);
}

static inline void set_tpDict(AlifTypeObject* _self, AlifObject* _dict) { // 427
	if (_self->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		AlifInterpreter* interp = _alifInterpreter_get();
		ManagedStaticTypeState* state = _alifStaticType_getState(interp, _self);
		state->dict = _dict;
		return;
	}
	_self->dict = _dict;
}

static inline AlifObject* lookup_tpBases(AlifTypeObject* _self) { // 454
	return _self->bases;
}

AlifObject* alifType_getBases(AlifTypeObject* _self) { // 460
	AlifObject* res_{};
	BEGIN_TYPE_LOCK();
	res_ = lookup_tpBases(_self);
	ALIF_INCREF(res_);
	END_TYPE_LOCK();
	return res_;
}

static inline void set_tpBases(AlifTypeObject* _self, AlifObject* _bases, AlifIntT _initial) { // 473
	if (_self->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		alif_setImmortal(_bases);
	}
	_self->bases = _bases;
}

static inline AlifObject* lookup_tpMro(AlifTypeObject* _self) { // 517 
	return _self->mro;
}


static inline void set_tpMro(AlifTypeObject* _self,
	AlifObject* _mro, AlifIntT _initial) { // 545
	if (_self->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		alif_setImmortal(_mro);
	}
	_self->mro = _mro;
}



static inline AlifObject* lookup_tpSubClasses(AlifTypeObject* self) { // 613
	if (self->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		AlifInterpreter* interp = _alifInterpreter_get();
		ManagedStaticTypeState* state = _alifStaticType_getState(interp, self);
		return state->subclasses;
	}
	return (AlifObject*)self->subclasses;
}


AlifObject* _alifType_getSubClasses(AlifTypeObject* self) { // 641
	AlifObject* list = alifList_new(0);
	if (list == nullptr) {
		return nullptr;
	}

	AlifObject* subclasses = lookup_tpSubClasses(self);  // borrowed ref
	if (subclasses == nullptr) {
		return list;
	}

	AlifSizeT i = 0;
	AlifObject* ref{};  // borrowed ref
	while (alifDict_next(subclasses, &i, nullptr, &ref)) {
		AlifTypeObject* subclass = type_fromRef(ref);
		if (subclass == nullptr) {
			continue;
		}

		if (alifList_append(list, ALIFOBJECT_CAST(subclass)) < 0) {
			ALIF_DECREF(list);
			ALIF_DECREF(subclass);
			return nullptr;
		}
		ALIF_DECREF(subclass);
	}
	return list;
}


static TypeCache* get_typeCache(void) { // 838
	AlifInterpreter* interp = _alifInterpreter_get();
	return &interp->types.typeCache;
}



static void setVersion_unlocked(AlifTypeObject* tp, AlifUIntT version) { // 999
	if (version) {
		alifAtomic_addUint16(&tp->versionsUsed, 1);
	}
	alifAtomic_storeUint32Relaxed(&tp->versionTag, version);
}



static void typeModified_unlocked(AlifTypeObject* _type) { // 1031
	//if (_type->versionTag == 0) {
	//	return;
	//}

	//AlifObject* subClasses = lookup_tpSubClasses(_type);
	//if (subClasses != nullptr) {
	//	AlifSizeT i = 0;
	//	AlifObject* ref{};
	//	while (alifDict_next(subClasses, &i, nullptr, &ref)) {
	//		AlifTypeObject* subClass = type_fromRef(ref);
	//		if (subClass == nullptr) {
	//			continue;
	//		}
	//		typeModified_unlocked(subClass);
	//		ALIF_DECREF(subClass);
	//	}
	//}

	//if (_type->watched) {
	//	AlifInterpreter* interp = _alifInterpreter_get();
	//	AlifIntT bits = _type->watched;
	//	AlifIntT i_ = 0;
	//	while (bits) {
	//		if (bits & 1) {
	//			AlifTypeWatchCallback cb = interp->typeWatchers[i_];
	//			if (cb and (cb(_type) < 0)) {
	//				//alifErr_formatUnraisable(
	//				//	"Exception ignored in type watcher callback #%d for %R",
	//				//	i_, type);
	//			}
	//		}
	//		i_++;
	//		bits >>= 1;
	//	}
	//}

	//setVersion_unlocked(_type, 0); /* 0 is not a valid version tag */
	//if (alifType_hasFeature(_type, ALIF_TPFLAGS_HEAPTYPE)) {
	//	((AlifHeapTypeObject*)_type)->specCache.getItem = nullptr;
	//}
}



void alifType_modified(AlifTypeObject* _type) { // 1099
	if (_type->versionTag == 0) {
		return;
	}

	BEGIN_TYPE_LOCK();
	typeModified_unlocked(_type);
	END_TYPE_LOCK();
}



static AlifIntT isSubType_withMro(AlifObject*, AlifTypeObject*, AlifTypeObject*); // 1112

static void type_mroModified(AlifTypeObject* _type, AlifObject* _bases) { // 1115
	AlifSizeT i_{}, n_{};
	AlifIntT custom = !ALIF_IS_TYPE(_type, &_alifTypeType_);
	AlifIntT unbound{};

	if (custom) {
		AlifObject* mroMeth{}, * typeMroMeth{};
		mroMeth = lookup_maybeMethod((AlifObject*)_type, &ALIF_ID(Mro), &unbound);
		if (mroMeth == nullptr) {
			goto clear;
		}
		typeMroMeth = lookup_maybeMethod(
			(AlifObject*) & _alifTypeType_, &ALIF_ID(Mro), &unbound);
		if (typeMroMeth == nullptr) {
			ALIF_DECREF(mroMeth);
			goto clear;
		}
		AlifIntT customMro = (mroMeth != typeMroMeth);
		ALIF_DECREF(mroMeth);
		ALIF_DECREF(typeMroMeth);
		if (customMro) {
			goto clear;
		}
	}
	n_ = ALIFTUPLE_GET_SIZE(_bases);
	for (i_ = 0; i_ < n_; i_++) {
		AlifObject* b = ALIFTUPLE_GET_ITEM(_bases, i_);
		AlifTypeObject* cls = ALIFTYPE_CAST(b);

		if (!isSubType_withMro(lookup_tpMro(_type), _type, cls)) {
			goto clear;
		}
	}
	return;

clear:
	setVersion_unlocked(_type, 0); /* 0 is not a valid version tag */
	if (alifType_hasFeature(_type, ALIF_TPFLAGS_HEAPTYPE)) {
		((AlifHeapTypeObject*)_type)->specCache.getItem = nullptr;
	}
}



void alifType_setVersion(AlifTypeObject* _tp, AlifUIntT _version) { // 1184
	BEGIN_TYPE_LOCK();
	setVersion_unlocked(_tp, _version);
	END_TYPE_LOCK();
}




#define MAX_VERSIONS_PER_CLASS 1000 // 1218

static AlifIntT assign_versionTag(AlifInterpreter* interp, AlifTypeObject* type) { // 1220
	if (type->versionTag != 0) {
		return 1;
	}
	if (!alifType_hasFeature(type, ALIF_TPFLAGS_READY)) {
		return 0;
	}
	if (type->versionsUsed >= MAX_VERSIONS_PER_CLASS) {
		return 0;
	}

	AlifObject* bases = lookup_tpBases(type);
	AlifSizeT n = ALIFTUPLE_GET_SIZE(bases);
	for (AlifSizeT i = 0; i < n; i++) {
		AlifObject* b = ALIFTUPLE_GET_ITEM(bases, i);
		if (!assign_versionTag(interp, ALIFTYPE_CAST(b))) {
			return 0;
		}
	}
	if (type->flags & ALIF_TPFLAGS_IMMUTABLETYPE) {
		/* static types */
		if (NEXT_GLOBAL_VERSION_TAG > ALIF_MAX_GLOBAL_TYPE_VERSION_TAG) {
			/* We have run out of version numbers */
			return 0;
		}
		setVersion_unlocked(type, NEXT_GLOBAL_VERSION_TAG++);
	}
	else {
		/* heap types */
		if (NEXT_VERSION_TAG(interp) == 0) {
			/* We have run out of version numbers */
			return 0;
		}
		setVersion_unlocked(type, NEXT_VERSION_TAG(interp)++);
	}
	return 1;
}


static AlifObject* type_abstractMethods(AlifTypeObject* type, void* context) { // 1491
	AlifObject* mod = nullptr;
	if (type == &_alifTypeType_) {
		//alifErr_setObject(_alifExcAttributeError_, &ALIF_ID(__abstractMethods__));
	}
	else {
		AlifObject* dict = lookup_tpDict(type);
		if (alifDict_getItemRef(dict, &ALIF_ID(__abstractMethods__), &mod) == 0) {
			//alifErr_setObject(_alifExcAttributeError_, &ALIF_ID(__abstractMethods__));
		}
	}
	return mod;
}



static AlifObject* type_call(AlifObject* _self, AlifObject* _args, AlifObject* _kwds) { // 2136
	AlifTypeObject* type = (AlifTypeObject*)_self;
	AlifObject* obj{};
	AlifThread* thread = _alifThread_get();

	if (type == &_alifTypeType_) {
		AlifSizeT nargs = ALIFTUPLE_GET_SIZE(_args);

		if (nargs == 1 and (_kwds == nullptr or !ALIFDICT_GET_SIZE(_kwds))) {
			obj = (AlifObject*)ALIF_TYPE(ALIFTUPLE_GET_ITEM(_args, 0));
			return ALIF_NEWREF(obj);
		}
		if (nargs != 3) {
			//alifErr_setString(_alifExcTypeError_,
			//	"type() takes 1 or 3 arguments");
			return nullptr;
		}
	}

	if (type->new_ == nullptr) {
		//_alifErr_format(thread, _alifExcTypeError_,
		//	"cannot create '%s' instances", type->name);
		return nullptr;
	}

	obj = type->new_(type, _args, _kwds);
	obj = _alif_checkFunctionResult(thread, (AlifObject*)type, obj, nullptr);
	if (obj == nullptr)
		return nullptr;

	if (!ALIFOBJECT_TYPECHECK(obj, type))
		return obj;

	type = ALIF_TYPE(obj);
	if (type->init != nullptr) {
		AlifIntT res = type->init(obj, _args, _kwds);
		if (res < 0) {
			ALIF_SETREF(obj, nullptr);
		}
		else {
		}
	}
	return obj;
}


AlifObject* alifType_allocNoTrack(AlifTypeObject* _type, AlifSizeT _nitems) { // 2217
	AlifObject* obj_{};
	AlifUSizeT size = alifObject_varSize(_type, _nitems + 1);

	const AlifUSizeT preSize = alifType_preHeaderSize(_type);
	if (_type->flags & ALIF_TPFLAGS_INLINE_VALUES) {
		size += alifInline_valuesSize(_type);
	}
	char* alloc = (char*)alifObject_mallocWithType(_type, size + preSize);
	if (alloc == nullptr) {
		//return alifErr_noMemory();
		return nullptr;
	}
	obj_ = (AlifObject*)(alloc + preSize);
	if (preSize) {
		((AlifObject**)alloc)[0] = nullptr;
		((AlifObject**)alloc)[1] = nullptr;
	}
	if (ALIFTYPE_IS_GC(_type)) {
		alifObject_gcLink(obj_);
	}
	memset(obj_, '\0', size);

	if (_type->itemSize == 0) {
		alifObject_init(obj_, _type);
	}
	else {
		alifObject_initVar((AlifVarObject*)obj_, _type, _nitems);
	}
	if (_type->flags & ALIF_TPFLAGS_INLINE_VALUES) {
		alifObject_initInlineValues(obj_, _type);
	}
	return obj_;
}



AlifObject* alifType_genericAlloc(AlifTypeObject* _type, AlifSizeT _nitems) { // 2260
	AlifObject* obj = alifType_allocNoTrack(_type, _nitems);
	if (obj == nullptr) {
		return nullptr;
	}

	if (ALIFTYPE_IS_GC(_type)) {
		ALIFOBJECT_GC_TRACK(obj);
	}
	return obj;
}


static inline AlifMemberDef* _alifHeapType_getMembers(AlifHeapTypeObject* _type) { // 2282
	return (AlifMemberDef*)alifObject_getItemData((AlifObject*)_type);
}

static void clear_slots(AlifTypeObject* _type, AlifObject* _self) { // 2363
	AlifSizeT i{}, n{};
	AlifMemberDef* mp{};

	n = ALIF_SIZE(_type);
	mp = _alifHeapType_getMembers((AlifHeapTypeObject*)_type);
	for (i = 0; i < n; i++, mp++) {
		if (mp->type == ALIF_T_OBJECT_EX and !(mp->flags & ALIF_READONLY)) {
			char* addr = (char*)_self + mp->offset;
			AlifObject* obj = *(AlifObject**)addr;
			if (obj != nullptr) {
				*(AlifObject**)addr = nullptr;
				ALIF_DECREF(obj);
			}
		}
	}
}

static void subtype_dealloc(AlifObject* self) { // 2442
	AlifTypeObject* type{}, * base{};
	Destructor basedealloc{};
	AlifIntT has_finalizer{};

	AlifIntT typeNeedsDecref{};

	type = ALIF_TYPE(self);

	if (!ALIFTYPE_IS_GC(type)) {
		if (type->finalize) {
			if (alifObject_callFinalizerFromDealloc(self) < 0)
				return;
		}
		if (type->del) {
			type->del(self);
			if (ALIF_REFCNT(self) > 0) {
				return;
			}
		}

		base = type;
		while ((basedealloc = base->dealloc) == subtype_dealloc) {
			base = base->base;
		}

		type = ALIF_TYPE(self);

		AlifIntT type_needs_decref = (type->flags & ALIF_TPFLAGS_HEAPTYPE
			and !(base->flags & ALIF_TPFLAGS_HEAPTYPE));


		basedealloc(self);

		if (type_needs_decref) {
			alif_decreaseRefType(type);
		}

		/* Done */
		return;
	}

	alifObject_gcUnTrack(self);
	ALIF_TRASHCAN_BEGIN(self, subtype_dealloc);

	base = type;
	while ((/*basedealloc =*/ base->dealloc) == subtype_dealloc) {
		base = base->base;
	}

	has_finalizer = type->finalize or type->del;

	if (type->finalize) {
		ALIFOBJECT_GC_TRACK(self);
		if (alifObject_callFinalizerFromDealloc(self) < 0) {
			/* Resurrected */
			goto endlabel;
		}
		ALIFOBJECT_GC_UNTRACK(self);
	}

	if (type->weakListOffset and !base->weakListOffset) {
		alifObject_clearWeakRefs(self);
	}

	if (type->del) {
		ALIFOBJECT_GC_TRACK(self);
		type->del(self);
		if (ALIF_REFCNT(self) > 0) {
			/* Resurrected */
			goto endlabel;
		}
		ALIFOBJECT_GC_UNTRACK(self);
	}
	if (has_finalizer) {
		if (type->weakListOffset and !base->weakListOffset) {
			//_alifWeakRef_clearWeakRefsNoCallbacks(self);
		}
	}

	base = type;
	while ((basedealloc = base->dealloc) == subtype_dealloc) {
		if (ALIF_SIZE(base))
			clear_slots(base, self);
		base = base->base;
	}

	if (type->flags & ALIF_TPFLAGS_MANAGED_DICT) {
		//alifObject_clearManagedDict(self);
	}
	else if (type->dictOffset and !base->dictOffset) {
		AlifObject** dictptr = alifObject_computedDictPointer(self);
		if (dictptr != nullptr) {
			AlifObject* dict = *dictptr;
			if (dict != nullptr) {
				ALIF_DECREF(dict);
				*dictptr = nullptr;
			}
		}
	}

	type = ALIF_TYPE(self);

	if (ALIFTYPE_IS_GC(base)) {
		ALIFOBJECT_GC_TRACK(self);
	}

	typeNeedsDecref = (type->flags & ALIF_TPFLAGS_HEAPTYPE
		and !(base->flags & ALIF_TPFLAGS_HEAPTYPE));

	basedealloc(self);

	if (typeNeedsDecref) {
		alif_decreaseRefType(type);
	}

endlabel:
	ALIF_TRASHCAN_END
}

static AlifTypeObject* solid_base(AlifTypeObject*); // 2632


static AlifIntT typeIsSubType_baseChain(AlifTypeObject* _a, AlifTypeObject* _b) { // 2636
	do {
		if (_a == _b)
			return 1;
		_a = _a->base;
	} while (_a != nullptr);

	return (_b == &_alifBaseObjectType_);
}


static AlifIntT isSubType_withMro(AlifObject* _methResOrder,
	AlifTypeObject* _a, AlifTypeObject* _b) { // 2648
	AlifIntT res{};
	if (_methResOrder != nullptr) {
		AlifSizeT i{}, n{};
		n = ALIFTUPLE_GET_SIZE(_methResOrder);
		res = 0;
		for (i = 0; i < n; i++) {
			if (ALIFTUPLE_GET_ITEM(_methResOrder, i) == (AlifObject*)_b) {
				res = 1;
				break;
			}
		}
	}
	else {
		/* a is not completely initialized yet; follow base */
		res = typeIsSubType_baseChain(_a, _b);
	}
	return res;
}

AlifIntT alifType_isSubType(AlifTypeObject* a, AlifTypeObject* b) { // 2673
	return isSubType_withMro(a->mro, a, b);
}


AlifObject* alifObject_lookupSpecial(AlifObject* _self, AlifObject* _attr) { // 2694
	AlifObject* res{};

	res = alifType_lookupRef(ALIF_TYPE(_self), _attr);
	if (res != nullptr) {
		DescrGetFunc f{};
		if ((f = ALIF_TYPE(res)->descrGet) != nullptr) {
			ALIF_SETREF(res, f(res, _self, (AlifObject*)(ALIF_TYPE(_self))));
		}
	}
	return res;
}


static AlifObject* lookup_maybeMethod(AlifObject* _self, AlifObject* _attr, AlifIntT* _unbound) { // 2738
	AlifObject* res = alifType_lookupRef(ALIF_TYPE(_self), _attr);
	if (res == nullptr) {
		return nullptr;
	}

	if (alifType_hasFeature(ALIF_TYPE(res), ALIF_TPFLAGS_METHOD_DESCRIPTOR)) {
		*_unbound = 1;
	}
	else {
		*_unbound = 0;
		DescrGetFunc f_ = ALIF_TYPE(res)->descrGet;
		if (f_ != nullptr) {
			ALIF_SETREF(res, f_(res, _self, (AlifObject*)(ALIF_TYPE(_self))));
		}
	}
	return res;
}

static AlifObject* lookup_method(AlifObject* _self, AlifObject* _attr, AlifIntT* _unbound) { // 2760
	AlifObject* res_ = lookup_maybeMethod(_self, _attr, _unbound);
	if (res_ == nullptr
		//and !alifErr_occurred()
		) {
		//alifErr_setObject(_alifExcAttributeError_, _attr);
		return nullptr; // temp
	}
	return res_;
}

static AlifObject* call_unboundNoArg(AlifIntT _unbound,
	AlifObject* _func, AlifObject* _self) { // 2786
	if (_unbound) {
		//return alifObject_callOneArg(_func, _self);
	}
	else {
		//return alifObject_callNoArgs(_func);
	}
	return nullptr; // temp
}

static AlifIntT tail_contains(AlifObject* _tuple, AlifIntT _whence, AlifObject* _o) { // 2867
	AlifSizeT j_{}, size{};
	size = ALIFTUPLE_GET_SIZE(_tuple);

	for (j_ = _whence + 1; j_ < size; j_++) {
		if (ALIFTUPLE_GET_ITEM(_tuple, j_) == _o)
			return 1;
	}
	return 0;
}

static AlifObject* class_name(AlifObject* _cls) { // 2881
	AlifObject* name{};
	if (alifObject_getOptionalAttr(_cls, &ALIF_ID(__name__), &name) == 0) {
		name = alifObject_repr(_cls);
	}
	return name;
}

static AlifIntT check_duplicates(AlifObject* _tuple) { // 2890
	AlifSizeT i_{}, j_{}, n_{};
	n_ = ALIFTUPLE_GET_SIZE(_tuple);
	for (i_ = 0; i_ < n_; i_++) {
		AlifObject* o_ = ALIFTUPLE_GET_ITEM(_tuple, i_);
		for (j_ = i_ + 1; j_ < n_; j_++) {
			if (ALIFTUPLE_GET_ITEM(_tuple, j_) == o_) {
				o_ = class_name(o_);
				if (o_ != nullptr) {
					if (ALIFUSTR_CHECK(o_)) {
						//alifErr_format(_alifExcTypeError_,
							//"duplicate base class %U", o);
					}
					else {
						//alifErr_format(_alifExcTypeError_,
							//"duplicate base class);
					}
					ALIF_DECREF(o_);
				}
				return -1;
			}
		}
	}
	return 0;
}



static AlifIntT p_merge(AlifObject* _acc,
	AlifObject** _toMerge, AlifSizeT _toMergeSize) { // 2981
	AlifIntT res = 0;
	AlifSizeT i_{}, j_{}, emptyCnt{};
	AlifIntT* remain{};

	remain = (int*)alifMem_dataAlloc(_toMergeSize * sizeof(int)); // ALIFMEM_NEW(int, _toMergeSize)
	if (remain == nullptr) {
		//alifErr_noMemory();
		return -1;
	}
	for (i_ = 0; i_ < _toMergeSize; i_++)
		remain[i_] = 0;

again:
	emptyCnt = 0;
	for (i_ = 0; i_ < _toMergeSize; i_++) {
		AlifObject* candidate{};

		AlifObject* cur_tuple = _toMerge[i_];

		if (remain[i_] >= ALIFTUPLE_GET_SIZE(cur_tuple)) {
			emptyCnt++;
			continue;
		}

		candidate = ALIFTUPLE_GET_ITEM(cur_tuple, remain[i_]);
		for (j_ = 0; j_ < _toMergeSize; j_++) {
			AlifObject* j_lst = _toMerge[j_];
			if (tail_contains(j_lst, remain[j_], candidate))
				goto skip; /* continue outer loop */
		}
		res = alifList_append(_acc, candidate);
		if (res < 0)
			goto out;

		for (j_ = 0; j_ < _toMergeSize; j_++) {
			AlifObject* j_lst = _toMerge[j_];
			if (remain[j_] < ALIFTUPLE_GET_SIZE(j_lst) and
				ALIFTUPLE_GET_ITEM(j_lst, remain[j_]) == candidate) {
				remain[j_]++;
			}
		}
		goto again;
	skip:;
	}

	if (emptyCnt != _toMergeSize) {
		//setMro_error(_toMerge, _toMergeSize, remain);
		res = -1;
	}

out:
	alifMem_dataFree(remain);

	return res;
}



static AlifObject* mro_implementationUnlocked(AlifTypeObject* _type) { // 3052 
	if (!alifType_isReady(_type)) {
		if (alifType_ready(_type) < 0)
			return nullptr;
	}

	AlifObject* bases = lookup_tpBases(_type);
	AlifSizeT n = ALIFTUPLE_GET_SIZE(bases);
	for (AlifSizeT i = 0; i < n; i++) {
		AlifTypeObject* base = ALIFTYPE_CAST(ALIFTUPLE_GET_ITEM(bases, i));
		if (lookup_tpMro(base) == nullptr) {
			//alifErr_format(_alifExcTypeError_,
				//"Cannot extend an incomplete type '%.100s'",
				//base->name);
			return nullptr;
		}
	}

	if (n == 1) {
		AlifTypeObject* base = ALIFTYPE_CAST(ALIFTUPLE_GET_ITEM(bases, 0));
		AlifObject* baseMro = lookup_tpMro(base);
		AlifSizeT k = ALIFTUPLE_GET_SIZE(baseMro);
		AlifObject* result = alifTuple_new(k + 1);
		if (result == nullptr) {
			return nullptr;
		}

		ALIFTUPLE_SET_ITEM(result, 0, ALIF_NEWREF(_type));
		for (AlifSizeT i = 0; i < k; i++) {
			AlifObject* cls = ALIFTUPLE_GET_ITEM(baseMro, i);
			ALIFTUPLE_SET_ITEM(result, i + 1, ALIF_NEWREF(cls));
		}
		return result;
	}

	if (check_duplicates(bases) < 0) {
		return nullptr;
	}

	AlifObject** toMerge = (AlifObject**)alifMem_dataAlloc(n + 1);
	if (toMerge == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}

	for (AlifSizeT i = 0; i < n; i++) {
		AlifTypeObject* base = ALIFTYPE_CAST(ALIFTUPLE_GET_ITEM(bases, i));
		toMerge[i] = lookup_tpMro(base);
	}
	toMerge[n] = bases;

	AlifObject* result = alifList_new(1);
	if (result == nullptr) {
		alifMem_dataFree(toMerge);
		return nullptr;
	}

	ALIFLIST_SET_ITEM(result, 0, ALIF_NEWREF(_type));
	if (p_merge(result, toMerge, n + 1) < 0) {
		ALIF_CLEAR(result);
	}
	alifMem_dataFree(toMerge);

	return result;
}


static AlifIntT mro_check(AlifTypeObject* _type, AlifObject* _mro) { // 3163
	AlifTypeObject* solid{};
	AlifSizeT i_{}, n_{};

	solid = solid_base(_type);

	n_ = ALIFTUPLE_GET_SIZE(_mro);
	for (i_ = 0; i_ < n_; i_++) {
		AlifObject* obj = ALIFTUPLE_GET_ITEM(_mro, i_);
		if (!ALIFTYPE_CHECK(obj)) {
			//alifErr_format(
			//	_alifExcTypeError_,
			//	"mro() returned a non-class ('%.500s')",
			//	ALIF_TYPE(obj)->name);
			return -1;
		}
		AlifTypeObject* base = (AlifTypeObject*)obj;

		if (!isSubType_withMro(lookup_tpMro(solid), solid, solid_base(base))) {
			//alifErr_format(
			//	_alifExcTypeError_,
			//	"mro() returned base with unsuitable layout ('%.500s')",
			//	base->name);
			return -1;
		}
	}

	return 0;
}


static AlifObject* mro_invoke(AlifTypeObject* _type) { // 3210
	AlifObject* mroResult{};
	AlifObject* newMro{};

	const AlifIntT custom = !ALIF_IS_TYPE(_type, &_alifTypeType_);

	if (custom) {
		AlifIntT unbound{};
		AlifObject* mroMeth = lookup_method(
			(AlifObject*)_type, &ALIF_ID(Mro), &unbound);
		if (mroMeth == nullptr)
			return nullptr;
		mroResult = call_unboundNoArg(unbound, mroMeth, (AlifObject*)_type);
		ALIF_DECREF(mroMeth);
	}
	else {
		mroResult = mro_implementationUnlocked(_type);
	}
	if (mroResult == nullptr)
		return nullptr;

	newMro = alifSequence_tuple(mroResult);
	ALIF_DECREF(mroResult);
	if (newMro == nullptr) {
		return nullptr;
	}

	if (ALIFTUPLE_GET_SIZE(newMro) == 0) {
		ALIF_DECREF(newMro);
		//alifErr_format(_alifExcTypeError_, "type MRO must not be empty");
		return nullptr;
	}

	if (custom and mro_check(_type, newMro) < 0) {
		ALIF_DECREF(newMro);
		return nullptr;
	}
	return newMro;
}


static AlifIntT mro_internalUnlocked(AlifTypeObject* _type,
	AlifIntT _initial, AlifObject** _pOldMro) { // 3276

	AlifObject* newMro{}, * oldMro{};
	AlifIntT reent{};

	oldMro = ALIF_XNEWREF(lookup_tpMro(_type));
	newMro = mro_invoke(_type);
	reent = (lookup_tpMro(_type) != oldMro);
	ALIF_XDECREF(oldMro);
	if (newMro == nullptr) {
		return -1;
	}

	if (reent) {
		ALIF_DECREF(newMro);
		return 0;
	}

	set_tpMro(_type, newMro, _initial);

	type_mroModified(_type, newMro);
	type_mroModified(_type, lookup_tpBases(_type));

	if (!(_type->flags & ALIF_TPFLAGS_STATIC_BUILTIN)) {
		alifType_modified(_type);
	}
	else {
	}

	if (_pOldMro != nullptr)
		*_pOldMro = oldMro; 
	else
		ALIF_XDECREF(oldMro);

	return 1;
}



static AlifTypeObject* best_base(AlifObject* bases) { // 3337
	AlifSizeT i{}, n{};
	AlifTypeObject* base{}, * winner{}, * candidate{};

	n = ALIFTUPLE_GET_SIZE(bases);
	base = nullptr;
	winner = nullptr;
	for (i = 0; i < n; i++) {
		AlifObject* base_proto = ALIFTUPLE_GET_ITEM(bases, i);
		if (!ALIFTYPE_CHECK(base_proto)) {
			//alifErr_setString(
			//	_alifExcTypeError_,
			//	"bases must be types");
			return nullptr;
		}
		AlifTypeObject* base_i = (AlifTypeObject*)base_proto;

		if (!alifType_isReady(base_i)) {
			if (alifType_ready(base_i) < 0)
				return nullptr;
		}
		if (!_alifType_hasFeature(base_i, ALIF_TPFLAGS_BASETYPE)) {
			//alifErr_format(_alifExcTypeError_,
			//	"type '%.100s' is not an acceptable base type",
			//	base_i->name);
			return nullptr;
		}
		candidate = solid_base(base_i);
		if (winner == nullptr) {
			winner = candidate;
			base = base_i;
		}
		else if (alifType_isSubType(winner, candidate))
			;
		else if (alifType_isSubType(candidate, winner)) {
			winner = candidate;
			base = base_i;
		}
		else {
			//alifErr_setString(
			//	_alifExcTypeError_,
			//	"multiple bases have "
			//	"instance lay-out conflict");
			return nullptr;
		}
	}

	return base;
}



static AlifIntT shape_differs(AlifTypeObject* _t1, AlifTypeObject* _t2) { // 3392
	return ( _t1->basicSize != _t2->basicSize
		or _t1->itemSize != _t2->itemSize );
}

static AlifTypeObject* solid_base(AlifTypeObject* _type) { // 3401
	AlifTypeObject* base{};

	if (_type->base) {
		base = solid_base(_type->base);
	}
	else {
		base = &_alifBaseObjectType_;
	}
	if (shape_differs(_type, base)) {
		return _type;
	}
	else {
		return base;
	}
}



static void fixup_slotDispatchers(AlifTypeObject*); // 3424
static AlifIntT typeNew_setNames(AlifTypeObject*); // 3425
static AlifIntT typeNew_initSubclass(AlifTypeObject*, AlifObject*); // 3426



static AlifGetSetDef _subtypeGetSetsFull_[] = { // 3561
	{"__dict__", nullptr/*subtype_dict*/, nullptr/*subtype_setDict*/,
	 nullptr},
	{"__weakRef__", nullptr/*subtype_getWeakRef*/, nullptr,
	 nullptr},
	{0}
};

static AlifGetSetDef _subtypeGetSetsDictOnly_[] = { // 3569
	{"__dict__", nullptr/*subtype_dict*/, nullptr/*subtype_setDict*/,
	 nullptr},
	{0}
};

static AlifGetSetDef _subtypeGetSetsWeakRefOnly_[] = { // 3575
	{"__weakRef__", nullptr/*subtype_getWeakRef*/, nullptr,
	 nullptr},
	{0}
};


static AlifIntT valid_identifier(AlifObject* s) { // 3581
	if (!ALIFUSTR_CHECK(s)) {
		//alifErr_format(_alifExcTypeError_,
		//	"__slots__ items must be strings, not '%.200s'",
		//	ALIF_TYPE(s)->name);
		return 0;
	}
	if (!alifUStr_isIdentifier(s)) {
		//alifErr_setString(_alifExcTypeError_,
		//	"__slots__ must be identifiers");
		return 0;
	}
	return 1;
}


static AlifIntT type_init(AlifObject* _cls, AlifObject* _args, AlifObject* _kwds) { // 3598
	if (_kwds != nullptr and ALIFTUPLE_GET_SIZE(_args) == 1 and
		ALIFDICT_GET_SIZE(_kwds) != 0) {
		//alifErr_setString(_alifExcTypeError_,
		//	"type.__init__() takes no keyword arguments");
		return -1;
	}

	if ((ALIFTUPLE_GET_SIZE(_args) != 1 and ALIFTUPLE_GET_SIZE(_args) != 3)) {
		//alifErr_setString(_alifExcTypeError_,
		//	"type.__init__() takes 1 or 3 arguments");
		return -1;
	}

	return 0;
}


/* Determine the most derived metatype. */
AlifTypeObject* _alifType_calculateMetaclass(AlifTypeObject* _metatype,
	AlifObject* _bases) { // 3635
	AlifSizeT i{}, nbases{};
	AlifTypeObject* winner{};
	AlifObject* tmp{};
	AlifTypeObject* tmptype{};

	nbases = ALIFTUPLE_GET_SIZE(_bases);
	winner = _metatype;
	for (i = 0; i < nbases; i++) {
		tmp = ALIFTUPLE_GET_ITEM(_bases, i);
		tmptype = ALIF_TYPE(tmp);
		if (alifType_isSubType(winner, tmptype))
			continue;
		if (alifType_isSubType(tmptype, winner)) {
			winner = tmptype;
			continue;
		}
		/* else: */
		//alifErr_setString(_alifExcTypeError_,
		//	"metaclass conflict: "
		//	"the metaclass of a derived class "
		//	"must be a (non-strict) subclass "
		//	"of the metaclasses of all its bases");
		return nullptr;
	}
	return winner;
}



// Forward declaration
static AlifObject* type_new(AlifTypeObject*, AlifObject*, AlifObject*); // 3673

class TypeNewCtx { // 3676
public:
	AlifTypeObject* metatype{};
	AlifObject* args{};
	AlifObject* kwds{};
	AlifObject* origDict{};
	AlifObject* name{};
	AlifObject* bases{};
	AlifTypeObject* base{};
	AlifObject* slots{};
	AlifSizeT nslot{};
	AlifIntT addDict{};
	AlifIntT addWeak{};
	AlifIntT mayAddDict{};
	AlifIntT mayAddWeak{};
};


static AlifIntT typeNew_visitSlots(TypeNewCtx* _ctx) { // 3694
	AlifObject* slots = _ctx->slots;
	AlifSizeT nslot = _ctx->nslot;
	for (AlifSizeT i = 0; i < nslot; i++) {
		AlifObject* name = ALIFTUPLE_GET_ITEM(slots, i);
		if (!valid_identifier(name)) {
			return -1;
		}
		if (_alifUStr_equal(name, &ALIF_ID(__dict__))) {
			if (!_ctx->mayAddDict or _ctx->addDict != 0) {
				//alifErr_setString(_alifExcTypeError_,
				//	"__dict__ slot disallowed: "
				//	"we already got one");
				return -1;
			}
			_ctx->addDict++;
		}
		if (_alifUStr_equal(name, &ALIF_ID(__weakRef__))) {
			if (!_ctx->mayAddWeak or _ctx->addWeak != 0) {
				//alifErr_setString(_alfiExcTypeError_,
				//	"__weakref__ slot disallowed: "
				//	"we already got one");
				return -1;
			}
			_ctx->addWeak++;
		}
	}
	return 0;
}

static AlifObject* typeNew_copySlots(TypeNewCtx* ctx, AlifObject* dict) { // 3732
	AlifObject* slots = ctx->slots;
	AlifSizeT nslot = ctx->nslot;

	AlifObject* tuple{}; //* alif

	AlifSizeT new_nslot = nslot - ctx->addDict - ctx->addWeak;
	AlifObject* new_slots = alifList_new(new_nslot);
	if (new_slots == nullptr) {
		return nullptr;
	}

	AlifSizeT j = 0;
	for (AlifSizeT i = 0; i < nslot; i++) {
		AlifObject* slot = ALIFTUPLE_GET_ITEM(slots, i);
		if ((ctx->addDict and _alifUStr_equal(slot, &ALIF_ID(__dict__))) ||
			(ctx->addWeak and _alifUStr_equal(slot, &ALIF_ID(__weakRef__))))
		{
			continue;
		}

		slot = alif_mangle(ctx->name, slot);
		if (!slot) {
			goto error;
		}
		ALIFLIST_SET_ITEM(new_slots, j, slot);

		AlifIntT r = alifDict_contains(dict, slot);
		if (r < 0) {
			goto error;
		}
		if (r > 0) {
			if (!_alifUStr_equal(slot, &ALIF_ID(__qualname__)) and
				!_alifUStr_equal(slot, &ALIF_ID(__classCell__)) and
				!_alifUStr_equal(slot, &ALIF_ID(__classDictCell__)))
			{
				//alifErr_format(_alifExcValueError_,
				//	"%R in __slots__ conflicts with class variable",
				//	slot);
				goto error;
			}
		}

		j++;
	}

	if (alifList_sort(new_slots) == -1) {
		goto error;
	}

	tuple = alifList_asTuple(new_slots);
	ALIF_DECREF(new_slots);
	if (tuple == nullptr) {
		return nullptr;
	}

	return tuple;

error:
	ALIF_DECREF(new_slots);
	return nullptr;
}


static void typeNew_slotsBases(TypeNewCtx* _ctx) { // 3801
	AlifSizeT nbases = ALIFTUPLE_GET_SIZE(_ctx->bases);
	if (nbases > 1 and
		((_ctx->mayAddDict and _ctx->addDict == 0) or
			(_ctx->mayAddWeak and _ctx->addWeak == 0)))
	{
		for (AlifSizeT i = 0; i < nbases; i++) {
			AlifObject* obj = ALIFTUPLE_GET_ITEM(_ctx->bases, i);
			if (obj == (AlifObject*)_ctx->base) {
				/* Skip primary base */
				continue;
			}
			AlifTypeObject* base = ALIFTYPE_CAST(obj);

			if (_ctx->mayAddDict and _ctx->addDict == 0 and
				base->dictOffset != 0)
			{
				_ctx->addDict++;
			}
			if (_ctx->mayAddWeak and _ctx->addWeak == 0 and
				base->weakListOffset != 0)
			{
				_ctx->addWeak++;
			}
			if (_ctx->mayAddDict and _ctx->addDict == 0) {
				continue;
			}
			if (_ctx->mayAddWeak and _ctx->addWeak == 0) {
				continue;
			}
			/* Nothing more to check */
			break;
		}
	}
}


static AlifIntT typeNew_slotsImpl(TypeNewCtx* _ctx, AlifObject* _dict) { // 3840
	/* Are slots allowed? */
	if (_ctx->nslot > 0 and _ctx->base->itemSize != 0) {
		//alifErr_format(_alifExcTypeError_,
		//	"nonempty __slots__ not supported for subtype of '%s'",
		//	_ctx->base->name);
		return -1;
	}

	if (typeNew_visitSlots(_ctx) < 0) {
		return -1;
	}

	AlifObject* new_slots = typeNew_copySlots(_ctx, _dict);
	if (new_slots == nullptr) {
		return -1;
	}

	ALIF_XSETREF(_ctx->slots, new_slots);
	_ctx->nslot = ALIFTUPLE_GET_SIZE(new_slots);

	/* Secondary bases may provide weakrefs or dict */
	typeNew_slotsBases(_ctx);
	return 0;
}

static AlifSizeT typeNew_slots(TypeNewCtx* _ctx, AlifObject* _dict) { // 3870
	_ctx->addDict = 0;
	_ctx->addWeak = 0;
	_ctx->mayAddDict = (_ctx->base->dictOffset == 0);
	_ctx->mayAddWeak = (_ctx->base->weakListOffset == 0
		&& _ctx->base->itemSize == 0);

	if (_ctx->slots == nullptr) {
		if (_ctx->mayAddDict) {
			_ctx->addDict++;
		}
		if (_ctx->mayAddWeak) {
			_ctx->addWeak++;
		}
	}
	else {
		/* Have slots */
		if (typeNew_slotsImpl(_ctx, _dict) < 0) {
			return -1;
		}
	}
	return 0;
}



static AlifTypeObject* typeNew_alloc(TypeNewCtx* _ctx) { // 3898
	AlifTypeObject* metatype = _ctx->metatype;
	AlifTypeObject* type{};

	// Allocate the type object
	type = (AlifTypeObject*)metatype->alloc(metatype, _ctx->nslot);
	if (type == nullptr) {
		return nullptr;
	}
	AlifHeapTypeObject* et = (AlifHeapTypeObject*)type;


	type->flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HEAPTYPE |
		ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC);

	// Initialize essential fields
	//type->asAsync = &et->async;
	type->asNumber = &et->number;
	type->asSequence = &et->sequence;
	type->asMapping = &et->mapping;
	type->asBuffer = &et->buffer;

	set_tpBases(type, ALIF_NEWREF(_ctx->bases), 1);
	type->base = (AlifTypeObject*)ALIF_NEWREF(_ctx->base);

	type->dealloc = subtype_dealloc;
	/* Always override allocation strategy to use regular heap */
	type->alloc = alifType_genericAlloc;
	type->free = alifObject_gcDel;

	//type->traverse = subtype_traverse;
	//type->clear = subtype_clear;

	et->name = ALIF_NEWREF(_ctx->name);
	et->module_ = nullptr;
	//et->tpName = nullptr;

	_alifType_assignId(et);

	return type;
}


static AlifIntT typeNew_setName(const TypeNewCtx* ctx, AlifTypeObject* type) { // 3947
	AlifSizeT nameSize{};
	type->name = alifUStr_asUTF8AndSize(ctx->name, &nameSize);
	if (!type->name) {
		return -1;
	}
	if (strlen(type->name) != (AlifUSizeT)nameSize) {
		//alifErr_setString(_alifExcValueError_,
		//	"type name must not contain null characters");
		return -1;
	}
	return 0;
}


static AlifIntT typeNew_setModule(AlifTypeObject* _type) { // 3965
	AlifObject* dict = lookup_tpDict(_type);
	AlifIntT r = alifDict_contains(dict, &ALIF_ID(__module__));
	if (r < 0) {
		return -1;
	}
	if (r > 0) {
		return 0;
	}

	AlifObject* globals = alifEval_getGlobals();
	if (globals == nullptr) {
		return 0;
	}

	AlifObject* module{};
	r = alifDict_getItemRef(globals, &ALIF_ID(__name__), &module);
	if (module) {
		r = alifDict_setItem(dict, &ALIF_ID(__module__), module);
		ALIF_DECREF(module);
	}
	return r;
}


static AlifIntT typeNew_setHtName(AlifTypeObject* _type) { // 3994
	AlifHeapTypeObject* et = (AlifHeapTypeObject*)_type;
	AlifObject* dict = lookup_tpDict(_type);
	AlifObject* qualname;
	if (alifDict_getItemRef(dict, &ALIF_ID(__qualname__), &qualname) < 0) {
		return -1;
	}
	if (qualname != nullptr) {
		if (!ALIFUSTR_CHECK(qualname)) {
			//alifErr_format(_alifExcTypeError_,
			//	"type __qualname__ must be a str, not %s",
			//	ALIF_TYPE(qualname)->name);
			ALIF_DECREF(qualname);
			return -1;
		}
		et->qualname = qualname;
		if (alifDict_delItem(dict, &ALIF_ID(__qualname__)) < 0) {
			return -1;
		}
	}
	else {
		et->qualname = ALIF_NEWREF(et->name);
	}
	return 0;
}




static AlifIntT typeNew_staticMethod(AlifTypeObject* _type, AlifObject* _attr) { // 4062
	AlifObject* dict = lookup_tpDict(_type);
	AlifObject* func = alifDict_getItemWithError(dict, _attr);
	if (func == nullptr) {
		//if (alifErr_occurred()) {
		//	return -1;
		//}
		return 0;
	}
	if (!ALIFFUNCTION_CHECK(func)) {
		return 0;
	}

	AlifObject* staticFunc = alifStaticMethod_new(func);
	if (staticFunc == nullptr) {
		return -1;
	}
	if (alifDict_setItem(dict, _attr, staticFunc) < 0) {
		ALIF_DECREF(staticFunc);
		return -1;
	}
	ALIF_DECREF(staticFunc);
	return 0;
}


static AlifIntT typeNew_classMethod(AlifTypeObject* _type, AlifObject* _attr) { // 4090
	AlifObject* dict = lookup_tpDict(_type);
	AlifObject* func = alifDict_getItemWithError(dict, _attr);
	if (func == nullptr) {
		//if (alifErr_occurred()) {
		//	return -1;
		//}
		return 0;
	}
	if (!ALIFFUNCTION_CHECK(func)) {
		return 0;
	}

	AlifObject* method = alifClassMethod_new(func);
	if (method == nullptr) {
		return -1;
	}

	if (alifDict_setItem(dict, _attr, method) < 0) {
		ALIF_DECREF(method);
		return -1;
	}
	ALIF_DECREF(method);
	return 0;
}



static AlifIntT typeNew_descriptors(const TypeNewCtx* _ctx, AlifTypeObject* _type) { // 4120
	AlifHeapTypeObject* et = (AlifHeapTypeObject*)_type;
	AlifSizeT slotoffset = _ctx->base->basicSize;
	if (et->slots != nullptr) {
		AlifMemberDef* mp = _alifHeapType_getMembers(et);
		AlifSizeT nslot = ALIFTUPLE_GET_SIZE(et->slots);
		for (AlifSizeT i = 0; i < nslot; i++, mp++) {
			mp->name = alifUStr_asUTF8(
				ALIFTUPLE_GET_ITEM(et->slots, i));
			if (mp->name == nullptr) {
				return -1;
			}
			mp->type = ALIF_T_OBJECT_EX;
			mp->offset = slotoffset;

			slotoffset += sizeof(AlifObject*);
		}
	}

	if (_ctx->addWeak) {
		_type->flags |= ALIF_TPFLAGS_MANAGED_WEAKREF;
		_type->weakListOffset = MANAGED_WEAKREF_OFFSET;
	}
	if (_ctx->addDict) {
		_type->flags |= ALIF_TPFLAGS_MANAGED_DICT;
		_type->dictOffset = -1;
	}

	_type->basicSize = slotoffset;
	_type->itemSize = _ctx->base->itemSize;
	_type->members = _alifHeapType_getMembers(et);
	return 0;
}



static void typeNew_setSlots(const TypeNewCtx* ctx, AlifTypeObject* type) { // 4163
	if (type->weakListOffset and type->dictOffset) {
		type->getSet = _subtypeGetSetsFull_;
	}
	else if (type->weakListOffset and !type->dictOffset) {
		type->getSet = _subtypeGetSetsWeakRefOnly_;
	}
	else if (!type->weakListOffset and type->dictOffset) {
		type->getSet = _subtypeGetSetsDictOnly_;
	}
	else {
		type->getSet = nullptr;
	}

	/* Special case some slots */
	if (type->dictOffset != 0 or ctx->nslot > 0) {
		AlifTypeObject* base = ctx->base;
		if (base->getAttr == nullptr and base->getAttro == nullptr) {
			type->getAttro = alifObject_genericGetAttr;
		}
		if (base->setAttr == nullptr and base->setAttro == nullptr) {
			type->setAttro = alifObject_genericSetAttr;
		}
	}
}


static AlifIntT typeNew_setClassCell(AlifTypeObject* _type) { // 4193
	AlifObject* dict = lookup_tpDict(_type);
	AlifObject* cell = alifDict_getItemWithError(dict, &ALIF_ID(__classCell__));
	if (cell == nullptr) {
		//if (alifErr_occurred()) {
		//	return -1;
		//}
		return 0;
	}

	/* At least one method requires a reference to its defining class */
	if (!ALIFCELL_CHECK(cell)) {
		//alifErr_format(_alifExcTypeError_,
		//	"__classcell__ must be a nonlocal cell, not %.200R",
		//	ALIF_TYPE(cell));
		return -1;
	}

	(void)alifCell_set(cell, (AlifObject*)_type);
	if (alifDict_delItem(dict, &ALIF_ID(__classCell__)) < 0) {
		return -1;
	}
	return 0;
}


static AlifIntT typeNew_setClassDictCell(AlifTypeObject* _type) { // 4220
	AlifObject* dict = lookup_tpDict(_type);
	AlifObject* cell = alifDict_getItemWithError(dict, &ALIF_ID(__classDictCell__));
	if (cell == nullptr) {
		//if (alifErr_occurred()) {
		//	return -1;
		//}
		return 0;
	}

	/* At least one method requires a reference to the dict of its defining class */
	if (!ALIFCELL_CHECK(cell)) {
		//alifErr_format(_alifExcTypeError_,
		//	"__classdictcell__ must be a nonlocal cell, not %.200R",
		//	ALIF_TYPE(cell));
		return -1;
	}

	(void)alifCell_set(cell, (AlifObject*)dict);
	if (alifDict_delItem(dict, &ALIF_ID(__classDictCell__)) < 0) {
		return -1;
	}
	return 0;
}


static AlifIntT typeNew_setAttrs(const TypeNewCtx* _ctx, AlifTypeObject* _type) { // 4247
	if (typeNew_setName(_ctx, _type) < 0) {
		return -1;
	}

	if (typeNew_setModule(_type) < 0) {
		return -1;
	}

	if (typeNew_setHtName(_type) < 0) {
		return -1;
	}

	//if (typeNew_setDoc(_type) < 0) {
	//	return -1;
	//}

	/* Special-case __new__: if it's a plain function,
	   make it a static function */
	if (typeNew_staticMethod(_type, &ALIF_ID(__new__)) < 0) {
		return -1;
	}

	/* Special-case __init_subclass__ and __class_getitem__:
	   if they are plain functions, make them classmethods */
	if (typeNew_classMethod(_type, &ALIF_ID(__initSubclass__)) < 0) {
		return -1;
	}
	if (typeNew_classMethod(_type, &ALIF_ID(__classGetItem__)) < 0) {
		return -1;
	}

	if (typeNew_descriptors(_ctx, _type) < 0) {
		return -1;
	}

	typeNew_setSlots(_ctx, _type);

	if (typeNew_setClassCell(_type) < 0) {
		return -1;
	}
	if (typeNew_setClassDictCell(_type) < 0) {
		return -1;
	}
	return 0;
}


static AlifIntT typeNew_getSlots(TypeNewCtx* _ctx, AlifObject* _dict) { // 4297
	AlifObject* slots = alifDict_getItemWithError(_dict, &ALIF_ID(__slots__));
	if (slots == nullptr) {
		//if (alifErr_occurred()) {
		//	return -1;
		//}
		_ctx->slots = nullptr;
		_ctx->nslot = 0;
		return 0;
	}

	// Make it into a tuple
	AlifObject* new_slots{};
	if (ALIFUSTR_CHECK(slots)) {
		new_slots = alifTuple_pack(1, slots);
	}
	else {
		new_slots = alifSequence_tuple(slots);
	}
	if (new_slots == nullptr) {
		return -1;
	}
	_ctx->slots = new_slots;
	_ctx->nslot = ALIFTUPLE_GET_SIZE(new_slots);
	return 0;
}


static AlifTypeObject* typeNew_init(TypeNewCtx* _ctx) { // 4328

	AlifTypeObject* type{};
	AlifHeapTypeObject* et{};

	AlifObject* dict = alifDict_copy(_ctx->origDict);
	if (dict == nullptr) {
		goto error;
	}

	if (typeNew_getSlots(_ctx, dict) < 0) {
		goto error;
	}

	if (typeNew_slots(_ctx, dict) < 0) {
		goto error;
	}

	type = typeNew_alloc(_ctx);
	if (type == nullptr) {
		goto error;
	}

	set_tpDict(type, dict);

	et = (AlifHeapTypeObject*)type;
	et->slots = _ctx->slots;
	_ctx->slots = nullptr;

	return type;

error:
	ALIF_CLEAR(_ctx->slots);
	ALIF_XDECREF(dict);
	return nullptr;
}


static AlifObject* type_newImpl(TypeNewCtx* _ctx) { // 4365
	AlifTypeObject* type = typeNew_init(_ctx);
	if (type == nullptr) {
		return nullptr;
	}

	if (typeNew_setAttrs(_ctx, type) < 0) {
		goto error;
	}

	/* Initialize the rest */
	if (alifType_ready(type) < 0) {
		goto error;
	}

	// Put the proper slots in place
	fixup_slotDispatchers(type);

	if (!_alifDict_hasOnlyStringKeys(type->dict)) {
		//if (alifErr_WarnFormat(
		//	_alifExcRuntimeWarning_,
		//	1,
		//	"non-string key in the __dict__ of class %.200s",
		//	type->name) == -1)
		//{
		//	goto error;
		//}
	}

	if (typeNew_setNames(type) < 0) {
		goto error;
	}

	if (typeNew_initSubclass(type, _ctx->kwds) < 0) {
		goto error;
	}


	return (AlifObject*)type;

error:
	ALIF_DECREF(type);
	return nullptr;
}


static AlifIntT typeNew_getBases(TypeNewCtx* _ctx, AlifObject** _type) { // 4414
	AlifSizeT nbases = ALIFTUPLE_GET_SIZE(_ctx->bases);
	if (nbases == 0) {
		// Adjust for empty tuple bases
		_ctx->base = &_alifBaseObjectType_;
		AlifObject* new_bases = alifTuple_pack(1, _ctx->base);
		if (new_bases == nullptr) {
			return -1;
		}
		_ctx->bases = new_bases;
		return 0;
	}

	for (AlifSizeT i = 0; i < nbases; i++) {
		AlifObject* base = ALIFTUPLE_GET_ITEM(_ctx->bases, i);
		if (ALIFTYPE_CHECK(base)) {
			continue;
		}
		AlifIntT rc = alifObject_hasAttrWithError(base, &ALIF_ID(__mroEntries__));
		if (rc < 0) {
			return -1;
		}
		if (rc) {
			//alifErr_setString(_alifExcTypeError_,
			//	"type() doesn't support MRO entry resolution; "
			//	"use types.new_class()");
			return -1;
		}
	}

	// Search the bases for the proper metatype to deal with this
	AlifTypeObject* winner{};
	winner = _alifType_calculateMetaclass(_ctx->metatype, _ctx->bases);
	if (winner == nullptr) {
		return -1;
	}

	if (winner != _ctx->metatype) {
		if (winner->new_ != type_new) {
			/* Pass it to the winner */
			*_type = winner->new_(winner, _ctx->args, _ctx->kwds);
			if (*_type == nullptr) {
				return -1;
			}
			return 1;
		}

		_ctx->metatype = winner;
	}

	/* Calculate best base, and check that all bases are type objects */
	AlifTypeObject* base = best_base(_ctx->bases);
	if (base == nullptr) {
		return -1;
	}

	_ctx->base = base;
	_ctx->bases = ALIF_NEWREF(_ctx->bases);
	return 0;
}


static AlifObject* type_new(AlifTypeObject* _metatype,
	AlifObject* _args, AlifObject* _kwds) { // 4478
	AlifObject* name{}, * bases{}, * orig_dict{};
	if (!alifArg_parseTuple(_args, "UO!O!:type.__new__",
		&name,
		&_alifTupleType_, &bases,
		&_alifDictType_, &orig_dict))
	{
		return nullptr;
	}

	TypeNewCtx ctx = {
		.metatype = _metatype,
		.args = _args,
		.kwds = _kwds,
		.origDict = orig_dict,
		.name = name,
		.bases = bases,
		.base = nullptr,
		.slots = nullptr,
		.nslot = 0,
		.addDict = 0,
		.addWeak = 0,
		.mayAddDict = 0,
		.mayAddWeak = 0 };
	AlifObject* type = nullptr;
	AlifIntT res = typeNew_getBases(&ctx, &type);
	if (res < 0) {
		return nullptr;
	}
	if (res == 1) {
		return type;
	}

	type = type_newImpl(&ctx);
	ALIF_DECREF(ctx.bases);
	return type;
}


static AlifObject* type_vectorCall(AlifObject* metatype, AlifObject* const* args,
	AlifUSizeT nargsf, AlifObject* kwnames) { // 4527
	AlifSizeT nargs = ALIFVECTORCALL_NARGS(nargsf);
	if (nargs == 1 and metatype == (AlifObject*)&_alifTypeType_) {
		if (!_ALIFARG_NOKWNAMES("type", kwnames)) {
			return nullptr;
		}
		return ALIF_NEWREF(ALIF_TYPE(args[0]));
	}
	AlifThread* tstate = _alifThread_get();
	return alifObject_makeTpCall(tstate, metatype, args, nargs, kwnames);
}


void* alifObject_getItemData(AlifObject* _obj) { // 5276
	if (!alifType_hasFeature(ALIF_TYPE(_obj), ALIF_TPFLAGS_ITEMS_AT_END)) {
		//alifErr_format(_alifExcTypeError_,
		//	"type '%s' does not have ALIF_TPFLAGS_ITEMS_AT_END",
		//	ALIF_TYPE(obj)->name);
		return nullptr;
	}
	return (char*)_obj + ALIF_TYPE(_obj)->basicSize;
}


static AlifObject* findName_inMro(AlifTypeObject* _type, AlifObject* _name, AlifIntT* _error) { //  5291
	AlifHashT hash = alifObject_hashFast(_name);
	if (hash == -1) {
		*_error = -1;
		return nullptr;
	}

	AlifObject* mro = lookup_tpMro(_type);
	if (mro == nullptr) {
		if (!is_readying(_type)) {
			if (alifType_ready(_type) < 0) {
				*_error = -1;
				return nullptr;
			}
			mro = lookup_tpMro(_type);
		}
		if (mro == nullptr) {
			*_error = 1;
			return nullptr;
		}
	}

	AlifObject* res = nullptr;
	ALIF_INCREF(mro);
	AlifSizeT n = ALIFTUPLE_GET_SIZE(mro);
	for (AlifSizeT i = 0; i < n; i++) {
		AlifObject* base = ALIFTUPLE_GET_ITEM(mro, i);
		AlifObject* dict = lookup_tpDict(ALIFTYPE_CAST(base));
		if (alifDict_getItemRefKnownHash((AlifDictObject*)dict, _name, hash, &res) < 0) {
			*_error = -1;
			goto done;
		}
		if (res != nullptr) {
			break;
		}
	}
	*_error = 0;
done:
	ALIF_DECREF(mro);
	return res;
}





static void updateCache_gilDisabled(TypeCacheEntry* _entry, AlifObject* _name,
	AlifUIntT _versionTag, AlifObject* _value) { // 5375
	//alifSeqLock_lockWrite(&_entry->sequence);

	//if (_entry->name == _name and
	//	_entry->value == _value and
	//	_entry->version == _versionTag) {
	//	alifSeqLock_abandonWrite(&_entry->sequence);
	//	return;
	//}

	//AlifObject* oldValue = update_cache(_entry, _name, _versionTag, _value);

	//alifSeqLock_unlockWrite(&_entry->sequence);

	//ALIF_DECREF(oldValue);
}


AlifObject* alifType_lookupRef(AlifTypeObject* _type, AlifObject* _name) { // 5420
	AlifObject* res{};
	AlifIntT error{};
	AlifInterpreter* interp = _alifInterpreter_get();

	AlifUIntT h_ = MCACHE_HASH_METHOD(_type, _name);
	TypeCache* cache = get_typeCache();
	TypeCacheEntry* entry = &cache->hashTable[h_];
	while (1) {
		uint32_t sequence = alifSeqLock_beginRead(&entry->sequence);
		uint32_t entryVersion = alifAtomic_loadUint32Relaxed(&entry->version);
		uint32_t typeVersion = alifAtomic_loadUint32Acquire(&_type->versionTag);
		if (entryVersion == typeVersion and
			alifAtomic_loadPtrRelaxed(&entry->name) == _name) {
			AlifObject* value = (AlifObject*)alifAtomic_loadPtrRelaxed(&entry->value);
			if (value == nullptr or alif_tryIncRef(value)) {
				if (alifSeqLock_endRead(&entry->sequence, sequence)) {
					return value;
				}
				ALIF_XDECREF(value);
			}
			else {
				break;
			}
		}
		else {
			break;
		}
	}

	AlifIntT hasVersion = 0;
	AlifIntT version = 0;
	BEGIN_TYPE_LOCK();
	res = findName_inMro(_type, _name, &error);
	if (MCACHE_CACHEABLE_NAME(_name)) {
		hasVersion = assign_versionTag(interp, _type);
		version = _type->versionTag;
	}
	END_TYPE_LOCK();

	if (error) {

		if (error == -1) {
			//alifErr_clear();
		}
		return nullptr;
	}

	if (hasVersion) {
		updateCache_gilDisabled(entry, _name, version, res);
	}
	return res;
}

AlifObject* alifType_getAttroImpl(AlifTypeObject* _type,
	AlifObject* _name, AlifIntT* _suppressMissingAttribute) { // 5580
	AlifTypeObject* metatype = ALIF_TYPE(_type);
	AlifObject* metaAttribute{}, * attribute{};
	DescrGetFunc metaGet{};
	AlifObject* res{};

	if (!ALIFUSTR_CHECK(_name)) {
		//alifErr_format(_alifExctypeError_,
			//"attribute name must be string, not '%.200s'",
			//ALIF_TYPE(name)->name);
		return nullptr;
	}

	if (!alifType_isReady(_type)) {
		if (alifType_ready(_type) < 0)
			return nullptr;
	}

	/* No readable descriptor found yet */
	metaGet = nullptr;

	/* Look for the attribute in the metatype */
	metaAttribute = alifType_lookupRef(metatype, _name);

	if (metaAttribute != nullptr) {
		metaGet = ALIF_TYPE(metaAttribute)->descrGet;

		if (metaGet != nullptr and alifDescr_isData(metaAttribute)) {
			res = metaGet(metaAttribute, (AlifObject*)_type,
				(AlifObject*)metatype);
			ALIF_DECREF(metaAttribute);
			return res;
		}
	}

	attribute = alifType_lookupRef(_type, _name);
	if (attribute != nullptr) {
		DescrGetFunc localGet = ALIF_TYPE(attribute)->descrGet;

		ALIF_XDECREF(metaAttribute);

		if (localGet != nullptr) {
			res = localGet(attribute, (AlifObject*)nullptr,
				(AlifObject*)_type);
			ALIF_DECREF(attribute);
			return res;
		}

		return attribute;
	}
	if (metaGet != nullptr) {
		AlifObject* res{};
		res = metaGet(metaAttribute, (AlifObject*)_type,
			(AlifObject*)metatype);
		ALIF_DECREF(metaAttribute);
		return res;
	}

	if (metaAttribute != nullptr) {
		return metaAttribute;
	}

	if (_suppressMissingAttribute == nullptr) {
		//alfiErr_format(_alifExcAttributeError_,
			//"type object '%.100s' has no attribute '%U'",
			//type->name, _name);
	}
	else {
		// signal the caller we have not set an _alifExcAttributeError_ and gave up
		*_suppressMissingAttribute = 1;
	}
	return nullptr;
}


AlifObject* alifType_getAttro(AlifObject* _type, AlifObject* _name) { // 5672
	return alifType_getAttroImpl((AlifTypeObject*)_type, _name, nullptr);
}

static void type_dealloc(AlifObject* self) { // 5911
	AlifTypeObject* type = (AlifTypeObject*)self;

	ALIFOBJECT_GC_UNTRACK(type);
	//type_deallocCommon(type);

	//alifObject_clearWeakRefs((AlifObject*)type);

	ALIF_XDECREF(type->base);
	ALIF_XDECREF(type->dict);
	ALIF_XDECREF(type->bases);
	ALIF_XDECREF(type->mro);
	//ALIF_XDECREF(type->cache);
	//clear_tpSubclasses(type);

	AlifHeapTypeObject* et = (AlifHeapTypeObject*)type;
	ALIF_XDECREF(et->name);
	ALIF_XDECREF(et->qualname);
	ALIF_XDECREF(et->slots);
	if (et->cachedKeys) {
		alifDictKeys_decRef(et->cachedKeys);
	}
	ALIF_XDECREF(et->module_);
	alifMem_objFree(et->name);

	alifType_releaseID(et);

	ALIF_TYPE(type)->free((AlifObject*)type);
}

static AlifObject* type___subclasses___impl(AlifTypeObject* _self) { // 5960
	return _alifType_getSubClasses(_self);
}

static AlifObject* type_prepare(AlifObject* self, AlifObject* const* args,
	AlifSizeT nargs, AlifObject* kwnames) { // 5967
	return alifDict_new();
}

static AlifMethodDef _typeMethods_[] = { // 6083
	//TYPE_MRO_METHODDEF
	TYPE___SUBCLASSES___METHODDEF
	{"__prepare__", ALIF_CPPFUNCTION_CAST(type_prepare),
	 METHOD_FASTCALL | METHOD_KEYWORDS | METHOD_CLASS,
	 /*ALIFDOC_STR("__prepare__($cls, name, bases, /, **kwds)\n"
			   "--\n"
			   "\n"
			   "Create the namespace for the class statement")*/},
	//TYPE___INSTANCECHECK___METHODDEF
	//TYPE___SUBCLASSCHECK___METHODDEF
	//TYPE___DIR___METHODDEF
	//TYPE___SIZEOF___METHODDEF
	{0}
};




AlifTypeObject _alifTypeType_ = { // 6195
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "نوع",
	.basicSize = sizeof(AlifHeapTypeObject),
	.itemSize = sizeof(AlifMemberDef),
	.dealloc = type_dealloc,
	.vectorCallOffset = offsetof(AlifTypeObject, vectorCall),
	.call = type_call,
	.getAttro = alifType_getAttro,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_TYPE_SUBCLASS |
	ALIF_TPFLAGS_HAVE_VECTORCALL |
	ALIF_TPFLAGS_ITEMS_AT_END,

	.methods = _typeMethods_,
	.dictOffset = offsetof(AlifTypeObject, dict),
	.init = type_init,
	.new_ = type_new,
	.free = alifObject_gcDel,
	.vectorCall = type_vectorCall,
};



static AlifObject* object_new(AlifTypeObject*, AlifObject*, AlifObject*); // 6286

static AlifIntT excess_args(AlifObject* _args, AlifObject* _kwds) { // 6289
	return ALIFTUPLE_GET_SIZE(_args) or
		(_kwds and ALIFDICT_CHECK(_kwds) and ALIFDICT_GET_SIZE(_kwds));
}

static AlifIntT object_init(AlifObject* _self, AlifObject* _args, AlifObject* _kwds) { // 6296
	AlifTypeObject* type = ALIF_TYPE(_self);
	if (excess_args(_args, _kwds)) {
		if (type->init != object_init) {
			//alifErr_setString(_alifExcTypeError_,
			//	"object.__init__() takes exactly one argument (the instance to initialize)");
			return -1;
		}
		if (type->new_ == object_new) {
			//alifErr_format(_alifExcTypeError_,
			//	"%.200s.__init__() takes exactly one argument (the instance to initialize)",
			//	type->name);
			return -1;
		}
	}
	return 0;
}

static AlifObject* object_new(AlifTypeObject* type,
	AlifObject* args, AlifObject* kwds) { // 6316
	if (excess_args(args, kwds)) {
		if (type->new_ != object_new) {
			//alifErr_setString(_alifExcTypeError_,
			//	"object.__new__() takes exactly one argument (the type to instantiate)");
			return nullptr;
		}
		if (type->init == object_init) {
			//alifErr_format(_alifExcTypeError_, "%.200s() takes no arguments",
			//	type->name);
			return nullptr;
		}
	}

	if (type->flags & ALIF_TPFLAGS_IS_ABSTRACT) {
		AlifObject* abstract_methods{};
		AlifObject* sorted_methods{};
		AlifObject* joined{};
		AlifObject* comma_w_quotes_sep{};
		AlifSizeT method_count{};

		abstract_methods = type_abstractMethods(type, nullptr);
		if (abstract_methods == nullptr)
			return nullptr;
		sorted_methods = alifSequence_list(abstract_methods);
		ALIF_DECREF(abstract_methods);
		if (sorted_methods == nullptr)
			return nullptr;
		if (alifList_sort(sorted_methods)) {
			ALIF_DECREF(sorted_methods);
			return nullptr;
		}
		comma_w_quotes_sep = alifUStr_fromString("', '");
		if (!comma_w_quotes_sep) {
			ALIF_DECREF(sorted_methods);
			return nullptr;
		}
		joined = alifUStr_join(comma_w_quotes_sep, sorted_methods);
		ALIF_DECREF(comma_w_quotes_sep);
		if (joined == nullptr) {
			ALIF_DECREF(sorted_methods);
			return nullptr;
		}
		method_count = alifObject_length(sorted_methods);
		ALIF_DECREF(sorted_methods);
		if (method_count == -1) {
			ALIF_DECREF(joined);
			return nullptr;
		}

		//alifErr_format(_alifExcTypeError_,
		//	"Can't instantiate abstract class %s "
		//	"without an implementation for abstract method%s '%U'",
		//	type->name,
		//	method_count > 1 ? "s" : "",
		//	joined);
		ALIF_DECREF(joined);
		return nullptr;
	}
	AlifObject* obj = type->alloc(type, 0);
	if (obj == nullptr) {
		return nullptr;
	}
	return obj;
}


static void object_dealloc(AlifObject* _self) { // 6386
	ALIF_TYPE(_self)->free(_self);
}


static AlifObject* object_initSubclass(AlifObject* cls, AlifObject* arg) { // 7317
	return ALIF_NONE;
}




static AlifObject* object___format__Impl(AlifObject* self, AlifObject* format_spec) { // 7340
	if (ALIFUSTR_GET_LENGTH(format_spec) > 0) {
		//alifErr_format(_alifExcTypeError_,
		//	"unsupported format string passed to %.200s.__format__",
		//	ALIF_TYPE(self)->name);
		return nullptr;
	}
	return alifObject_str(self);
}




static AlifMethodDef _objectMethods_[] = { // 7433

	{"__initSubclass__", object_initSubclass, METHOD_CLASS | METHOD_NOARGS},
	OBJECT___FORMAT___METHODDEF,
	{0}
};




AlifTypeObject _alifBaseObjectType_ = { // 7453
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "كائن",              
	.basicSize = sizeof(AlifObject),
	.itemSize = 0,
	.dealloc = object_dealloc,
	.hash = alifObject_genericHash,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE,
	.methods = _objectMethods_,
	.init = object_init,
	.alloc = alifType_genericAlloc,
	.new_ = object_new,
	.free = alifMem_objFree,
};



static AlifIntT type_addMethod(AlifTypeObject* type, AlifMethodDef* meth) { // 7496
	AlifObject* descr{};
	AlifIntT isdescr = 1;
	if (meth->flags & METHOD_CLASS) {
		if (meth->flags & METHOD_STATIC) {
			//alifErr_setString(_alifExcValueError_,
			//	"method cannot be both class and static");
			return -1;
		}
		descr = alifDescr_newClassMethod(type, meth);
	}
	else if (meth->flags & METHOD_STATIC) {
		AlifObject* cfunc = ALIFCPPFUNCTION_NEWEX(meth, (AlifObject*)type, nullptr);
		if (cfunc == nullptr) {
			return -1;
		}
		descr = alifStaticMethod_new(cfunc);
		isdescr = 0;
		ALIF_DECREF(cfunc);
	}
	else {
		descr = alifDescr_newMethod(type, meth);
	}
	if (descr == nullptr) {
		return -1;
	}

	AlifObject* name{};
	if (isdescr) {
		name = ALIFDESCR_NAME(descr);
	}
	else {
		name = alifUStr_fromString(meth->name);
		if (name == nullptr) {
			ALIF_DECREF(descr);
			return -1;
		}
	}

	AlifIntT err{};
	AlifObject* dict = lookup_tpDict(type);
	if (!(meth->flags & METHOD_COEXIST)) {
		err = alifDict_setDefaultRef(dict, name, descr, nullptr) < 0;
	}
	else {
		err = alifDict_setItem(dict, name, descr) < 0;
	}
	if (!isdescr) {
		ALIF_DECREF(name);
	}
	ALIF_DECREF(descr);
	if (err) {
		return -1;
	}
	return 0;
}

static AlifIntT type_addMethods(AlifTypeObject* _type) { // 7557
	AlifMethodDef* meth = _type->methods;
	if (meth == nullptr) {
		return 0;
	}

	for (; meth->name != nullptr; meth++) {
		if (type_addMethod(_type, meth) < 0) {
			return -1;
		}
	}
	return 0;
}



static void inherit_special(AlifTypeObject* type, AlifTypeObject* base) { // 7623
	if (!(type->flags & ALIF_TPFLAGS_HAVE_GC) and
		(base->flags & ALIF_TPFLAGS_HAVE_GC) and
		(!type->traverse /*and !type->clear*/)) {
		type->flags |= ALIF_TPFLAGS_HAVE_GC;
		if (type->traverse == nullptr)
			type->traverse = base->traverse;
		//if (type->clear == nullptr)
		//	type->clear = base->clear;
	}
	type->flags |= (base->flags & ALIF_TPFLAGS_PREHEADER);

	if (type->basicSize == 0)
		type->basicSize = base->basicSize;

	/* Copy other non-function slots */

#define COPYVAL(SLOT) \
    if (type->SLOT == 0) { type->SLOT = base->SLOT; }

	COPYVAL(itemSize);
	COPYVAL(weakListOffset);
	COPYVAL(dictOffset);

#undef COPYVAL

	/* Setup fast subclass flags */
	AlifObject* mro = lookup_tpMro(base);
	if (isSubType_withMro(mro, base, (AlifTypeObject*)_alifExcBaseException_)) {
		type->flags |= ALIF_TPFLAGS_BASE_EXC_SUBCLASS;
	}
	else if (isSubType_withMro(mro, base, &_alifTypeType_)) {
		type->flags |= ALIF_TPFLAGS_TYPE_SUBCLASS;
	}
	else if (isSubType_withMro(mro, base, &_alifLongType_)) {
		type->flags |= ALIF_TPFLAGS_LONG_SUBCLASS;
	}
	else if (isSubType_withMro(mro, base, &_alifBytesType_)) {
		type->flags |= ALIF_TPFLAGS_BYTES_SUBCLASS;
	}
	else if (isSubType_withMro(mro, base, &_alifUStrType_)) {
		type->flags |= ALIF_TPFLAGS_UNICODE_SUBCLASS;
	}
	else if (isSubType_withMro(mro, base, &_alifTupleType_)) {
		type->flags |= ALIF_TPFLAGS_TUPLE_SUBCLASS;
	}
	else if (isSubType_withMro(mro, base, &_alifListType_)) {
		type->flags |= ALIF_TPFLAGS_LIST_SUBCLASS;
	}
	else if (isSubType_withMro(mro, base, &_alifDictType_)) {
		type->flags |= ALIF_TPFLAGS_DICT_SUBCLASS;
	}

	/* Setup some inheritable flags */
	if (alifType_hasFeature(base, _ALIF_TPFLAGS_MATCH_SELF)) {
		type->flags |= _ALIF_TPFLAGS_MATCH_SELF;
	}
	if (alifType_hasFeature(base, ALIF_TPFLAGS_ITEMS_AT_END)) {
		type->flags |= ALIF_TPFLAGS_ITEMS_AT_END;
	}
}


static AlifIntT overrides_hash(AlifTypeObject* _type) { // 7688
	AlifObject* dict = lookup_tpDict(_type);

	AlifIntT r = alifDict_contains(dict, &ALIF_ID(__eq__));
	if (r == 0) {
		r = alifDict_contains(dict, &ALIF_ID(__hash__));
	}
	return r;
}



static AlifIntT inherit_slots(AlifTypeObject* _type, AlifTypeObject* _base) { // 7701
	AlifTypeObject* baseBase{};

#undef SLOTDEFINED
#undef COPYSLOT
#undef COPYNUM
#undef COPYSEQ
#undef COPYMAP
#undef COPYBUF

#define SLOTDEFINED(_slot) \
    (_base->_slot != 0 and \
     (baseBase == nullptr or _base->_slot != baseBase->_slot))

#define COPYSLOT(_slot) \
    if (!_type->_slot and SLOTDEFINED(_slot)) _type->_slot = _base->_slot

#define COPYASYNC(_slot) COPYSLOT(asSsync->_slot)
#define COPYNUM(_slot) COPYSLOT(asNumber->_slot)
#define COPYSEQ(_slot) COPYSLOT(asSequence->_slot)
#define COPYMAP(_slot) COPYSLOT(asMapping->_slot)
#define COPYBUF(_slot) COPYSLOT(asBuffer->_slot)

	if (_type->asNumber != nullptr and _base->asNumber != nullptr) {
		baseBase = _base->base;
		if (baseBase->asNumber == nullptr)
			baseBase = nullptr;
		COPYNUM(add_);
		COPYNUM(subtract);
		COPYNUM(multiply);
		COPYNUM(remainder);
		COPYNUM(divmod);
		COPYNUM(power);
		COPYNUM(negative);
		COPYNUM(positive);
		COPYNUM(absolute);
		COPYNUM(bool_);
		COPYNUM(invert);
		COPYNUM(lshift);
		COPYNUM(rshift);
		COPYNUM(and_);
		COPYNUM(xor_);
		COPYNUM(or_);
		COPYNUM(int_);
		COPYNUM(float_);
		COPYNUM(inplaceAdd);
		COPYNUM(inplaceSubtract);
		COPYNUM(inplaceMultiply);
		COPYNUM(inplaceRemainder);
		COPYNUM(inplacePower);
		COPYNUM(inplaceLshift);
		COPYNUM(inplaceRshift);
		COPYNUM(inplaceAnd);
		COPYNUM(inplaceXor);
		COPYNUM(inplaceOr);
		COPYNUM(trueDivide);
		COPYNUM(floorDivide);
		COPYNUM(inplaceTrueDivide);
		COPYNUM(inplaceFloorDivide);
		COPYNUM(index);
		COPYNUM(matrixMultiply);
		//COPYNUM(inplaceMatrixMultiply);
	}

	//if (_type->asAsync != nullptr and _base->asAsync != nullptr) {
	//	baseBase = _base->base;
	//	if (baseBase->asAsync == nullptr)
	//		baseBase = nullptr;
	//	COPYASYNC(am_await);
	//	COPYASYNC(am_aiter);
	//	COPYASYNC(am_anext);
	//}

	if (_type->asSequence != nullptr and _base->asSequence != nullptr) {
		baseBase = _base->base;
		if (baseBase->asSequence == nullptr)
			baseBase = nullptr;
		COPYSEQ(length);
		COPYSEQ(concat);
		COPYSEQ(repeat);
		COPYSEQ(item);
		COPYSEQ(assItem);
		COPYSEQ(contains);
		COPYSEQ(inplaceConcat);
		COPYSEQ(inplaceRepeat);
	}

	if (_type->asMapping != nullptr and _base->asMapping != nullptr) {
		baseBase = _base->base;
		if (baseBase->asMapping == nullptr)
			baseBase = nullptr;
		COPYMAP(length);
		COPYMAP(subscript);
		COPYMAP(assSubscript);
	}

	if (_type->asBuffer != nullptr and _base->asBuffer != nullptr) {
		baseBase = _base->base;
		if (baseBase->asBuffer == nullptr)
			baseBase = nullptr;
		COPYBUF(getBuffer);
		COPYBUF(releaseBuffer);
	}

	baseBase = _base->base;

	COPYSLOT(dealloc);
	if (_type->getAttr == nullptr and _type->getAttro == nullptr) {
		_type->getAttr = _base->getAttr;
		_type->getAttro = _base->getAttro;
	}
	if (_type->setAttr == nullptr and _type->setAttro == nullptr) {
		_type->setAttr = _base->setAttr;
		_type->setAttro = _base->setAttro;
	}
	COPYSLOT(repr);
	{
		COPYSLOT(vectorCallOffset);

		if (!_type->call and
			_alifType_hasFeature(_base, ALIF_TPFLAGS_HAVE_VECTORCALL))
		{
			_type->flags |= ALIF_TPFLAGS_HAVE_VECTORCALL;
		}
		COPYSLOT(call);
	}
	COPYSLOT(str);
	{
		/* Copy comparison-related slots only when
		   not overriding them anywhere */
		if (_type->richCompare == nullptr and
			_type->hash == nullptr)
		{
			AlifIntT r = overrides_hash(_type);
			if (r < 0) {
				return -1;
			}
			if (!r) {
				_type->richCompare = _base->richCompare;
				_type->hash = _base->hash;
			}
		}
	}
	{
		COPYSLOT(iter);
		COPYSLOT(iterNext);
	}
	{
		COPYSLOT(descrGet);
		if (_base->descrGet and
			_type->descrGet == _base->descrGet and
			_alifType_hasFeature(_type, ALIF_TPFLAGS_IMMUTABLETYPE) and
			_alifType_hasFeature(_base, ALIF_TPFLAGS_METHOD_DESCRIPTOR))
		{
			_type->flags |= ALIF_TPFLAGS_METHOD_DESCRIPTOR;
		}
		COPYSLOT(descrSet);
		COPYSLOT(dictOffset);
		COPYSLOT(init);
		COPYSLOT(alloc);
		COPYSLOT(isGC);
		//COPYSLOT(finalize);
		if ((_type->flags & ALIF_TPFLAGS_HAVE_GC) ==
			(_base->flags & ALIF_TPFLAGS_HAVE_GC)) {
			/* They agree about gc. */
			COPYSLOT(free);
		}
		else if ((_type->flags & ALIF_TPFLAGS_HAVE_GC) and
			_type->free == nullptr and
			_base->free == alifMem_objFree) {
			_type->free = alifObject_gcDel;
		}
	}
	return 0;
}





static AlifIntT add_operators(AlifTypeObject*); // 7897
static AlifIntT add_tpNewWrapper(AlifTypeObject*); // 7898


#define COLLECTION_FLAGS (ALIF_TPFLAGS_SEQUENCE | ALIF_TPFLAGS_MAPPING) // 7900

static AlifIntT typeReady_preChecks(AlifTypeObject* _type) { // 7902
	if (_type->name == nullptr) {
		//alifErr_format(_alifExcSystemError_,
			//"Type does not define the tp_name field.");
		return -1;
	}
	return 0;
}

static AlifIntT typeReady_setBase(AlifTypeObject* _type) { // 7933

	AlifTypeObject* base = _type->base;
	if (base == nullptr and _type != &_alifBaseObjectType_) {
		base = &_alifBaseObjectType_;
		if (_type->flags & ALIF_TPFLAGS_HEAPTYPE) {
			_type->base = (AlifTypeObject*)ALIF_NEWREF((AlifObject*)base);
		}
		else {
			_type->base = base;
		}
	}

	if (base != nullptr and !alifType_isReady(base)) {
		if (alifType_ready(base) < 0) {
			return -1;
		}
	}

	return 0;
}

static AlifIntT typeReady_setType(AlifTypeObject* _type) { // 7961
	AlifTypeObject* base = _type->base;
	if (ALIF_IS_TYPE(_type, nullptr) and base != nullptr) {
		ALIF_SET_TYPE(_type, ALIF_TYPE(base));
	}

	return 0;
}

static AlifIntT typeReady_setBases(AlifTypeObject* _type, AlifIntT _initial) { // 7980
	if (_type->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		if (!_initial) {
			return 0;
		}
	}

	AlifObject* bases = lookup_tpBases(_type);
	if (bases == nullptr) {
		AlifTypeObject* base = _type->base;
		if (base == nullptr) {
			bases = alifTuple_new(0);
		}
		else {
			bases = alifTuple_pack(1, base);
		}
		if (bases == nullptr) {
			return -1;
		}
		set_tpBases(_type, bases, 1);
	}
	return 0;
}

static AlifIntT typeReady_setDict(AlifTypeObject* _type) { // 8009
	if (lookup_tpDict(_type) != nullptr) {
		return 0;
	}

	AlifObject* dict = alifDict_new();
	if (dict == nullptr) {
		return -1;
	}
	set_tpDict(_type, dict);
	return 0;
}



static AlifIntT typeReady_fillDict(AlifTypeObject* type) { // 8061
	if (add_operators(type) < 0) {
		return -1;
	}
	if (type_addMethods(type) < 0) {
		return -1;
	}
	//if (type_addMembers(type) < 0) {
	//	return -1;
	//}
	//if (type_addGetSet(type) < 0) {
	//	return -1;
	//}
	//if (type_dictSetDoc(type) < 0) {
	//	return -1;
	//}
	return 0;
}



static AlifIntT typeReady_mro(AlifTypeObject* _type, AlifIntT _initial) { // 8112

	if (_type->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		if (!_initial) {
			return 0;
		}
	}

	if (mro_internalUnlocked(_type, _initial, nullptr) < 0) {
		return -1;
	}
	AlifObject* mro = lookup_tpMro(_type);

	if (!(_type->flags & ALIF_TPFLAGS_HEAPTYPE)) {
		AlifSizeT n = ALIFTUPLE_GET_SIZE(mro);
		for (AlifSizeT i = 0; i < n; i++) {
			AlifTypeObject* base = ALIFTYPE_CAST(ALIFTUPLE_GET_ITEM(mro, i));
			if (base->flags & ALIF_TPFLAGS_HEAPTYPE) {
				//alifErr_format(_alifExcTypeError_,
					//"type '%.100s' is not dynamically allocated but "
					//"its base type '%.100s' is dynamically allocated",
					//_type->name, base->name);
				return -1;
			}
		}
	}
	return 0;
}



static void typeReady_inheritAsStructs(AlifTypeObject* _type, AlifTypeObject* _base) { // 8159
	//if (_type->asAsync == nullptr) {
	//	_type->asAsync = _base->asAsync;
	//}
	if (_type->asNumber == nullptr) {
		_type->asNumber = _base->asNumber;
	}
	if (_type->asSequence == nullptr) {
		_type->asSequence = _base->asSequence;
	}
	if (_type->asMapping == nullptr) {
		_type->asMapping = _base->asMapping;
	}
	if (_type->asBuffer == nullptr) {
		_type->asBuffer = _base->asBuffer;
	}
}


static void inherit_patmaFlags(AlifTypeObject* _type, AlifTypeObject* _base) { // 8179
	if ((_type->flags & COLLECTION_FLAGS) == 0) {
		_type->flags |= _base->flags & COLLECTION_FLAGS;
	}
}


static AlifIntT typeReady_inherit(AlifTypeObject* _type) { // 8186
	AlifTypeObject* base = _type->base;
	if (base != nullptr) {
		inherit_special(_type, base);
	}

	// Inherit slots
	AlifObject* mro = lookup_tpMro(_type);
	AlifSizeT n = ALIFTUPLE_GET_SIZE(mro);
	for (AlifSizeT i = 1; i < n; i++) {
		AlifObject* b = ALIFTUPLE_GET_ITEM(mro, i);
		if (ALIFTYPE_CHECK(b)) {
			if (inherit_slots(_type, (AlifTypeObject*)b) < 0) {
				return -1;
			}
			inherit_patmaFlags(_type, (AlifTypeObject*)b);
		}
	}

	if (base != nullptr) {
		typeReady_inheritAsStructs(_type, base);
	}

	/* Sanity check for free. */
	if (ALIFTYPE_IS_GC(_type) and (_type->flags & ALIF_TPFLAGS_BASETYPE) and
		(_type->free == nullptr or _type->free == alifMem_objFree)) {
		//alifErr_format(_alifExcTypeError_, "type '%.100s' participates in "
		//	"gc and is a base type but has inappropriate "
		//	"free slot",
		//	_type->name);
		return -1;
	}

	return 0;
}




static AlifIntT typeReady_setHash(AlifTypeObject* type) { // 8237
	if (type->hash != nullptr) {
		return 0;
	}

	AlifObject* dict = lookup_tpDict(type);
	AlifIntT r = alifDict_contains(dict, &ALIF_ID(__hash__));
	if (r < 0) {
		return -1;
	}
	if (r > 0) {
		return 0;
	}

	if (alifDict_setItem(dict, &ALIF_ID(__hash__), ALIF_NONE) < 0) {
		return -1;
	}
	type->hash = alifObject_hashNotImplemented;
	return 0;
}


static AlifIntT typeReady_setNew(AlifTypeObject* _type, AlifIntT _initial) { // 8279
	AlifTypeObject* base = _type->base;
	if (_type->new_ == nullptr
		and base == &_alifBaseObjectType_
		and !(_type->flags & ALIF_TPFLAGS_HEAPTYPE))
	{
		_type->flags |= ALIF_TPFLAGS_DISALLOW_INSTANTIATION;
	}

	if (!(_type->flags & ALIF_TPFLAGS_DISALLOW_INSTANTIATION)) {
		if (_type->new_ != nullptr) {
			if (_initial or base == nullptr or _type->new_ != base->new_) {
				if (add_tpNewWrapper(_type) < 0) {
					return -1;
				}
			}
		}
		else {
			_type->new_ = base->new_;
		}
	}
	else {
		_type->new_ = nullptr;
	}
	return 0;
}

static AlifIntT typeReady_managedDict(AlifTypeObject* _type) { // 8322
	if (!(_type->flags & ALIF_TPFLAGS_MANAGED_DICT)) {
		return 0;
	}
	if (!(_type->flags & ALIF_TPFLAGS_HEAPTYPE)) {
		//alifErr_format(_alifExcSystemError_,
		//	"type %s has the ALIF_TPFLAGS_MANAGED_DICT flag "
		//	"but not ALIF_TPFLAGS_HEAPTYPE flag",
		//	type->name);
		return -1;
	}
	AlifHeapTypeObject* et = (AlifHeapTypeObject*)_type;
	if (et->cachedKeys == nullptr) {
		et->cachedKeys = _alifDict_newKeysForClass(et);
		if (et->cachedKeys == nullptr) {
			//alifErr_noMemory();
			return -1;
		}
	}
	if (_type->itemSize == 0) {
		_type->flags |= ALIF_TPFLAGS_INLINE_VALUES;
	}
	return 0;
}

static AlifIntT typeReady_postChecks(AlifTypeObject* type) { // 8349
	if (type->flags & ALIF_TPFLAGS_HAVE_GC
		and type->traverse == nullptr) {
		//alifErr_format(_alifExcSystemError_,
		//	"type %s has the ALIF_TPFLAGS_HAVE_GC flag "
		//	"but has no traverse function", type->name);
		return -1;
	}
	if (type->flags & ALIF_TPFLAGS_MANAGED_DICT) {
		if (type->dictOffset != -1) {
			//alifErr_format(_alifExcSystemError_,
			//	"type %s has the ALIF_TPFLAGS_MANAGED_DICT flag "
			//	"but tp_dictoffset is set to incompatible value",
			//	type->name);
			return -1;
		}
	}
	else if (type->dictOffset < (AlifSizeT)sizeof(AlifObject)) {
		if (type->dictOffset + type->basicSize <= 0) {
			//alifErr_format(_alifExcSystemError_,
			//	"type %s has a dictOffset that is too small",
			//	type->name);
		}
	}
	return 0;
}

static AlifIntT type_ready(AlifTypeObject* _type, AlifIntT _initial) { // 8383

	start_readying(_type);

	if (typeReady_preChecks(_type) < 0) {
		goto error;
	}

	if (typeReady_setDict(_type) < 0) {
		goto error;
	}
	if (typeReady_setBase(_type) < 0) {
		goto error;
	}
	if (typeReady_setType(_type) < 0) {
		goto error;
	}
	if (typeReady_setBases(_type, _initial) < 0) {
		goto error;
	}
	if (typeReady_mro(_type, _initial) < 0) {
		goto error;
	}
	if (typeReady_setNew(_type, _initial) < 0) {
		goto error;
	}
	if (typeReady_fillDict(_type) < 0) {
		goto error;
	}
	if (_initial) {
		if (typeReady_inherit(_type) < 0) {
			goto error;
		}
		//if (typeReady_preheader(_type) < 0) {
		//	goto error;
		//}
	}
	if (typeReady_setHash(_type) < 0) {
		goto error;
	}
	//if (typeReady_addSubclasses(_type) < 0) {
	//	goto error;
	//}
	if (_initial) {
		if (typeReady_managedDict(_type) < 0) {
			goto error;
		}
		//if (typeReady_postChecks(_type) < 0) {
		//	goto error;
		//}
	}

	_type->flags |= ALIF_TPFLAGS_READY;
	stop_readying(_type);

	return 0;

error:
	stop_readying(_type);
	return -1;
}

AlifIntT alifType_ready(AlifTypeObject* _type) { // 8462
	if (_type->flags & ALIF_TPFLAGS_READY) {
		return 0;
	}

	if (!(_type->flags & ALIF_TPFLAGS_HEAPTYPE)) {
		_type->flags |= ALIF_TPFLAGS_IMMUTABLETYPE;
		alif_setImmortalUntracked((AlifObject*)_type);
	}

	AlifIntT res{};
	BEGIN_TYPE_LOCK();
	if (!(_type->flags & ALIF_TPFLAGS_READY)) {
		res = type_ready(_type, 1);
	}
	else {
		res = 0;
	}
	END_TYPE_LOCK();
	return res;
}


static AlifIntT init_staticType(AlifInterpreter* _interp, AlifTypeObject* _self,
	AlifIntT _isBuiltin, AlifIntT _initial) { // 8490

	if ((_self->flags & ALIF_TPFLAGS_READY) == 0) {
		_self->flags |= ALIF_TPFLAGS_STATIC_BUILTIN;
		_self->flags |= ALIF_TPFLAGS_IMMUTABLETYPE;

		alifType_setVersion(_self, NEXT_GLOBAL_VERSION_TAG++);
	}

	managedStatic_typeStateInit(_interp, _self, _isBuiltin, _initial);

	AlifIntT res{};
	BEGIN_TYPE_LOCK();
	res = type_ready(_self, _initial);
	END_TYPE_LOCK();
	if (res < 0) {
		//alifStaticType_clearWeakRefs(_interp, _self);
		//managedStatic_typeStateClear(_interp, _self, _isBuiltin, _initial);
	}

	return res;
}


AlifIntT alifStaticType_initBuiltin(AlifInterpreter* _interp,
	AlifTypeObject* _self) { // 8539
	return init_staticType(_interp, _self, 1, alif_isMainInterpreter(_interp));
}


static AlifObject* wrap_init(AlifObject* _self, AlifObject* _args,
	void* _wrapped, AlifObject* _kwds) { // 9220
	InitProc func = (InitProc)_wrapped;

	if (func(_self, _args, _kwds) < 0)
		return nullptr;
	return ALIF_NONE;
}


static AlifObject* tpNew_wrapper(AlifObject* _self,
	AlifObject* _args, AlifObject* _kwds) { // 9230
	AlifTypeObject* staticbase{};
	AlifObject* arg0{}, * res{};

	if (_self == nullptr or !ALIFTYPE_CHECK(_self)) {
		//alifErr_format(_alifExcSystemError_,
		//	"__new__() called with non-type 'self'");
		return nullptr;
	}
	AlifTypeObject* type = (AlifTypeObject*)_self;

	if (!ALIFTUPLE_CHECK(_args) or ALIFTUPLE_GET_SIZE(_args) < 1) {
		//alifErr_format(_alifExcTypeError_,
		//	"%s.__new__(): not enough arguments",
		//	type->name);
		return nullptr;
	}
	arg0 = ALIFTUPLE_GET_ITEM(_args, 0);
	if (!ALIFTYPE_CHECK(arg0)) {
		//alifErr_format(_alifExcTypeError_,
		//	"%s.__new__(X): X is not a type object (%s)",
		//	type->name,
		//	ALIF_TYPE(arg0)->name);
		return nullptr;
	}
	AlifTypeObject* subtype = (AlifTypeObject*)arg0;

	if (!alifType_isSubType(subtype, type)) {
		//alifErr_format(_alifExcTypeError_,
		//	"%s.__new__(%s): %s is not a subtype of %s",
		//	type->name,
		//	subtype->name,
		//	subtype->name,
		//	type->name);
		return nullptr;
	}

	staticbase = subtype;
	while (staticbase and (staticbase->new_ == slot_tpNew))
		staticbase = staticbase->base;
	if (staticbase and staticbase->new_ != type->new_) {
		//alifErr_format(_alifExcTypeError_,
		//	"%s.__new__(%s) is not safe, use %s.__new__()",
		//	type->name, subtype->name, staticbase->name);
		return nullptr;
	}

	_args = alifTuple_getSlice(_args, 1, ALIFTUPLE_GET_SIZE(_args));
	if (_args == nullptr)
		return nullptr;
	res = type->new_(subtype, _args, _kwds);
	ALIF_DECREF(_args);
	return res;
}



static AlifMethodDef _tpNewMethodDef_[] = { // 9294
	{"__new__", ALIF_CPPFUNCTION_CAST(tpNew_wrapper), METHOD_VARARGS | METHOD_KEYWORDS
	 /*,ALIFDOC_STR("__new__($type, *args, **kwargs)\n--\n\n"
			   "Create and return a new object.  "
			   "See help(type) for accurate signature.")*/},
	{0}
};



static AlifIntT add_tpNewWrapper(AlifTypeObject* _type) { // 9302
	AlifObject* dict = lookup_tpDict(_type);
	AlifIntT r_ = alifDict_contains(dict, &ALIF_ID(__new__));
	if (r_ > 0) {
		return 0;
	}
	if (r_ < 0) {
		return -1;
	}

	AlifObject* func = ALIFCPPFUNCTION_NEWEX(_tpNewMethodDef_, (AlifObject*)_type, nullptr);
	if (func == nullptr) {
		return -1;
	}
	r_ = alifDict_setItem(dict, &ALIF_ID(__new__), func);
	ALIF_DECREF(func);
	return r_;
}





static AlifObject* slot_tpCall(AlifObject* self, AlifObject* args, AlifObject* kwds) { // 9740
	AlifThread* thread = _alifThread_get();
	int unbound;

	AlifObject* meth = lookup_method(self, &ALIF_ID(__call__), &unbound);
	if (meth == NULL) {
		return NULL;
	}

	AlifObject* res{};
	if (unbound) {
		res = _alifObject_callPrepend(thread, meth, self, args, kwds);
	}
	else {
		res = _alifObject_call(thread, meth, args, kwds);
	}

	ALIF_DECREF(meth);
	return res;
}



static AlifIntT slot_tpInit(AlifObject* _self,
	AlifObject* _args, AlifObject* _kwds) { // 9986
	AlifThread* tstate = _alifThread_get();

	AlifIntT unbound{};
	AlifObject* meth = lookup_method(_self, &ALIF_STR(__init__), &unbound);
	if (meth == nullptr) {
		return -1;
	}

	AlifObject* res{};
	if (unbound) {
		res = _alifObject_callPrepend(tstate, meth, _self, _args, _kwds);
	}
	else {
		res = _alifObject_call(tstate, meth, _args, _kwds);
	}
	ALIF_DECREF(meth);
	if (res == nullptr)
		return -1;
	if (res != ALIF_NONE) {
		//alifErr_format(_alifExcTypeError_,
		//	"__init__() should return None, not '%.200s'",
		//	ALIF_TYPE(res)->name);
		ALIF_DECREF(res);
		return -1;
	}
	ALIF_DECREF(res);
	return 0;
}


static AlifObject* slot_tpNew(AlifTypeObject* _type,
	AlifObject* _args, AlifObject* _kwds) { // 10018
	AlifThread* thread = _alifThread_get();
	AlifObject* func{}, * result{};

	func = alifObject_getAttr((AlifObject*)_type, &ALIF_ID(__new__));
	if (func == nullptr) {
		return nullptr;
	}

	result = _alifObject_callPrepend(thread, func, (AlifObject*)_type, _args, _kwds);
	ALIF_DECREF(func);
	return result;
}

 // 10381
#define FLSLOT(_name, _slot, _function, _wrapper, _doc, _flags) \
    {.name = #_name, .offset = offsetof(AlifTypeObject, _slot), .function = (void *)(_function), .wrapper = _wrapper, \
     .doc = nullptr, .flags = _flags, .nameStrObj = &ALIF_STR(_name) }

static AlifTypeSlotDef _slotDefs_[] = { // 10416
	FLSLOT(__init__, init, slot_tpInit, (WrapperFunc)(void(*)(void))wrap_init,
		   nullptr, ALIFWRAPPERFLAG_KEYWORDS),


	{nullptr}
};



static void** slot_ptr(AlifTypeObject* type, AlifIntT ioffset) { // 10618
	char* ptr{};
	long offset = ioffset;

	if ((AlifUSizeT)offset >= offsetof(AlifHeapTypeObject, buffer)) {
		ptr = (char*)type->asBuffer;
		offset -= offsetof(AlifHeapTypeObject, buffer);
	}
	else if ((AlifUSizeT)offset >= offsetof(AlifHeapTypeObject, sequence)) {
		ptr = (char*)type->asSequence;
		offset -= offsetof(AlifHeapTypeObject, sequence);
	}
	else if ((AlifUSizeT)offset >= offsetof(AlifHeapTypeObject, mapping)) {
		ptr = (char*)type->asMapping;
		offset -= offsetof(AlifHeapTypeObject, mapping);
	}
	else if ((AlifUSizeT)offset >= offsetof(AlifHeapTypeObject, number)) {
		ptr = (char*)type->asNumber;
		offset -= offsetof(AlifHeapTypeObject, number);
	}
	//else if ((AlifUSizeT)offset >= offsetof(AlifHeapTypeObject, async)) {
	//	ptr = (char*)type->asAsync;
	//	offset -= offsetof(AlifHeapTypeObject, async);
	//}
	else {
		ptr = (char*)type;
	}
	if (ptr != nullptr)
		ptr += offset;
	return (void**)ptr;
}

static void** resolve_slotDups(AlifTypeObject* type, AlifObject* name) { // 10657
	AlifInterpreter* interp = _alifInterpreter_get();
#define pname ALIF_INTERP_CACHED_OBJECT(interp, typeSlotsPname)
#define ptrs ALIF_INTERP_CACHED_OBJECT(interp, typeSlotsPtrs)
	AlifTypeSlotDef* p{}, ** pp{};
	void** res{}, ** ptr{};

	if (pname != name) {
		/* Collect all slotdefs that match name into ptrs. */
		pname = name;
		pp = ptrs;
		for (p = _slotDefs_; p->nameStrObj; p++) {
			if (p->nameStrObj == name)
				*pp++ = p;
		}
		*pp = nullptr;
	}

	res = nullptr;
	for (pp = ptrs; *pp; pp++) {
		ptr = slot_ptr(type, (*pp)->offset);
		if (ptr == nullptr or *ptr == nullptr)
			continue;
		if (res != nullptr)
			return nullptr;
		res = ptr;
	}
	return res;
#undef pname
#undef ptrs
}

static AlifTypeSlotDef* update_oneSlot(AlifTypeObject* type, AlifTypeSlotDef* p) { // 10752
	AlifObject* descr{};
	AlifWrapperDescrObject* d{};

	void* specific = nullptr;

	void* generic = nullptr;

	int use_generic = 0;

	int offset = p->offset;
	int error;
	void** ptr = slot_ptr(type, offset);

	if (ptr == nullptr) {
		do {
			++p;
		} while (p->offset == offset);
		return p;
	}
	do {
		descr = findName_inMro(type, p->nameStrObj, &error);
		if (descr == nullptr) {
			if (error == -1) {
				//alifErr_clear();
			}
			if (ptr == (void**)&type->iterNext) {
				specific = (void*)_alifObject_nextNotImplemented;
			}
			continue;
		}
		if (ALIF_IS_TYPE(descr, &_alifWrapperDescrType_) and
			((AlifWrapperDescrObject*)descr)->base->nameStrObj == p->nameStrObj) {
			void** tptr = resolve_slotDups(type, p->nameStrObj);
			if (tptr == nullptr or tptr == ptr)
				generic = p->function;
			d = (AlifWrapperDescrObject*)descr;
			if ((specific == nullptr or specific == d->wrapped) and
				d->base->wrapper == p->wrapper and
				isSubType_withMro(lookup_tpMro(type), type, ALIFDESCR_TYPE(d)))
			{
				specific = d->wrapped;
			}
			else {
				use_generic = 1;
			}
		}
		else if (ALIF_IS_TYPE(descr, &_alifCPPFunctionType_) and
			ALIFCPPFUNCTION_GET_FUNCTION(descr) ==
			ALIF_CPPFUNCTION_CAST(tpNew_wrapper) and
			ptr == (void**)&type->new_)
		{
			specific = (void*)type->new_;
		}
		else if (descr == ALIF_NONE and
			ptr == (void**)&type->hash) {
			specific = (void*)alifObject_hashNotImplemented;
		}
		else {
			use_generic = 1;
			generic = p->function;
			if (p->function == slot_tpCall) {
				type->flags &= ~ALIF_TPFLAGS_HAVE_VECTORCALL;
			}
		}
		ALIF_DECREF(descr);
	} while ((++p)->offset == offset);
	if (specific && !use_generic)
		*ptr = specific;
	else
		*ptr = generic;
	return p;
}


static void fixup_slotDispatchers(AlifTypeObject* _type) { // 10920
	BEGIN_TYPE_LOCK();

	for (AlifTypeSlotDef* p = _slotDefs_; p->name; ) {
		p = update_oneSlot(_type, p);
	}

	END_TYPE_LOCK();
}


static AlifIntT typeNew_setNames(AlifTypeObject* _type) { // 10973
	AlifObject* dict = lookup_tpDict(_type);
	AlifObject* names_to_set = alifDict_copy(dict);
	if (names_to_set == nullptr) {
		return -1;
	}

	AlifSizeT i = 0;
	AlifObject* key{}, * value{};
	while (alifDict_next(names_to_set, &i, &key, &value)) {
		AlifObject* set_name = alifObject_lookupSpecial(value,
			&ALIF_ID(__setName__));
		if (set_name == nullptr) {
			//if (alifErr_occurred()) {
			//	goto error;
			//}
			continue;
		}

		AlifObject* res = alifObject_callFunctionObjArgs(set_name, _type, key, nullptr);
		ALIF_DECREF(set_name);

		if (res == nullptr) {
			//_alifErr_formatNote(
			//	"Error calling __set_name__ on '%.100s' instance %R "
			//	"in '%.100s'",
			//	ALIF_TYPE(value)->name, key, type->name);
			goto error;
		}
		else {
			ALIF_DECREF(res);
		}
	}

	ALIF_DECREF(names_to_set);
	return 0;

error:
	ALIF_DECREF(names_to_set);
	return -1;
}



static AlifIntT typeNew_initSubclass(AlifTypeObject* _type, AlifObject* _kwds) { // 11019
	AlifObject* args[2] = { (AlifObject*)_type, (AlifObject*)_type };
	AlifObject* super = alifObject_vectorCall((AlifObject*)&_alifSuperType_,
		args, 2, nullptr);
	if (super == nullptr) {
		return -1;
	}

	AlifObject* func = alifObject_getAttr(super, &ALIF_ID(__initSubclass__));
	ALIF_DECREF(super);
	if (func == nullptr) {
		return -1;
	}

	AlifObject* result = alifObject_vectorCallDict(func, nullptr, 0, _kwds);
	ALIF_DECREF(func);
	if (result == nullptr) {
		return -1;
	}

	ALIF_DECREF(result);
	return 0;
}



static AlifIntT slot_inherited(AlifTypeObject* _type,
	AlifTypeSlotDef* _slotdef, void** _slot) { // 11171
	void** slotBase = slot_ptr(_type->base, _slotdef->offset);
	if (slotBase == nullptr or *_slot != *slotBase) {
		return 0;
	}

	/* Some slots are inherited in pairs. */
	if (_slot == (void*)&_type->hash) {
		return (_type->richCompare == _type->base->richCompare);
	}
	else if (_slot == (void*)&_type->richCompare) {
		return (_type->hash == _type->base->hash);
	}

	return 1;
}


static AlifIntT add_operators(AlifTypeObject* _type) { // 11134
	AlifObject* dict = lookup_tpDict(_type);
	AlifTypeSlotDef* p{};
	AlifObject* descr{};
	void** ptr{};


	for (p = _slotDefs_; p->name; p++) {
		if (p->wrapper == nullptr)
			continue;
		ptr = slot_ptr(_type, p->offset);
		if (!ptr or !*ptr)
			continue;
		if (_type->flags & ALIF_TPFLAGS_STATIC_BUILTIN
			and _type->base != nullptr
			and slot_inherited(_type, p, ptr)) {
			continue;
		}
		AlifIntT r = alifDict_contains(dict, p->nameStrObj);
		if (r > 0)
			continue;
		if (r < 0) {
			return -1;
		}
		if (*ptr == (void*)alifObject_hashNotImplemented) {
			if (alifDict_setItem(dict, p->nameStrObj, ALIF_NONE) < 0)
				return -1;
		}
		else {
			descr = alifDescr_newWrapper(_type, p, *ptr);
			if (descr == nullptr)
				return -1;
			if (alifDict_setItem(dict, p->nameStrObj, descr) < 0) {
				ALIF_DECREF(descr);
				return -1;
			}
			ALIF_DECREF(descr);
		}
	}
	return 0;
}


class SuperObject { // 11183
public:
	ALIFOBJECT_HEAD{};
	AlifTypeObject* type{};
	AlifObject* obj{};
	AlifTypeObject* objType{};
};



static void super_dealloc(AlifObject* self) { // 11200
	SuperObject* su = (SuperObject*)self;

	ALIFOBJECT_GC_UNTRACK(self);
	ALIF_XDECREF(su->obj);
	ALIF_XDECREF(su->type);
	ALIF_XDECREF(su->objType);
	ALIF_TYPE(self)->free(self);
}

static AlifObject* _super_lookupDescr(AlifTypeObject* _suType,
	AlifTypeObject* _suObjType, AlifObject* _name) { // 11232
	AlifObject* mro{}, * res{};
	AlifSizeT i{}, n{};

	BEGIN_TYPE_LOCK();
	mro = lookup_tpMro(_suObjType);
	ALIF_XINCREF(mro);
	END_TYPE_LOCK();

	if (mro == nullptr)
		return nullptr;

	n = ALIFTUPLE_GET_SIZE(mro);

	for (i = 0; i + 1 < n; i++) {
		if ((AlifObject*)(_suType) == ALIFTUPLE_GET_ITEM(mro, i))
			break;
	}
	i++;  /* skip su->type (if any)  */
	if (i >= n) {
		ALIF_DECREF(mro);
		return nullptr;
	}

	do {
		AlifObject* obj = ALIFTUPLE_GET_ITEM(mro, i);
		AlifObject* dict = lookup_tpDict(ALIFTYPE_CAST(obj));

		if (alifDict_getItemRef(dict, _name, &res) != 0) {
			ALIF_DECREF(mro);
			return res;
		}

		i++;
	} while (i < n);
	ALIF_DECREF(mro);
	return nullptr;
}


static AlifObject* do_superLookup(SuperObject* _su, AlifTypeObject* _suType,
	AlifObject* _suObj, AlifTypeObject* _suObjType,
	AlifObject* _name, AlifIntT* _method) { // 11283
	AlifObject* res{};
	AlifIntT temp_su = 0;

	if (_suObjType == nullptr) {
		goto skip;
	}

	res = _super_lookupDescr(_suType, _suObjType, _name);
	if (res != nullptr) {
		if (_method and _alifType_hasFeature(ALIF_TYPE(res), ALIF_TPFLAGS_METHOD_DESCRIPTOR)) {
			*_method = 1;
		}
		else {
			DescrGetFunc f = ALIF_TYPE(res)->descrGet;
			if (f != nullptr) {
				AlifObject* res2{};
				res2 = f(res,
					(_suObj == (AlifObject*)_suObjType) ? nullptr : _suObj,
					(AlifObject*)_suObjType);
				ALIF_SETREF(res, res2);
			}
		}

		return res;
	}
	//else if (alifErr_occurred()) {
	//	return nullptr;
	//}

skip:
	if (_su == nullptr) {
		AlifObject* args[] = { (AlifObject*)_suType, _suObj };
		_su = (SuperObject*)alifObject_vectorCall((AlifObject*)&_alifSuperType_, args, 2, nullptr);
		if (_su == nullptr) {
			return nullptr;
		}
		temp_su = 1;
	}
	res = alifObject_genericGetAttr((AlifObject*)_su, _name);
	if (temp_su) {
		ALIF_DECREF(_su);
	}
	return res;
}


static AlifObject* super_getAttro(AlifObject* self, AlifObject* name) { // 11334
	SuperObject* su = (SuperObject*)self;

	if (ALIFUSTR_CHECK(name) and
		ALIFUSTR_GET_LENGTH(name) == 9 and
		_alifUStr_equal(name, &ALIF_ID(__class__)))
		return alifObject_genericGetAttr(self, name);

	return do_superLookup(su, su->type, su->obj, su->objType, name, nullptr);
}


static AlifTypeObject* super_check(AlifTypeObject* type, AlifObject* obj) { // 11349
	if (ALIFTYPE_CHECK(obj) and alifType_isSubType((AlifTypeObject*)obj, type)) {
		return (AlifTypeObject*)ALIF_NEWREF(obj);
	}

	/* Normal case */
	if (alifType_isSubType(ALIF_TYPE(obj), type)) {
		return (AlifTypeObject*)ALIF_NEWREF(ALIF_TYPE(obj));
	}
	else {
		/* Try the slow way */
		AlifObject* class_attr{};

		if (alifObject_getOptionalAttr(obj, &ALIF_ID(__class__), &class_attr) < 0) {
			return nullptr;
		}
		if (class_attr != nullptr and
			ALIFTYPE_CHECK(class_attr) and
			(AlifTypeObject*)class_attr != ALIF_TYPE(obj))
		{
			AlifIntT ok = alifType_isSubType(
				(AlifTypeObject*)class_attr, type);
			if (ok) {
				return (AlifTypeObject*)class_attr;
			}
		}
		ALIF_XDECREF(class_attr);
	}

	const char* type_or_instance{}, * obj_str{};

	if (ALIFTYPE_CHECK(obj)) {
		type_or_instance = "نوع"; //* alif
		obj_str = ((AlifTypeObject*)obj)->name;
	}
	else {
		type_or_instance = "نسخة_من"; //* alif
		obj_str = ALIF_TYPE(obj)->name;
	}

	//alifErr_format(_alifExcTypeError_,
	//	"super(type, obj): obj (%s %.200s) is not "
	//	"an instance or subtype of type (%.200s).",
	//	type_or_instance, obj_str, type->name);

	return nullptr;
}


static AlifIntT super_initWithoutArgs(AlifInterpreterFrame* cframe, AlifCodeObject* co,
	AlifTypeObject** type_p, AlifObject** obj_p) { // 11460
	if (co->argCount == 0) {
		//alifErr_setString(_alifExcRuntimeError_,
		//	"super(): no arguments");
		return -1;
	}

	AlifObject* firstarg = alifStackRef_asAlifObjectBorrow(_alifFrame_getLocalsArray(cframe)[0]);
	// The first argument might be a cell.
	if (firstarg != nullptr and (_alifLocals_getKind(co->localsPlusKinds, 0) & CO_FAST_CELL)) {
		if (ALIFINTERPRETERFRAME_LASTI(cframe) >= 0) {
			firstarg = ALIFCELL_GET(firstarg);
		}
	}
	if (firstarg == nullptr) {
		//alifErr_setString(_alifExcRuntimeError_,
		//	"super(): arg[0] deleted");
		return -1;
	}

	// Look for __class__ in the free vars.
	AlifTypeObject* type = nullptr;
	AlifIntT i = alifUnstableCode_getFirstFree(co);
	for (; i < co->nLocalsPlus; i++) {
		AlifObject* name = ALIFTUPLE_GET_ITEM(co->localsPlusNames, i);
		if (_alifUStr_equal(name, &ALIF_ID(__class__))) {
			AlifObject* cell = alifStackRef_asAlifObjectBorrow(_alifFrame_getLocalsArray(cframe)[i]);
			if (cell == nullptr or !ALIFCELL_CHECK(cell)) {
				//alifErr_setString(_alifExcRuntimeError_,
				//	"super(): bad __class__ cell");
				return -1;
			}
			type = (AlifTypeObject*)ALIFCELL_GET(cell);
			if (type == nullptr) {
				//alifErr_setString(_alifExcRuntimeError_,
				//	"super(): empty __class__ cell");
				return -1;
			}
			if (!ALIFTYPE_CHECK(type)) {
				//alifErr_format(_alifExcRuntimeError_,
				//	"super(): __class__ is not a type (%s)",
				//	ALIF_TYPE(type)->name);
				return -1;
			}
			break;
		}
	}
	if (type == nullptr) {
		//alifErr_setString(_alifExcRuntimeError_,
		//	"super(): __class__ cell not found");
		return -1;
	}

	*type_p = type;
	*obj_p = firstarg;
	return 0;
}


static inline AlifIntT super_initImpl(AlifObject* self,
	AlifTypeObject* type, AlifObject* obj) { // 11549
	SuperObject* su = (SuperObject*)self;
	AlifTypeObject* obj_type = nullptr;
	if (type == nullptr) {
		AlifThread* tstate = _alifThread_get();
		AlifInterpreterFrame* frame = _alifThreadState_getFrame(tstate);
		if (frame == nullptr) {
			//alifErr_setString(_alifExcRuntimeError_,
			//	"super(): no current frame");
			return -1;
		}
		AlifIntT res = super_initWithoutArgs(frame, _alifFrame_getCode(frame), &type, &obj);

		if (res < 0) {
			return -1;
		}
	}

	if (obj == ALIF_NONE)
		obj = nullptr;
	if (obj != nullptr) {
		obj_type = super_check(type, obj);
		if (obj_type == nullptr)
			return -1;
		ALIF_INCREF(obj);
	}
	ALIF_XSETREF(su->type, (AlifTypeObject*)ALIF_NEWREF(type));
	ALIF_XSETREF(su->obj, obj);
	ALIF_XSETREF(su->objType, obj_type);
	return 0;
}



static AlifObject* super_vectorCall(AlifObject* self, AlifObject* const* args,
	AlifUSizeT nargsf, AlifObject* kwnames) { // 11611
	if (!_ALIFARG_NOKWNAMES("super", kwnames)) {
		return nullptr;
	}
	AlifSizeT nargs = ALIFVECTORCALL_NARGS(nargsf);
	if (!_ALIFARG_CHECKPOSITIONAL("super()", nargs, 0, 2)) {
		return nullptr;
	}
	AlifTypeObject* type = nullptr;
	AlifObject* obj = nullptr;
	AlifTypeObject* self_type = (AlifTypeObject*)self;
	AlifObject* su = self_type->alloc(self_type, 0);
	if (su == nullptr) {
		return nullptr;
	}
	// 1 or 2 argument form super().
	if (nargs != 0) {
		AlifObject* arg0 = args[0];
		if (!ALIFTYPE_CHECK(arg0)) {
			//alifErr_format(_alifExcTypeError_,
			//	"super() argument 1 must be a type, not %.200s", ALIF_TYPE(arg0)->name);
			goto fail;
		}
		type = (AlifTypeObject*)arg0;
	}
	if (nargs == 2) {
		obj = args[1];
	}
	if (super_initImpl(su, type, obj) < 0) {
		goto fail;
	}
	return su;
fail:
	ALIF_DECREF(su);
	return nullptr;
}



AlifTypeObject _alifSuperType_ = { // 11652
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "super",
	.basicSize = sizeof(SuperObject),
	/* methods */
	.dealloc = super_dealloc,
	//.repr = super_repr,
	.getAttro = super_getAttro,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
		ALIF_TPFLAGS_BASETYPE,
	//.init = super_init,
	.alloc = alifType_genericAlloc,
	//.new_ = alifType_genericNew,
	.free = alifObject_gcDel,
	.vectorCall = (VectorCallFunc)super_vectorCall,
};
