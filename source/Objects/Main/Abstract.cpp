#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Memory.h"
#include "AlifCore_Integer.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_List.h"



AlifObject* alifObject_getItem(AlifObject* _o, AlifObject* _key) { // 149
	if (_o == nullptr or _key == nullptr) {
		//return null_error();
	}

	AlifMappingMethods* m_ = ALIF_TYPE(_o)->asMapping;
	if (m_ and m_->subScript) {
		AlifObject* item = m_->subScript(_o, _key);
		return item;
	}

	AlifSequenceMethods* ms = ALIF_TYPE(_o)->asSequence;
	if (ms && ms->item_) {
		if (alifIndex_check(_key)) {
			AlifSizeT keyValue{};
			keyValue = alifNumber_asSizeT(_key, nullptr/*alifExcIndexError*/); // need review
			if (keyValue == -1
				//and alifErr_occurred()
				)
				return nullptr;
			return alifSequence_getItem(_o, keyValue);
		}
		else {
			//return type_error("sequence index must "
			//	"be integer, not '%.200s'", _key);
			return nullptr; // 
		}
	}

	if (ALIFTYPE_CHECK(_o)) {
		AlifObject* meth{}, * result{};

		if ((AlifTypeObject*)_o == &_alifTypeType_) {
			return alif_genericAlias(_o, _key);
		}

		AlifObject* name = alifUStr_decodeStringToUTF8(L"__class_getitem__");
		if (alifObject_getOptionalAttr(_o, name, &meth) < 0) {
			return nullptr;
		}
		if (meth && meth != ALIF_NONE) {
			result = alifObject_callOneArg(meth, _key);

		}
	}
}

AlifIntT alifMapping_getOptionalItem(AlifObject* _obj, AlifObject* _key, AlifObject** _result) { // 203
	if (ALIFDICT_CHECKEXACT(_obj)) {
		return alifDict_getItemRef(_obj, _key, _result);
	}

	*_result = alifObject_getItem(_obj, _key);
	if (*_result) {
		return 1;
	}
	//if (!alifErr_exceptionMatches(alifExcKeyError)) {
	//	return -1;
	//}
	//alifErr_clear();
	return 0;

}

int alifObject_setItem(AlifObject* _o, AlifObject* _key, AlifObject* _value)
{
	if (_o == nullptr || _key == nullptr || _value == nullptr) {
		return -1;
	}

	AlifMappingMethods* m_ = ALIF_TYPE(_o)->asMapping;
	if (m_ && m_->assSubScript) {
		int res_ = m_->assSubScript(_o, _key, _value);
		return res_;
	}

	if (ALIF_TYPE(_o)->asSequence) {
		if ((_key->type_->asNumber != nullptr && _key->type_->asNumber->index_ != nullptr)) {
			int64_t keyValue;
			keyValue = alifInteger_asSizeT(_key);
			return alifSequence_setItem(_o, keyValue, _value);
		}
		else if (ALIF_TYPE(_o)->asSequence->assItem) {
			return -1;
		}
	}

	return -1;
}

void alifBuffer_release(AlifBuffer* _view)
{
    AlifObject* obj_ = _view->obj;
    AlifBufferProcs* pb_;
    if (obj_ == nullptr)
        return;
    pb_ = ALIF_TYPE(obj_)->asBuffer;
    if (pb_ && pb_->releaseBuffer) {
        pb_->releaseBuffer(obj_, _view);
    }
    _view->obj = nullptr;
    ALIF_DECREF(obj_);
}

int alifObject_getBuffer(AlifObject* _obj, AlifBuffer* _view, int _flags)
{
    if (_flags != 0) {  /* fast path */
        if (_flags == 0x100 || _flags == 0x200) {
            return -1;
        }
    }
    AlifBufferProcs* pb_ = ALIF_TYPE(_obj)->asBuffer;

    if (pb_ == nullptr || pb_->getBuffer == nullptr) {

        return -1;
    }
    int res_ = (*pb_->getBuffer)(_obj, _view, _flags);
    return res_;
}

