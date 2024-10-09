#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Object.h"
#include "AlifCore_Long.h"
#include "AlifCore_State.h"




static AlifObject* null_error(void) { // 27
	AlifThread* thread = _alifThread_get();
	//if (!alifErr_occurred(thread)) {
	//	alifErr_setString(thread, _alifExcSystemError_,
	//		"null argument to internal routine");
	//}
	return nullptr;
}


AlifSizeT alifObject_size(AlifObject* _o) { // 53
	if (_o == nullptr) {
		null_error();
		return -1;
	}

	AlifSequenceMethods* m_ = ALIF_TYPE(_o)->asSequence;
	if (m_ and m_->length) {
		AlifSizeT len = m_->length(_o);
		return len;
	}

	return alifMapping_size(_o);
}

#undef ALIFOBJECT_LENGTH
AlifSizeT alifObject_length(AlifObject* _o) { // 72
	return alifObject_size(_o);
}
#define ALIFOBJECT_LENGTH alifObject_size

AlifIntT alifObject_hasLen(AlifObject* _o) { // 79
	return (ALIF_TYPE(_o)->asSequence and ALIF_TYPE(_o)->asSequence->length)
		or (ALIF_TYPE(_o)->asMapping and ALIF_TYPE(_o)->asMapping->length);
}



AlifSizeT alifObject_lengthHint(AlifObject* _o,
	AlifSizeT _defaultValue) { // 91
	AlifObject* hint{}, * result{};
	AlifSizeT res{};
	if (alifObject_hasLen(_o)) {
		res = ALIFOBJECT_LENGTH(_o);
		if (res < 0) {
			AlifThread* thread = _alifThread_get();
			//if (!_alifErr_exceptionMatches(thread, _alifExcTypeError_)) {
			//	return -1;
			//}
			//alifErr_clear(thread);
		}
		else {
			return res;
		}
	}
	hint = alifObject_lookupSpecial(_o, &ALIF_ID(__lengthHint__));
	if (hint == nullptr) {
		//if (alifErr_occurred()) {
		//	return -1;
		//}
		return _defaultValue;
	}
	//result = alifObject_callNoArgs(hint);
	//ALIF_DECREF(hint);
	//if (result == nullptr) {
	//	AlifThread* tstate = _alifThread_get();
	//	//if (alifErr_exceptionMatches(tstate, _alifExcTypeError_)) {
	//	//	alifErr_clear(tstate);
	//	//	return _defaultValue;
	//	//}
	//	return -1;
	//}
	else if (result == ALIF_NOTIMPLEMENTED) {
		ALIF_DECREF(result);
		return _defaultValue;
	}
	if (!ALIFLONG_CHECK(result)) {
		//alifErr_format(_alifExcTypeError_, "__length_hint__ must be an integer, not %.100s",
		//	ALIF_TYPE(result)->name);
		ALIF_DECREF(result);
		return -1;
	}
	res = alifLong_asSizeT(result);
	ALIF_DECREF(result);
	if (res < 0 /*and alifErr_occurred()*/) {
		return -1;
	}
	if (res < 0) {
		//alifErr_format(_alifExcValueError_, "__lengthHint__() should return >= 0");
		return -1;
	}
	return res;
}

