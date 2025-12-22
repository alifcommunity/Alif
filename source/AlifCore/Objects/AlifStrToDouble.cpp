#include "alif.h"

#include "AlifCore_DoubleToASCII.h"





double _alifParse_infOrNan(const char* p, char** endptr) { // 27
	double retval{};
	const char* s{};
	AlifIntT negate = 0;

	s = p;
	if (*s == '-') {
		negate = 1;
		s++;
	}
	else if (*s == '+') {
		s++;
	}
	if (strcmp(s, "لانهائي") == 0) { //* alif
		s += 14;
		retval = negate ? -ALIF_HUGE_VAL : ALIF_HUGE_VAL;
	}
	else if (strcmp(s, "عدم") == 0) { //* alif
		s += 6;
		retval = negate ? -fabs(ALIF_NAN) : fabs(ALIF_NAN);
	}
	else {
		s = p;
		retval = -1.0;
	}
	*endptr = (char*)s;
	return retval;
}


//#if ALIF_SHORT_FLOAT_REPR == 1 // 90

static double _alifOS_asciiStrToDouble(const char* _nPtr, char** _endPtr) { // 92
	double result;
	//_ALIF_SET_53BIT_PRECISION_HEADER;

	errno = 0;

	//_ALIF_SET_53BIT_PRECISION_START;
	result = _alif_dgStrToDouble(_nPtr, _endPtr);
	//_ALIF_SET_53BIT_PRECISION_END;

	if (*_endPtr == _nPtr)
		/* string might represent an inf or nan */
		result = _alifParse_infOrNan(_nPtr, _endPtr);

	return result;

}

//#else // 115

 // func

//#endif // 272



double alifOS_stringToDouble(const char* _s,
	char** _endPtr, AlifObject* _overflowException) { // 298
	double x{}, result = -1.0;
	char* failPos{};

	errno = 0;
	x = _alifOS_asciiStrToDouble(_s, &failPos);

	if (errno == ENOMEM) {
		//alifErr_noMemory();
		failPos = (char*)_s;
	}
	else if (!_endPtr and (failPos == _s or *failPos != '\0')) {
		//alifErr_format(_alifExcValueError_,
		//	"could not convert string to float: "
		//	"'%.200s'", _s);
	}
	else if (failPos == _s) {
		//alifErr_format(_alifExcValueError_,
		//	"could not convert string to float: "
		//	"'%.200s'", _s);
	}
	else if (errno == ERANGE and fabs(x) >= 1.0 && _overflowException) {
		//alifErr_format(_overflowException,
		//	"value too large to convert to float: "
		//	"'%.200s'", _s);
	}
	else result = x;

	if (_endPtr != nullptr) *_endPtr = failPos;
	return result;
}



AlifObject* _alifString_toNumberWithUnderscores(const char* _s,
	AlifSizeT _origLen, const char* _what, AlifObject* _obj, void* _arg,
	AlifObject* (*_innerFunc)(const char*, AlifSizeT, void*)) { // 344
	char prev{};
	const char* p{}, * last{};
	char* dup{}, * end{};
	AlifObject* result{};

	if (strchr(_s, '_') == nullptr) {
		return _innerFunc(_s, _origLen, _arg);
	}

	dup = (char*)alifMem_dataAlloc(_origLen + 1);
	if (dup == nullptr) {
		//return alifErr_noMemory();
		return nullptr; //* delete
	}
	end = dup;
	prev = '\0';
	last = _s + _origLen;
	for (p = _s; *p; p++) {
		if (*p == '_') {
			/* Underscores are only allowed after digits. */
			if (!(prev >= '0' && prev <= '9')) {
				goto error;
			}
		}
		else {
			*end++ = *p;
			/* Underscores are only allowed before digits. */
			if (prev == '_' && !(*p >= '0' && *p <= '9')) {
				goto error;
			}
		}
		prev = *p;
	}
	/* Underscores are not allowed at the end. */
	if (prev == '_') {
		goto error;
	}
	/* No embedded NULs allowed. */
	if (p != last) {
		goto error;
	}
	*end = '\0';
	result = _innerFunc(dup, end - dup, _arg);
	alifMem_dataFree(dup);
	return result;

error:
	alifMem_dataFree(dup);
	//alifErr_format(_alifExcValueError_,
	//	"could not convert string to %s: "
	//	"%R", what, obj);
	return nullptr;
}









 // 923