#define NB_SLOT(_x) offsetof(AlifNumberMethods, _x)
#define NB_BINOP(_method, _slot) (*(BinaryFunc*)(&((char*)_method)[_slot])) // 911


static AlifObject* binary_op1(AlifObject* _x, AlifObject* _y, const AlifIntT _opSlot, const wchar_t* _opName) { // 926

	BinaryFunc slotX;
	if (ALIF_TYPE(_x)->asNumber != nullptr) {
		slotX = NB_BINOP(ALIF_TYPE(_x)->asNumber, _opSlot);
	}
	else slotX = nullptr;

	BinaryFunc slotY{};
	if (!ALIF_IS_TYPE(_y, ALIF_TYPE(_x)) and ALIF_TYPE(_y)->asNumber != nullptr) {
		slotY = NB_BINOP(ALIF_TYPE(_y)->asNumber, _opSlot);
		if (slotY == slotX) slotY = nullptr;
	}
	else slotY = nullptr;

	if (slotX) {
		AlifObject* x_{};
		if (slotY and alifType_isSubType(ALIF_TYPE(_y), ALIF_TYPE(_x))) {
			x_ = slotY(_x, _y);
			if (x_ != ALIF_NOTIMPLEMENTED) return x_;
			ALIF_DECREF(x_);
			slotY = nullptr;
		}
		x_ = slotX(_x, _y);
		if (x_ != ALIF_NOTIMPLEMENTED) return x_;
		ALIF_DECREF(x_);
	}
	if (slotY) {
		AlifObject* y_ = slotY(_x, _y);
		if (y_ != ALIF_NOTIMPLEMENTED) return y_;
		ALIF_DECREF(y_);
	}

	return ALIF_NOTIMPLEMENTED;
}


#define BINARY_OP1(_x, _y, _opSlot, _opName) binary_op1(_x, _y, _opSlot, _opName) // 982


static AlifObject* binary_op(AlifObject* _x, AlifObject* _y, const AlifIntT _opSlot, const wchar_t* _opName) { // 997
	AlifObject* result = BINARY_OP1(_x, _y, _opSlot, _opName);
	if (result == ALIF_NOTIMPLEMENTED) {
		ALIF_DECREF(result);

		if (_opSlot == NB_SLOT(rshift_) and ALIFCPPFUNCTION_CHECKEXACT(_x) and
			wcscmp(((AlifCFunctionObject*)_x)->method->name, L"اطبع") == 0)
		{
			// error
			return nullptr;
		}
		//return binOp_typeError(_v, _w, _opName);
	}
	return result;
}

#define BINARY_FUNC(_func, _op, _opName) \
    AlifObject* _func(AlifObject* _x, AlifObject* _y) { \
        return binary_op(_x, _y, NB_SLOT(_op), _opName); \
    }	
BINARY_FUNC(alifNumber_subtract, subtract_, L"-")


AlifObject* alifNumber_add(AlifObject* _x, AlifObject* _y) { // 1138

	AlifObject* res_ = BINARY_OP1(_x, _y, NB_SLOT(add_), L"+");
	if (res_ != ALIF_NOTIMPLEMENTED) return res_;

	ALIF_DECREF(res_);

	AlifSequenceMethods* method_ = ALIF_TYPE(_x)->asSequence;
	if (method_ and method_->concat_) {
		res_ = (*method_->concat_)(_x, _y);
		return res_;
	}

	//return binOp_typeError(_x, _y, L"+");
	return nullptr; // temp
}

static AlifObject* binary_iop1(AlifObject* _v, AlifObject* _w, const int _iOpSlot, const int _opSlot
//#ifndef NDEBUG
	, const wchar_t* _opName
//#endif
)
{
	AlifNumberMethods* mv_ = ALIF_TYPE(_v)->asNumber;
	if (mv_ != nullptr) {
		BinaryFunc slot_ = NB_BINOP(mv_, (_iOpSlot));
		if (slot_) {
			AlifObject* x = (slot_)(_v, _w);
			if (x != ALIF_NOTIMPLEMENTED) {
				return x;
			}
			ALIF_DECREF(x);
		}
	}
//#ifdef NDEBUG
//	return binary_op1(_v, _w, _opSlot);
//#else
	return binary_op1(_v, _w, _opSlot, _opName);
//#endif
}

