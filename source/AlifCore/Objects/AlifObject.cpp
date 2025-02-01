#include "alif.h"

#include "AlifCore_BiaseRefCount.h"
#include "AlifCore_Eval.h"
#include "AlifCore_CriticalSection.h"
#include "AlifCore_Dict.h"
#include "AlifCore_FreeList.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_HashTable.h"
#include "AlifCore_Object.h"
#include "AlifCore_Long.h"
#include "AlifCore_Optimizer.h"
#include "AlifCore_Memory.h"
#include "AlifCore_State.h"
#include "AlifCore_TypeObject.h"




void alif_decRefSharedDebug(AlifObject* _obj,
	const char* _filename, AlifIntT _lineno) { // 322
	AlifIntT shouldQueue;

	AlifSizeT newShared;
	AlifSizeT shared = alifAtomic_loadSizeRelaxed(&_obj->refShared);
	do {
		shouldQueue = (shared == 0 or shared == ALIF_REF_MAYBE_WEAKREF);

		if (shouldQueue) {
			newShared = ALIF_REF_QUEUED;
		}
		else {
			newShared = shared - (1 << ALIF_REF_SHARED_SHIFT);
		}

	} while (!alifAtomic_compareExchangeSize(&_obj->refShared,
		&shared, newShared));

	if (shouldQueue) {
		alifBRC_queueObject(_obj);
	}
	else if (newShared == ALIF_REF_MERGED) {
		// refcount is zero AND merged
		alif_dealloc(_obj);
	}
}



void alif_decRefShared(AlifObject* _o) { // 368
	alif_decRefSharedDebug(_o, nullptr, 0);
}


void alif_mergeZeroLocalRefcount(AlifObject* _op) { // 374

	AlifSizeT shared = alifAtomic_loadSizeAcquire(&_op->refShared);
	if (shared == 0) {
		alif_dealloc(_op);
		return;
	}

	alifAtomic_storeUintptrRelaxed(&_op->threadID, 0);

	AlifSizeT newShared{};
	do {
		newShared = (shared & ~ALIFREF_SHARED_FLAG_MASK) | ALIF_REF_MERGED;
	} while (!alifAtomic_compareExchangeSize(&_op->refShared,
		&shared, newShared));

	if (newShared == ALIF_REF_MERGED) {
		alif_dealloc(_op);
	}
}

AlifObject* alifObject_init(AlifObject* _op, AlifTypeObject* _tp) { // 437
	if (_op == nullptr) {
		//return alifErr_noMemory();
		return nullptr;
	}

	_alifObject_init(_op, _tp);
	return _op;
}

AlifVarObject* alifObject_initVar(AlifVarObject* _op,
	AlifTypeObject* _tp, AlifSizeT _size) { // 448
	if (_op == nullptr) {
		//return (AlifVarObject*)alifErr_noMemory();
		return nullptr;
	}

	_alifObject_initVar(_op, _tp, _size);
	return _op;
}


AlifObject* alifObject_new(AlifTypeObject* _tp) { // 459
	AlifObject* op = (AlifObject*)alifMem_objAlloc(_alifObject_size(_tp));
	if (op == nullptr) {
		//return alifErr_noMemory();
		return nullptr; // temp
	}
	_alifObject_init(op, _tp);
	return op;
}


void alifObject_callFinalizer(AlifObject* _self) { // 483
	AlifTypeObject* tp = ALIF_TYPE(_self);

	if (tp->finalize == nullptr)
		return;
	if (ALIFTYPE_IS_GC(tp) and _alifGC_finalized(_self))
		return;

	tp->finalize(_self);
	if (ALIFTYPE_IS_GC(tp)) {
		_alifGC_setFinalized(_self);
	}
}


AlifIntT alifObject_callFinalizerFromDealloc(AlifObject* _self) { // 500
	if (ALIF_REFCNT(_self) != 0) {
	}

	ALIF_SET_REFCNT(_self, 1);

	alifObject_callFinalizer(_self);


	ALIF_SET_REFCNT(_self, ALIF_REFCNT(_self) - 1);
	if (ALIF_REFCNT(_self) == 0) {
		return 0;         /* this is the normal path out */
	}

	//_alif_resurrectReference(self);

	return -1;
}


AlifObject* alifObject_repr(AlifObject* _v) { // 662
	AlifObject* res{};
	//if (alifErr_checkSignals())
	//	return nullptr;
#ifdef USE_STACKCHECK
	if (alifOS_checkStack()) {
		alifErr_setString(_alifExcMemoryError_, "stack overflow");
		return nullptr;
	}
#endif
	if (_v == nullptr)
		return alifUStr_fromString("<nullptr>"); //* alif
	if (ALIF_TYPE(_v)->repr == nullptr)
		return alifUStr_fromFormat("<%s object at %p>",
			ALIF_TYPE(_v)->name, _v);

	AlifThread* thread = _alifThread_get();

	if (_alif_enterRecursiveCallThread(thread,
		" while getting the repr of an object")) {
		return nullptr;
	}
	res = (*ALIF_TYPE(_v)->repr)(_v);
	_alif_leaveRecursiveCallThread(thread);

	if (res == nullptr) {
		return nullptr;
	}
	if (!ALIFUSTR_CHECK(res)) {
		//alifErr_format(thread, _alifExcTypeError_,
		//	"__repr__ returned non-string (type %.200s)",
		//	ALIF_TYPE(res)->name);
		ALIF_DECREF(res);
		return nullptr;
	}
	return res;
}

