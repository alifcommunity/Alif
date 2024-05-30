#include "alif.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"
#include "AlifCore_Memory.h"

long double alifFloat_getMax(void)
{
	return FLOAT_MAX;
}

long double alifFloat_getMin(void)
{
	return FLOAT_MIN;
}

AlifObject* alifNew_float(long double _value) {

	AlifFloatObject* object_ = (AlifFloatObject*)alifMem_objAlloc(sizeof(AlifFloatObject));

	alifSubObject_init((AlifObject*)object_, &_typeFloat_);

	object_->digits_ = _value;

	return (AlifObject*)object_;
}

AlifObject* alifFloat_fromDouble(long double _floatValue) {

	AlifFloatObject* object_ = (AlifFloatObject*)alifMem_objAlloc(sizeof(AlifFloatObject));
	
	alifSubObject_init((AlifObject*)object_, &_typeFloat_);
	object_->digits_ = _floatValue;
	return (AlifObject*)object_;

}

long double digit_strToDouble(const wchar_t* _str) {
	bool negative_ = false, isE_ = false;
	double whole_ = 0.0;
	long double fraction_ = 0.0;
	int exponent_ = 0;
	double _base_ = 10.0;

	// Skip leading whitespace
	while (std::isspace(*_str)) ++_str;

	if (*_str == '-') {
		negative_ = true;
		++_str;
	}
	else if (*_str == '+') {
		++_str;
	}

	if (*_str == '0') {
		return negative_ ? -0.0 : 0.0;
	}

	while (std::isdigit(*_str) && *_str != '0') {
		whole_ = whole_ * _base_ + (*_str - '0');
		++_str;
	}

	int64_t divisor = 10;
	if (*_str == '.') {
		++_str;

		// Read fractional part
		while (std::isdigit(*_str)) {
			// fraction type_ long double to get high res precition
			fraction_ = fraction_ + (*_str - '0') / (long double)divisor;
			divisor *= _base_;
			++_str;
		}
	}

	// Handle exponent
	if (*_str == 'e' || *_str == 'E') {
		++_str;
		isE_ = true;
		bool expNegative = false;
		if (*_str == '-') {
			expNegative = true;
			++_str;
		}
		else if (*_str == '+') {
			++_str;
		}

		// Read exponent value
		while (std::isdigit(*_str)) {
			exponent_ = exponent_ * 10 + (*_str - '0');
			++_str;
		}

		exponent_ = expNegative ? -exponent_ : exponent_;
		_base_ = std::pow(10.0, exponent_);
	}

	long double result_ = whole_ + (long double)fraction_;
	result_ = negative_ ? -result_ : result_;

	if (isE_) {
		result_ *= _base_;
	}

	if (result_ > FLOAT_MAX ||
		result_ < FLOAT_MIN) {
		return 0;
	}

	return result_;
}

AlifObject* alifFloat_fromString(AlifObject* _str) {

	if (_str->type_ != &_alifUStrType_) {
		std::wcout << L"يجب ان يكون نوع المدخل نص للتحويل الى عدد عشري\n" << std::endl;
		exit(-1);
	}

	AlifUStrObject* string_ = (AlifUStrObject*)_str;
	if (string_->kind == 4) {
		std::wcout << L"نوع الترميز غير صحيح في النص للتحويل الى عدد عشري\n" << std::endl;
		exit(-1);
	}

	const wchar_t* number_ = (const wchar_t*)string_->UTF;

	long double value_ = digit_strToDouble(number_);

	return alifFloat_fromDouble(value_);

}

long double alifFloat_asLongDouble(AlifObject* _object) {

	if (_object == nullptr) {
		std::wcout << L"لا يمكن ادخال كائن فارغ\n" << std::endl;
		exit(-1);
	}

	if (_object->type_ == &_typeFloat_) {
		return ((AlifFloatObject*)_object)->digits_;
	}

	AlifObject* result_ = (_object->type_->asNumber->float_)(_object);
	if (result_ == nullptr) {
		return -1;
	}
	long double value_ = ((AlifFloatObject*)result_)->digits_;
	return value_;

}

