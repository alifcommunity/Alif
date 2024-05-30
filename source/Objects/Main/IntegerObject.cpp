#include "alif.h"

#include "AlifCore_Integer.h"
#include "AlifCore_UString.h"
#include "AlifCore_Object.h"
#include "AlifCore_Memory.h"

//////////////////////////////////

// لا يزال كائن عدد في نسخته الاولى ولا يقبل الا اعداد عشرية

//////////////////////////////////

static inline void alifSub_decref_int(AlifIntegerObject* _op)
{
	alifSub_decref_specialized((AlifObject*)_op, (Destructor)alifMem_objAlloc);
}

AlifIntegerObject* alifNew_integer(size_t _value, bool _sign) {

	AlifIntegerObject* object_ = (AlifIntegerObject*)alifMem_objAlloc(sizeof(AlifIntegerObject));

	alifSubObject_init((AlifObject*)object_,&_alifIntegerType_);

	object_->digits_ = _value;
	object_->sign_ = _sign;
	return object_;
}

AlifObject* alifSubInteger_copy(AlifIntegerObject* _src) {

	return (AlifObject*)alifNew_integer(_src->digits_, _src->sign_);

}

AlifObject* alifInteger_fromLongLong(int64_t _value) {

	bool sign_ = true;
	int64_t finalValue = _value;

	if (_value < 0) {
		sign_ = false;
		finalValue = -_value;
	}

	AlifObject* object_ = (AlifObject*)alifNew_integer(finalValue, sign_);

	return object_;
}

AlifObject* alifInteger_fromString(const wchar_t* _str) {

	size_t value_ = 0;
	bool sign_ = true;

	if (*_str == L'-') {
		sign_ = false;
		_str++;
	}
	else if (*_str == L'+') {
		_str++;
	}

	int len_ = count_characters(_str);

	if (len_ > MAX_LEN_DIGITS) {
		std::wcout << L"عدد الارقام المدخله اكبر من المتوقع\n" << std::endl;
		exit(-1);
	}

	for (int i = 0; i < len_; i++)
	{
		if (*_str >= L'0' && *_str <= L'9') {
			value_ = value_ * 10 + (*_str - L'0');
			_str++;
		}
	}

	AlifObject* object_ = (AlifObject*)alifNew_integer(value_, sign_);

	return object_;
}

AlifObject* alifinteger_fromUnicodeObject(AlifObject* _u, int _base)
{
	AlifObject* result_, * asciidig_;
	const wchar_t* buffer_;
	int64_t buflen;

	asciidig_ = alifSubUnicode_transformDecimalAndSpaceToASCII(_u);
	if (asciidig_ == nullptr)
		return nullptr;

	buffer_ = (const wchar_t*)(((AlifUStrObject*)asciidig_)->UTF);
	result_ = alifInteger_fromString(buffer_);
	if (result_ != nullptr ) {
		ALIF_DECREF(asciidig_);
		return result_;
	}
	ALIF_DECREF(asciidig_);
	ALIF_XDECREF(result_);

	return nullptr;
}

int64_t alifOS_strToLong(const wchar_t* _str) {

	int64_t value_ = 0;
	bool sign_ = true;

	if (*_str == L'-') {
		sign_ = false;
		_str++;
	}
	else if (*_str == L'+') {
		_str++;
	}

	int len_ = count_characters(_str);

	if (len_ > MAX_LEN_DIGITS) {
		std::wcout << L"عدد الارقام المدخله اكبر من المتوقع\n" << std::endl;
		exit(-1);
	}

	for (int i = 0; i < len_; i++)
	{
		if (*_str >= L'0' && *_str <= L'9') {
			value_ = value_ * 10 + (*_str - L'0');
			_str++;
		}
	}

	value_ = sign_ ? value_ : -value_;

	return value_;

}

AlifObject* alifInteger_fromSizeT(size_t _value, bool _sign) {

	return (AlifObject*)alifNew_integer(_value, _sign);

}