AlifObject* alifObject_str(AlifObject* _v) { // 711
	AlifObject* res_{};
	//if (alifErr_checkSignals())
	//	return nullptr;
#ifdef USE_STACKCHECK
	if (alifOS_checkStack()) {
		//alifErr_setString(_alifExcMemoryError_, "stack overflow");
		return nullptr;
	}
#endif
	if (_v == nullptr)
		return alifUStr_fromString("<nullptr>"); //* alif
	if (ALIFUSTR_CHECKEXACT(_v)) {
		return ALIF_NEWREF(_v);
	}
	if (ALIF_TYPE(_v)->str == nullptr)
		return alifObject_repr(_v);

	AlifThread* thread = _alifThread_get();

	if (_alif_enterRecursiveCallThread(thread, " while getting the str of an object")) {
		return nullptr;
	}
	res_ = (*ALIF_TYPE(_v)->str)(_v);
	_alif_leaveRecursiveCallThread(thread);

	if (res_ == nullptr) {
		return nullptr;
	}
	if (!ALIFUSTR_CHECK(res_)) {
		//_alifErr_format(tstate, _alifExcTypeError_,
			//"__str__ returned non-string (type %.200s)",
			//ALIF_TYPE(res_)->name);
		ALIF_DECREF(res_);
		return nullptr;
	}
	return res_;
}

AlifObject* _alifObject_functionStr(AlifObject* _x) { // 886
	AlifObject* qualname{};
	AlifIntT ret = alifObject_getOptionalAttr(_x, &ALIF_ID(__qualname__), &qualname);
	if (qualname == nullptr) {
		if (ret < 0) {
			return nullptr;
		}
		return alifObject_str(_x);
	}
	AlifObject* module{};
	AlifObject* result = nullptr;
	ret = alifObject_getOptionalAttr(_x, &ALIF_ID(__module__), &module);
	if (module != nullptr and module != ALIF_NONE) {
		ret = alifObject_richCompareBool(module, &ALIF_ID(Builtins), ALIF_NE);
		if (ret < 0) {
			// error
			goto done;
		}
		if (ret > 0) {
			result = alifUStr_fromFormat("%S.%S()", module, qualname);
			goto done;
		}
	}
	else if (ret < 0) {
		goto done;
	}
	result = alifUStr_fromFormat("%S()", qualname);
done:
	ALIF_DECREF(qualname);
	ALIF_XDECREF(module);
	return result;
}




AlifIntT _alifSwappedOp_[] = { ALIF_GT, ALIF_GE, ALIF_EQ, ALIF_NE, ALIF_LT, ALIF_LE }; // 953

static const char* const _opStrings_[] = { "<", "<=", "==", "!=", ">", ">=" }; // 955

static AlifObject* do_richCompare(AlifThread* _thread,
	AlifObject* _v, AlifObject* _w, AlifIntT _op) { // 959
	RichCmpFunc f_{};
	AlifObject* res_{};
	AlifIntT checkedReverseOp = 0;

	if (!ALIF_IS_TYPE(_v, ALIF_TYPE(_w)) and
		alifType_isSubType(ALIF_TYPE(_w), ALIF_TYPE(_v)) and
		(f_ = ALIF_TYPE(_w)->richCompare) != nullptr) {
		checkedReverseOp = 1;
		res_ = (*f_)(_w, _v, _alifSwappedOp_[_op]);
		if (res_ != ALIF_NOTIMPLEMENTED)
			return res_;
		ALIF_DECREF(res_);
	}
	if ((f_ = ALIF_TYPE(_v)->richCompare) != nullptr) {
		res_ = (*f_)(_v, _w, _op);
		if (res_ != ALIF_NOTIMPLEMENTED)
			return res_;
		ALIF_DECREF(res_);
	}
	if (!checkedReverseOp and (f_ = ALIF_TYPE(_w)->richCompare) != nullptr) {
		res_ = (*f_)(_w, _v, _alifSwappedOp_[_op]);
		if (res_ != ALIF_NOTIMPLEMENTED)
			return res_;
		ALIF_DECREF(res_);
	}

	switch (_op) {
	case ALIF_EQ:
		res_ = (_v == _w) ? ALIF_TRUE : ALIF_FALSE;
		break;
	case ALIF_NE:
		res_ = (_v != _w) ? ALIF_TRUE : ALIF_FALSE;
		break;
	default:
		//alifErr_format(_thread, _alifExctypeError_,
		//	"'%s' ليس مدعوم بين حالات من '%.100 s' و '%.100 s'",
		//	_opStrings_[_op],
		//	ALIF_TYPE(_v)->name, ALIF_TYPE(_w)->name);
		return nullptr;
	}
	return ALIF_NEWREF(res_);
}

