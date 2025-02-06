#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Call.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Object.h"
#include "AlifCore_Long.h"
#include "AlifCore_Errors.h"
#include "AlifCore_State.h"
#include "AlifCore_UnionObject.h"




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
			keyValue = alifNumber_asSizeT(_key, nullptr/*_alifExcIndexError_*/); // need review
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
			keyValue = alifNumber_asSizeT(_key, nullptr/*_alifExcIndexError_*/); // need review
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


AlifIntT alifObject_delItem(AlifObject* _o, AlifObject* _key) { // 256
	if (_o == nullptr or _key == nullptr) {
		null_error();
		return -1;
	}

	AlifMappingMethods* m = ALIF_TYPE(_o)->asMapping;
	if (m and m->assSubscript) {
		AlifIntT res = m->assSubscript(_o, _key, (AlifObject*)nullptr);
		return res;
	}

	if (ALIF_TYPE(_o)->asSequence) {
		if (alifIndex_check(_key)) {
			AlifSizeT keyValue{};
			keyValue = alifNumber_asSizeT(_key, nullptr/*_alifExcIndexError_*/);
			if (keyValue == -1 /*and alifErr_occurred()*/)
				return -1;
			return alifSequence_delItem(_o, keyValue);
		}
		else if (ALIF_TYPE(_o)->asSequence->assItem) {
			//type_error("sequence index must be "
			//	"integer, not '%.200s'", key);
			return -1;
		}
	}

	//type_error("'%.200s' object does not support item deletion", o);
	return -1;
}



AlifIntT alifObject_getBuffer(AlifObject* obj, AlifBuffer* view, AlifIntT flags) { // 425
	if (flags != ALIFBUF_SIMPLE) {  /* fast path */
		if (flags == ALIFBUF_READ or flags == ALIFBUF_WRITE) {
			//ALIFERR_BADINTERNALCALL();
			return -1;
		}
	}
	AlifBufferProcs* pb = ALIF_TYPE(obj)->asBuffer;

	if (pb == nullptr or pb->getBuffer == nullptr) {
		//alifErr_format(_alifExcTypeError_,
		//	"a bytes-like object is required, not '%.100s'",
		//	ALIF_TYPE(obj)->name);
		return -1;
	}
	AlifIntT res = (*pb->getBuffer)(obj, view, flags);
	return res;
}






AlifIntT alifBuffer_fillInfo(AlifBuffer* _view, AlifObject* _obj, void* _buf,
	AlifSizeT _len, AlifIntT _readonly, AlifIntT _flags) { // 760
	if (_view == nullptr) {
		//alifErr_setString(_alifExcBufferError_,
		//	"alifBuffer_fillInfo: view==nullptr argument is obsolete");
		return -1;
	}

	if (_flags != ALIFBUF_SIMPLE) {  /* fast path */
		if (_flags == ALIFBUF_READ or _flags == ALIFBUF_WRITE) {
			//ALIFERR_BADINTERNALCALL();
			return -1;
		}
		if (((_flags & ALIFBUF_WRITABLE) == ALIFBUF_WRITABLE) and
			(_readonly == 1)) {
			//alifErr_setString(_alifExcBufferError_,
			//	"Object is not writable.");
			return -1;
		}
	}

	_view->obj = ALIF_XNEWREF(_obj);
	_view->buf = _buf;
	_view->len = _len;
	_view->readonly = _readonly;
	_view->itemSize = 1;
	_view->format = nullptr;
	if ((_flags & ALIFBUF_FORMAT) == ALIFBUF_FORMAT)
		_view->format = (char*)"B"; //* alif
	_view->nDim = 1;
	_view->shape = nullptr;
	if ((_flags & ALIFBUF_ND) == ALIFBUF_ND)
		_view->shape = &(_view->len);
	_view->strides = nullptr;
	if ((_flags & ALIFBUF_STRIDES) == ALIFBUF_STRIDES)
		_view->strides = &(_view->itemSize);
	_view->subOffsets = nullptr;
	_view->internal = nullptr;
	return 0;
}





void alifBuffer_release(AlifBuffer* _view) { // 803
	AlifObject* obj = _view->obj;
	AlifBufferProcs* pb{};
	if (obj == nullptr) return;
	pb = ALIF_TYPE(obj)->asBuffer;
	if (pb and pb->releaseBuffer) {
		pb->releaseBuffer(obj, _view);
	}
	_view->obj = nullptr;
	ALIF_DECREF(obj);
}