AlifObject* alifInteger_fromDouble(long double _value) {

	bool sign_ = true;
	size_t digits_ = 0;

	long double fractional_ = fmod(_value, 1.0);

	if (_value < 0) {
		sign_ = false;
		if (fractional_ >= 0.5) {
			digits_ = (_value + 1) * -1;
		}
		else {
			digits_ = _value * -1;
		}
	}
	else {
		if (fractional_ >= 0.5) {
			digits_ = _value + 1;
		}
		else {
			digits_ = _value;
		}
	}

	return (AlifObject*)alifNew_integer(digits_, sign_);
}

size_t alifInteger_asSizeT(AlifObject* _object) {

	if (_object == nullptr) {
		std::wcout << L"لا يوحد كائن رقم لارجاع قيمة\n" << std::endl;
		exit(-1);
	}

	AlifIntegerObject* value_ = (AlifIntegerObject*)_object;

	return value_->digits_;

}

bool alifInteger_asSign(AlifObject* _object) {

	if (_object == nullptr) {
		std::wcout << L"لا يوحد كائن رقم لارجاع علامة\n" << std::endl;
		exit(-1);
	}

	AlifIntegerObject* value_ = (AlifIntegerObject*)_object;

	return value_->sign_;
}

long alifInteger_asLong(AlifObject* _object) {

	if (_object == nullptr) {
		std::wcout << L"لا يوحد كائن رقم لارجاع علامة\n" << std::endl;
		exit(-1);
	}

	AlifIntegerObject* obj_ = (AlifIntegerObject*)_object;

	return (long)obj_->digits_;

}

int64_t alifInteger_asLongLong(AlifObject* _object) {

	if (_object == nullptr) {
		std::wcout << L"لا يوحد كائن رقم لارجاع علامة\n" << std::endl;
		exit(-1);
	}

	AlifIntegerObject* obj_ = (AlifIntegerObject*)_object;

	return (int64_t)obj_->digits_;

}

void flipSign(AlifIntegerObject* _num)
{
	_num->sign_ = !_num->sign_;
}

static AlifIntegerObject* add(AlifIntegerObject* _a, AlifIntegerObject* _b)
{
	AlifIntegerObject* result_ = alifNew_integer(0, true);
	result_->sign_ = false;

	result_->digits_ = _a->digits_ + _b->digits_;

	if (result_->digits_ < _a->digits_ or result_->digits_ < _b->digits_) {

		std::wcout << L"ناتج جمع رقمين اكبر من المتوقع\n" << std::endl;
		exit(-1);
	}

	return result_;
}

AlifIntegerObject* sub(AlifIntegerObject* _a, AlifIntegerObject* _b)
{
	AlifIntegerObject* result_ = alifNew_integer(0, true);
	result_->sign_ = false;

	if (_a->digits_ >= _b->digits_) {
		result_->digits_ = _a->digits_ - _b->digits_;
	}
	else {

		result_->sign_ = true;
		result_->digits_ = _b->digits_ - _a->digits_;
	}

	return result_;
}

static AlifIntegerObject* number_add(AlifIntegerObject* _a, AlifIntegerObject* _b)
{
	if (_a->sign_ && _b->sign_) {

		AlifIntegerObject* z_ = alifNew_integer(0, true);

		z_ = add(_a, _b);

		if (z_->digits_ != 0) {
			flipSign(z_);
		}
		return z_;
	}
	else if (!_a->sign_ && !_b->sign_) {

		return add(_a, _b);
	}
	else if (_a->sign_) {

		return sub(_b, _a);
	}
	else {

		return sub(_a, _b);
	}
}

static AlifIntegerObject* number_sub(AlifIntegerObject* _a, AlifIntegerObject* _b) {
	
	return number_add(_a,_b);
}

static AlifIntegerObject* number_mul(AlifIntegerObject* _a, AlifIntegerObject* _b) {

	AlifIntegerObject* z_ = alifNew_integer(0, true);

	if (_a->digits_ > 0 && _b->digits_ > UINT64_MAX / _a->digits_) {
		std::wcout << L"ناتج ضرب رقمين اكبر من المتوقع\n" << std::endl;
		exit(-1);
	}
	else {
		z_->digits_ = _a->digits_ * _b->digits_;
	}

	if (!_a->sign_ && !_b->sign_ || _a->sign_ && _b->sign_) {
		z_->sign_ = true;
	}
	else {
		z_->sign_ = false;
	}

	return z_;
}

