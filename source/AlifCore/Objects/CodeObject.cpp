#include "alif.h"
#include "Opcode.h"

#include "AlifCore_Code.h"
#include "AlifCore_Frame.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_Object.h"
#include "AlifCore_OpcodeUtils.h"
#include "AlifCore_OpcodeMetaData.h"
#include "AlifCore_State.h"
#include "AlifCore_SetObject.h"
#include "AlifCore_Tuple.h"





static const char* code_eventName(AlifCodeEvent _event) { // 20
	switch (_event) {
		#define CASE(_op)                \
        case Alif_Code_Event_##_op:         \
            return "Alif_Code_Event_" #_op;
		ALIF_FOREACH_CODE_EVENT(CASE)
		#undef CASE
	}
	ALIF_UNREACHABLE();
}

static void notify_codeWatchers(AlifCodeEvent _event, AlifCodeObject* _co) { // 32
	AlifInterpreter* interp = _alifInterpreter_get();
	uint8_t bits = interp->activeCodeWatchers;
	AlifIntT i = 0;
	while (bits) {
		if (bits & 1) {
			AlifCodeWatchCallback cb = interp->codeWatchers[i];
			if (cb(_event, _co) < 0) {
				//alifErr_formatUnraisable(
					//"Exception ignored in %s watcher callback for %R",
					//code_eventName(event), co);
			}
		}
		i++;
		bits >>= 1;
	}
}

static AlifIntT should_internString(AlifObject* _o) { // 106
#ifdef ALIF_GIL_DISABLED
	AlifInterpreter* interp = _alifInterpreter_get();
	if (alifAtomic_loadInt(&interp->gc.immortalize) < 0) {
		return 1;
	}
#endif

	const unsigned char* s_{}, * e_{};

	if (!ALIFUSTR_IS_ASCII(_o))
		return 0;

	s_ = ALIFUSTR_1BYTE_DATA(_o);
	e_ = s_ + ALIFUSTR_GET_LENGTH(_o);
	for (; s_ != e_; s_++) {
		if (!ALIF_ISALNUM(*s_) and *s_ != '_')
			return 0;
	}
	return 1;
}

#ifdef ALIF_GIL_DISABLED
static AlifObject* intern_oneConstant(AlifObject*); // 133
#endif

static AlifIntT intern_strings(AlifObject* _tuple) { // 137
	AlifInterpreter* interp = _alifInterpreter_get();
	AlifSizeT i{};

	for (i = ALIFTUPLE_GET_SIZE(_tuple); --i >= 0; ) {
		AlifObject* v = ALIFTUPLE_GET_ITEM(_tuple, i);
		if (v == nullptr or !ALIFUSTR_CHECKEXACT(v)) {
			//alifErr_setString(_alifExcSystemError_,
			//	"non-string found in code slot");
			return -1;
		}
		alifUStr_internImmortal(interp, &ALIFTUPLE_ITEMS(_tuple)[i]);
	}
	return 0;
}