wchar_t* longDouble_toWchar_t(long double _value) {

	int bufferSize = snprintf(nullptr, 0, "%.17Le", _value);
	wchar_t* str_ = (wchar_t*)alifMem_dataAlloc(bufferSize + 1);
	swprintf(str_, bufferSize + 1, L"%.17Le", _value);

	return str_;
}

static AlifObject* float_repr(AlifFloatObject* _object) {

	wchar_t* buffer_ = longDouble_toWchar_t(_object->digits_);

	AlifObject* result_ = alifUStr_objFromWChar(buffer_);
	alifMem_dataFree(buffer_);
	return result_;
}

static AlifObject* float_add(AlifFloatObject* _a, AlifFloatObject* _b) {

	long double v_, w_;

	v_ = _a->digits_;
	w_ = _b->digits_;
	if (v_ && w_ > (FLOAT_MAX / 2)) {
		std::wcout << L"ناتج جمع رقمين عشريين اكبر من المتوقع\n" << std::endl;
		exit(-1);
	}
	v_ = v_ + w_;
	return (AlifObject*)alifNew_float(v_);
}

static AlifObject* float_sub(AlifFloatObject* _a, AlifFloatObject* _b) {

	long double v_, w_;

	v_ = _a->digits_;
	w_ = _b->digits_;
	v_ = v_ - w_;
	return (AlifObject*)alifNew_float(v_);
}

static AlifObject* float_mul(AlifFloatObject* _a, AlifFloatObject* _b) {

	long double v_, w_;

	v_ = _a->digits_;
	w_ = _b->digits_;
	v_ = v_ * w_;
	return (AlifObject*)alifNew_float(v_);
}

static AlifObject* float_div(AlifFloatObject* _a, AlifFloatObject* _b) {

	long double v_, w_;

	v_ = _a->digits_;
	w_ = _b->digits_;
	if (w_ == 0.0) {
		std::wcout << L"لا يمكن القسمة على صفر في رقم عشري\n" << std::endl;
		exit(-1);
	}
	v_ = v_ / w_;
	return (AlifObject*)alifNew_float(v_);
}

static AlifObject* float_rem(AlifFloatObject* _a, AlifFloatObject* _b) {

	long double v_, w_;
	long double mod_;

	v_ = _a->digits_;
	w_ = _b->digits_;
	if (w_ == 0.0) {
		std::wcout << L"لا يمكن القسمة على صفر في رقم عشري\n" << std::endl;
		exit(-1);
	}

	mod_ = fmod(v_, w_);
	if (mod_) {
		if ((w_ < 0) != (mod_ < 0)) {
			mod_ += w_;
		}
	}
	else {
		mod_ = copysign(0.0, w_);
	}

	v_ = v_ * w_;
	return (AlifObject*)alifNew_float(v_);
}

static AlifObject* float_neg(AlifFloatObject* _a) {
	return (AlifObject*)alifNew_float(-_a->digits_);
}

static AlifObject* float_abs(AlifFloatObject* _a) {
	return (AlifObject*)alifNew_float(fabs(_a->digits_));
}

static bool float_bool(AlifFloatObject* _a) {
	return _a->digits_ != 0.0;
}

static AlifObject* float_pow(AlifFloatObject* _a, AlifFloatObject* _b) {

	long double v_, w_;
	
	v_ = _a->digits_;
	w_ = _b->digits_;

	return (AlifObject*)alifNew_float(std::pow(v_,w_));

}

static size_t float_hash(AlifFloatObject* _a) {

	size_t hash_ = 0;

	hash_ ^= std::hash<long double>{}(_a->digits_) + 0x9e3779b9 + (hash_ << 6) + (hash_ >> 2);

	return hash_;
}