AlifObject* alifObject_format(AlifObject* _obj, AlifObject* _formatSpec) { // 839
	AlifObject* meth{};
	AlifObject* empty = nullptr;
	AlifObject* result = nullptr;

	if (_formatSpec != nullptr and !ALIFUSTR_CHECK(_formatSpec)) {
		//alifErr_format(_alifExcSystemError_,
		//	"Format specifier must be a string, not %.200s",
		//	ALIF_TYPE(_formatSpec)->name);
		return nullptr;
	}

	/* Fast path for common types. */
	if (_formatSpec == nullptr or ALIFUSTR_GET_LENGTH(_formatSpec) == 0) {
		if (ALIFUSTR_CHECKEXACT(_obj)) {
			return ALIF_NEWREF(_obj);
		}
		if (ALIFLONG_CHECKEXACT(_obj)) {
			return alifObject_str(_obj);
		}
		//* todo
		// لماذا العدد العشري لا يضف الى التنفيذ السريع هنا !!
	}

	/* If no format_spec is provided, use an empty string */
	if (_formatSpec == nullptr) {
		empty = alifUStr_new(0, 0);
		_formatSpec = empty;
	}

	/* Find the (unbound!) __format__ method */
	meth = alifObject_lookupSpecial(_obj, &ALIF_ID(__format__));
	if (meth == nullptr) {
		AlifThread* thread = _alifThread_get();
		//if (!_alifErr_occurred(tstate)) {
		//	_alifErr_format(tstate, _alifExcTypeError_,
		//		"Type %.100s doesn't define __format__",
		//		ALIF_TYPE(_obj)->name);
		//}
		goto done;
	}

	/* And call it. */
	result = alifObject_callOneArg(meth, _formatSpec);
	ALIF_DECREF(meth);

	if (result and !ALIFUSTR_CHECK(result)) {
		//alifErr_format(_alifExcTypeError_,
		//	"__format__ must return a str, not %.200s",
		//	ALIF_TYPE(result)->name);
		ALIF_SETREF(result, nullptr);
		goto done;
	}

done:
	ALIF_XDECREF(empty);
	return result;
}


#define NB_SLOT(_x) offsetof(AlifNumberMethods, _x) // 910
#define NB_BINOP(_methods, _slot) \
        (*(BinaryFunc*)(& ((char*)_methods)[_slot]))
#define NB_TERNOP(_methods, _slot) \
        (*(TernaryFunc*)(&((char*)_methods)[_slot]))



static AlifObject* binary_op1(AlifObject* _v, AlifObject* _w, const AlifIntT _opSlot) { // 926
	BinaryFunc slotV{};
	if (ALIF_TYPE(_v)->asNumber != nullptr) {
		slotV = NB_BINOP(ALIF_TYPE(_v)->asNumber, _opSlot);
	}
	else {
		slotV = nullptr;
	}

	BinaryFunc slotW{};
	if (!ALIF_IS_TYPE(_w, ALIF_TYPE(_v)) and ALIF_TYPE(_w)->asNumber != nullptr) {
		slotW = NB_BINOP(ALIF_TYPE(_w)->asNumber, _opSlot);
		if (slotW == slotV) {
			slotW = nullptr;
		}
	}
	else {
		slotW = nullptr;
	}

	if (slotV) {
		AlifObject* x{};
		if (slotW and alifType_isSubType(ALIF_TYPE(_w), ALIF_TYPE(_v))) {
			x = slotW(_v, _w);
			if (x != ALIF_NOTIMPLEMENTED)
				return x;
			ALIF_DECREF(x); /* can't do it */
			slotW = nullptr;
		}
		x = slotV(_v, _w);
		if (x != ALIF_NOTIMPLEMENTED) {
			return x;
		}
		ALIF_DECREF(x); /* can't do it */
	}
	if (slotW) {
		AlifObject* x = slotW(_v, _w);
		if (x != ALIF_NOTIMPLEMENTED) {
			return x;
		}
		ALIF_DECREF(x); /* can't do it */
	}
	return ALIF_NOTIMPLEMENTED;
}

#define BINARY_OP1(_v, _w, _opSlot) binary_op1(_v, _w, _opSlot) // 980

static AlifObject* binary_op(AlifObject* _v, AlifObject* _w,
	const AlifIntT _opSlot) { // 997
	AlifObject* result = BINARY_OP1(_v, _w, _opSlot);
	if (result == ALIF_NOTIMPLEMENTED) {
		ALIF_DECREF(result);
		//return binOp_typeError(_v, _w, _opName);
		return nullptr; //* alif
	}
	return result;
}


