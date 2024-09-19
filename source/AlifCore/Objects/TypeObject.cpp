#include "alif.h"

#include "AlifCore_Dict.h"
#include "AlifCore_Lock.h"
#include "AlifCore_Object.h"
#include "AlifCore_ObjectAlloc.h"
#include "AlifCore_State.h"
#include "AlifCore_TypeObject.h"
#include "AlifCore_CriticalSection.h" // ربما يمكن حذفه بعد الإنتهاء من تطوير اللغة






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



static inline AlifUSizeT managedStatic_typeIndexGet(AlifTypeObject* _self) { // 130
	return (AlifUSizeT)_self->subclasses - 1;
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

ManagedStaticTypeState* alifStaticType_getState(AlifInterpreter* _interp, AlifTypeObject* _self) { // 193
	return managedStatic_typeStateGet(_interp, _self);
}

static inline void start_readying(AlifTypeObject* _type) { // 356
	if (_type->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		AlifInterpreter* interp = alifInterpreter_get();
		ManagedStaticTypeState* state = managedStatic_typeStateGet(interp, _type);
		state->readying = 1;
		return;
	}
	_type->flags |= ALIF_TPFLAGS_READYING;
}

static inline void stop_readying(AlifTypeObject* _type) { // 371 
	if (_type->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		AlifInterpreter* interp = alifInterpreter_get();
		ManagedStaticTypeState* state = managedStatic_typeStateGet(interp, _type);
		state->readying = 0;
		return;
	}
	_type->flags &= ~ALIF_TPFLAGS_READYING;
}

static inline AlifObject* lookup_tpDict(AlifTypeObject* _self) { // 401
	if (_self->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		AlifInterpreter* interp = alifInterpreter_get();
		ManagedStaticTypeState* state = alifStaticType_getState(interp, _self);
		return state->dict;
	}
	return _self->dict;
}

AlifObject* alifType_getDict(AlifTypeObject* _self) { // 413
	return lookup_tpDict(_self);
}

static inline void set_tpDict(AlifTypeObject* _self, AlifObject* _dict) { // 427
	if (_self->flags & ALIF_TPFLAGS_STATIC_BUILTIN) {
		AlifInterpreter* interp = alifInterpreter_get();
		ManagedStaticTypeState* state = alifStaticType_getState(interp, _self);
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




static AlifIntT typeIsSubType_baseChain(AlifTypeObject* _a, AlifTypeObject* _b) { // 2636
	do {
		if (_a == _b)
			return 1;
		_a = _a->base;
	} while (_a != nullptr);

	return (_b == &_alifBaseObjectType_);
}


static AlifIntT isSubType_withMethResOrder(AlifObject* _methResOrder,
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
	return isSubType_withMethResOrder(a->methResOrder, a, b);
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

static AlifObject* call_unboundNoArg(AlifIntT _unbound, AlifObject* _func, AlifObject* _self) { // 2786
	if (_unbound) {
		return alifObject_callOneArg(_func, _self);
	}
	else {
		return alifObject_callNoArgs(_func);
	}
}

static AlifObject* class_name(AlifObject* _cls) { // 2881
	AlifObject* name{};
	if (alifObject_getOptionalAttr(_cls, &ALIF_ID(__name__), &name) == 0) {
		name = AlifObject_repr(_cls);
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

	AlifObject** toMerge = (AlifObject**)alifMem_objAlloc(n + 1);
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
		alifMem_objFree(toMerge);
		return nullptr;
	}

	ALIFLIST_SET_ITEM(result, 0, ALIF_NEWREF(_type));
	if (p_merge(result, toMerge, n + 1) < 0) {
		ALIF_CLEAR(result);
	}
	alifMem_objFree(toMerge);

	return result;
}

static AlifObject* mro_invoke(AlifTypeObject* _type) { // 3210
	AlifObject* mroResult{};
	AlifObject* newMro{};

	const AlifIntT custom = !ALIF_IS_TYPE(_type, &_alifTypeType_);

	if (custom) {
		AlifIntT unbound{};
		AlifObject* mroMeth = lookup_method(
			(AlifObject*)_type, &ALIF_ID(mro), &unbound);
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


static AlifIntT mro_internalUnlocked(AlifTypeObject* _type, AlifIntT _initial, AlifObject** _pOldMro) { // 3276

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

AlifObject* alifType_lookupRef(AlifTypeObject* _type, AlifObject* _name) { // 5420
	AlifObject* res{};
	AlifIntT error{};
	AlifInterpreter* interp = alifInterpreter_get();

	AlifUIntT h = MCACHE_HASH_METHOD(_type, _name);
	TypeCache* cache = get_typeCache();
	TypeCacheEntry* entry = &cache->hashTable[h];
#ifdef ALIF_GIL_DISABLED
	while (1) {
		uint32_t sequence = alifSeqLock_beginRead(&entry->sequence);
		uint32_t entryVersion = alifAtomic_loadUint32Relaxed(&entry->version);
		uint32_t typeVersion = alifAtomic_loadUint32Acquire(&_type->versionTag);
		if (entryVersion == typeVersion and
			alifAtomic_loadPtrRelaxed(&entry->name) == _name) {
			AlifObject* value = alifAtomic_loadPtrRelaxed(&entry->value);
			if (value == nullptr or alif_tryInRref(value)) {
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
	if (MCACHE_CACHEABLE_NAME(name)) {
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

static void type_dealloc(AlifObject* self) { // 5911
	AlifTypeObject* type = (AlifTypeObject*)self;

	ALIFOBJECT_GC_UNTRACK(type);
	//type_deallocCommon(type);

	//alifObject_clearWeakRefs((AlifObject*)type);

	ALIF_XDECREF(type->base);
	//ALIF_XDECREF(type->dict);
	//ALIF_XDECREF(type->bases);
	//ALIF_XDECREF(type->mro);
	//ALIF_XDECREF(type->cache);
	//clear_tpSubClasses(type);	

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
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_TYPE_SUBCLASS |
	ALIF_TPFLAGS_HAVE_VECTORCALL |
	ALIF_TPFLAGS_ITEMS_AT_END,

	.base = 0,
};

















AlifTypeObject _alifBaseObjectType_ = { // 7453
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "كائن",              
	.basicSize = sizeof(AlifObject),
	.itemSize = 0,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE,
};


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
			AlifTypeObject* base = ALIFTYPE_CAST(ALIFTUPLE_GET_SIZE(mro, i));
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
	//if (typeReady_mro(_type, _initial) < 0) {
	//	goto error;
	//}
	//if (typeReady_setNew(_type, _initial) < 0) {
	//	goto error;
	//}
	//if (typeReady_fillDict(_type, _def) < 0) {
	//	goto error;
	//}
	//if (_initial) {
	//	if (typeReady_inherit(_type) < 0) {
	//		goto error;
	//	}
	//	if (typeReady_preheader(_type) < 0) {
	//		goto error;
	//	}
	//}
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
		if (typeReady_postChecks(_type) < 0) {
			goto error;
		}
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
