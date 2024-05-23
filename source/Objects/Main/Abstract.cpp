#include "alif.h"

#include "AlifCore_Memory.h" // temp
#include "AlifCore_AlifState.h"
#include "AlifCore_list.h"

void alifBuffer_release(AlifBuffer* _view)
{
    AlifObject* obj_ = _view->obj;
    AlifBufferProcs* pb_;
    if (obj_ == NULL)
        return;
    pb_ = ALIF_TYPE(obj_)->asBuffer;
    if (pb_ && pb_->releaseBuffer) {
        pb_->releaseBuffer(obj_, _view);
    }
    _view->obj = NULL;
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
#define NB_BINOP(_method, _slot) (*(BinaryFunc)(&((wchar_t*)_method)[_slot])) // 911


static AlifObject* binary_op1(AlifObject* _x, AlifObject* _y, const AlifIntT _opSlot, const wchar_t* _opName) { // 926

	BinaryFunc slotX{};
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
		//if (slotY and alifType_isSubType(ALIF_TYPE(_y), ALIF_TYPE(_x))) {
		//	x_ = slotY(_x, _y);
		//	if (x_ != ALIF_NOTIMPLEMENTED) return x_;
		//	ALIF_DECREF(x_);
		//	slotY = nullptr;
		//}
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


AlifObject* alifSubNumber_index(AlifObject* _item)
{
    //if (_item == nullptr) {
    //    return null_error();
    //}

    if (_item->type_ == &_typeInteger_) {
        return ALIF_NEWREF(_item);
    }
    if (!(_item->type_->asNumber != nullptr && _item->type_->asNumber->index_)) {
        //Err_Format(Exc_TypeError,
        //    "'%.200s' object cannot be interpreted "
        //    "as an integer", _TYPE(_item)->tp_name);
        return nullptr;
    }

    AlifObject* result_ = ALIF_TYPE(_item)->asNumber->index_(_item);
    if (!result_ || (result_->type_ == & _typeInteger_)) {
        return result_;
    }

    if (!(result_->type_ == &_typeInteger_)) {
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

AlifObject* alifInteger_float(AlifObject* _o)
{
    if (_o == nullptr) {
        //return null_error();
    }

    if (_o->type_ == &_typeFloat_) {
        return ALIF_NEWREF(_o);
    }

    AlifNumberMethods* m_ = ALIF_TYPE(_o)->asNumber;
    if (m_ && m_->float_) { /* This should include subclasses of float */
        AlifObject* res_ = m_->float_(_o);
        if (!res_ || (res_->type_ == &_typeFloat_)) {
            return res_;
        }

        if (!(res_->type_ == &_typeFloat_)) {
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
    if (_o->type_ == &_typeFloat_) {
        return alifFloat_fromDouble(((AlifFloatObject*)_o)->digits_);
    }
    return alifFloat_fromString(_o);
}

int alifSequence_check(AlifObject* s)
{
    if (s->type_ == &typeDict)
        return 0;
    return s->type_->asSequence &&
        s->type_->asSequence->item_ != nullptr;
}

int alifIter_check(AlifObject* obj)
{
    AlifInitObject* tp = obj->type_;
    return (tp->iterNext != nullptr);
}

AlifObject* alifSequence_list(AlifObject* v)
{
    AlifObject* result;  
    AlifObject* rv;          

    //if (v == nullptr) {
        //return null_error();
    //}

    result = alifNew_list(0);
    if (result == nullptr)
        return nullptr;

    rv = alifList_extend((AlifListObject*)result, v);
    if (rv == nullptr) {
        return nullptr;
    }
    return result;
}

AlifObject* alifSequence_fast(AlifObject* v, const wchar_t* m)
{
    AlifObject* it;

    //if (v == nullptr) {
        //return null_error();
    //}

    if ((v->type_ == &typeList) || (v->type_ == & typeTuple)) {
        return v;
    }

    it = alifObject_getIter(v);
    if (it == nullptr) {
        //if (_Err_ExceptionMatches(tstate, Exc_TypeError)) {
            //_Err_SetString(tstate, Exc_TypeError, m);
        //}
        return nullptr;
    }

    v = alifSequence_list(it);
    //_DECREF(it);

    return v;
}

AlifObject* alifObject_getIter(AlifObject* o)
{
    AlifInitObject* t = o->type_;
    GetIterFunc f;

    f = t->iter_;
    if (f == nullptr) {
        if (alifSequence_check(o))
            return alifNew_seqIter(o);
        //return type_error("'%.200s' object is not iterable", o);
        return nullptr;
    }
    else {
        AlifObject* res = (*f)(o);
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