static AlifIntT intern_constants(AlifObject* _tuple, AlifIntT* _modified) { // 158
	AlifInterpreter* interp = _alifInterpreter_get();
	for (AlifSizeT i = ALIFTUPLE_GET_SIZE(_tuple); --i >= 0; ) {
		AlifObject* v = ALIFTUPLE_GET_ITEM(_tuple, i);
		if (ALIFUSTR_CHECKEXACT(v)) {
			if (should_internString(v)) {
				AlifObject* w = v;
				alifUStr_internMortal(interp, &v);
				if (w != v) {
					ALIFTUPLE_SET_ITEM(_tuple, i, v);
					if (_modified) {
						*_modified = 1;
					}
				}
			}
		}
		else if (ALIFTUPLE_CHECKEXACT(v)) {
			if (intern_constants(v, nullptr) < 0) {
				return -1;
			}
		}
		else if (ALIFFROZENSET_CHECKEXACT(v)) {
			AlifObject* w = v;
			AlifObject* tmp = alifSequence_tuple(v);
			if (tmp == nullptr) {
				return -1;
			}
			AlifIntT tmpModified = 0;
			if (intern_constants(tmp, &tmpModified) < 0) {
				ALIF_DECREF(tmp);
				return -1;
			}
			if (tmpModified) {
				v = alifFrozenSet_new(tmp);
				if (v == nullptr) {
					ALIF_DECREF(tmp);
					return -1;
				}

				ALIFTUPLE_SET_ITEM(_tuple, i, v);
				ALIF_DECREF(w);
				if (_modified) {
					*_modified = 1;
				}
			}
			ALIF_DECREF(tmp);
		}
#ifdef ALIF_GIL_DISABLED
		else if (ALIFSLICE_CHECK(v)) {
			AlifSliceObject* slice = (AlifSliceObject*)v;
			AlifObject* tmp = alifTuple_new(3);
			if (tmp == nullptr) {
				return -1;
			}
			ALIFTUPLE_SET_ITEM(tmp, 0, ALIF_NEWREF(slice->start));
			ALIFTUPLE_SET_ITEM(tmp, 1, ALIF_NEWREF(slice->stop));
			ALIFTUPLE_SET_ITEM(tmp, 2, ALIF_NEWREF(slice->step));
			AlifIntT tmpModified = 0;
			if (intern_constants(tmp, &tmpModified) < 0) {
				ALIF_DECREF(tmp);
				return -1;
			}
			if (tmpModified) {
				v = alifSlice_new(ALIFTUPLE_GET_ITEM(tmp, 0),
					ALIFTUPLE_GET_ITEM(tmp, 1),
					ALIFTUPLE_GET_ITEM(tmp, 2));
				if (v == nullptr) {
					ALIF_DECREF(tmp);
					return -1;
				}
				ALIFTUPLE_SET_ITEM(_tuple, i, v);
				ALIF_DECREF(slice);
				if (_modified) {
					*_modified = 1;
				}
			}
			ALIF_DECREF(tmp);
		}

		AlifThread* tstate = alifThread_get();
		if (!ALIF_ISIMMORTAL(v) and !ALIFCODE_CHECK(v) and
			!ALIFUSTR_CHECKEXACT(v) and
			alifAtomic_loadInt(&tstate->interpreter->gc.immortalize) >= 0)
		{
			AlifObject* interned = intern_oneConstant(v);
			if (interned == nullptr) {
				return -1;
			}
			else if (interned != v) {
				ALIFTUPLE_SET_ITEM(_tuple, i, interned);
				ALIF_SETREF(v, interned);
				if (_modified) {
					*_modified = 1;
				}
			}
		}
#endif
	}
	return 0;
}



void alifSet_localsPlusInfo(AlifIntT _offset, AlifObject* _name,
	AlifLocalsKind _kind, AlifObject* _names, AlifObject* _kinds) { // 327
	ALIFTUPLE_SET_ITEM(_names, _offset, ALIF_NEWREF(_name));
	_alifLocals_setKind(_kinds, _offset, _kind);
}

static void get_localsPlusCounts(AlifObject* _names, AlifObject* _kinds,
	AlifIntT* pnlocals, AlifIntT* pncellvars,
	AlifIntT* pnfreevars) { // 335
	AlifIntT nlocals = 0;
	AlifIntT ncellvars = 0;
	AlifIntT nfreevars = 0;
	AlifSizeT nlocalsplus = ALIFTUPLE_GET_SIZE(_names);
	for (AlifIntT i = 0; i < nlocalsplus; i++) {
		AlifLocalsKind kind = _alifLocals_getKind(_kinds, i);
		if (kind & CO_FAST_LOCAL) {
			nlocals += 1;
			if (kind & CO_FAST_CELL) {
				ncellvars += 1;
			}
		}
		else if (kind & CO_FAST_CELL) {
			ncellvars += 1;
		}
		else if (kind & CO_FAST_FREE) {
			nfreevars += 1;
		}
	}
	if (pnlocals != nullptr) {
		*pnlocals = nlocals;
	}
	if (pncellvars != nullptr) {
		*pncellvars = ncellvars;
	}
	if (pnfreevars != nullptr) {
		*pnfreevars = nfreevars;
	}
}