AlifObject* alifObject_getItem(AlifObject* _o, AlifObject* _key) { // 150

	if (_o == nullptr or _key == nullptr) {
		return null_error();
	}

	AlifMappingMethods* m_ = ALIF_TYPE(_o)->asMapping;
	if (m_ and m_->subscript) {
		AlifObject* item = m_->subscript(_o, _key);
		return item;
	}

	AlifSequenceMethods* ms = ALIF_TYPE(_o)->asSequence;
	if (ms and ms->item) {
		if (alifIndex_check(_key)) {
			AlifSizeT keyValue;
			keyValue = alifNumber_asSizeT(_key, _alifExcIndexError_);
			return alifSequence_getItem(_o, keyValue);
		}
		else {
			//return type_error("sequence index must "
			//	"be integer, not '%.200s'", _key);
			return nullptr; // temp
		}
	}

	if (ALIFTYPE_CHECK(_o)) {
		AlifObject* meth{}, * result{};

		if ((AlifTypeObject*)_o == &_alifTypeType_) {
			return alif_genericAlias(_o, _key);
		}

		if (alifObject_getOptionalAttr(_o, &ALIF_ID(__classGetItem__), &meth) < 0) {
			return nullptr;
		}
		if (meth and meth != ALIF_NONE) {
			//result = alifObject_callOneArg(meth, _key);
			ALIF_DECREF(meth);
			return result;
		}
		ALIF_XDECREF(meth);
		//alifErr_format(_alifExcTypeError_, "type '%.200s' is not subscriptable",
		//	((AlifTypeObject*)_o)->name);
		return nullptr;
	}

	//return type_error("'%.200s' object is not subscriptable", _o);
	return nullptr; // temp
}



AlifIntT alifMapping_getOptionalItem(AlifObject* _obj,
	AlifObject* _key, AlifObject** _result) { // 203
	if (ALIFDICT_CHECKEXACT(_obj)) {
		return alifDict_getItemRef(_obj, _key, _result);
	}

	*_result = alifObject_getItem(_obj, _key);
	if (*_result) {
		return 1;
	}
	//if (!alifErr_exceptionMatches(_alifExcKeyError_)) {
	//	return -1;
	//}
	//alifErr_clear();
	return 0;
}


AlifIntT alifObject_setItem(AlifObject* _o,
	AlifObject* _key, AlifObject* _value) { // 222
	if (_o == nullptr or _key == nullptr or _value == nullptr) {
		null_error();
		return -1;
	}

	AlifMappingMethods* m_ = ALIF_TYPE(_o)->asMapping;
	if (m_ and m_->assSubscript) {
		AlifIntT res = m_->assSubscript(_o, _key, _value);
		return res;
	}

	if (ALIF_TYPE(_o)->asSequence) {
		if (alifIndex_check(_key)) {
			AlifSizeT keyValue{};
			keyValue = alifNumber_asSizeT(_key, _alifExcIndexError_);
			if (keyValue == -1 /*and alifErr_occurred()*/)
				return -1;
			return alifSequence_setItem(_o, keyValue, _value);
		}
		else if (ALIF_TYPE(_o)->asSequence->assItem) {
			//type_error("sequence index must be "
			//	"integer, not '%.200s'", _key);
			return -1;
		}
	}

	//type_error("'%.200s' object does not support item assignment", _o);
	return -1;
}


AlifObject * _alifNumber_index(AlifObject * _item) { // 1397
	if (_item == nullptr) {
		return null_error();
	}

	if (ALIFLONG_CHECK(_item)) {
		return ALIF_NEWREF(_item);
	}
	if (!alifIndex_check(_item)) {
		//alifErr_format(_alifExcTypeError_,
			//"'%.200s' object cannot be interpreted "
			//"as an integer", ALIF_TYPE(_item)->name);
		return nullptr;
	}

	AlifObject* result = ALIF_TYPE(_item)->asNumber->index(_item);
	if (!result or ALIFLONG_CHECKEXACT(result)) {
		return result;
	}

	if (!ALIFLONG_CHECK(result)) {
		//alifErr_format(_alifExcTypeError_,
			//"__index__ returned non-int (type %.200s)",
			//ALIF_TYPE(result)->tp_name);
		ALIF_DECREF(result);
		return nullptr;
	}
	//if (alifErr_warnFormat(_alifExcDeprecationWarning_, 1,
		//"__index__ returned non-int (type %.200s).  "
		//"The ability to return an instance of a strict subclass of int "
		//"is deprecated, and may be removed in a future version of alif.",
		//ALIF_TYPE(result)->name)) {
		//ALIF_DECREF(result);
		//return nullptr;
	//}
	return result;
}