static AlifIntegerObject* number_mod(AlifIntegerObject* _a, AlifIntegerObject* _b) {

	AlifIntegerObject* z_ = alifNew_integer(0, true);
	z_->sign_ = true;

	if (_a->digits_ > 0 && _b->digits_ > UINT64_MAX / _a->digits_) {
		std::wcout << L"ناتج القسمة المتبقية لرقمين اكبر من المتوقع\n" << std::endl;
		exit(-1);
	}

	if (_a->sign_ == _b->sign_) {
		z_->digits_ = _a->digits_ % _b->digits_;
		z_->sign_ = (!_a->sign_ && !_b->sign_) ? false : true;
	}
	else {
		z_->digits_ = _b->digits_ - 1 - (_a->digits_ - 1) % _b->digits_;
		z_->sign_ = (_a->sign_ && !_b->sign_) ? false : true;
	}

	return z_;

}

static AlifIntegerObject* number_pow(AlifIntegerObject* _a, AlifIntegerObject* _b) {

	if (_b->sign_ == false) {

		std::wcout << L"لا يمكن ان يكون الاس سالب\n" << std::endl;
		exit(-1);
	}

	AlifIntegerObject* z_ = alifNew_integer(0, true);

	if (_b->digits_ == 0) {
		z_->digits_ = 1;
		z_->sign_ = true;
		return z_;
	}

	z_->digits_ = 1;
	if (!_a->sign_ && (_b->digits_ % 2) == 1) {
		z_->sign_ = false;
	}

	for (size_t i = 0; i < _b->digits_; ++i) {

		if (z_->digits_ > SIZE_MAX / _a->digits_) {
			std::wcout << L"ناتج عملية ضرب القوة لرقمين اكبر من المتوقع\n" << std::endl;
			exit(-1);
		}

		z_->digits_ *= _a->digits_;
	}

	return z_;
}

static AlifIntegerObject* number_neg(AlifIntegerObject* _a) {

	AlifIntegerObject* z_ = alifNew_integer(0, true);

	z_->_base_ = _a->_base_;
	z_->digits_ = _a->digits_;
	z_->sign_ = (_a->sign_) ? false : true;
	return z_;
}


static AlifIntegerObject* number_abs(AlifIntegerObject* _a) {

	if (!_a->sign_) {
		return number_neg(_a);
	}
	else {
		AlifIntegerObject* z_ = alifNew_integer(0, true);

		z_->_base_ = _a->_base_;
		z_->digits_ = _a->digits_;
		z_->sign_ = true;
		return z_;
	}

}

static bool number_bool(AlifIntegerObject* _a) {

	if (_a->digits_ == 0) {
		return 0;
	}
	else {
		return 1;
	}
}

static AlifObject* integer_integer(AlifObject* _v)
{
	if (_v->type_ == &_alifIntegerType_) {
		return ALIF_NEWREF(_v);
	}
	else {
		return alifSubInteger_copy((AlifIntegerObject*)_v);
	}
}

int countDigits(size_t _value) {
	int count_ = 0;
	while (_value > 0) {
		_value /= 10;
		count_++;
	}
	return count_;
}

AlifObject* integer_to_string(AlifObject* _a) {

	if (_a == nullptr) {
		std::wcout << L"لا يمكن تحويل الكائن رقم الى نص لانه فارغ\n" << std::endl;
		exit(-1);
	}

	size_t value_ = ((AlifIntegerObject*)(_a))->digits_;

	bool sign_ = ((AlifIntegerObject*)(_a))->sign_;

	int8_t count_ = countDigits(value_);

	wchar_t str_[22]{};

	if (sign_ == false) {
		str_[0] = L'-';
	}

	for (int8_t i = 0; i < count_; i++)
	{
		str_[i] = L'0' + (wchar_t)(value_ % 10);
		value_ /= 10;
	}
	str_[20] = L'\0';

	AlifObject* result = alifUnicode_decodeStringToUTF8(str_);

	return result;
}