AlifIntT _alifCode_validate(AlifCodeConstructor* _con) { // 392
	/* Check argument types */
	if (_con->argCount < _con->posOnlyArgCount or _con->posOnlyArgCount < 0 or
		_con->kwOnlyArgCount < 0 or
		_con->stackSize < 0 or _con->flags < 0 or
		_con->code == nullptr or !ALIFBYTES_CHECK(_con->code) or
		_con->consts == nullptr or !ALIFTUPLE_CHECK(_con->consts) or
		_con->names == nullptr or !ALIFTUPLE_CHECK(_con->names) or
		_con->localsPlusNames == nullptr or !ALIFTUPLE_CHECK(_con->localsPlusNames) or
		_con->localsPlusKinds == nullptr or !ALIFBYTES_CHECK(_con->localsPlusKinds) or
		ALIFTUPLE_GET_SIZE(_con->localsPlusNames)
		!= ALIFBYTES_GET_SIZE(_con->localsPlusKinds) or
		_con->name == nullptr or !ALIFUSTR_CHECK(_con->name) or
		_con->qualname == nullptr or !ALIFUSTR_CHECK(_con->qualname) or
		_con->filename == nullptr or !ALIFUSTR_CHECK(_con->filename) or
		_con->lineTable == nullptr or !ALIFBYTES_CHECK(_con->lineTable) or
		_con->exceptionTable == nullptr or !ALIFBYTES_CHECK(_con->exceptionTable)
		) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}

	if (ALIFBYTES_GET_SIZE(_con->code) > INT_MAX) {
		//alifErr_setString(_alifExcOverflowError_,
		//	"code: code larger than INT_MAX");
		return -1;
	}
	if (ALIFBYTES_GET_SIZE(_con->code) % sizeof(AlifCodeUnit) != 0 or
		!ALIF_IS_ALIGNED(ALIFBYTES_AS_STRING(_con->code), sizeof(AlifCodeUnit))
		) {
		//alifErr_setString(_alifExcValueError_, "code: co_code is malformed");
		return -1;
	}

	AlifIntT nlocals{};
	get_localsPlusCounts(_con->localsPlusNames, _con->localsPlusKinds,
		&nlocals, nullptr, nullptr);
	AlifIntT nplainlocals = nlocals -
		_con->argCount -
		_con->kwOnlyArgCount -
		((_con->flags & CO_VARARGS) != 0) -
		((_con->flags & CO_VARKEYWORDS) != 0);
	if (nplainlocals < 0) {
		//alifErr_setString(_alifExcValueError_, "code: varnames is too small");
		return -1;
	}

	return 0;
}



static void init_code(AlifCodeObject* _co, AlifCodeConstructor* _con) { // 452
	AlifIntT nlocalsplus = (AlifIntT)ALIFTUPLE_GET_SIZE(_con->localsPlusNames);
	AlifIntT nlocals, ncellvars, nfreevars;
	get_localsPlusCounts(_con->localsPlusNames, _con->localsPlusKinds,
		&nlocals, &ncellvars, &nfreevars);
	if (_con->stackSize == 0) {
		_con->stackSize = 1;
	}

	AlifInterpreter* interp = _alifInterpreter_get();
	_co->filename = ALIF_NEWREF(_con->filename);
	_co->name = ALIF_NEWREF(_con->name);
	_co->qualname = ALIF_NEWREF(_con->qualname);
	alifUStr_internMortal(interp, &_co->filename);
	alifUStr_internMortal(interp, &_co->name);
	alifUStr_internMortal(interp, &_co->qualname);
	_co->flags = _con->flags;

	_co->firstLineno = _con->firstLineno;
	_co->lineTable = ALIF_NEWREF(_con->lineTable);

	_co->consts = ALIF_NEWREF(_con->consts);
	_co->names = ALIF_NEWREF(_con->names);

	_co->localsPlusNames = ALIF_NEWREF(_con->localsPlusNames);
	_co->localsPlusKinds = ALIF_NEWREF(_con->localsPlusKinds);

	_co->argCount = _con->argCount;
	_co->posOnlyArgCount = _con->posOnlyArgCount;
	_co->kwOnlyArgCount = _con->kwOnlyArgCount;

	_co->stackSize = _con->stackSize;

	_co->exceptiontable = ALIF_NEWREF(_con->exceptionTable);

	/* derived values */
	_co->nLocalsPlus = nlocalsplus;
	_co->nLocals = nlocals;
	_co->frameSize = nlocalsplus + _con->stackSize + FRAME_SPECIALS_SIZE;
	_co->nCellVars = ncellvars;
	_co->nFreeVars = nfreevars;
#ifdef ALIF_GIL_DISABLED
	ALIFMUTEX_LOCK(&interp->funcState.mutex);
#endif
	_co->version = interp->funcState.nextVersion;
	if (interp->funcState.nextVersion != 0) {
		interp->funcState.nextVersion++;
	}
#ifdef ALIF_GIL_DISABLED
	ALIFMUTEX_UNLOCK(&interp->funcState.mutex);
#endif
	//_co->monitoring = nullptr;
	//_co->instrumentationVersion = 0;
	/* not set */
	_co->weakRefList = nullptr;
	_co->extra = nullptr;
	_co->cached = nullptr;
	_co->executors = nullptr;

	memcpy(ALIFCODE_CODE(_co), ALIFBYTES_AS_STRING(_con->code),
		ALIFBYTES_GET_SIZE(_con->code));
	AlifIntT entryPoint = 0;
	while (entryPoint < ALIF_SIZE(_co) and
		ALIFCODE_CODE(_co)[entryPoint].op.code != RESUME) {
		entryPoint++;
	}
	_co->firstTraceable = entryPoint;
	//alifCode_quicken(_co);
	notify_codeWatchers(AlifCodeEvent::Alif_Code_Event_Create, _co);
}




