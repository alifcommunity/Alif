#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Object.h"
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

	AlifSequenceMethods* m = ALIF_TYPE(_o)->asSequence;
	if (m and m->length) {
		AlifSizeT len = m->length(_o);
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
	result = alifObject_callNoArgs(hint);
	ALIF_DECREF(hint);
	if (result == nullptr) {
		AlifThread* tstate = _alifThread_get();
		//if (alifErr_exceptionMatches(tstate, _alifExcTypeError_)) {
		//	alifErr_clear(tstate);
		//	return _defaultValue;
		//}
		return -1;
	}
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









AlifIntT alifSequence_check(AlifObject* _s) { // 1668
	if (ALIFDICT_CHECK(_s))
		return 0;
	return ALIF_TYPE(_s)->asSequence and
		ALIF_TYPE(_s)->asSequence->item != nullptr;
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

	AlifMappingMethods* m = ALIF_TYPE(_o)->asMapping;
	if (m and m->length) {
		AlifSizeT len = m->length(_o);
		return len;
	}

	if (ALIF_TYPE(_o)->asSequence and ALIF_TYPE(_o)->asSequence->length) {
		//type_error("%.200s is not a mapping", o);
		return -1;
	}
	//type_error("object of type '%.200s' has no len()", o);
	return -1;
}




AlifObject* alifObject_getIter(AlifObject* _o) { // 2809
	AlifTypeObject* t_ = ALIF_TYPE(_o);
	GetIterFunc f_{};

	f_ = t_->iter;
	if (f_ == nullptr) {
		if (alifSequence_check(_o))
			return alifSeqIter_new(_o);
		//return type_error("'%.200s' object is not iterable", o);
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