//static AlifObject* float_compare(AlifObject* a, AlifObject* b, int op) {
//
//	long result;
//	AlifFloatObject* v = (AlifFloatObject*)a;
//	AlifFloatObject* w = (AlifFloatObject*)b;
//
//	switch (op) {
//	case ALIF_LT:
//		return v->digits_ < w->digits_ ? ALIF_TRUE : ALIF_FALSE;
//
//	case ALIF_LE:
//		return v->digits_ < w->digits_ ? ALIF_TRUE : ALIF_FALSE;
//
//	case ALIF_EQ:
//		return v->digits_ == w->digits_ ? ALIF_TRUE : ALIF_FALSE;
//
//	case ALIF_NE:
//		return v->digits_ != w->digits_ ? ALIF_TRUE : ALIF_FALSE;
//
//	case ALIF_GT:
//		return v->digits_ > w->digits_ ? ALIF_TRUE : ALIF_FALSE;
//
//	case ALIF_GE:
//		return v->digits_ > w->digits_ ? ALIF_TRUE : ALIF_FALSE;
//
//	default:
//		std::wcout << L"عملية المقارنة غير صحيحة في كائن رقم عشري\n" << std::endl;
//		exit(-1);
//	}
//}

static AlifObject* float_compare(AlifObject* _v, AlifObject* _w, int _op)
{
	long double i_, j_;
	int r_ = 0;

	i_ = ((AlifFloatObject*)_v)->digits_;

	/* Switch on the type of w.  Set i and j to doubles to be compared,
	 * and op to the richcomp to use.
	 */
	if (((AlifFloatObject*)_w)->_base_.type_ == &_typeFloat_)
		j_ = ((AlifFloatObject*)_w)->digits_;

	else if (!isfinite(i_)) {
		if (((AlifIntegerObject*)_w)->_base_.type_ == &_alifIntegerType_)
			/* If i is an infinity, its magnitude exceeds any
			 * finite integer, so it doesn't matter which int we
			 * compare i with.  If i is a NaN, similarly.
			 */
			j_ = 0.0;
		//else
			//goto Unimplemented;
	}

	else if (((AlifIntegerObject*)_w)->_base_.type_ == &_alifIntegerType_) {
		int vSign = i_ == 0.0 ? 0 : i_ < 0.0 ? -1 : 1;
		int wSign = ((AlifIntegerObject*)_w)->sign_;
		size_t nBits{};
		int exponent_{};

		if (vSign != wSign) {
			/* Magnitudes are irrelevant -- the signs alone
			 * determine the outcome.
			 */
			i_ = (double)vSign;
			j_ = (double)wSign;
			goto Compare;
		}
		/* The signs are the same. */
		/* Convert w to a double if it fits.  In particular, 0 fits. */
		//nBits = _PyLong_NumBits(w);
		//if (nBits == (size_t)-1 && PyErr_Occurred()) {
		//	/* This long is so large that size_t isn't big enough
		//	 * to hold the # of bits.  Replace with little doubles
		//	 * that give the same outcome -- w is so large that
		//	 * its magnitude must exceed the magnitude of any
		//	 * finite float.
		//	 */
		//	PyErr_Clear();
		//	i = (double)vsign;
		//	j = wsign * 2.0;
		//	goto Compare;
		//}
		//if (nbits <= 48) {
		//	j = PyLong_AsDouble(w);
		//	/* It's impossible that <= 48 bits overflowed. */
		//	goto Compare;
		//}

		//if (vsign < 0) {
		//	/* "Multiply both sides" by -1; this also swaps the
		//	 * comparator.
		//	 */
		//	i = -i;
		//	op = _Py_SwappedOp[op];
		//}
		//(void)frexp(i, &exponent);
		/* exponent is the # of bits in v before the radix point;
		 * we know that nbits (the # of bits in w) > 48 at this point
		 */
		if (exponent_ < 0 || (size_t)exponent_ < nBits) {
			i_ = 1.0;
			j_ = 2.0;
			goto Compare;
		}
		if ((size_t)exponent_ > nBits) {
			i_ = 2.0;
			j_ = 1.0;
			goto Compare;
		}
		/* v and w have the same number of bits before the radix
		 * point.  Construct two ints that have the same comparison
		 * outcome.
		 */
		{
			double fracPart;
			double intPart;
			AlifObject* result_ = nullptr;
			AlifObject* vv_ = nullptr;
			AlifObject* ww_ = _w;

			if (wSign < 0) {
				//ww_ = PyNumber_Negative(w);
				if (ww_ == nullptr)
					goto Error;
			}
			else
				ALIF_INCREF(ww_);

			fracPart = modf(i_, &intPart);
			vv_ = alifInteger_fromDouble(intPart);
			if (vv_ == nullptr)
				goto Error;

			if (fracPart != 0.0) {
				/* Shift left, and or a 1 bit into vv
				 * to represent the lost fraction.
				 */
				AlifObject* temp_{};

				//temp_ = _PyLong_Lshift(ww, 1);
				if (temp_ == nullptr)
					goto Error;
				ALIF_SETREF(ww_, temp_);

				//temp_ = _PyLong_Lshift(vv_, 1);
				if (temp_ == nullptr)
					goto Error;
				ALIF_SETREF(vv_, temp_);

				//temp_ = PyNumber_Or(vv, _PyLong_GetOne());
				if (temp_ == nullptr)
					goto Error;
				ALIF_SETREF(vv_, temp_);
			}

			r_ = alifObject_richCompareBool(vv_, ww_, _op);
			if (r_ < 0)
				goto Error;
			result_ = alifBool_fromInteger(r_);
		Error:
			ALIF_XDECREF(vv_);
			ALIF_XDECREF(ww_);
			return result_;
		}
	}

	//else        /* w isn't float or int */
		//goto Unimplemented;

Compare:
	switch (_op) {
	case ALIF_EQ:
		r_ = i_ == j_;
		break;
	case ALIF_NE:
		r_ = i_ != j_;
		break;
	case ALIF_LE:
		r_ = i_ <= j_;
		break;
	case ALIF_GE:
		r_ = i_ >= j_;
		break;
	case ALIF_LT:
		r_ = i_ < j_;
		break;
	case ALIF_GT:
		r_ = i_ > j_;
		break;
	}
	return alifBool_fromInteger(r_);

//Unimplemented:
	//ALIF_RETURN_NOTIMPLEMENTED;
}