static AlifObject* ternary_op(AlifObject* _v, AlifObject* _w, AlifObject* _z,
	const AlifIntT _opSlot/*, const char* _opName */) { // 1016
	AlifNumberMethods* mv = ALIF_TYPE(_v)->asNumber;
	AlifNumberMethods* mw = ALIF_TYPE(_w)->asNumber;

	TernaryFunc slotV{};
	if (mv != nullptr) {
		slotV = NB_TERNOP(mv, _opSlot);
	}
	else {
		slotV = nullptr;
	}

	TernaryFunc slotW{};
	if (!ALIF_IS_TYPE(_w, ALIF_TYPE(_v)) and mw != nullptr) {
		slotW = NB_TERNOP(mw, _opSlot);
		if (slotW == slotV) {
			slotW = nullptr;
		}
	}
	else {
		slotW = nullptr;
	}

	if (slotV) {
		AlifObject* x{};
		if (slotW and alifType_isSubType(ALIF_TYPE(_w), ALIF_TYPE(_v))) {
			x = slotW(_v, _w, _z);
			if (x != ALIF_NOTIMPLEMENTED) {
				return x;
			}
			ALIF_DECREF(x); /* can't do it */
			slotW = nullptr;
		}
		x = slotV(_v, _w, _z);
		if (x != ALIF_NOTIMPLEMENTED) {
			return x;
		}
		ALIF_DECREF(x); /* can't do it */
	}
	if (slotW) {
		AlifObject* x = slotW(_v, _w, _z);
		if (x != ALIF_NOTIMPLEMENTED) {
			return x;
		}
		ALIF_DECREF(x); /* can't do it */
	}

	AlifNumberMethods* mz = ALIF_TYPE(_z)->asNumber;
	if (mz != nullptr) {
		TernaryFunc slotz = NB_TERNOP(mz, _opSlot);
		if (slotz == slotV or slotz == slotW) {
			slotz = nullptr;
		}
		if (slotz) {
			AlifObject* x = slotz(_v, _w, _z);
			if (x != ALIF_NOTIMPLEMENTED) {
				return x;
			}
			ALIF_DECREF(x); /* can't do it */
		}
	}

	if (_z == ALIF_NONE) {
		//alifErr_format(
		//	_alifExcTypeError_,
		//	"unsupported operand type(s) for %.100s: "
		//	"'%.100s' and '%.100s'",
		//	_opName,
		//	ALIF_TYPE(_v)->name,
		//	ALIF_TYPE(_w)->name);
	}
	else {
		//alifErr_format(
		//	_alifExcTypeError_,
		//	"unsupported operand type(s) for %.100s: "
		//	"'%.100s', '%.100s', '%.100s'",
		//	_opName,
		//	ALIF_TYPE(_v)->name,
		//	ALIF_TYPE(_w)->name,
		//	ALIF_TYPE(_z)->name);
	}
	return nullptr;
}


 // 1110
#define BINARY_FUNC(_func, _op) \
    AlifObject* _func(AlifObject *_v, AlifObject *_w) { \
        return binary_op(_v, _w, NB_SLOT(_op)); \
    }

BINARY_FUNC(alifNumber_or, or_) // 1116
BINARY_FUNC(alifNumber_xor, xor_)
BINARY_FUNC(alifNumber_and, and_)
BINARY_FUNC(alifNumber_lshift, lshift)
BINARY_FUNC(alifNumber_rshift, rshift)
BINARY_FUNC(alifNumber_subtract, subtract)
BINARY_FUNC(alifNumber_divmod, divmod)


AlifObject* alifNumber_add(AlifObject* _v, AlifObject* _w) { // 1124
	AlifObject* result = BINARY_OP1(_v, _w, NB_SLOT(add_));
	if (result != ALIF_NOTIMPLEMENTED) {
		return result;
	}
	ALIF_DECREF(result);

	AlifSequenceMethods* m = ALIF_TYPE(_v)->asSequence;
	if (m and m->concat) {
		result = (*m->concat)(_v, _w);
		return result;
	}

	//return binOp_typeError(_v, _w, "+");
	return nullptr; //* alif
}

static AlifObject* sequence_repeat(SizeArgFunc _repeatFunc, AlifObject* _seq, AlifObject* _n) { // 1143
	AlifSizeT count{};
	if (alifIndex_check(_n)) {
		count = alifNumber_asSizeT(_n, nullptr /*_alifExcOverflowError_*/);
		if (count == -1 /*and alifErr_occurred()*/) {
			return nullptr;
		}
	}
	else {
		//return type_error("can't multiply sequence by "
		//	"non-int of type '%.200s'", _n);
		return nullptr; //* alif
	}
	AlifObject* res = (*_repeatFunc)(_seq, count);
	return res;
}