//#ifdef NDEBUG
//#  define BINARY_IOP1(_v, w, iop_slot, op_slot, op_name) binary_iop1(_v, w, iop_slot, op_slot)
//#else
#  define BINARY_IOP1(_v, w, iop_slot, op_slot, op_name) binary_iop1(_v, w, iop_slot, op_slot, op_name)
//#endif

static AlifObject* binary_iop(AlifObject* _v, AlifObject* _w, const int _iOpSlot, const int _opSlot,
	const wchar_t* _opName)
{
	AlifObject* result_ = BINARY_IOP1(_v, _w, _iOpSlot, _opSlot, _opName);
	if (result_ == ALIF_NOTIMPLEMENTED) {
		ALIF_DECREF(result_);
		return nullptr; // سيتم تعجديلها لاحقا عندما يتم اضافة تحقق من الاخطاء
		//return binop_type_error(_v, _w, _opName);
	}
	return result_;
}

#define INPLACE_BINOP(func, iop, op, op_name) \
    AlifObject * \
    func(AlifObject *_x, AlifObject *_y) { \
        return binary_iop(_x, _y, NB_SLOT(iop), NB_SLOT(op), op_name); \
    }

INPLACE_BINOP(alifNumber_inPlaceSubtract, inplaceSubtract, subtract_, L"-=")
INPLACE_BINOP(alifInteger_inPlaceOr, inplaceOr, orLogic, L"|=")

AlifObject* alifNumber_inPlaceAdd(AlifObject* _x, AlifObject* _y)
{
	AlifObject* result = BINARY_IOP1(_x, _y, NB_SLOT(inplaceAdd), NB_SLOT(add_), L"+=");
	if (result == ALIF_NOTIMPLEMENTED) {
		AlifSequenceMethods* m = ALIF_TYPE(_x)->asSequence;
		ALIF_DECREF(result);
		if (m != nullptr) {
			BinaryFunc func = m->inplaceConcat;
			if (func == nullptr)
				func = m->concat_;
			if (func != nullptr) {
				result = func(_x, _y);
				return result;
			}
		}
		//result = binOp_typeError(_x, _y, L"+=");
	}
	return result;
}

AlifObject* alifSubNumber_index(AlifObject* _item)
{
    //if (_item == nullptr) {
    //    return null_error();
    //}

    if (_item->type_ == &_alifIntegerType_) {
        return ALIF_NEWREF(_item);
    }
    if (!(_item->type_->asNumber != nullptr && _item->type_->asNumber->index_)) {
        //Err_Format(Exc_TypeError,
        //    "'%.200s' object cannot be interpreted "
        //    "as an integer", _TYPE(_item)->tp_name);
        return nullptr;
    }

    AlifObject* result_ = ALIF_TYPE(_item)->asNumber->index_(_item);
    if (!result_ || (result_->type_ == & _alifIntegerType_)) {
        return result_;
    }

    if (!(result_->type_ == &_alifIntegerType_)) {
        //Err_Format(Exc_TypeError,
        //    "__index__ returned non-int (type %.200s)",
        //    _TYPE(result)->tp_name);
        ALIF_DECREF(result_);
        return nullptr;
    }
    /* Issue #17576: warn if 'result' not of exact type int. */
    //if (Err_WarnFormat(Exc_DeprecationWarning, 1,
    //    "__index__ returned non-int (type %.200s).  "
    //    "The ability to return an instance of a strict subclass of int "
    //    "is deprecated, and may be removed in a future version of thon.",
    //    _TYPE(result)->tp_name)) {
    //    _DECREF(result);
    //    return nullptr;
    //}
    return result_;
}