AlifObject* alifObject_richCompare(AlifObject* _v, AlifObject* _w, AlifIntT _op) { // 1011
	AlifThread* thread = _alifThread_get();

	if (_v == nullptr or _w == nullptr) {
		//if (!alifErr_occurred(thread)) {
		//	ALIFERR_BADINTERNALCALLl();
		//}
		return nullptr;
	}
	if (_alif_enterRecursiveCallThread(thread, " في المقارنة")) {
		return nullptr;
	}
	AlifObject* res_ = do_richCompare(thread, _v, _w, _op);

	_alif_leaveRecursiveCallThread(thread);
	return res_;
}


AlifIntT alifObject_richCompareBool(AlifObject* _v,
	AlifObject* _w, AlifIntT _op) { // 1033
	AlifObject* res_{};
	AlifIntT ok_{};
	if (_v == _w) {
		if (_op == ALIF_EQ)
			return 1;
		else if (_op == ALIF_NE)
			return 0;
	}

	res_ = alifObject_richCompare(_v, _w, _op);
	if (res_ == nullptr)
		return -1;
	if (ALIFBOOL_CHECK(res_))
		ok_ = (res_ == ALIF_TRUE);
	else
		ok_ = alifObject_isTrue(res_);
	ALIF_DECREF(res_);
	return ok_;
}

AlifHashT alifObject_hashNotImplemented(AlifObject* _v) { // 1059
	//alifErrFormat(_alifExcTypeError_, "unhashable type: '%.200s'",
	//	ALIF_TYPE(_v)->name);
	return -1;
}

AlifHashT alifObject_hash(AlifObject* _v) { // 1067
	AlifTypeObject* tp_ = ALIF_TYPE(_v);
	if (tp_->hash != nullptr)
		return (*tp_->hash)(_v);
	if (!alifType_isReady(tp_)) {
		if (alifType_ready(tp_) < 0)
			return -1;
		if (tp_->hash != nullptr)
			return (*tp_->hash)(_v);
	}
	return alifObject_hashNotImplemented(_v);
}

AlifObject* alifObject_getAttrString(AlifObject* _v, const char* _name) { // 1088
	AlifObject* w_{}, * res_{};

	if (ALIF_TYPE(_v)->getAttr != nullptr)
		return (*ALIF_TYPE(_v)->getAttr)(_v, (char*)_name);
	w_ = alifUStr_fromString(_name);
	if (w_ == nullptr)
		return nullptr;
	res_ = alifObject_getAttr(_v, w_);
	ALIF_DECREF(w_);
	return res_;
}


AlifIntT alifObject_setAttrString(AlifObject* _v,
	const char* _name, AlifObject* _w) { // 1126
	AlifObject* s_{};
	AlifIntT res_{};

	if (ALIF_TYPE(_v)->setAttr != nullptr)
		return (*ALIF_TYPE(_v)->setAttr)(_v, (char*)_name, _w);
	s_ = alifUStr_internFromString(_name);
	if (s_ == nullptr) return -1;
	res_ = alifObject_setAttr(_v, s_, _w);
	ALIF_XDECREF(s_);
	return res_;
}

AlifIntT alifObject_setAttributeErrorContext(AlifObject* _v, AlifObject* _name) { // 1177
	//if (!alifErr_exceptionMatches(_alifExcAttributeError_)) {
		//return 0;
	//}
	//AlifObject* exc = alifErr_getRaisedException();
	//if (!alifErr_givenExceptionMatches(exc, _alifExcAttributeError_)) {
		//goto restore;
	//}
	//AlifAttributeErrorObject* theExc = (AlifAttributeErrorObject*)exc;
	//if (theExc->name or theExc->obj) {
		//goto restore;
	//}
	//if (alifObject_setAttr(exc, &ALIF_ID(_name), _name) or
		//alifObject_setAttr(exc, &ALIF_ID(obj), _v)) {
		//return 1;
	//}
//restore:
	//alifErr_setRaisedException(exc);
	return 0;
}

AlifObject* alifObject_getAttr(AlifObject* _v, AlifObject* _name) { // 1204
	AlifTypeObject* tp_ = ALIF_TYPE(_v);
	if (!ALIFUSTR_CHECK(_name)) {
		//alifErr_format(_alifExcTypeError_,
			//"attribute name must be string, not '%.200s'",
			//ALIF_TYPE(_name)->_name);
		return nullptr;
	}

	AlifObject* result = nullptr;
	if (tp_->getAttro != nullptr) {
		result = (*tp_->getAttro)(_v, _name);
	}
	else if (tp_->getAttr != nullptr) {
		const char* nameStr = alifUStr_asUTF8(_name);
		if (nameStr == nullptr) {
			return nullptr;
		}
		result = (*tp_->getAttr)(_v, (char*)nameStr);
	}
	else {
		//alifErr_format(_alifExcAttributeError_,
			//"'%.100s' object has no attribute '%U'",
			//tp->name, _name);
	}

	if (result == nullptr) {
		alifObject_setAttributeErrorContext(_v, _name);
	}
	return result;
}