AlifObject* alifNumber_multiply(AlifObject* _v, AlifObject* _w) { // 1162
	AlifObject* result = BINARY_OP1(_v, _w, NB_SLOT(multiply));
	if (result == ALIF_NOTIMPLEMENTED) {
		AlifSequenceMethods* mv = ALIF_TYPE(_v)->asSequence;
		AlifSequenceMethods* mw = ALIF_TYPE(_w)->asSequence;
		ALIF_DECREF(result);
		if (mv and mv->repeat) {
			return sequence_repeat(mv->repeat, _v, _w);
		}
		else if (mw and mw->repeat) {
			return sequence_repeat(mw->repeat, _w, _v);
		}
		//result = binop_typeError(_v, _w, "*");
		result = nullptr; //* alif
	}
	return result;
}


BINARY_FUNC(alifNumber_floorDivide, floorDivide) // 1182
BINARY_FUNC(alifNumber_trueDivide, trueDivide) // 1183
BINARY_FUNC(alifNumber_remainder, remainder)

AlifObject* alifNumber_power(AlifObject* _v, AlifObject* _w, AlifObject* _z) { // 1186
	return ternary_op(_v, _w, _z, NB_SLOT(power));
}

AlifObject* _alifNumber_powerNoMod(AlifObject* _lhs, AlifObject* _rhs) { // 1192
	return alifNumber_power(_lhs, _rhs, ALIF_NONE);
}

static AlifObject* binary_iop1(AlifObject* _v, AlifObject* _w,
	const AlifIntT _iopSlot, const AlifIntT _opSlot) { // 1214
	AlifNumberMethods* mv = ALIF_TYPE(_v)->asNumber;
	if (mv != nullptr) {
		BinaryFunc slot = NB_BINOP(mv, _iopSlot);
		if (slot) {
			AlifObject* x = (slot)(_v, _w);
			if (x != ALIF_NOTIMPLEMENTED) {
				return x;
			}
			ALIF_DECREF(x);
		}
	}
	return binary_op1(_v, _w, _opSlot);
}

#define BINARY_IOP1(_v, _w, _iopSlot, _opSlot) binary_iop1(_v, _w, _iopSlot, _opSlot) // 1241

static AlifObject* binary_iop(AlifObject* _v, AlifObject* _w,
	const AlifIntT _iopSlot, const AlifIntT _opSlot) { // 1246
	AlifObject* result = BINARY_IOP1(_v, _w, _iopSlot, _opSlot);
	if (result == ALIF_NOTIMPLEMENTED) {
		ALIF_DECREF(result);
		//return binOp_typeError(_v, _w, opName);
	}
	return result;
}

static AlifObject* ternary_iOp(AlifObject* _v, AlifObject* _w,
	AlifObject* _z, const AlifIntT _iopSlot,
	const AlifIntT _opSlot/*, const char* _opName*/) { // 1258
	AlifNumberMethods* mv = ALIF_TYPE(_v)->asNumber;
	if (mv != nullptr) {
		TernaryFunc slot = NB_TERNOP(mv, _iopSlot);
		if (slot) {
			AlifObject* x = (slot)(_v, _w, _z);
			if (x != ALIF_NOTIMPLEMENTED) {
				return x;
			}
			ALIF_DECREF(x);
		}
	}
	return ternary_op(_v, _w, _z, _opSlot/*, _opName*/);
}

 // 1276
#define INPLACE_BINOP(_func, _iop, _op) \
    AlifObject * _func(AlifObject *_v, AlifObject *_w) { \
        return binary_iop(_v, _w, NB_SLOT(_iop), NB_SLOT(_op)); \
    }

INPLACE_BINOP(alifNumber_inPlaceOr, inplaceOr, or_)
INPLACE_BINOP(alifNumber_inPlaceXor, inplaceXor, xor_)
INPLACE_BINOP(alifNumber_inPlaceAnd, inplaceAnd, and_)
INPLACE_BINOP(alifNumber_inPlaceLshift, inplaceLshift, lshift)
INPLACE_BINOP(alifNumber_inPlaceRshift, inplaceRshift, rshift)
INPLACE_BINOP(alifNumber_inPlaceSubtract, inplaceSubtract, subtract)
INPLACE_BINOP(alifNumber_inPlaceFloorDivide, inplaceFloorDivide, floorDivide)
INPLACE_BINOP(alifNumber_inPlaceTrueDivide, inplaceTrueDivide, trueDivide)
INPLACE_BINOP(alifNumber_inPlaceRemainder, inplaceRemainder, remainder) // 1291