static void float_dealloc(AlifObject* object) {

	alifMem_objFree(object);

}

static AlifObject* float_is_integer_(AlifObject* self) {

	long double value = ((AlifFloatObject*)self)->digits_;

	if (isfinite(value)) {
		return nullptr;
	}

	AlifObject* object = (floor(value) == value) ? ALIF_TRUE : ALIF_FALSE;
	return object;

}

static AlifObject* float__ceil__(AlifObject* self) {

	long double value = ((AlifFloatObject*)self)->digits_;

	return alifInteger_fromDouble(ceil(value));

}

static AlifObject* float__floor__(AlifObject* self) {

	long double value = ((AlifFloatObject*)self)->digits_;

	return alifInteger_fromDouble(floor(value));

}

static AlifObject* float_float(AlifObject* _v)
{
	if (_v->type_ == &_typeFloat_) {
		return ALIF_NEWREF(_v);
	}
	else {
		return alifFloat_fromDouble(((AlifFloatObject*)_v)->digits_);
	}
}

static AlifObject* float_subtype_new(AlifTypeObject* , AlifObject*);

static AlifObject* float_new_impl(AlifTypeObject* _type, AlifObject* _x)
{
	if (_type != &_typeFloat_) {
		//if (_x == nullptr) {
		//	_x = alifSubLong_GetZero();
		//}
		return float_subtype_new(_type, _x); /* Wimp out */
	}

	if (_x == nullptr) {
		return alifFloat_fromDouble(0.0);
	}

	if (_x->type_ == &_alifUStrType_)
		return alifFloat_fromString(_x);
	return alifInteger_float(_x);
}

