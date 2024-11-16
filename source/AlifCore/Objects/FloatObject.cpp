#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_FreeList.h"

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
	.int_ = float___trunc___impl,
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