AlifObject* alifNumber_inPlaceAdd(AlifObject* _v, AlifObject* _w) { // 1293
	AlifObject* result = BINARY_IOP1(_v, _w, NB_SLOT(inplaceAdd), NB_SLOT(add_));
	if (result == ALIF_NOTIMPLEMENTED) {
		AlifSequenceMethods* m = ALIF_TYPE(_v)->asSequence;
		ALIF_DECREF(result);
		if (m != nullptr) {
			BinaryFunc func = m->inplaceConcat;
			if (func == nullptr)
				func = m->concat;
			if (func != nullptr) {
				result = func(_v, _w);
				return result;
			}
		}
		//result = binOp_typeError(_v, _w, "+=");
	}
	return result;
}

AlifObject* alifNumber_inPlaceMultiply(AlifObject* _v, AlifObject* _w) { // 1316
	AlifObject* result = BINARY_IOP1(_v, _w, NB_SLOT(inplaceMultiply), NB_SLOT(multiply));
	if (result == ALIF_NOTIMPLEMENTED) {
		SizeArgFunc f = nullptr;
		AlifSequenceMethods* mv = ALIF_TYPE(_v)->asSequence;
		AlifSequenceMethods* mw = ALIF_TYPE(_w)->asSequence;
		ALIF_DECREF(result);
		if (mv != nullptr) {
			f = mv->inplaceRepeat;
			if (f == nullptr)
				f = mv->repeat;
			if (f != nullptr)
				return sequence_repeat(f, _v, _w);
		}
		else if (mw != nullptr) {
			if (mw->repeat)
				return sequence_repeat(mw->repeat, _w, _v);
		}
		//result = binOp_typeError(_v, _w, "*=");
	}
	return result;
}

AlifObject* alifNumber_inPlacePower(AlifObject* _v, AlifObject* _w, AlifObject* _z) { // 1345
	return ternary_iOp(_v, _w, _z, NB_SLOT(inplacePower), NB_SLOT(power));
}

AlifObject* _alifNumber_inPlacePowerNoMod(AlifObject* _lhs, AlifObject* _rhs) { // 1352
	return alifNumber_inPlacePower(_lhs, _rhs, ALIF_NONE);
}

 // 1361
#define UNARY_FUNC(_func, _op, _methName, _descr)                           \
    AlifObject* _func(AlifObject *o) {                                                  \
        if (o == nullptr) {                                                 \
            return null_error();                                         \
        }                                                                \
                                                                         \
        AlifNumberMethods *m = ALIF_TYPE(o)->asNumber;                   \
        if (m and m->_op) {                                                \
            AlifObject *res = (*m->_op)(o);                                 \
            return res;                                                  \
        }                                                                \
                                                                         \
        /*return type_error("bad operand type for "_descr": '%.200s'", o);*/  \
        return nullptr; \
    }

UNARY_FUNC(alifNumber_negative, negative, __neg__, "unary -")
UNARY_FUNC(alifNumber_positive, positive, __pos__, "unary +")
UNARY_FUNC(alifNumber_invert, invert, __invert__, "unary ~")
UNARY_FUNC(alifNumber_absolute, absolute, __abs__, "abs()")
UNARY_FUNC(alifNumber_sqrt, sqrt, __sqrt__, "sqrt()") //* alif