static AlifIntT intern_codeConstants(AlifCodeConstructor* _con) { // 616
#ifdef ALIF_GIL_DISABLED
	AlifInterpreter* interp = _alifInterpreter_get();
	AlifCodeState* state = &interp->codeState;
	ALIFMUTEX_LOCK(&state->mutex);
#endif
	if (intern_strings(_con->names) < 0) {
		goto error;
	}
	if (intern_constants(_con->consts, nullptr) < 0) {
		goto error;
	}
	if (intern_strings(_con->localsPlusNames) < 0) {
		goto error;
	}
#ifdef ALIF_GIL_DISABLED
	ALIFMUTEX_UNLOCK(&state->mutex);
#endif
	return 0;

error:
#ifdef ALIF_GIL_DISABLED
	ALIFMUTEX_UNLOCK(&state->mutex);
#endif
	return -1;
}


AlifCodeObject* alifCode_new(AlifCodeConstructor* _con) { // 647
	if (intern_codeConstants(_con) < 0) {
		return nullptr;
	}

	AlifObject* replacementLocations = nullptr;

	//if (!alif_getConfig()->codeDebugRanges) {
	//	replacementLocations = remove_columnInfo(_con->lineTable);
	//	if (replacementLocations == nullptr) {
	//		return nullptr;
	//	}
	//	_con->lineTable = replacementLocations;
	//}

	AlifSizeT size = ALIFBYTES_GET_SIZE(_con->code) / sizeof(AlifCodeUnit);
	AlifCodeObject* co{};
#ifdef ALIF_GIL_DISABLED
	co = ALIFOBJECT_GC_NEWVAR(AlifCodeObject, &_alifCodeType_, size);
#else
	co = ALIFOBJECT_NEWVAR(AlifCodeObject, &_alifCodeType_, size);
#endif
	if (co == nullptr) {
		ALIF_XDECREF(replacementLocations);
		//alifErr_noMemory();
		return nullptr;
	}
	init_code(co, _con);
#ifdef ALIF_GIL_DISABLED
	alifObject_setDeferredRefcount((AlifObject*)co);
	ALIFOBJECT_GC_TRACK(co);
#endif
	ALIF_XDECREF(replacementLocations);
	return co;
}














AlifTypeObject _alifCodeType_ = { // 2276
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "شفرة",
	.basicSize = offsetof(AlifCodeObject, codeAdaptive),
	.itemSize = sizeof(AlifCodeUnit),


	.getAttro = alifObject_genericGetAttr,

#ifdef ALIF_GIL_DISABLED
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
#else
	.flags = ALIF_TPFLAGS_DEFAULT
#endif
};