AlifObject* alifNumber_index(AlifObject* _item) { // 1442
	AlifObject* result = _alifNumber_index(_item);
	if (result != nullptr and !ALIFLONG_CHECKEXACT(result)) {
		ALIF_SETREF(result, _alifLong_copy((AlifLongObject*)result));
	}
	return result;
}

AlifSizeT alifNumber_asSizeT(AlifObject* _item, AlifObject* _err) { // 1455
	AlifSizeT result{};
	AlifObject* runErr{};
	AlifObject* value = _alifNumber_index(_item);
	AlifThread* tState = alifThread_get();

	if (value == nullptr)
		return -1;

	result = alifLong_asSizeT(value);
	if (result != -1)
		goto finish;

	tState = _alifThread_get();
	//runErr = alifErr_occurred(tState);
	if (!runErr) {
		goto finish;
	}

	//if (!alifErr_givenExceptionMatches(runErr, _alifExcOverflowError_)) {
	//	goto finish;
	//}
	//alifErr_clear(tState);

	if (!_err) {
		if (_alifLong_isNegative((AlifLongObject*)value))
			result = ALIF_SIZET_MIN;
		else
			result = ALIF_SIZET_MAX;
	}
	else {
		//alifErr_format(tState, err,
			//"cannot fit '%.200s' into an index-sized integer",
			//ALIF_TYPE(_item)->name);
	}

finish:
	ALIF_DECREF(value);
	return result;
}



AlifIntT alifSequence_check(AlifObject* _s) { // 1668
	if (ALIFDICT_CHECK(_s))
		return 0;
	return ALIF_TYPE(_s)->asSequence and
		ALIF_TYPE(_s)->asSequence->item != nullptr;
}


AlifObject* alifSequence_getItem(AlifObject* _s, AlifSizeT _i) { // 1829
	if (_s == nullptr) {
		return null_error();
	}

	AlifSequenceMethods* m_ = ALIF_TYPE(_s)->asSequence;
	if (m_ and m_->item) {
		if (_i < 0) {
			if (m_->length) {
				AlifSizeT l_ = (*m_->length)(_s);
				if (l_ < 0) {
					return nullptr;
				}
				_i += l_;
			}
		}
		AlifObject* res_ = m_->item(_s, _i);
		return res_;
	}

	if (ALIF_TYPE(_s)->asMapping and ALIF_TYPE(_s)->asMapping->subscript) {
		//return type_error("%.200s is not a sequence", _s);
		return nullptr; // temp
	}
	//return type_error("'%.200s' object does not support indexing", _s);
	return nullptr; // temp
}

AlifIntT alifSequence_setItem(AlifObject* _s, AlifSizeT _i, AlifObject* _o) { // 1880
	if (_s == nullptr) {
		null_error();
		return -1;
	}

	AlifSequenceMethods* m_ = ALIF_TYPE(_s)->asSequence;
	if (m_ and m_->assItem) {
		if (_i < 0) {
			if (m_->length) {
				AlifSizeT l = (*m_->length)(_s);
				if (l < 0) {
					return -1;
				}
				_i += l;
			}
		}
		AlifIntT res = m_->assItem(_s, _i, _o);
		return res;
	}

	if (ALIF_TYPE(_s)->asMapping and ALIF_TYPE(_s)->asMapping->assSubscript) {
		//type_error("%.200s is not a sequence", _s);
		return -1;
	}
	//type_error("'%.200s' object does not support item assignment", _s);
	return -1;
}