AlifIntT alifObject_getOptionalAttr(AlifObject* _v,
	AlifObject* _name, AlifObject** _result) { // 1238
	AlifTypeObject* tp_ = ALIF_TYPE(_v);

	if (!ALIFUSTR_CHECK(_name)) {
		//alifErr_format(_alifExcTypeError_,
			//"attribute name must be string, not '%.200s'",
			//ALIF_TYPE(_name)->name);
		*_result = nullptr;
		return -1;
	}

	if (tp_->getAttro == alifObject_genericGetAttr) {
		*_result = alifObject_genericGetAttrWithDict(_v, _name, nullptr, 1);
		if (*_result != nullptr) {
			return 1;
		}
		//if (alifErr_occurred()) {
			//return -1;
		//}
		return 0;
	}
	if (tp_->getAttro == alifType_getAttro) {
		AlifIntT suppressMissingAttributeException = 0;
		*_result = alifType_getAttroImpl((AlifTypeObject*)_v, _name, &suppressMissingAttributeException);
		if (suppressMissingAttributeException) {
			return 0;
		}
	}
	else if (tp_->getAttro == (GetAttroFunc)alifModule_getAttro) {
		*_result = alifModule_getAttroImpl((AlifModuleObject*)_v, _name, 1);
		if (*_result != nullptr) {
			return 1;
		}
		//if (alifErr_occurred()) {
			//return -1;
		//}
		return 0;
	}
	else if (tp_->getAttro != nullptr) {
		*_result = (*tp_->getAttro)(_v, _name);
	}
	else if (tp_->getAttr != nullptr) {
		const char* nameStr = alifUStr_asUTF8(_name);
		if (nameStr == nullptr) {
			*_result = nullptr;
			return -1;
		}
		*_result = (*tp_->getAttr)(_v, (char*)nameStr);
	}
	else {
		*_result = nullptr;
		return 0;
	}

	if (*_result != nullptr) {
		return 1;
	}
	//if (!alifErr_exceptionMatches(_alifExcAttributeError_)) {
		//return -1;
	//}
	//alifErr_clear();
	return 0;
}


AlifIntT alifObject_hasAttrWithError(AlifObject* _obj, AlifObject* _name) { // 1330
	AlifObject* res{};
	AlifIntT rc = alifObject_getOptionalAttr(_obj, _name, &res);
	ALIF_XDECREF(res);
	return rc;
}


AlifIntT alifObject_setAttr(AlifObject* _v,
	AlifObject* _name, AlifObject* _value) { // 1354
	AlifTypeObject* tp_ = ALIF_TYPE(_v);
	AlifIntT err{};

	if (!ALIFUSTR_CHECK(_name)) {
		//alifErr_format(_alifExcTypeError_,
			//"attribute name must be string, not '%.200s'",
			//ALIF_TYPE(_name)->name);
		return -1;
	}
	ALIF_INCREF(_name);

	AlifInterpreter* interp = _alifInterpreter_get();
	alifUStr_internMortal(interp, &_name);
	if (tp_->setAttro != nullptr) {
		err = (*tp_->setAttro)(_v, _name, _value);
		ALIF_DECREF(_name);
		return err;
	}
	if (tp_->setAttr != nullptr) {
		const char* nameStr = alifUStr_asUTF8(_name);
		if (nameStr == nullptr) {
			ALIF_DECREF(_name);
			return -1;
		}
		err = (*tp_->setAttr)(_v, (char*)nameStr, _value);
		ALIF_DECREF(_name);
		return err;
	}
	ALIF_DECREF(_name);
	if (tp_->getAttr == nullptr and tp_->getAttro == nullptr) {
		//alfiErr_format(_alifExcTypeError_,
			//"'%.100s' object has no attributes "
			//"(%s .%U)", tp_->name,
			//_value == nullptr ? "del" : "assign to", _name);
	}
	else {
		//alifErr_format(_alifExcTypeError_,
			//"'%.100s' object has only read-only attributes "
			//"(%s .%U)", tp_->name,
			//_value == nullptr ? "del" : "assign to", _name);
	}
	return -1;
}

AlifIntT alifObject_delAttr(AlifObject* _v, AlifObject* _name) { // 1404
	return alifObject_setAttr(_v, _name, nullptr);
}


AlifObject** alifObject_computedDictPointer(AlifObject* _obj) { // 1409
	AlifTypeObject* tp = ALIF_TYPE(_obj);

	AlifSizeT dictOffset = tp->dictOffset;
	if (dictOffset == 0) {
		return nullptr;
	}

	if (dictOffset < 0) {
		AlifSizeT tsize = ALIF_SIZE(_obj);
		if (tsize < 0) {
			tsize = -tsize;
		}
		AlifUSizeT size = alifObject_varSize(tp, tsize);
		dictOffset += (AlifSizeT)size;
	}
	return (AlifObject**)((char*)_obj + dictOffset);
}

AlifObject* alifObject_selfIter(AlifObject* _obj) { // 1461
	return ALIF_NEWREF(_obj);
}

AlifObject* _alifObject_nextNotImplemented(AlifObject* _self) { // 1472
	//alifErr_format(_alifExcTypeError_,
	//	"'%.200s' object is not iterable",
	//	ALIF_TYPE(_self)->name);
	return nullptr;
}


