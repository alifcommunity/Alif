#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_FloatObject.h"
#include "AlifCore_FreeList.h"
#include "AlifCore_Long.h"
#include "AlifCore_Math.h"
#include "AlifCore_StructSeq.h"

#include <float.h>

#include "clinic/FloatObject.cpp.h"






static AlifTypeObject _floatInfoType_; // 42





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


static AlifObject* float_fromStringInner(const char* s, AlifSizeT len, void* obj) { // 137
	double x{};
	const char* end{};
	const char* last = s + len;
	/* strip leading whitespace */
	while (s < last and ALIF_ISSPACE(*s)) {
		s++;
	}
	if (s == last) {
		//alifErr_format(_alifExcValueError_,
		//	"could not convert string to float: "
		//	"%R", obj);
		return nullptr;
	}

	/* strip trailing whitespace */
	while (s < last - 1 and ALIF_ISSPACE(last[-1])) {
		last--;
	}

	x = alifOS_stringToDouble(s, (char**)&end, nullptr);
	if (end != last) {
		//alifErr_format(_alifExcValueError_,
		//	"could not convert string to float: "
		//	"%R", obj);
		return nullptr;
	}
	else if (x == -1.0 and alifErr_occurred()) {
		return nullptr;
	}
	else {
		return alifFloat_fromDouble(x);
	}
}


AlifObject* alifFloat_fromString(AlifObject* v) { // 177
	const char* s{};
	AlifObject* s_buffer = nullptr;
	AlifSizeT len{};
	AlifBuffer view = { nullptr, nullptr };
	AlifObject* result = nullptr;

	if (ALIFUSTR_CHECK(v)) {
		s_buffer = _alifUStr_transformDecimalAndSpaceToASCII(v);
		if (s_buffer == nullptr)
			return nullptr;
		s = alifUStr_asUTF8AndSize(s_buffer, &len);
	}
	else if (ALIFBYTES_CHECK(v)) {
		s = ALIFBYTES_AS_STRING(v);
		len = ALIFBYTES_GET_SIZE(v);
	}
	else if (ALIFBYTEARRAY_CHECK(v)) {
		s = ALIFBYTEARRAY_AS_STRING(v);
		len = ALIFBYTEARRAY_GET_SIZE(v);
	}
	else if (alifObject_getBuffer(v, &view, ALIFBUF_SIMPLE) == 0) {
		s = (const char*)view.buf;
		len = view.len;
		/* Copy to NUL-terminated buffer. */
		s_buffer = alifBytes_fromStringAndSize(s, len);
		if (s_buffer == nullptr) {
			alifBuffer_release(&view);
			return nullptr;
		}
		s = ALIFBYTES_AS_STRING(s_buffer);
	}
	else {
		//alifErr_format(_alifExcTypeError_,
		//	"float() argument must be a string or a real number, not '%.200s'",
		//	ALIF_TYPE(v)->name);
		return nullptr;
	}
	result = _alifString_toNumberWithUnderscores(s, len, "float", v, v,
		float_fromStringInner);
	alifBuffer_release(&view);
	ALIF_XDECREF(s_buffer);
	return result;
}