#define OFS_INF 0
#define OFS_NAN 1
#define OFS_E 2

/* The lengths of these are known to the code below, so don't change them */
static const char* const _lcFloatStrings_[] = { // 928
	"inf",
	"nan",
	"e",
};
static const char* const _ucFloatStrings_[] = { // 933
	"INF",
	"NAN",
	"E",
};


static char* format_floatShort(double _d, char _formatCode,
	AlifIntT _mode, AlifIntT _precision, AlifIntT _alwaysAddSign,
	AlifIntT _addDot0IfInteger, AlifIntT _useAltFormatting,
	AlifIntT _noNegativeZero, const char* const* _floatStrings,
	AlifIntT* _type) { // 968

	char* buf = nullptr;
	char* p = nullptr;
	AlifSizeT bufsize = 0;
	char* digits{}, * digits_end{};
	AlifIntT decpt_as_int{}, sign{}, exp_len{}, exp = 0, use_exp = 0;
	AlifSizeT decpt{}, digits_len{}, vdigits_start{}, vdigits_end{};

	digits = _alif_dgDoubletoASCII(_d, _mode, _precision, &decpt_as_int, &sign,
		&digits_end);

	decpt = (AlifSizeT)decpt_as_int;
	if (digits == nullptr) {
		/* The only failure mode is no memory. */
		//alifErr_noMemory();
		goto exit;
	}
	digits_len = digits_end - digits;

	if (_noNegativeZero and sign == 1 and
		(digits_len == 0 or (digits_len == 1 and digits[0] == '0'))) {
		sign = 0;
	}

	if (digits_len and !ALIF_ISDIGIT(digits[0])) {
		if (digits[0] == 'n' or digits[0] == 'N')
			sign = 0;

		bufsize = 5;
		buf = (char*)alifMem_dataAlloc(bufsize);
		if (buf == nullptr) {
			//alifErr_noMemory();
			goto exit;
		}
		p = buf;

		if (sign == 1) {
			*p++ = '-';
		}
		else if (_alwaysAddSign) {
			*p++ = '+';
		}
		if (digits[0] == 'i' or digits[0] == 'I') {
			strncpy(p, _floatStrings[OFS_INF], 3);
			p += 3;

			if (_type)
				*_type = ALIF_DTST_INFINITE;
		}
		else if (digits[0] == 'n' or digits[0] == 'N') {
			strncpy(p, _floatStrings[OFS_NAN], 3);
			p += 3;

			if (_type)
				*_type = ALIF_DTST_NAN;
		}
		else {
			ALIF_UNREACHABLE();
		}
		goto exit;
	}

	if (_type)
		*_type = ALIF_DTST_FINITE;

	vdigits_end = digits_len;
	switch (_formatCode) {
	case 'e':
		use_exp = 1;
		vdigits_end = _precision;
		break;
	case 'f':
		vdigits_end = decpt + _precision;
		break;
	case 'g':
		if (decpt <= -4 or decpt >
			(_addDot0IfInteger ? _precision - 1 : _precision))
			use_exp = 1;
		if (_useAltFormatting)
			vdigits_end = _precision;
		break;
	case 'r':
		if (decpt <= -4 or decpt > 16)
			use_exp = 1;
		break;
	default:
		//ALIFERR_BADINTERNALCALL();
		goto exit;
	}

	if (use_exp) {
		exp = (int)decpt - 1;
		decpt = 1;
	}

	vdigits_start = decpt <= 0 ? decpt - 1 : 0;
	if (!use_exp and _addDot0IfInteger)
		vdigits_end = vdigits_end > decpt ? vdigits_end : decpt + 1;
	else
		vdigits_end = vdigits_end > decpt ? vdigits_end : decpt;


	bufsize = 3 + (vdigits_end - vdigits_start) + (use_exp ? 5 : 0);

	buf = (char*)alifMem_dataAlloc(bufsize);
	if (buf == nullptr) {
		//alifErr_noMemory();
		goto exit;
	}
	p = buf;

	if (sign == 1)
		*p++ = '-';
	else if (_alwaysAddSign)
		*p++ = '+';

	if (decpt <= 0) {
		memset(p, '0', decpt - vdigits_start);
		p += decpt - vdigits_start;
		*p++ = '.';
		memset(p, '0', 0 - decpt);
		p += 0 - decpt;
	}
	else {
		memset(p, '0', 0 - vdigits_start);
		p += 0 - vdigits_start;
	}

	if (0 < decpt and decpt <= digits_len) {
		strncpy(p, digits, decpt - 0);
		p += decpt - 0;
		*p++ = '.';
		strncpy(p, digits + decpt, digits_len - decpt);
		p += digits_len - decpt;
	}
	else {
		strncpy(p, digits, digits_len);
		p += digits_len;
	}

	if (digits_len < decpt) {
		memset(p, '0', decpt - digits_len);
		p += decpt - digits_len;
		*p++ = '.';
		memset(p, '0', vdigits_end - decpt);
		p += vdigits_end - decpt;
	}
	else {
		memset(p, '0', vdigits_end - digits_len);
		p += vdigits_end - digits_len;
	}

	if (p[-1] == '.' and !_useAltFormatting)
		p--;

	if (use_exp) {
		*p++ = _floatStrings[OFS_E][0];
		exp_len = sprintf(p, "%+.02d", exp);
		p += exp_len;
	}
exit:
	if (buf) {
		*p = '\0';
	}
	if (digits)
		_alif_dgFreeDoubleToASCII(digits);

	return buf;
}