AlifSizeT alifNumber_asSizeT(AlifObject* _item, AlifObject* _err)
{
	AlifSizeT result{};
	AlifObject* runerr{};
	AlifObject* _value = alifSubNumber_index(_item);
	AlifThread* thread_{};
	if (_value == nullptr) return -1;

	result = alifInteger_asSizeT(_value);
	if (result != -1) goto finish;

	thread_ = alifThread_get();
	//runerr = alifErr_occurred(thread_);
	if (!runerr) goto finish;

	//if (!alifErr_givenExceptionMatches(runerr, alifExcOverflowError)) {
	//	goto finish;
	//}
	//alifErr_clear(thread_);

	if (!_err) {
		if (alifInteger_isNegative((AlifIntegerObject*)_value))
			result = ALIF_SIZET_MIN;
		else
			result = ALIF_SIZET_MAX;
	}
	else {
		//alifErr_format(tstate, err, "cannot fit '%.200s' into an index-sized integer", ALIF_TYPE(item)->name_);
		return -1; //
	}

finish:
	ALIF_DECREF(_value);
	return result;
}


AlifObject* alifInteger_float(AlifObject* _o)
{
    if (_o == nullptr) {
        //return null_error();
    }

    if (_o->type_ == &_alifFloatType) {
        return ALIF_NEWREF(_o);
    }

    AlifNumberMethods* m_ = ALIF_TYPE(_o)->asNumber;
    if (m_ && m_->float_) { /* This should include subclasses of float */
        AlifObject* res_ = m_->float_(_o);
        if (!res_ || (res_->type_ == &_alifFloatType)) {
            return res_;
        }

        if (!(res_->type_ == &_alifFloatType)) {
            //Err_Format(Exc_TypeError,
            //    "%.50s.__float__ returned non-float (type %.50s)",
            //    ALIF_TYPE(_o)->name_, ALIF_TYPE(res_)->name_);
            ALIF_DECREF(res_);
            return nullptr;
        }
        /* Issue #26983: warn if 'res' not of exact type float. */
        //if (Err_WarnFormat(Exc_DeprecationWarning, 1,
        //    "%.50s.__float__ returned non-float (type %.50s).  "
        //    "The ability to return an instance of a strict subclass of float "
        //    "is deprecated, and may be removed in a future version of thon.",
        //    ALIF_TYPE(_o)->name_, _TYPE(res_)->name_)) {
        //    ALIF_DECREF(res);
        //    return nullptr;
        //}
        long double val_ = ((AlifFloatObject*)res_)->digits_;
        ALIF_DECREF(res_);
        return alifFloat_fromDouble(val_);
    }

    if (m_ && m_->index_) {
        AlifObject* res_ = alifSubNumber_index(_o);
        if (!res_) {
            return nullptr;
        }
        long double val_ = alifInteger_asDouble(res_);
        ALIF_DECREF(res_);
        //if (val_ == -1.0 && Err_Occurred()) {
        //    return nullptr;
        //}
        return alifFloat_fromDouble(val_);
    }

    /* A float subclass with nb_float == nullptr */
    if (_o->type_ == &_alifFloatType) {
        return alifFloat_fromDouble(((AlifFloatObject*)_o)->digits_);
    }
    return alifFloat_fromString(_o);
}

int alifSequence_check(AlifObject* _s)
{
    if (_s->type_ == &_alifDictType_)
        return 0;
    return _s->type_->asSequence &&
        _s->type_->asSequence->item_ != nullptr;
}

AlifObject* alifSequence_getItem(AlifObject* _s, int64_t _i)
{
	if (_s == nullptr) {
		return nullptr;
	}

	AlifSequenceMethods* m_ = ALIF_TYPE(_s)->asSequence;
	if (m_ && m_->item_) {
		if (_i < 0) {
			if (m_->length_) {
				int64_t l_ = (*m_->length_)(_s);
				if (l_ < 0) {
					return nullptr;
				}
				_i += l_;
			}
		}
		AlifObject* res_ = m_->item_(_s, _i);
		return res_;
	}

	if (ALIF_TYPE(_s)->asMapping && ALIF_TYPE(_s)->asMapping->subScript) {
		return nullptr;// error
	}
	return nullptr; // error
}

