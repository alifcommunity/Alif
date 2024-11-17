#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_FreeList.h"
#include "AlifCore_Math.h"



AlifObject* alifFloat_fromDouble(double _fVal) { // 123
	AlifFloatObject* op_ = ALIF_FREELIST_POP(AlifFloatObject, floats);
	if (op_ == nullptr) {
		op_ = (AlifFloatObject*)alifMem_objAlloc(sizeof(AlifFloatObject));
		if (!op_) {
			//return alifErr_noMemory();
			return nullptr; // temp
		}
		_alifObject_init((AlifObject*)op_, &_alifFloatType_);
	}
	op_->val = _fVal;
	return (AlifObject*)op_;
}

 // 314
#define CONVERT_TO_DOUBLE(_obj, _dbl)                     \
    if (ALIFFLOAT_CHECK(_obj))                             \
        _dbl = ALIFFLOAT_AS_DOUBLE(_obj);                   \
    else if (convert_toDouble(&(_obj), &(_dbl)) < 0)     \
        return _obj;


static AlifIntT convert_toDouble(AlifObject** _v, double* _dbl) { // 322
	AlifObject* obj = *_v;

	if (ALIFLONG_CHECK(obj)) {
		*_dbl = alifLong_asDouble(obj);
		if (*_dbl == -1.0 /*and alifErr_occurred()*/) {
			*_v = nullptr;
			return -1;
		}
	}
	else {
		*_v = ALIF_NEWREF(ALIF_NOTIMPLEMENTED);
		return -1;
	}
	return 0;
}


static AlifObject* float_add(AlifObject* _v, AlifObject* _w) { // 555
	double a{}, b{};
	CONVERT_TO_DOUBLE(_v, a);
	CONVERT_TO_DOUBLE(_w, b);
	a = a + b;
	return alifFloat_fromDouble(a);
}

static AlifObject* float_sub(AlifObject* _v, AlifObject* _w) { // 565
	double a{}, b{};
	CONVERT_TO_DOUBLE(_v, a);
	CONVERT_TO_DOUBLE(_w, b);
	a = a - b;
	return alifFloat_fromDouble(a);
}

static AlifObject* float_mul(AlifObject* _v, AlifObject* _w) { // 575
	double a{}, b{};
	CONVERT_TO_DOUBLE(_v, a);
	CONVERT_TO_DOUBLE(_w, b);
	a = a * b;
	return alifFloat_fromDouble(a);
}

static AlifObject* float_div(AlifObject* _v, AlifObject* _w) { // 585
	double a{}, b{};
	CONVERT_TO_DOUBLE(_v, a);
	CONVERT_TO_DOUBLE(_w, b);
	if (b == 0.0) {
		//alifErr_setString(_alifExcZeroDivisionError_,
		//	"division by zero");
		return nullptr;
	}
	a = a / b;
	return alifFloat_fromDouble(a);
}

static AlifObject* float_rem(AlifObject* _v, AlifObject* _w) { // 601
	double vx_{}, wx_{};
	double mod_{};
	CONVERT_TO_DOUBLE(_v, vx_);
	CONVERT_TO_DOUBLE(_w, wx_);
	if (wx_ == 0.0) {
		//alifErr_setString(_alifExcZeroDivisionError_,
			//"division by zero");
		return nullptr;
	}
	mod_ = fmod(vx_, wx_);
	if (mod_) {
		if ((wx_ < 0) != (mod_ < 0)) {
			mod_ += wx_;
		}
	}
	else {
		mod_ = copysign(0.0, wx_);
	}
	return alifFloat_fromDouble(mod_);
}

static void _float_divMod(double _vx, double _wx, double* _floorDiv, double* _mod) { // 629
	double div_{};
	*_mod = fmod(_vx, _wx);
	div_ = (_vx - *_mod) / _wx;
	if (*_mod) {
		if ((_wx < 0) != (*_mod < 0)) {
			*_mod += _wx;
			div_ -= 1.0;
		}
	}
	else {
		*_mod = copysign(0.0, _wx);
	}
	if (div_) {
		*_floorDiv = floor(div_);
		if (div_ - *_floorDiv > 0.5) {
			*_floorDiv += 1.0;
		}
	}
	else {
		*_floorDiv = copysign(0.0, _vx / _wx); 
	}
}

static AlifObject* float_divmod(AlifObject* _v, AlifObject* _w) { // 667
	double vx_{}, wx_{};
	double mod_{}, floorDiv{};
	CONVERT_TO_DOUBLE(_v, vx_);
	CONVERT_TO_DOUBLE(_w, wx_);
	if (wx_ == 0.0) {
		//alifErr_setString(_alifExcZeroDivisionError_, "division by zero");
		return nullptr;
	}
	_float_divMod(vx_, wx_, &floorDiv, &mod_);
	return alif_buildValue("(dd)", floorDiv, mod_);
}

static AlifObject* float_floorDiv(AlifObject* _v, AlifObject* _w) { // 682
	double vx_{}, wx_{};
	double mod_{}, floorDiv{};
	CONVERT_TO_DOUBLE(_v, vx_);
	CONVERT_TO_DOUBLE(_w, wx_);
	if (wx_ == 0.0) {
		//alifErr_setString(_alifExcZeroDivisionError_, "division by zero");
		return nullptr;
	}
	_float_divMod(vx_, wx_, &floorDiv, &mod_);
	return alifFloat_fromDouble(floorDiv);
}