AlifIntT _alifObject_getMethod(AlifObject* _obj,
	AlifObject* _name, AlifObject** _method) { // 1492
	AlifIntT meth_found = 0;

	AlifTypeObject* tp = ALIF_TYPE(_obj);
	if (!alifType_isReady(tp)) {
		if (alifType_ready(tp) < 0) {
			return 0;
		}
	}

	if (tp->getAttro != alifObject_genericGetAttr or !ALIFUSTR_CHECKEXACT(_name)) {
		*_method = alifObject_getAttr(_obj, _name);
		return 0;
	}

	AlifObject* descr = alifType_lookupRef(tp, _name);
	DescrGetFunc f = nullptr;
	if (descr != nullptr) {
		if (_alifType_hasFeature(ALIF_TYPE(descr), ALIF_TPFLAGS_METHOD_DESCRIPTOR)) {
			meth_found = 1;
		}
		else {
			f = ALIF_TYPE(descr)->descrGet;
			if (f != nullptr and alifDescr_isData(descr)) {
				*_method = f(descr, _obj, (AlifObject*)ALIF_TYPE(_obj));
				ALIF_DECREF(descr);
				return 0;
			}
		}
	}
	AlifObject* dict{}, * attr{};
	if ((tp->flags & ALIF_TPFLAGS_INLINE_VALUES) and
		alifObject_tryGetInstanceAttribute(_obj, _name, &attr)) {
		if (attr != nullptr) {
			*_method = attr;
			ALIF_XDECREF(descr);
			return 0;
		}
		dict = nullptr;
	}
	else if ((tp->flags & ALIF_TPFLAGS_MANAGED_DICT)) {
		dict = (AlifObject*)alifObject_getManagedDict(_obj);
	}
	else {
		AlifObject** dictptr = alifObject_computedDictPointer(_obj);
		if (dictptr != nullptr) {
			dict = *dictptr;
		}
		else {
			dict = nullptr;
		}
	}
	if (dict != nullptr) {
		ALIF_INCREF(dict);
		if (alifDict_getItemRef(dict, _name, _method) != 0) {
			// found or error
			ALIF_DECREF(dict);
			ALIF_XDECREF(descr);
			return 0;
		}
		// not found
		ALIF_DECREF(dict);
	}

	if (meth_found) {
		*_method = descr;
		return 1;
	}

	if (f != nullptr) {
		*_method = f(descr, _obj, (AlifObject*)ALIF_TYPE(_obj));
		ALIF_DECREF(descr);
		return 0;
	}

	if (descr != nullptr) {
		*_method = descr;
		return 0;
	}

	//alifErr_format(_alifExcAttributeError_,
	//	"'%.100s' object has no attribute '%U'",
	//	tp->name, name);

	//_alifObject_setAttributeErrorContext(obj, name);
	return 0;
}


AlifObject* alifObject_genericGetAttrWithDict(AlifObject* _obj, AlifObject* _name,
	AlifObject* _dict, AlifIntT _suppress) { // 1587

	AlifTypeObject* tp = ALIF_TYPE(_obj);
	AlifObject* descr = nullptr;
	AlifObject* res = nullptr;
	DescrGetFunc f_{};

	if (!ALIFUSTR_CHECK(_name)) {
		//alifErr_format(_alifExcTypeError_,
			//"attribute name must be string, not '%.200s'",
			//ALIF_TYPE(name)->name);
		return nullptr;
	}
	ALIF_INCREF(_name);

	if (!alifType_isReady(tp)) {
		if (alifType_ready(tp) < 0)
			goto done;
	}

	descr = alifType_lookupRef(tp, _name);

	f_ = nullptr;
	if (descr != nullptr) {
		f_ = ALIF_TYPE(descr)->descrGet;
		if (f_ != nullptr and alifDescr_isData(descr)) {
			res = f_(descr, _obj, (AlifObject*)ALIF_TYPE(_obj));
			if (res == nullptr and _suppress
				/*and alifErr_exceptionMatches(alifExcAttributeError)*/
				) {
				//alifErr_clear();
			}
			goto done;
		}
	}
	if (_dict == nullptr) {
		if ((tp->flags & ALIF_TPFLAGS_INLINE_VALUES)) {
			if (ALIFUSTR_CHECKEXACT(_name) and
				alifObject_tryGetInstanceAttribute(_obj, _name, &res)) {
				if (res != nullptr) {
					goto done;
				}
			}
			else {
				_dict = (AlifObject*)alifObject_materializeManagedDict(_obj);
				if (_dict == nullptr) {
					res = nullptr;
					goto done;
				}
			}
		}
		else if ((tp->flags & ALIF_TPFLAGS_MANAGED_DICT)) {
			_dict = (AlifObject*)alifObject_getManagedDict(_obj);
		}
		else {
			AlifObject** dictPtr = alifObject_computedDictPointer(_obj);
			if (dictPtr) {
				_dict = *dictPtr;
			}
		}
	}
	if (_dict != nullptr) {
		ALIF_INCREF(_dict);
		AlifIntT rc = alifDict_getItemRef(_dict, _name, &res);
		ALIF_DECREF(_dict);
		if (res != nullptr) {
			goto done;
		}
		else if (rc < 0) {
			if (_suppress
				/*and alifErr_exceptionMatches(alifExcAttributeError)*/
				) {
				//alifErr_clear();
			}
			else {
				goto done;
			}
		}
	}

	if (f_ != nullptr) {
		res = f_(descr, _obj, (AlifObject*)ALIF_TYPE(_obj));
		if (res == nullptr and _suppress
			/*and alifErr_exceptionMatches(alifExAttributeError)*/
			) {
			//alifErr_clear();
		}
		goto done;
	}

	if (descr != nullptr) {
		res = descr;
		descr = nullptr;
		goto done;
	}

	if (!_suppress) {
		//alifErr_format(_alifExcAttributeError_,
		//	"'%.100s' object has no attribute '%U'",
		//	tp->name, _name);

		//alifObject_setAttributeErrorContext(_obj, _name);
	}
done:
	ALIF_XDECREF(descr);
	ALIF_DECREF(_name);
	return res;
}