static size_t integer_hash(AlifIntegerObject* _a) {

	size_t hash_;
	bool sign_;

	hash_ = _a->digits_;
	sign_ = _a->sign_;

	hash_ ^= sign_ + 0x9e3779b9 + (hash_ << 6) + (hash_ >> 2);

	return hash_;

}

long double alifInteger_asDouble(AlifObject* _v) {

	bool sign_ = ((AlifIntegerObject*)_v)->sign_;
	long double digits_ = ((AlifIntegerObject*)_v)->digits_;

	long double result_ = sign_ ? digits_ : - digits_;

	return result_;

}

static AlifObject* integer_compare(AlifObject* _a, AlifObject* _b, int _op) {

	long result_;
	AlifIntegerObject* v_ = (AlifIntegerObject*)_a;
	AlifIntegerObject* w_ = (AlifIntegerObject*)_b;

	if (_a == _b) {
		result_ = 0;
	}
	else {
		result_ = 1;
	}

	ALIF_RETURN_RICHCOMPARE(result_, 0 , _op);
}

static AlifObject* integer_float(AlifObject* _v)
{
	long double result_;
	result_ = alifInteger_asDouble(_v);
	return alifFloat_fromDouble(result_);
}

static AlifObject* long_get0(AlifObject* ALIF_UNUSED(_self), void* ALIF_UNUSED(_context))
{
	return  alifInteger_fromLongLong(1L);
}

static AlifObject* long_get1(AlifObject* ALIF_UNUSED(_self), void* ALIF_UNUSED(_ignored))
{
	return alifInteger_fromLongLong(1L);
}

static AlifObject* int___sizeof__(AlifObject* _self) {

	AlifObject* returnValue = nullptr;
	size_t size_{};

	size_ = sizeof(AlifIntegerObject);

	return alifInteger_fromSizeT(size_, true);

}

static AlifObject* long_long_method(AlifObject* _self)
{
	return integer_integer(_self);
}

static AlifObject* int_is_integer(AlifObject* _self) {

	return ALIF_TRUE;

}

static AlifMethodDef _integerMethods_[] = {
	{L"__sizeof__", (AlifCFunction)int___sizeof__, METHOD_NOARGS},
	{L"is_integer", (AlifCFunction)int_is_integer, METHOD_NOARGS,},
	{nullptr,              nullptr}
};

static AlifGetSetDef _integerGetSet_[] = {
	{L"real",
	 (Getter)long_long_method, (Setter)nullptr,
	 L"the real part of a complex number",
	 nullptr},
	{L"imag",
	 long_get0, (Setter)nullptr,
	 L"the imaginary part of a complex number",
	 nullptr},
	{L"numerator",
	 (Getter)long_long_method, (Setter)nullptr,
	 L"the numerator of a rational number in lowest terms",
	 nullptr},
	{L"denominator",
	 long_get1, (Setter)nullptr,
	 L"the denominator of a rational number in lowest terms",
	 nullptr},
	{nullptr},  /* Sentinel */
};

static AlifNumberMethods _integerAsNumber_ = {

	(BinaryFunc)number_add,
	(BinaryFunc)number_sub,
	(BinaryFunc)number_mul,
	(BinaryFunc)number_mod,
	0,
	(BinaryFunc)number_pow,
	(UnaryFunc)number_neg,
	integer_integer,
	(UnaryFunc)number_abs,
	(Inquiry)number_bool,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	integer_float, 
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
	0,
	integer_integer,
};

AlifTypeObject _alifIntegerType_ = {
	0,
	0,
	0,
	L"int",
	offsetof(AlifIntegerObject, digits_),
	sizeof(size_t),
	0,
	0,
	0,
	0,
	integer_to_string,
	&_integerAsNumber_,
	0,
	0,
	(HashFunc)integer_hash,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	integer_compare,
	0,                                    
	0,                          
	0,                              
	_integerMethods_,                             
	0,                             
	_integerGetSet_,                            
	0,                          
	0,                          
	0,                               
	0,                               
	0,                                
	0,                          
	0,                           
	0,
	alifMem_objFree,
};