char* alifOS_doubleToString(double _val, char _formatCode,
	AlifIntT _precision, AlifIntT _flags, AlifIntT* _type) { // 1221
	const char* const* float_strings = _lcFloatStrings_;
	AlifIntT mode{};

	/* Validate format_code, and map upper and lower case. Compute the
	   mode and make any adjustments as needed. */
	switch (_formatCode) {
		/* exponent */
	case 'E':
		float_strings = _ucFloatStrings_;
		_formatCode = 'e';
		ALIF_FALLTHROUGH;
	case 'e':
		mode = 2;
		_precision++;
		break;

		/* fixed */
	case 'F':
		float_strings = _ucFloatStrings_;
		_formatCode = 'f';
		ALIF_FALLTHROUGH;
	case 'f':
		mode = 3;
		break;

		/* general */
	case 'G':
		float_strings = _ucFloatStrings_;
		_formatCode = 'g';
		ALIF_FALLTHROUGH;
	case 'g':
		mode = 2;
		/* precision 0 makes no sense for 'g' format; interpret as 1 */
		if (_precision == 0)
			_precision = 1;
		break;

		/* repr format */
	case 'r':
		mode = 0;
		/* Supplied precision is unused, must be 0. */
		if (_precision != 0) {
			//ALIFERR_BADINTERNALCALL();
			return nullptr;
		}
		break;

	default:
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}

	return format_floatShort(_val, _formatCode, mode, _precision,
		_flags & ALIF_DTSF_SIGN,
		_flags & ALIF_DTSF_ADD_DOT_0,
		_flags & ALIF_DTSF_ALT,
		_flags & ALIF_DTSF_NO_NEG_0,
		float_strings, _type);
}