double alifFloat_asDouble(AlifObject* _op) { // 250
	AlifNumberMethods* nb_{};
	AlifObject* res_{};
	double val_{};

	if (_op == nullptr) {
		//alifErr_badArgument();
		return -1;
	}

	if (ALIFFLOAT_CHECK(_op)) {
		return ALIFFLOAT_AS_DOUBLE(_op);
	}

	nb_ = ALIF_TYPE(_op)->asNumber;
	if (nb_ == nullptr or nb_->float_ == nullptr) {
		if (nb_ and nb_->index) {
			AlifObject* res = _alifNumber_index(_op);
			if (!res) {
				return -1;
			}
			double val_ = alifLong_asDouble(res);
			ALIF_DECREF(res);
			return val_;
		}
		//alifErr_format(_alifExcTypeError_, "must be real number, not %.50s",
			//ALIF_TYPE(_op)->name);
		return -1;
	}

	res_ = (*nb_->float_) (_op);
	if (res_ == nullptr) {
		return -1;
	}
	if (!ALIFFLOAT_CHECKEXACT(res_)) {
		if (!ALIFFLOAT_CHECK(res_)) {
			//alifErr_format(_alifExcTypeError_,
				//"%.50s.__float__ returned non-float (type %.50s)",
				//ALIF_TYPE(_op)->tp_name, ALIF_TYPE(res_)->tp_name);
			ALIF_DECREF(res_);
			return -1;
		}
		//if (alifErr_warnFormat(_alifExcDeprecationWarning_, 1,
			//"%.50s.__float__ returned non-float (type %.50s).  "
			//"The ability to return an instance of a strict subclass of float "
			//"is deprecated, and may be removed in a future version of alif.",
			//ALIF_TYPE(_op)->tp_name, ALIF_TYPE(res_)->tp_name)) {
			//ALIF_DECREF(res_);
			//return -1;
		//}
	}

	val_ = ALIFFLOAT_AS_DOUBLE(res_);
	ALIF_DECREF(res_);
	return val_;
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

static AlifObject* float_repr(AlifFloatObject* _v) { // 341
	AlifObject* result{};
	char* buf{};

	buf = alifOS_doubleToString(ALIFFLOAT_AS_DOUBLE(_v),
		'r', 0, ALIF_DTSF_ADD_DOT_0, nullptr);
	if (!buf) {
		//return alifErr_noMemory();
		return nullptr; //* alif
	}
	result = alifUStr_fromASCII(buf, strlen(buf));
	alifMem_dataFree(buf);
	return result;
}

static AlifObject* float_richCompare(AlifObject* _v, AlifObject* _w, AlifIntT _op) { // 373
	double i{}, j{};
	AlifIntT r = 0;

	i = ALIFFLOAT_AS_DOUBLE(_v);

	if (ALIFFLOAT_CHECK(_w))
		j = ALIFFLOAT_AS_DOUBLE(_w);

	else if (!isfinite(i)) {
		if (ALIFLONG_CHECK(_w))
			j = 0.0;
		else
			goto Unimplemented;
	}

	else if (ALIFLONG_CHECK(_w)) {
		AlifIntT vsign = i == 0.0 ? 0 : i < 0.0 ? -1 : 1;
		AlifIntT wsign = _alifLong_sign(_w);
		AlifIntT exponent{};

		if (vsign != wsign) {
			i = (double)vsign;
			j = (double)wsign;
			goto Compare;
		}

		uint64_t nbits64 = _alifLong_numBits(_w);
		if (nbits64 > (unsigned int)DBL_MAX_EXP) {
			if (nbits64 == (uint64_t)-1 and alifErr_occurred()) {
				//alifErr_clear();
			}
			i = (double)vsign;
			j = wsign * 2.0;
			goto Compare;
		}
		AlifIntT nbits = (AlifIntT)nbits64;
		if (nbits <= 48) {
			j = alifLong_asDouble(_w);
			goto Compare;
		}
		if (vsign < 0) {
			i = -i;
			_op = _alifSwappedOp_[_op];
		}
		(void)frexp(i, &exponent);
		if (exponent < nbits) {
			i = 1.0;
			j = 2.0;
			goto Compare;
		}
		if (exponent > nbits) {
			i = 2.0;
			j = 1.0;
			goto Compare;
		}

		{
			double fracpart;
			double intpart;
			AlifObject* result = nullptr;
			AlifObject* vv = nullptr;
			AlifObject* ww = _w;

			if (wsign < 0) {
				ww = alifNumber_negative(_w);
				if (ww == nullptr)
					goto Error;
			}
			else
				ALIF_INCREF(ww);

			fracpart = modf(i, &intpart);
			vv = alifLong_fromDouble(intpart);
			if (vv == nullptr)
				goto Error;

			if (fracpart != 0.0) {
				AlifObject* temp{};

				temp = _alifLong_lShift(ww, 1);
				if (temp == nullptr)
					goto Error;
				ALIF_SETREF(ww, temp);

				temp = _alifLong_lShift(vv, 1);
				if (temp == nullptr)
					goto Error;
				ALIF_SETREF(vv, temp);

				temp = alifNumber_or(vv, _alifLong_getOne());
				if (temp == nullptr)
					goto Error;
				ALIF_SETREF(vv, temp);
			}

			r = alifObject_richCompareBool(vv, ww, _op);
			if (r < 0)
				goto Error;
			result = alifBool_fromLong(r);
		Error:
			ALIF_XDECREF(vv);
			ALIF_XDECREF(ww);
			return result;
		}
	}

	else        /* w isn't float or int */
		goto Unimplemented;

Compare:
	switch (_op) {
	case ALIF_EQ:
		r = i == j;
		break;
	case ALIF_NE:
		r = i != j;
		break;
	case ALIF_LE:
		r = i <= j;
		break;
	case ALIF_GE:
		r = i >= j;
		break;
	case ALIF_LT:
		r = i < j;
		break;
	case ALIF_GT:
		r = i > j;
		break;
	}
	return alifBool_fromLong(r);

Unimplemented:
	return ALIF_NOTIMPLEMENTED;
}

static AlifHashT float_hash(AlifFloatObject* _v) { // 549
	return _alif_hashDouble((AlifObject*)_v, _v->val);
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



static AlifObject* float_subTypeNew(AlifTypeObject*, AlifObject*); // 1563

static AlifObject* float_newImpl(AlifTypeObject* _type, AlifObject* _x
) { // 1575
	if (_type != &_alifFloatType_) {
		if (_x == nullptr) {
			_x = _alifLong_getZero();
		}
		return float_subTypeNew(_type, _x); /* Wimp out */
	}

	if (_x == nullptr) {
		return alifFloat_fromDouble(0.0);
	}
	if (ALIFUSTR_CHECKEXACT(_x))
		return alifFloat_fromString(_x);
	return alifNumber_float(_x);
}

static AlifObject* float_subTypeNew(AlifTypeObject* _type, AlifObject* _x) { // 1610
	AlifObject* tmp{}, * newobj{};

	tmp = float_newImpl(&_alifFloatType_, _x);
	if (tmp == nullptr)
		return nullptr;
	newobj = _type->alloc(_type, 0);
	if (newobj == nullptr) {
		ALIF_DECREF(tmp);
		return nullptr;
	}
	((AlifFloatObject*)newobj)->val = ((AlifFloatObject*)tmp)->val;
	ALIF_DECREF(tmp);
	return newobj;
}

static AlifObject* float_vectorCall(AlifObject* _type, AlifObject* const* _args,
	AlifUSizeT _nargsf, AlifObject* _kwnames) { // 1621
	if (!_ALIFARG_NOKWNAMES("عشري", _kwnames)) {
		return nullptr;
	}

	AlifSizeT nargs = ALIFVECTORCALL_NARGS(_nargsf);
	if (!_ALIFARG_CHECKPOSITIONAL("عشري", nargs, 0, 1)) {
		return nullptr;
	}

	AlifObject* x = nargs >= 1 ? _args[0] : nullptr;
	return float_newImpl(ALIFTYPE_CAST(_type), x);
}



static AlifObject* float___format___impl(AlifObject* _self, AlifObject* _formatSpec) { // 1762
	AlifUStrWriter writer{};
	AlifIntT ret{};

	alifUStrWriter_init(&writer);
	ret = _alifFloat_formatAdvancedWriter( &writer, _self,
		_formatSpec, 0, ALIFUSTR_GET_LENGTH(_formatSpec));
	if (ret == -1) {
		alifUStrWriter_dealloc(&writer);
		return nullptr;
	}
	return alifUStrWriter_finish(&writer);
}

static AlifMethodDef _floatMethods_[] = { // 1781
	//FLOAT_FROM_NUMBER_METHODDEF
	//FLOAT_CONJUGATE_METHODDEF
	//FLOAT___TRUNC___METHODDEF
	//FLOAT___FLOOR___METHODDEF
	//FLOAT___CEIL___METHODDEF
	//FLOAT___ROUND___METHODDEF
	//FLOAT_AS_INTEGER_RATIO_METHODDEF
	//FLOAT_FROMHEX_METHODDEF
	//FLOAT_HEX_METHODDEF
	//FLOAT_IS_INTEGER_METHODDEF
	//FLOAT___GETNEWARGS___METHODDEF
	//FLOAT___GETFORMAT___METHODDEF
	FLOAT___FORMAT___METHODDEF
	{nullptr,              nullptr}           /* sentinel */
};


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
	.name = "عشري",
	.basicSize = sizeof(AlifFloatObject),
	.itemSize = 0,
	.repr = (ReprFunc)float_repr,
	.asNumber = &_floatAsNumber_,
	.hash = (HashFunc)float_hash,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
		_ALIF_TPFLAGS_MATCH_SELF,
	.richCompare = float_richCompare,
	.methods = _floatMethods_,
	.vectorCall = (VectorCallFunc)float_vectorCall,
};










AlifIntT alifFloat_initTypes(AlifInterpreter* _interp) { // 1951
	/* Init float info */
	if (_alifStructSequence_initBuiltin(_interp, &_floatInfoType_,
		nullptr /*&_floatInfoDesc_*/) < 0) {
		//return ALIFSTATUS_ERR("can't init float info type");
		return -1; //* alif
	}

	return 1; //* alif
}