AlifObject* _alifNumber_index(AlifObject * _item) { // 1397
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
	AlifThread* thread = alifThread_get();

	if (value == nullptr)
		return -1;

	result = alifLong_asSizeT(value);
	if (result != -1)
		goto finish;

	thread = _alifThread_get();
	runErr = _alifErr_occurred(thread);
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


AlifObject* alifNumber_float(AlifObject* _o) { // 1588
	if (_o == nullptr) {
		return null_error();
	}

	if (ALIFFLOAT_CHECKEXACT(_o)) {
		return ALIF_NEWREF(_o);
	}

	AlifNumberMethods* m = ALIF_TYPE(_o)->asNumber;
	if (m and m->float_) {
		AlifObject* res = m->float_(_o);
		if (!res or ALIFFLOAT_CHECKEXACT(res)) {
			return res;
		}

		if (!ALIFFLOAT_CHECK(res)) {
			//alifErr_format(_alifExcTypeError_,
			//	"%.50s.__float__ returned non-float (type %.50s)",
			//	ALIF_TYPE(o)->name, ALIF_TYPE(res)->name);
			ALIF_DECREF(res);
			return nullptr;
		}
		//if (alifErr_warnFormat(_alifExcDeprecationWarning_, 1,
		//	"%.50s.__float__ returned non-float (type %.50s).  "
		//	"The ability to return an instance of a strict subclass of float "
		//	"is deprecated, and may be removed in a future version of Alif.",
		//	ALIF_TYPE(o)->name, ALIF_TYPE(res)->name)) {
		//	ALIF_DECREF(res);
		//	return nullptr;
		//}
		double val = ALIFFLOAT_AS_DOUBLE(res);
		ALIF_DECREF(res);
		return alifFloat_fromDouble(val);
	}

	if (m and m->index) {
		AlifObject* res = _alifNumber_index(_o);
		if (!res) {
			return nullptr;
		}
		double val = alifLong_asDouble(res);
		ALIF_DECREF(res);
		if (val == -1.0 and alifErr_occurred()) {
			return nullptr;
		}
		return alifFloat_fromDouble(val);
	}

	if (ALIFFLOAT_CHECK(_o)) {
		return alifFloat_fromDouble(ALIFFLOAT_AS_DOUBLE(_o));
	}
	return alifFloat_fromString(_o);
}



AlifIntT alifSequence_check(AlifObject* _s) { // 1668
	if (ALIFDICT_CHECK(_s))
		return 0;
	return ALIF_TYPE(_s)->asSequence and
		ALIF_TYPE(_s)->asSequence->item != nullptr;
}


AlifSizeT alifSequence_size(AlifObject* _s) { // 1677
	if (_s == nullptr) {
		null_error();
		return -1;
	}

	AlifSequenceMethods* m = ALIF_TYPE(_s)->asSequence;
	if (m and m->length) {
		AlifSizeT len = m->length(_s);
		return len;
	}

	if (ALIF_TYPE(_s)->asMapping and ALIF_TYPE(_s)->asMapping->length) {
		//type_error("%.200s is not a sequence", s);
		return -1;
	}
	//type_error("object of type '%.200s' has no len()", s);
	return -1;
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


AlifIntT alifSequence_delItem(AlifObject* _s, AlifSizeT _i) { // 1913
	if (_s == nullptr) {
		null_error();
		return -1;
	}

	AlifSequenceMethods* m = ALIF_TYPE(_s)->asSequence;
	if (m and m->assItem) {
		if (_i < 0) {
			if (m->length) {
				AlifSizeT l = (*m->length)(_s);
				if (l < 0) {
					return -1;
				}
				_i += l;
			}
		}
		AlifIntT res = m->assItem(_s, _i, (AlifObject*)nullptr);
		return res;
	}

	if (ALIF_TYPE(_s)->asMapping and ALIF_TYPE(_s)->asMapping->assSubscript) {
		//type_error("%.200s is not a sequence", _s);
		return -1;
	}
	//type_error("'%.200s' object doesn't support item deletion", _s);
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

AlifObject* alifSequence_list(AlifObject* _v) { // 2075
	AlifObject* result{};  /* result list */
	AlifObject* rv{};          /* return value from alifList_extend */

	if (_v == nullptr) {
		return null_error();
	}

	result = alifList_new(0);
	if (result == nullptr)
		return nullptr;

	rv = _alifList_extend((AlifListObject*)result, _v);
	if (rv == nullptr) {
		ALIF_DECREF(result);
		return nullptr;
	}
	ALIF_DECREF(rv);
	return result;
}

AlifObject* alifSequence_fast(AlifObject* _v, const char* _m) { // 2098
	AlifObject* it{};

	if (_v == nullptr) {
		return null_error();
	}

	if (ALIFLIST_CHECKEXACT(_v) or ALIFTUPLE_CHECKEXACT(_v)) {
		return ALIF_NEWREF(_v);
	}

	it = alifObject_getIter(_v);
	if (it == nullptr) {
		AlifThread* thread = _alifThread_get();
		//if (alifErr_exceptionMatches(thread, _alifExcTypeError_)) {
		//	alifErr_setString(thread, _alifExcTypeError_, _m);
		//}
		return nullptr;
	}

	_v = alifSequence_list(it);
	ALIF_DECREF(it);

	return _v;
}


AlifIntT _alifSequence_iterSearch(AlifObject* seq, AlifObject* obj, AlifIntT operation) { // 2132
	AlifSizeT n{};
	AlifIntT wrapped{};
	AlifObject* it{};  /* iter(seq) */

	if (seq == nullptr or obj == nullptr) {
		null_error();
		return -1;
	}

	it = alifObject_getIter(seq);
	if (it == nullptr) {
		//if (alifErr_exceptionMatches(_alifExcTypeError_)) {
		//	if (operation == ALIF_ITERSEARCH_CONTAINS) {
		//		type_error(
		//			"argument of type '%.200s' is not a container or iterable",
		//			seq
		//		);
		//	}
		//	else {
		//		type_error("argument of type '%.200s' is not iterable", seq);
		//	}
		//}
		return -1;
	}

	n = wrapped = 0;
	for (;;) {
		int cmp;
		AlifObject* item = alifIter_next(it);
		if (item == nullptr) {
			if (alifErr_occurred())
				goto Fail;
			break;
		}

		cmp = alifObject_richCompareBool(item, obj, ALIF_EQ);
		ALIF_DECREF(item);
		if (cmp < 0)
			goto Fail;
		if (cmp > 0) {
			switch (operation) {
			case ALIF_ITERSEARCH_COUNT:
				if (n == ALIF_SIZET_MAX) {
					//alifErr_setString(_alifExcOverflowError_,
					//	"count exceeds C integer size");
					goto Fail;
				}
				++n;
				break;

			case ALIF_ITERSEARCH_INDEX:
				if (wrapped) {
					//alifErr_setString(_alifExcOverflowError_,
					//	"index exceeds C integer size");
					goto Fail;
				}
				goto Done;

			case ALIF_ITERSEARCH_CONTAINS:
				n = 1;
				goto Done;

			default:
				ALIF_UNREACHABLE();
			}
		}

		if (operation == ALIF_ITERSEARCH_INDEX) {
			if (n == ALIF_SIZET_MAX)
				wrapped = 1;
			++n;
		}
	}

	if (operation != ALIF_ITERSEARCH_INDEX)
		goto Done;

	//alifErr_setString(_alifExcValueError_,
	//	"sequence.index(x): x not in sequence");
	/* fall into failure code */
Fail:
	n = -1;
	/* fall through */
Done:
	ALIF_DECREF(it);
	return n;

}


AlifIntT alifSequence_contains(AlifObject* _seq, AlifObject* _ob) { // 2234
	AlifSequenceMethods* sqm = ALIF_TYPE(_seq)->asSequence;
	if (sqm != nullptr and sqm->contains != nullptr) {
		AlifIntT res = (*sqm->contains)(_seq, _ob);
		return res;
	}
	AlifSizeT result = _alifSequence_iterSearch(_seq, _ob, ALIF_ITERSEARCH_CONTAINS);
	return ALIF_SAFE_DOWNCAST(result, AlifSizeT, AlifIntT);
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

AlifIntT alifMapping_setItemString(AlifObject* o,
	const char* key, AlifObject* value) { // 2337
	AlifObject* okey{};
	AlifIntT r{};

	if (key == nullptr) {
		null_error();
		return -1;
	}

	okey = alifUStr_fromString(key);
	if (okey == nullptr)
		return -1;
	r = alifObject_setItem(o, okey, value);
	ALIF_DECREF(okey);
	return r;
}


static AlifObject* methodOutput_asList(AlifObject* o, AlifObject* meth) { // 2427
	AlifObject* it{}, * result{}, * meth_output{};

	meth_output = alifObject_callMethodNoArgs(o, meth);
	if (meth_output == nullptr or ALIFLIST_CHECKEXACT(meth_output)) {
		return meth_output;
	}
	it = alifObject_getIter(meth_output);
	if (it == nullptr) {
		AlifThread* tstate = _alifThread_get();
		//if (_alifErr_exceptionMatches(tstate, _alifExcTypeError_)) {
		//	_alifErr_format(tstate, _alifExcTypeError_,
		//		"%.200s.%U() returned a non-iterable (type %.200s)",
		//		ALIF_TYPE(o)->_name,
		//		meth,
		//		ALIF_TYPE(meth_output)->name);
		//}
		ALIF_DECREF(meth_output);
		return nullptr;
	}
	ALIF_DECREF(meth_output);
	result = alifSequence_list(it);
	ALIF_DECREF(it);
	return result;
}


AlifObject* alifMapping_keys(AlifObject* _o) { // 2456
	if (_o == nullptr) {
		return null_error();
	}
	if (ALIFDICT_CHECKEXACT(_o)) {
		return alifDict_keys(_o);
	}
	return methodOutput_asList(_o, &ALIF_ID(keys));
}


static AlifObject* abstract_getBases(AlifObject* _cls) { // 2518
	AlifObject* bases{};

	(void)alifObject_getOptionalAttr(_cls, &ALIF_ID(__bases__), &bases);
	if (bases != nullptr and !ALIFTUPLE_CHECK(bases)) {
		ALIF_DECREF(bases);
		return nullptr;
	}
	return bases;
}


static AlifIntT abstract_isSubclass(AlifObject* _derived, AlifObject* _cls) { // 2532
	AlifObject* bases = nullptr;
	AlifSizeT i{}, n{};
	AlifIntT r = 0;

	while (1) {
		if (_derived == _cls) {
			ALIF_XDECREF(bases);
			return 1;
		}
		ALIF_XSETREF(bases, abstract_getBases(_derived));
		if (bases == nullptr) {
			if (alifErr_occurred())
				return -1;
			return 0;
		}
		n = ALIFTUPLE_GET_SIZE(bases);
		if (n == 0) {
			ALIF_DECREF(bases);
			return 0;
		}
		if (n == 1) {
			_derived = ALIFTUPLE_GET_ITEM(bases, 0);
			continue;
		}
		break;
	}
	if (_alif_enterRecursiveCall(" in __isSubclass__")) {
		ALIF_DECREF(bases);
		return -1;
	}
	for (i = 0; i < n; i++) {
		r = abstract_isSubclass(ALIFTUPLE_GET_ITEM(bases, i), _cls);
		if (r != 0) {
			break;
		}
	}
	_alif_leaveRecursiveCall();
	ALIF_DECREF(bases);
	return r;
}


static AlifIntT check_class(AlifObject* cls, const char* error) { // 2583
	AlifObject* bases = abstract_getBases(cls);
	if (bases == nullptr) {
		/* Do not mask errors. */
		AlifThread* tstate = _alifThread_get();
		if (!_alifErr_occurred(tstate)) {
			_alifErr_setString(tstate, nullptr /*_alifExcTypeError_*/, error);
		}
		return 0;
	}
	ALIF_DECREF(bases);
	return -1;
}


static  AlifIntT recursive_isSubclass(AlifObject* _derived, AlifObject* _cls) { // 2708
	if (ALIFTYPE_CHECK(_cls) and ALIFTYPE_CHECK(_derived)) {
		/* Fast path (non-recursive) */
		return alifType_isSubType((AlifTypeObject*)_derived, (AlifTypeObject*)_cls);
	}
	if (!check_class(_derived,
		"isSubclass() arg 1 must be a class"))
		return -1;

	if (!ALIFUNION_CHECK(_cls) and !check_class(_cls,
		"isSubclass() arg 2 must be a class,"
		" a tuple of classes, or a union")) {
		return -1;
	}

	return abstract_isSubclass(_derived, _cls);
}


static AlifIntT object_isSubclass(AlifThread* _thread,
	AlifObject* _derived, AlifObject* _cls) { // 2728
	AlifObject* checker{};

	/* We know what type's __subclasscheck__ does. */
	if (ALIFTYPE_CHECKEXACT(_cls)) {
		/* Quick test for an exact match */
		if (_derived == _cls)
			return 1;
		return recursive_isSubclass(_derived, _cls);
	}

	if (ALIFUNION_CHECK(_cls)) {
		_cls = _alifUnion_args(_cls);
	}

	if (ALIFTUPLE_CHECK(_cls)) {

		if (_alif_enterRecursiveCallThread(_thread, " in __subclasscheck__")) {
			return -1;
		}
		AlifSizeT n = ALIFTUPLE_GET_SIZE(_cls);
		AlifIntT r = 0;
		for (AlifSizeT i = 0; i < n; ++i) {
			AlifObject* item = ALIFTUPLE_GET_ITEM(_cls, i);
			r = object_isSubclass(_thread, _derived, item);
			if (r != 0)
				/* either found it, or got an error */
				break;
		}
		_alif_leaveRecursiveCallThread(_thread);
		return r;
	}

	checker = alifObject_lookupSpecial(_cls, &ALIF_ID(__subClassCheck__));
	if (checker != nullptr) {
		AlifIntT ok = -1;
		if (_alif_enterRecursiveCallThread(_thread, " in __subClassCheck__")) {
			ALIF_DECREF(checker);
			return ok;
		}
		AlifObject* res = alifObject_callOneArg(checker, _derived);
		_alif_leaveRecursiveCallThread(_thread);
		ALIF_DECREF(checker);
		if (res != nullptr) {
			ok = alifObject_isTrue(res);
			ALIF_DECREF(res);
		}
		return ok;
	}
	else if (_alifErr_occurred(_thread)) {
		return -1;
	}

	/* Can be reached when infinite recursion happens. */
	return recursive_isSubclass(_derived, _cls);
}

AlifIntT alifObject_isSubclass(AlifObject* _derived, AlifObject* _cls) { // 2788
	AlifThread* thread = _alifThread_get();
	return object_isSubclass(thread, _derived, _cls);
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

AlifIntT alifIter_check(AlifObject* _obj) { // 2853
	AlifTypeObject* tp = ALIF_TYPE(_obj);
	return (tp->iterNext != nullptr
		and tp->iterNext != &_alifObject_nextNotImplemented);
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