AlifObject* alifObject_genericGetAttr(AlifObject* _obj, AlifObject* _name) { // 1699
	return alifObject_genericGetAttrWithDict(_obj, _name, nullptr, 0);
}

AlifIntT alifObject_genericSetAttrWithDict(AlifObject* _obj, AlifObject* _name,
	AlifObject* _value, AlifObject* _dict) { // 1705
	AlifTypeObject* tp = ALIF_TYPE(_obj);
	AlifObject* descr{};
	DescrSetFunc f{};
	AlifIntT res = -1;

	if (!ALIFUSTR_CHECK(_name)) {
		//alifErr_format(_alifExcTypeError_,
		//	"attribute name must be string, not '%.200s'",
		//	ALIF_TYPE(name)->name);
		return -1;
	}

	if (!alifType_isReady(tp) and alifType_ready(tp) < 0) {
		return -1;
	}

	ALIF_INCREF(_name);
	ALIF_INCREF(tp);
	descr = alifType_lookupRef(tp, _name);

	if (descr != nullptr) {
		f = ALIF_TYPE(descr)->descrSet;
		if (f != nullptr) {
			res = f(descr, _obj, _value);
			goto done;
		}
	}

	if (_dict == nullptr) {
		AlifObject** dictptr{};

		if ((tp->flags & ALIF_TPFLAGS_INLINE_VALUES)) {
			res = alifObject_storeInstanceAttribute(_obj, _name, _value);
			goto error_check;
		}

		if ((tp->flags & ALIF_TPFLAGS_MANAGED_DICT)) {
			AlifManagedDictPointer* managed_dict = alifObject_managedDictPointer(_obj);
			dictptr = (AlifObject**)&managed_dict->dict;
		}
		else {
			dictptr = alifObject_computedDictPointer(_obj);
		}
		if (dictptr == nullptr) {
			if (descr == nullptr) {
				if (tp->setAttro == alifObject_genericSetAttr) {
					//alifErr_format(_alifExcAttributeError_,
					//	"'%.100s' object has no attribute '%U' and no "
					//	"__dict__ for setting new attributes",
					//	tp->name, name);
				}
				else {
					//alifErr_format(_alifExcAttributeError_,
					//	"'%.100s' object has no attribute '%U'",
					//	tp->name, name);
				}
				//alifObject_setAttributeErrorContext(_obj, _name);
			}
			else {
				//alifErr_format(_alifExcAttributeError_,
				//	"'%.100s' object attribute '%U' is read-only",
				//	tp->name, name);
			}
			goto done;
		}
		else {
			res = alifObjectDict_setItem(tp, _obj, dictptr, _name, _value);
		}
	}
	else {
		ALIF_INCREF(_dict);
		if (_value == nullptr)
			res = alifDict_delItem(_dict, _name);
		else
			res = alifDict_setItem(_dict, _name, _value);
		ALIF_DECREF(_dict);
	}
error_check:
	//if (res < 0 and alifErr_exceptionMatches(_alifExcKeyError_)) {
	//	alifErr_format(_alifExcAttributeError_,
	//		"'%.100s' object has no attribute '%U'",
	//		tp->name, name);
	//	_alifObject_setAttributeErrorContext(obj, name);
	//}
done:
	ALIF_XDECREF(descr);
	ALIF_DECREF(tp);
	ALIF_DECREF(_name);
	return res;
}


AlifIntT alifObject_genericSetAttr(AlifObject* _obj,
	AlifObject* _name, AlifObject* _value) { // 1801
	return alifObject_genericSetAttrWithDict(_obj, _name, _value, nullptr);
}

AlifIntT alifObject_isTrue(AlifObject* _v) { // 1845
	AlifSizeT res_{};
	if (_v == ALIF_TRUE)
		return 1;
	if (_v == ALIF_FALSE)
		return 0;
	if (_v == ALIF_NONE)
		return 0;
	else if (ALIF_TYPE(_v)->asNumber != nullptr and
		ALIF_TYPE(_v)->asNumber->bool_ != nullptr)
		res_ = (*ALIF_TYPE(_v)->asNumber->bool_)(_v);
	else if (ALIF_TYPE(_v)->asMapping != nullptr and
		ALIF_TYPE(_v)->asMapping->length != nullptr)
		res_ = (*ALIF_TYPE(_v)->asMapping->length)(_v);
	else if (ALIF_TYPE(_v)->asSequence != nullptr and
		ALIF_TYPE(_v)->asSequence->length != nullptr)
		res_ = (*ALIF_TYPE(_v)->asSequence->length)(_v);
	else
		return 1;
	return (res_ > 0) ? 1 : ALIF_SAFE_DOWNCAST(res_, AlifSizeT, AlifIntT);
}


AlifIntT alifCallable_check(AlifObject* _x) { // 1884
	if (_x == nullptr)
		return 0;
	return ALIF_TYPE(_x)->call != nullptr;
}