AlifObject* alifSequence_tuple(AlifObject* v) { // 1992
	AlifObject* it{};  /* iter(v) */
	AlifSizeT n{};             /* guess for result tuple size */
	AlifObject* result = nullptr;
	AlifSizeT j{};

	if (v == nullptr) {
		return null_error();
	}

	if (ALIFTUPLE_CHECKEXACT(v)) {
		return ALIF_NEWREF(v);
	}
	if (ALIFLIST_CHECKEXACT(v))
		return alifList_asTuple(v);

	/* Get iterator. */
	it = alifObject_getIter(v);
	if (it == nullptr)
		return nullptr;

	/* Guess result size and allocate space. */
	n = alifObject_lengthHint(v, 10);
	if (n == -1)
		goto Fail;
	result = alifTuple_new(n);
	if (result == nullptr)
		goto Fail;

	/* Fill the tuple. */
	for (j = 0; ; ++j) {
		AlifObject* item = alifIter_next(it);
		if (item == nullptr) {
			//if (alifErr_occurred())
			//	goto Fail;
			break;
		}
		if (j >= n) {
			size_t newn = (size_t)n;
			/* The over-allocation strategy can grow a bit faster
			   than for lists because unlike lists the
			   over-allocation isn't permanent -- we reclaim
			   the excess before the end of this routine.
			   So, grow by ten and then add 25%.
			*/
			newn += 10u;
			newn += newn >> 2;
			if (newn > ALIF_SIZET_MAX) {
				/* Check for overflow */
				//alifErr_noMemory();
				ALIF_DECREF(item);
				goto Fail;
			}
			n = (AlifSizeT)newn;
			if (alifTuple_resize(&result, n) != 0) {
				ALIF_DECREF(item);
				goto Fail;
			}
		}
		ALIFTUPLE_SET_ITEM(result, j, item);
	}

	/* Cut tuple back if guess was too large. */
	if (j < n and alifTuple_resize(&result, j) != 0)
		goto Fail;

	ALIF_DECREF(it);
	return result;

Fail:
	ALIF_XDECREF(result);
	ALIF_DECREF(it);
	return nullptr;
}




AlifIntT alifMapping_check(AlifObject* _o) { // 2263
	return _o and ALIF_TYPE(_o)->asMapping and
		ALIF_TYPE(_o)->asMapping->subscript;
}


AlifSizeT alifMapping_size(AlifObject* _o) { // 2270
	if (_o == nullptr) {
		null_error();
		return -1;
	}

	AlifMappingMethods* m_ = ALIF_TYPE(_o)->asMapping;
	if (m_ and m_->length) {
		AlifSizeT len = m_->length(_o);
		return len;
	}

	if (ALIF_TYPE(_o)->asSequence and ALIF_TYPE(_o)->asSequence->length) {
		//type_error("%.200s is not a mapping", _o);
		return -1;
	}
	//type_error("object of type '%.200s' has no len()", _o);
	return -1;
}




AlifObject* alifObject_getIter(AlifObject* _o) { // 2809
	AlifTypeObject* t_ = ALIF_TYPE(_o);
	GetIterFunc f_{};

	f_ = t_->iter;
	if (f_ == nullptr) {
		if (alifSequence_check(_o))
			return alifSeqIter_new(_o);
		//return type_error("'%.200s' object is not iterable", _o);
		return nullptr;
	}
	else {
		AlifObject* res = (*f_)(_o);
		if (res != nullptr /*and !alifIter_check(res)*/) {
			//alifErr_format(_alifExcTypeError_,
			//	"iter() returned non-iterator "
			//	"of type '%.100s'",
			//	ALIF_TYPE(res)->name);
			ALIF_SETREF(res, nullptr);
		}
		return res;
	}
}

static AlifIntT iter_next(AlifObject* _iter, AlifObject** _item) { // 2871
	IterNextFunc iterNext = ALIF_TYPE(_iter)->iterNext;
	if ((*_item = iterNext(_iter))) {
		return 1;
	}

	AlifThread* threadState = _alifThread_get();
	//if (!alifErr_occurred(threadState)) {
		//return 0;
	//}
	//if (alifErr_exceptionMatches(threadState, _alifExcStopIteration_)) {
		//alifErr_clear(threadState);
		//return 0;
	//}

	return 0; // temp
	return -1;
}


AlifObject* alifIter_next(AlifObject* _iter) { // 2921
	AlifObject* item{};
	iter_next(_iter, &item);
	return item;
}