int alifSequence_setItem(AlifObject* _s, int64_t _i, AlifObject* _o)
{
	if (_s == nullptr) {
		return -1;
	}

	AlifSequenceMethods* m_ = ALIF_TYPE(_s)->asSequence;
	if (m_ && m_->assItem) {
		if (_i < 0) {
			if (m_->length_) {
				int64_t l_ = (*m_->length_)(_s);
				if (l_ < 0) {
					return -1;
				}
				_i += l_;
			}
		}
		int res_ = m_->assItem(_s, _i, _o);
		return res_;
	}

	if (ALIF_TYPE(_s)->asMapping && ALIF_TYPE(_s)->asMapping->assSubScript) {
		// error
		return -1;
	}
	// error
	return -1;
}

int alifIter_check(AlifObject* obj)
{
    AlifInitObject* tp = obj->type_;
    return (tp->iterNext != nullptr);
}

AlifIntT alifSequence_delItem(AlifObject* _s, AlifSizeT _i) { // 1959
	if (_s == nullptr) {
		//null_error();
		return -1;
	}

	AlifSequenceMethods* m_ = ALIF_TYPE(_s)->asSequence;
	if (m_ and m_->assItem) {
		if (_i < 0) {
			if (m_->length_) {
				AlifSizeT l = (*m_->length_)(_s);
				if (l < 0) return -1;
				_i += l;
			}
		}
		int res = m_->assItem(_s, _i, (AlifObject*)nullptr);
		return res;
	}

	if (ALIF_TYPE(_s)->asMapping and ALIF_TYPE(_s)->asMapping->assSubScript) {
		// error
		return -1;
	}
	// error
	return -1;
}

AlifObject* alifSequence_list(AlifObject* _v)
{
    AlifObject* result;  
    AlifObject* rv;          

    //if (_v == nullptr) {
        //return null_error();
    //}

    result = alifNew_list(0);
    if (result == nullptr)
        return nullptr;

    rv = alifList_extend((AlifListObject*)result, _v);
    if (rv == nullptr) {
        return nullptr;
    }
    return result;
}

AlifObject* alifSequence_fast(AlifObject* _v, const wchar_t* _m)
{
    AlifObject* it;

    //if (_v == nullptr) {
        //return null_error();
    //}

    if ((_v->type_ == &_alifListType_) || (_v->type_ == & _alifTupleType_)) {
        return _v;
    }

    it = alifObject_getIter(_v);
    if (it == nullptr) {
        //if (_Err_ExceptionMatches(tstate, Exc_TypeError)) {
            //_Err_SetString(tstate, Exc_TypeError, m_);
        //}
        return nullptr;
    }

    _v = alifSequence_list(it);
    //_DECREF(it);

    return _v;
}

AlifObject* alifObject_getIter(AlifObject* _o) {
    AlifInitObject* t = _o->type_;
    GetIterFunc f;

    f = t->iter_;
    if (f == nullptr) {
        if (alifSequence_check(_o))
            return alifNew_seqIter(_o);
        //return type_error("'%.200s' object is not iterable", _o);
        return nullptr;
    }
    else {
        AlifObject* res = (*f)(_o);
        if (res != nullptr && !alifIter_check(res)) {
            //Err_Format(Exc_TypeError,
                //"iter() returned non-iterator "
                //"of type_ '%.100s'",
                //TYPE(res)->tp_name);
            ALIF_SETREF(res, nullptr);
        }
        return res;
    }
}



AlifObject* alifIter_next(AlifObject* _iter)
{
	AlifObject* result_;
	result_ = (*ALIF_TYPE(_iter)->iterNext)(_iter);
	if (result_ == nullptr) {
		AlifThread* thread_ = alifThread_get();

	}
	return result_;
}