AlifObject* alifCode_constantKey(AlifObject* _op) { // 2331
	AlifObject* key_{};

	if (_op == ALIF_NONE or _op == ALIF_ELLIPSIS
		or ALIFLONG_CHECKEXACT(_op)
		or ALIFUSTR_CHECKEXACT(_op)
		or ALIFCODE_CHECK(_op))
	{
		key_ = ALIF_NEWREF(_op);
	}
	else if (ALIFBOOL_CHECK(_op) or ALIFBYTES_CHECKEXACT(_op)) {
		key_ = alifTuple_pack(2, ALIF_TYPE(_op), _op);
	}
	else if (ALIFFLOAT_CHECKEXACT(_op)) {
		double d_ = ALIFFLOAT_AS_DOUBLE(_op);
		if (d_ == 0.0 and copysign(1.0, d_) < 0.0)
			key_ = alifTuple_pack(3, ALIF_TYPE(_op), _op, ALIF_NONE);
		else
			key_ = alifTuple_pack(2, ALIF_TYPE(_op), _op);
	}
	else if (ALIFCOMPLEX_CHECKEXACT(_op)) {
		AlifComplex z_{};
		AlifIntT realNegZero{}, imagNegZero{};
		z_ = alifComplex_asCComplex(_op);
		realNegZero = z_.real == 0.0 and copysign(1.0, z_.real) < 0.0;
		imagNegZero = z_.imag == 0.0 and copysign(1.0, z_.imag) < 0.0;
		if (realNegZero and imagNegZero) {
			key_ = alifTuple_pack(3, ALIF_TYPE(_op), _op, ALIF_TRUE);
		}
		else if (imagNegZero) {
			key_ = alifTuple_pack(3, ALIF_TYPE(_op), _op, ALIF_FALSE);
		}
		else if (realNegZero) {
			key_ = alifTuple_pack(3, ALIF_TYPE(_op), _op, ALIF_NONE);
		}
		else {
			key_ = alifTuple_pack(2, ALIF_TYPE(_op), _op);
		}
	}
	else if (ALIFTUPLE_CHECKEXACT(_op)) {
		AlifSizeT i_{}, len_{};
		AlifObject* tuple{};

		len_ = ALIFTUPLE_GET_SIZE(_op);
		tuple = alifTuple_new(len_);
		if (tuple == nullptr)
			return nullptr;

		for (i_ = 0; i_ < len_; i_++) {
			AlifObject* item{}, * itemKey{};

			item = ALIFTUPLE_GET_ITEM(_op, i_);
			itemKey = alifCode_constantKey(item);
			if (itemKey == nullptr) {
				ALIF_DECREF(tuple);
				return nullptr;
			}

			ALIFTUPLE_SET_ITEM(tuple, i_, itemKey);
		}

		key_ = alifTuple_pack(2, tuple, _op);
		ALIF_DECREF(tuple);
	}
	else if (ALIFFROZENSET_CHECKEXACT(_op)) {
		AlifSizeT pos_ = 0;
		AlifObject* item{};
		AlifHashT hash{};
		AlifSizeT i_{}, len_{};
		AlifObject* tuple{}, * set_{};

		len_ = ALIFSET_GET_SIZE(_op);
		tuple = alifTuple_new(len_);
		if (tuple == nullptr)
			return nullptr;

		i_ = 0;
		while (alifSet_nextEntry(_op, &pos_, &item, &hash)) {
			AlifObject* itemKey{};

			itemKey = alifCode_constantKey(item);
			if (itemKey == nullptr) {
				ALIF_DECREF(tuple);
				return nullptr;
			}
			ALIFTUPLE_SET_ITEM(tuple, i_, itemKey);
			i_++;
		}
		set_ = alifFrozenSet_new(tuple);
		ALIF_DECREF(tuple);
		if (set_ == nullptr)
			return nullptr;

		key_ = alifTuple_pack(2, set_, _op);
		ALIF_DECREF(set_);
		return key_;
	}
	else {
		AlifObject* objID = alifLong_fromVoidPtr(_op);
		if (objID == nullptr)
			return nullptr;

		key_ = alifTuple_pack(2, objID, _op);
		ALIF_DECREF(objID);
	}
	return key_;
}




#ifdef ALIF_GIL_DISABLED
static AlifObject* intern_oneConstant(AlifObject* _op) { // 2461
	AlifInterpreter* interp = _alifInterpreter_get();
	AlifHashTableT* consts = interp->codeState.constants;
	AlifHashTableEntryT* entry = _alifHashTable_getEntry(consts, _op);
	if (entry == nullptr) {
		if (alifHashTable_set(consts, _op, _op) != 0) {
			return nullptr;
		}


		alif_setImmortal(_op);
		return _op;
	}
	return (AlifObject*)entry->value;
}



#endif // 2609
