static AlifObject* none_repr(AlifObject* _op) { // 1970
	return alifUStr_fromString("عدم");
}

AlifTypeObject _alifNoneType_ = { // 2049
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "عدم",
	.repr = none_repr,
};

AlifObject _alifNoneClass_ = ALIFOBJECT_HEAD_INIT(&_alifNoneType_); // 2090

AlifTypeObject _alifNotImplementedType_ = { // 2149
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "نوع_غير_مطبق",
	.basicSize = 0,
	.itemSize = 0,
};

AlifObject _alifNotImplementedClass_ = ALIFOBJECT_HEAD_INIT(&_alifNotImplementedType_); // 2190



static AlifTypeObject* staticTypes[] = {
	// The two most important base types: must be initialized first and
	// deallocated last.
	&_alifBaseObjectType_,
	&_alifTypeType_,

	//&_alifAsyncGenType_,
	//&_alifByteArrayIterType_, // 
	&_alifByteArrayType_,
	//&_alifBytesIterType_, // 
	&_alifBytesType_,
	&_alifCPPFunctionType_,
	//&_alifCallIterType_,
	&_alifCapsuleType_,
	&_alifCellType_,
	&_alifClassMethodDescrType_,
	&_alifClassMethodType_,
	&_alifCodeType_,
	&_alifComplexType_,
	//&_alifContextTokenType_,
	//&_alifContextVarType_,
	//&_alifContextType_,
	//&_alifCoroType_,
	&_alifDictItemsType_,
	&_alifDictIterItemType_,
	&_alifDictIterKeyType_,
	//&_alifDictIterValueType_,
	&_alifDictKeysType_,
	//&_alifDictProxyType_,
	&_alifDictRevIterItemType_,
	&_alifDictRevIterKeyType_,
	&_alifDictRevIterValueType_,
	&_alifDictValuesType_,
	&_alifDictType_,
	//&_alifEllipsisType_,
	//&_alifEnumType_,
	//&_alifFilterType_,
	&_alifFloatType_,
	&_alifFrameType_,
	//&_alifFrameLocalsProxyType_,
	&_alifFrozenSetType_,
	&_alifFunctionType_,
	//&_alifGenType_,
	//&_alifGetSetDescrType_,
	//&_alifInstanceMethodType_,
	//&_alifListIterType_,
	//&_alifListRevIterType_,
	&_alifListType_,
	//&_alifLongRangeIterType_,
	&_alifLongType_,
	//&_alifMapType_,
	//&_alifMemberDescrType_, // 
	//&_alifMemoryViewType_,
	//&_alifMethodDescrType_,
	&_alifMethodType_,
	//&_alifModuleDefType_,
	&_alifModuleType_,
	//&_alifODictIterType_,
	//&_alifPickleBufferType_,
	//&_alifPropertyType_,
	//&_alifRangeIterType_,
	&_alifRangeType_,
	//&_alifReversedType_,
	//&_alifSTEntryType_,
	&_alifSeqIterType_,
	//&_alifSetIterType_,
	&_alifSetType_,
	&_alifSliceType_,
	&_alifStaticMethodType_,
	//&_alifStdPrinterType_,
	&_alifSuperType_,
	//&_alifTraceBackType_,
	//&_alifTupleIterType_,
	&_alifTupleType_,
	//&_alifUStrIterType_,
	&_alifUStrType_,
	//&_alifWrapperDescrType_,
	//&_alifZipType_,
	//&_alifGenericAliasType_,
	//&_alifAnextAwaitableType_,
	//&_alifAsyncGenASendType_,
	//&_alifAsyncGenAThrowType_,
	//&_alifAsyncGenWrappedValueType_,
	//&_alifBufferWrapperType_,
	//&_alifContextTokenMissingType_,
	//&_alifCoroWrapperType_,

	//&_alifGenericAliasIterType_,
	//&_alifHamtItemsType_,
	//&_alifHamtKeysType_,
	//&_alifHamtValuesType_,
	//&_alifHamtArrayNodeType_,
	//&_alifHamtBitmapNodeType_,
	//&_alifHamtCollisionNodeType_,
	//&_alifHamtType_,
	//&_alifInstructionSequenceType_,
	//&_alifLegacyEventHandlerType_,
	//&_alifLineIterator_,
	//&_alifManagedBufferType_,
	//&_alifMemoryIterType_,
	//&_alifMethodWrapperType_,
	//&_alifNamespaceType_,
	&_alifNoneType_,
	&_alifNotImplementedType_,
	//&_alifPositionsIterator_,
	//&_alifUStrASCIIIterType_,
	//&_alifUnionType_,

	// class
	&_alifBoolType_,       
	&_alifCPPMethodType_,  
	//&_alifODictItemsType_,
	//&_alifODictKeysType_,
	//&_alifODictValuesType_,
	//&_alifODictType_,
};


AlifIntT alifTypes_initTypes(AlifInterpreter* _interp) { // 2362
	for (AlifUSizeT i = 0; i < ALIF_ARRAY_LENGTH(staticTypes); i++) {
		AlifTypeObject* type = staticTypes[i];
		if (alifStaticType_initBuiltin(_interp, type) < 0) {
			//return ALIFSTATUS_ERR("Can't initialize builtin type");
			return -1; // temp
		}
		if (type == &_alifTypeType_) {
			// nothing
		}
	}

	//if (alif_initializeGeneric(_interp) < 0) {
	//	return ALIFSTATUS_ERR("Can't initialize generic types");
	//}

	return 1;
}


