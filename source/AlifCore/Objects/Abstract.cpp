#include "alif.h"

//#include "AlifCore_Abstract.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Object.h"
#include "AlifCore_State.h"




static AlifObject* null_error(void) { // 27
	AlifThread* thread = alifThread_get();
	//if (!alifErr_occurred(thread)) {
	//	alifErr_setString(thread, _alifExcSystemError_,
	//		"null argument to internal routine");
	//}
	return nullptr;
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
	if (j < n and
		alifTuple_resize(&result, j) != 0)
		goto Fail;

	ALIF_DECREF(it);
	return result;

Fail:
	ALIF_XDECREF(result);
	ALIF_DECREF(it);
	return nullptr;
}









AlifObject* alifObject_getIter(AlifObject* _o) { // 2809
	AlifTypeObject* t = ALIF_TYPE(_o);
	GetIterFunc f{};

	f = t->iter;
	if (f == nullptr) {
		if (alifSequence_check(_o))
			return alifSeqIter_new(_o);
		//return type_error("'%.200s' object is not iterable", o);
		return nullptr;
	}
	else {
		AlifObject* res = (*f)(_o);
		if (res != nullptr and !alifIter_check(res)) {
			//alifErr_format(_alifExcTypeError_,
			//	"iter() returned non-iterator "
			//	"of type '%.100s'",
			//	ALIF_TYPE(res)->name);
			ALIF_SETREF(res, nullptr);
		}
		return res;
	}
}
