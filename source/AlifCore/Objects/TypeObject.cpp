#include "alif.h"

#include "AlifCore_Dict.h"
#include "AlifCore_Lock.h"
#include "AlifCore_Object.h"
#include "AlifCore_ObjectAlloc.h"
#include "AlifCore_State.h"
#include "AlifCore_TypeObject.h"
#include "AlifCore_CriticalSection.h" // ربما يمكن حذفه بعد الإنتهاء من تطوير اللغة




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


#ifdef ALIF_GIL_DISABLED // 57

// 64
#define TYPE_LOCK &alifInterpreter_get()->types.mutex
#define BEGIN_TYPE_LOCK() ALIF_BEGIN_CRITICAL_SECTION_MUT(TYPE_LOCK)
#define END_TYPE_LOCK() ALIF_END_CRITICAL_SECTION()

#define BEGIN_TYPE_DICT_LOCK(d) \
    ALIF_BEGIN_CRITICAL_SECTION2_MUT(TYPE_LOCK, &ALIFOBJECT_CAST(d)->mutex)

#define END_TYPE_DICT_LOCK() ALIF_END_CRITICAL_SECTION2()

#else

#define BEGIN_TYPE_LOCK()
#define END_TYPE_LOCK()
#define BEGIN_TYPE_DICT_LOCK(d)
#define END_TYPE_DICT_LOCK()

#endif // 84

static AlifObject* lookup_maybeMethod(AlifObject*, AlifObject*, AlifIntT*); // 102

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



static AlifTypeObject* managedStatic_typeGetDef(AlifTypeObject* _self,
	AlifIntT _isBuiltin) { // 317
	AlifUSizeT index = managedStatic_typeIndexGet(_self);
	AlifUSizeT fullIndex = _isBuiltin
		? index
		: index + ALIFMAX_MANAGED_STATIC_BUILTIN_TYPES;
	return &_alifDureRun_.types.managedStatic.types[fullIndex].def;
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


static TypeCache* get_typeCache(void) { // 838
	AlifInterpreter* interp = _alifInterpreter_get();
	return &interp->types.typeCache;
}



static void setVersion_unlocked(AlifTypeObject* tp, AlifUIntT version) { // 999
#ifndef ALIF_GIL_DISABLED
	AlifInterpreter* interp = _alifInterpreter_get();
	if (tp->versionTag != 0) {
		AlifTypeObject** slot =
			interp->types.typeVersionCache
			+ (tp->versionTag % TYPE_VERSION_CACHE_SIZE);
		*slot = nullptr;
	}
	if (version) {
		tp->versionsUsed++;
	}
#else
	if (version) {
		alifAtomic_addUint16(&tp->versionsUsed, 1);
	}
#endif
	alifAtomic_storeUint32Relaxed(&tp->versionTag, version);
#ifndef ALIF_GIL_DISABLED
	if (version != 0) {
		AlifTypeObject** slot =
			interp->types.typeVersionCache
			+ (version % TYPE_VERSION_CACHE_SIZE);
		*slot = tp;
	}
#endif
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
	size_t size = alifObject_varSize(_type, _nitems + 1);

	const size_t preSize = alifType_preHeaderSize(_type);
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
		AlifSizeT i, n;
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
AlifTypeObject* _alifType_calculateMetaClass(AlifTypeObject* _metatype,
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




static AlifObject* type_new(AlifTypeObject* metatype,
	AlifObject* args, AlifObject* kwds) { // 4478
	AlifObject* name{}, * bases{}, * orig_dict{};
	if (!alifArg_parseTuple(args, "UO!O!:type.__new__",
		&name,
		&_alifTupleType_, &bases,
		&_alifDictType_, &orig_dict))
	{
		return nullptr;
	}

	TypeNewCtx ctx = {
		.metatype = metatype,
		.args = args,
		.kwds = kwds,
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




#if ALIF_GIL_DISABLED

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

#endif

AlifObject* alifType_lookupRef(AlifTypeObject* _type, AlifObject* _name) { // 5420
	AlifObject* res{};
	AlifIntT error{};
	AlifInterpreter* interp = _alifInterpreter_get();

	AlifUIntT h_ = MCACHE_HASH_METHOD(_type, _name);
	TypeCache* cache = get_typeCache();
	TypeCacheEntry* entry = &cache->hashTable[h_];
#ifdef ALIF_GIL_DISABLED
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
#else
	if (entry->version == type->versionTag and
		entry->name == _name) {
		ALIF_XINCREF(entry->value);
		return entry->value;
	}
#endif

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
#if ALIF_GIL_DISABLED
		updateCache_gilDisabled(entry, _name, version, res);
#else
		AlifObject* oldValue = update_cache(entry, _name, version, res);
		ALIF_DECREF(oldValue);
#endif
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








AlifTypeObject _alifTypeType_ = { // 6195
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "نوع",
	.basicSize = sizeof(AlifHeapTypeObject),
	.itemSize = sizeof(AlifMemberDef),
	.dealloc = type_dealloc,
	.vectorCallOffset = offsetof(AlifTypeObject, vectorCall),
	.call = type_call,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_TYPE_SUBCLASS |
	ALIF_TPFLAGS_HAVE_VECTORCALL |
	ALIF_TPFLAGS_ITEMS_AT_END,

	.base = 0,
	.dictOffset = offsetof(AlifTypeObject, dict),
	.init = type_init,
	.new_ = type_new,
	.free = alifObject_gcDel,
};






static void object_dealloc(AlifObject* _self) { // 6386
	ALIF_TYPE(_self)->free(_self);
}










AlifTypeObject _alifBaseObjectType_ = { // 7453
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "كائن",              
	.basicSize = sizeof(AlifObject),
	.itemSize = 0,
	.dealloc = object_dealloc,
	.hash = alifObject_genericHash,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE,
	.alloc = alifType_genericAlloc,
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



static AlifIntT typeReady_fillDict(AlifTypeObject* type,
	AlifTypeObject* def) { // 8061
	//if (add_operators(type, def) < 0) {
	//	return -1;
	//}
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
		//inherit_special(_type, base);
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

static AlifIntT type_ready(AlifTypeObject* _type,
	AlifTypeObject* _def, AlifIntT _initial) { // 8383

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
	//if (typeReady_setNew(_type, _initial) < 0) {
	//	goto error;
	//}
	if (typeReady_fillDict(_type, _def) < 0) {
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
		//if (typeReady_managedDict(_type) < 0) {
		//	goto error;
		//}
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
		res = type_ready(_type, nullptr, 1);
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

	AlifTypeObject* def = managedStatic_typeGetDef(_self, _isBuiltin);
	if (_initial) {
		memcpy(def, _self, sizeof(AlifTypeObject));
	}

	AlifIntT res{};
	BEGIN_TYPE_LOCK();
	res = type_ready(_self, def, _initial);
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





static AlifMethodDef _tpNewMethodDef_[] = {
	//{"__new__", ALIFCPPFUNCTION_CAST(tpNew_wrapper), METHOD_VARARGS | METHOD_KEYWORDS,
	// ALIFDOC_STR("__new__($type, *args, **kwargs)\n--\n\n"
	//		   "Create and return a new object.  "
	//		   "See help(type) for accurate signature.")},
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