#define DOUBLE_IS_ODD_INTEGER(_x) (fmod(fabs(_x), 2.0) == 1.0) // 698


static AlifObject* float_pow(AlifObject* _v,
	AlifObject* _w, AlifObject* _z) { // 701
	double iv_{}, iw_{}, ix_{};
	AlifIntT negateResult = 0;

	if ((AlifObject*)_z != ALIF_NONE) {
		//alifErr_setString(_alifExcTypeError_, "pow() 3rd argument not "
			//"allowed unless all arguments are integers");
		return nullptr;
	}

	CONVERT_TO_DOUBLE(_v, iv_);
	CONVERT_TO_DOUBLE(_w, iw_);
	if (iw_ == 0) {             
		return alifFloat_fromDouble(1.0);
	}
	if (isnan(iv_)) {        
		return alifFloat_fromDouble(iv_);
	}
	if (isnan(iw_)) {        
		return alifFloat_fromDouble(iv_ == 1.0 ? 1.0 : iw_);
	}
	if (isinf(iw_)) {
		iv_ = fabs(iv_);
		if (iv_ == 1.0)
			return alifFloat_fromDouble(1.0);
		else if ((iw_ > 0.0) == (iv_ > 1.0))
			return alifFloat_fromDouble(fabs(iw_));
		else
			return alifFloat_fromDouble(0.0);
	}
	if (isinf(iv_)) {
		AlifIntT iw_is_odd = DOUBLE_IS_ODD_INTEGER(iw_);
		if (iw_ > 0.0)
			return alifFloat_fromDouble(iw_is_odd ? iv_ : fabs(iv_));
		else
			return alifFloat_fromDouble(iw_is_odd ?
				copysign(0.0, iv_) : 0.0);
	}
	if (iv_ == 0.0) { 
		AlifIntT iw_is_odd = DOUBLE_IS_ODD_INTEGER(iw_);
		if (iw_ < 0.0) {
			//alifErr_setString(_alifExcZeroDivisionError_,
				//"zero to a negative power");
			return nullptr;
		}
		return alifFloat_fromDouble(iw_is_odd ? iv_ : 0.0);
	}

	if (iv_ < 0.0) {

		if (iw_ != floor(iw_)) {
			//return _alifComplexType_.asNumber->power(_v, _w, _z);
		}
		iv_ = -iv_;
		negateResult = DOUBLE_IS_ODD_INTEGER(iw_);
	}

	if (iv_ == 1.0) {
		return alifFloat_fromDouble(negateResult ? -1.0 : 1.0);
	}

	errno = 0;
	ix_ = pow(iv_, iw_);
	_alif_adjustErange1(ix_);
	if (negateResult)
		ix_ = -ix_;

	if (errno != 0) {
		//alifErr_setFromErrno(errno == ERANGE ? _alifExcOverflowError_ :
			//_alifExcValueError_);
		return nullptr;
	}
	return alifFloat_fromDouble(ix_);
}


#undef DOUBLE_IS_ODD_INTEGER // 819

static AlifObject* float_neg(AlifFloatObject* _v) { // 821
	return alifFloat_fromDouble(-_v->val);
}

static AlifObject* float_abs(AlifFloatObject* _v) { // 827
	return alifFloat_fromDouble(fabs(_v->val));
}

static AlifIntT float_bool(AlifFloatObject* _v) { // 833
	return _v->val != 0.0;
}



static AlifObject* float___trunc__Impl(AlifObject* _self) { // 872
	return alifLong_fromDouble(ALIFFLOAT_AS_DOUBLE(_self));
}



static AlifObject* float_float(AlifObject* _v) { // 1079
	if (ALIFFLOAT_CHECKEXACT(_v)) {
		return ALIF_NEWREF(_v);
	}
	else {
		return alifFloat_fromDouble(((AlifFloatObject*)_v)->val);
	}
}






static AlifNumberMethods _floatAsNumber_ = { // 1811
	.add_ = float_add,
	.subtract = float_sub,
	.multiply = float_mul,
	.remainder = float_rem,
	.divmod = float_divmod,
	.power = float_pow,
	.negative = (UnaryFunc)float_neg,
	.positive = float_float,
	.absolute = (UnaryFunc)float_abs,
	.bool_ = (Inquiry)float_bool,
	.invert = 0,
	.lshift = 0,
	.rshift = 0,
	.and_ = 0,
	.xor_ = 0,
	.or_ = 0,
	.int_ = float___trunc__Impl,
	.reserved = 0,
	.float_ = float_float,
	.inplaceAdd = 0,
	.inplaceSubtract = 0,
	.inplaceMultiply = 0,
	.inplaceRemainder = 0,
	.inplacePower = 0,
	.inplaceLshift = 0,
	.inplaceRshift = 0,
	.inplaceAnd = 0,
	.inplaceXor = 0,
	.inplaceOr = 0,
	.floorDivide = float_floorDiv,
	.trueDivide = float_div,
	.inplaceFloorDivide = 0,     
	.inplaceTrueDivide = 0,       
};

AlifTypeObject _alifFloatType_ = { // 1847
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "عدد_عشري",
	.basicSize = sizeof(AlifFloatObject),
	.itemSize = 0,
	.asNumber = &_floatAsNumber_,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
		_ALIF_TPFLAGS_MATCH_SELF,
};