static inline void new_reference(AlifObject* _op) { // 2405
	_op->threadID = alif_threadID();
	_op->padding = 0;
	_op->mutex = {.bits = 0};
	_op->gcBits = 0;
	_op->refLocal = 1;
	_op->refShared = 0;

	RefTracerDureRunState* tracer = &_alifDureRun_.refTracer;
	if (tracer->tracerFunc != nullptr) {
		void* data = tracer->tracerData;
		tracer->tracerFunc(_op, AlifRefTracerEvent_::Alif_RefTracer_Create, data);
	}
}

void alif_newReference(AlifObject* _op) { // 2429
	new_reference(_op);
}



void alif_newReferenceNoTotal(AlifObject* _op) { //2438
	new_reference(_op);
}

void alif_setImmortalUntracked(AlifObject* _op) { // 2444

	_op->threadID = ALIF_UNOWNED_TID;
	_op->refLocal = ALIF_IMMORTAL_REFCNT_LOCAL;
	_op->refShared = 0;
}

void alif_setImmortal(AlifObject* _op) { // 2463
	if (alifObject_isGC(_op) and ALIFOBJECT_GC_IS_TRACKED(_op)) {
		ALIFOBJECT_GC_UNTRACK(_op);
	}
	alif_setImmortalUntracked(_op);
}

void alifObject_setDeferredRefcount(AlifObject* _op) { // 2472
	alifObject_setGCBits(_op, ALIFGC_BITS_DEFERRED);
	_op->refShared = ALIF_REF_SHARED(ALIF_REF_DEFERRED, 0);
}


AlifIntT alif_reprEnter(AlifObject* _obj) { // 2690
	AlifObject* dict{};
	AlifObject* list{};
	AlifSizeT i{};

	dict = alifThreadState_getDict();
	if (dict == nullptr)
		return 0;
	list = alifDict_getItemWithError(dict, &ALIF_ID(AlifRepr));
	if (list == nullptr) {
		if (alifErr_occurred()) {
			return -1;
		}
		list = alifList_new(0);
		if (list == nullptr)
			return -1;
		if (alifDict_setItem(dict, &ALIF_ID(AlifRepr), list) < 0)
			return -1;
		ALIF_DECREF(list);
	}
	i = ALIFLIST_GET_SIZE(list);
	while (--i >= 0) {
		if (ALIFLIST_GET_ITEM(list, i) == _obj)
			return 1;
	}
	if (alifList_append(list, _obj) < 0)
		return -1;
	return 0;
}

void alif_reprLeave(AlifObject* _obj) { // 2724
	AlifObject* dict{};
	AlifObject* list{};
	AlifSizeT i{};

	//AlifObject* exc = alifErr_getRaisedException();

	dict = alifThreadState_getDict();
	if (dict == nullptr)
		goto finally;

	list = alifDict_getItemWithError(dict, &ALIF_ID(AlifRepr));
	if (list == nullptr or !ALIFLIST_CHECK(list))
		goto finally;

	i = ALIFLIST_GET_SIZE(list);
	/* Count backwards because we always expect obj to be list[-1] */
	while (--i >= 0) {
		if (ALIFLIST_GET_ITEM(list, i) == _obj) {
			alifList_setSlice(list, i, i + 1, nullptr);
			break;
		}
	}

finally:
	//alifErr_setRaisedException(exc);
	return; //* delete
}


void _alifTrashThread_depositObject(AlifThread* _tstate, AlifObject* _op) { // 2761
	_op->threadID = (uintptr_t)_tstate->deleteLater;
	_tstate->deleteLater = _op;
}

void _alifTrashThread_destroyChain(AlifThread* _thread) { // 2777
	_thread->cppRecursionRemaining--;
	while (_thread->deleteLater) {
		AlifObject* op = _thread->deleteLater;
		Destructor dealloc = ALIF_TYPE(op)->dealloc;

		_thread->deleteLater = (AlifObject*)op->threadID;
		op->threadID = 0;
		alifAtomic_storeSizeRelaxed(&op->refShared, ALIF_REF_MERGED);

		(*dealloc)(op);
	}
	_thread->cppRecursionRemaining++;
}


void alif_dealloc(AlifObject* _op) { // 2868
	AlifTypeObject* type = ALIF_TYPE(_op);
	Destructor dealloc = type->dealloc;

	RefTracerDureRunState* tracer = &_alifDureRun_.refTracer;
	if (tracer->tracerFunc != nullptr) {
		void* data = tracer->tracerData;
		tracer->tracerFunc(_op, AlifRefTracerEvent_::Alif_RefTracer_Destroy, data);
	}

	(*dealloc)(_op);
}








#undef ALIF_TYPE
AlifTypeObject* ALIF_TYPE(AlifObject* _ob) { // 3042
	return _alif_type(_ob);
}


#undef ALIF_REFCNT
AlifSizeT ALIF_REFCNT(AlifObject* _ob) { // 3051
	return _alif_refCnt(_ob);
}