static AlifObject* float_subtype_new(AlifTypeObject* _type, AlifObject* _x)
{
	AlifObject* tmp_{}, * newObj{};

	tmp_ = float_new_impl(&_typeFloat_, _x);
	if (tmp_ == nullptr)
		return nullptr;
	newObj = _type->alloc_(_type, 0);
	if (newObj == nullptr) {
		ALIF_DECREF(tmp_);
		return nullptr;
	}
	((AlifFloatObject*)newObj)->digits_ = ((AlifFloatObject*)tmp_)->digits_;
	ALIF_DECREF(tmp_);
	return newObj;
}

static AlifObject* float_vectorcall(AlifObject* _type, AlifObject* const* _args,
	size_t _nargsf, AlifObject* _kwnames)
{
	if (!ALIFSUBARG_NOKWNAMES(L"float", _kwnames)) {
		return nullptr;
	}
	
	int64_t nArgs = ALIFVECTORCALL_NARGS(_nargsf);
	if (!ALIFSUBARG_CHECKPOSITIONAL(L"float", nArgs, 0, 1)) {
		return nullptr;
	}

	AlifObject* x_ = nArgs >= 1 ? _args[0] : nullptr;
	return float_new_impl((AlifTypeObject*)_type, x_);
}

static AlifObject* float_getReal(AlifObject* _v, void* _closure)
{
	return float_float(_v);
}

static AlifObject* float_getImag(AlifObject* _v, void* _closure)
{
	return alifFloat_fromDouble(0.0);
}

static AlifMethodDef _floatMethod_[] = {
	{L"__floor__", (AlifCFunction)float__floor__, METHOD_NOARGS},
	{L"__ceil__", (AlifCFunction)float__ceil__, METHOD_NOARGS},
	{L"is_integer", (AlifCFunction)float_is_integer_, METHOD_NOARGS},
	{nullptr,             nullptr}
};

static AlifGetSetDef _floatGetSet_[] = {
	{L"real",
	 float_getReal, (Setter)nullptr,
	 L"the real part of a complex number",
	 nullptr},
	{L"imag",
	 float_getImag, (Setter)nullptr,
	 L"the imaginary part of a complex number",
	 nullptr},
	{nullptr}  /* Sentinel */
};

static AlifNumberMethods _floatAsNumber_ = {
	(BinaryFunc)float_add,          
	(BinaryFunc)float_sub,          
	(BinaryFunc)float_mul,          
	(BinaryFunc)float_rem,          
	0,     
	(BinaryFunc)float_pow,          
	(UnaryFunc)float_neg, 
	float_float,        
	(UnaryFunc)float_abs, 
	(Inquiry)float_bool, 
	0,                  
	0,                  
	0,                  
	0,                  
	0,                  
	0,                
	0, 
	0,                  
	float_float,       
	0,                  
	0,                  
	0,                  
	0,                 
	0,                 
	0,                  
	0,                  
	0,               
	0,               
	0,              
	0,    
	(BinaryFunc)float_div,                
};

AlifTypeObject _typeFloat_ = {
	0,
	0,
	0,
	L"float",
	sizeof(AlifFloatObject),
	0,
	float_dealloc,                  
	0,                                          
	0,                                          
	0,                                         
	(ReprFunc)float_repr,
	&_floatAsNumber_,                          
	0,                      
	0,                                          
	(HashFunc)float_hash,                       
	0,                                         
	0,                                  
	0,                    
	0,                                       
	0,              
	0,                         
	0,                                         
	0,
	0,                                         
	(RichCmpFunc)float_compare,                      
	0,                                       
	0,             
	0,                                      
	_floatMethod_,
	0,                                          
	_floatGetSet_,                               
	0,                                          
	0,                                         
	0,                                          
	0,                                         
	0,                                         
	0,                                          
	0,                                        
	0,
	0,
	0,
	0,
	0,
	0,
	(VectorCallFunc)float_vectorcall,
};
