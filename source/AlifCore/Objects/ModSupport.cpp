#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Object.h"


static AlifObject* va_buildValue(const char*, va_list); // 10

static AlifSizeT count_format(const char* _format, char _endchar) { // 39
	AlifSizeT count = 0;
	AlifIntT level = 0;
	while (level > 0 or *_format != _endchar) {
		switch (*_format) {
		case '\0':
			/* Premature end */
			//alifErr_setString(_alifExcSystemError_,
			//	"unmatched paren in format");
			return -1;
		case '(':
		case '[':
		case '{':
			if (level == 0) {
				count++;
			}
			level++;
			break;
		case ')':
		case ']':
		case '}':
			level--;
			break;
		case '#':
		case '&':
		case ',':
		case ':':
		case ' ':
		case '\t':
			break;
		default:
			if (level == 0) {
				count++;
			}
		}
		_format++;
	}
	return count;
}




static AlifObject* do_mkTuple(const char**, va_list*, char, AlifSizeT); // 85
static AlifIntT do_mkStack(AlifObject**, const char**, va_list*, char, AlifSizeT);
static AlifObject* do_mkList(const char**, va_list*, char, AlifSizeT);
static AlifObject* do_mkDict(const char**, va_list*, char, AlifSizeT);
static AlifObject* do_mkValue(const char**, va_list*);

static AlifIntT check_end(const char** _pFormat, char _endChar) { // 91
	const char* f = *_pFormat;
	while (*f != _endChar) {
		if (*f != ' ' and *f != '\t' and *f != ',' and *f != ':') {
			//alifErr_setString(_alifExcSystemError_,
			//	"Unmatched paren in format");
			return 0;
		}
		f++;
	}
	if (_endChar) {
		f++;
	}
	*_pFormat = f;
	return 1;
}



static void do_ignore(const char** _pFormat,
	va_list* _pVa, char _endChar, AlifSizeT _n) { // 110
	AlifObject* v = alifTuple_new(_n);
	for (AlifSizeT i = 0; i < _n; i++) {
		//AlifObject* exc = alifErr_getRaisedException();
		AlifObject* w = do_mkValue(_pFormat, _pVa);
		//alifErr_setRaisedException(exc);
		if (w != nullptr) {
			if (v != nullptr) {
				ALIFTUPLE_SET_ITEM(v, i, w);
			}
			else {
				ALIF_DECREF(w);
			}
		}
	}
	ALIF_XDECREF(v);
	if (!check_end(_pFormat, _endChar)) {
		return;
	}
}



static AlifObject* do_mkDict(const char** _pFormat,
	va_list* _pVa, char _endChar, AlifSizeT _n) { // 134
	AlifObject* d{};
	AlifSizeT i{};
	if (_n < 0) return nullptr;
	if (_n % 2) {
		//alifErr_setString(_alifExcSystemError_,
		//	"Bad dict format");
		do_ignore(_pFormat, _pVa, _endChar, _n);
		return nullptr;
	}

	if ((d = alifDict_new()) == nullptr) {
		do_ignore(_pFormat, _pVa, _endChar, _n);
		return nullptr;
	}
	for (i = 0; i < _n; i += 2) {
		AlifObject* k{}, * v{};

		k = do_mkValue(_pFormat, _pVa);
		if (k == nullptr) {
			do_ignore(_pFormat, _pVa, _endChar, _n - i - 1);
			ALIF_DECREF(d);
			return nullptr;
		}
		v = do_mkValue(_pFormat, _pVa);
		if (v == nullptr or alifDict_setItem(d, k, v) < 0) {
			do_ignore(_pFormat, _pVa, _endChar, _n - i - 2);
			ALIF_DECREF(k);
			ALIF_XDECREF(v);
			ALIF_DECREF(d);
			return nullptr;
		}
		ALIF_DECREF(k);
		ALIF_DECREF(v);
	}
	if (!check_end(_pFormat, _endChar)) {
		ALIF_DECREF(d);
		return nullptr;
	}
	return d;
}


static AlifObject* do_mkList(const char** _pFormat,
	va_list* _pVa, char endChar, AlifSizeT n) { // 180
	AlifObject* v{};
	AlifSizeT i{};
	if (n < 0) return nullptr;
	v = alifList_new(n);
	if (v == nullptr) {
		do_ignore(_pFormat, _pVa, endChar, n);
		return nullptr;
	}
	for (i = 0; i < n; i++) {
		AlifObject* w = do_mkValue(_pFormat, _pVa);
		if (w == nullptr) {
			do_ignore(_pFormat, _pVa, endChar, n - i - 1);
			ALIF_DECREF(v);
			return nullptr;
		}
		ALIFLIST_SET_ITEM(v, i, w);
	}
	if (!check_end(_pFormat, endChar)) {
		ALIF_DECREF(v);
		return nullptr;
	}
	return v;
}


static AlifIntT do_mkStack(AlifObject** _stack, const char** _pFormat, va_list* _pVa,
	char _endChar, AlifSizeT _n) { // 211
	AlifSizeT i_{};

	if (_n < 0) {
		return -1;
	}

	for (i_ = 0; i_ < _n; i_++) {
		AlifObject* w = do_mkValue(_pFormat, _pVa);
		if (w == nullptr) {
			do_ignore(_pFormat, _pVa, _endChar, _n - i_ - 1);
			goto error;
		}
		_stack[i_] = w;
	}
	if (!check_end(_pFormat, _endChar)) {
		goto error;
	}
	return 0;

error:
	_n = i_;
	for (i_ = 0; i_ < _n; i_++) {
		ALIF_DECREF(_stack[i_]);
	}
	return -1;
}

static AlifObject* do_mkTuple(const char** _pFormat,
	va_list* _pVa, char _endChar, AlifSizeT n) { // 242
	AlifObject* v{};
	AlifSizeT i{};
	if (n < 0)
		return nullptr;
	if ((v = alifTuple_new(n)) == nullptr) {
		do_ignore(_pFormat, _pVa, _endChar, n);
		return nullptr;
	}
	for (i = 0; i < n; i++) {
		AlifObject* w = do_mkValue(_pFormat, _pVa);
		if (w == nullptr) {
			do_ignore(_pFormat, _pVa, _endChar, n - i - 1);
			ALIF_DECREF(v);
			return nullptr;
		}
		ALIFTUPLE_SET_ITEM(v, i, w);
	}
	if (!check_end(_pFormat, _endChar)) {
		ALIF_DECREF(v);
		return nullptr;
	}
	return v;
}




static AlifObject* do_mkValue(const char** _pFormat, va_list* _pVa) { // 272
	for (;;) {
		switch (*(*_pFormat)++) {
		case '(':
			return do_mkTuple(_pFormat, _pVa, ')',
				count_format(*_pFormat, ')'));

		case '[':
			return do_mkList(_pFormat, _pVa, ']',
				count_format(*_pFormat, ']'));

		case '{':
			return do_mkDict(_pFormat, _pVa, '}',
				count_format(*_pFormat, '}'));

		case 'b':
		case 'B':
		case 'h':
		case 'i':
			return alifLong_fromLong((long)va_arg(*_pVa, int));

		case 'H':
			return alifLong_fromLong((long)va_arg(*_pVa, unsigned int));

		case 'I':
		{
			AlifUIntT n{};
			n = va_arg(*_pVa, unsigned int);
			return alifLong_fromUnsignedLong(n);
		}

//		case 'n':
//#if SIZEOF_SIZE_T!=SIZEOF_LONG
//			return alifLong_fromSizeT(va_arg(*_pVa, AlifSizeT));
//#endif
//			/* Fall through from 'n' to 'l' if AlifSizeT is long */
//			ALIF_FALLTHROUGH;
		case 'l':
			return alifLong_fromLong(va_arg(*_pVa, long));

		case 'k':
		{
			unsigned long n;
			n = va_arg(*_pVa, unsigned long);
			return alifLong_fromUnsignedLong(n);
		}

		//case 'L':
		//	return alifLong_fromLongLong((long long)va_arg(*_pVa, long long));

		case 'K':
			return alifLong_fromUnsignedLongLong((long long)va_arg(*_pVa, unsigned long long));

		case 'u':
		{
			AlifObject* v{};
			const wchar_t* u = va_arg(*_pVa, wchar_t*);
			AlifSizeT n{};
			if (**_pFormat == '#') {
				++*_pFormat;
				n = va_arg(*_pVa, AlifSizeT);
			}
			else
				n = -1;
			if (u == nullptr) {
				v = ALIF_NEWREF(ALIF_NONE);
			}
			else {
				if (n < 0)
					n = wcslen(u);
				v = alifUStr_fromWideChar(u, n);
			}
			return v;
		}
		case 'f':
		case 'd':
			return alifFloat_fromDouble(
				(double)va_arg(*_pVa, double));

		//case 'D':
		//	return alifComplex_fromCComplex(
		//		*((AlifComplex*)va_arg(*_pVa, AlifComplex*)));

		case 'c':
		{
			char p[1];
			p[0] = (char)va_arg(*_pVa, AlifIntT);
			return alifBytes_fromStringAndSize(p, 1);
		}
		//case 'C':
		//{
		//	AlifIntT i = va_arg(*_pVa, AlifIntT);
		//	return alifUStr_fromOrdinal(i);
		//}

		case 's':
		case 'z':
		case 'U':   /* XXX deprecated alias */
		{
			AlifObject* v{};
			const char* str = va_arg(*_pVa, const char*);
			AlifSizeT n{};
			if (**_pFormat == '#') {
				++*_pFormat;
				n = va_arg(*_pVa, AlifSizeT);
			}
			else
				n = -1;
			if (str == nullptr) {
				v = ALIF_NEWREF(ALIF_NONE);
			}
			else {
				if (n < 0) {
					AlifUSizeT m = strlen(str);
					if (m > ALIF_SIZET_MAX) {
						//alifErr_setString(_alifExcOverflowError_,
						//	"string too long for Alif string");
						return nullptr;
					}
					n = (AlifSizeT)m;
				}
				v = alifUStr_fromStringAndSize(str, n);
			}
			return v;
		}

		case 'y':
		{
			AlifObject* v{};
			const char* str = va_arg(*_pVa, const char*);
			AlifSizeT n;
			if (**_pFormat == '#') {
				++*_pFormat;
				n = va_arg(*_pVa, AlifSizeT);
			}
			else
				n = -1;
			if (str == nullptr) {
				v = ALIF_NEWREF(ALIF_NONE);
			}
			else {
				if (n < 0) {
					AlifUSizeT m = strlen(str);
					if (m > ALIF_SIZET_MAX) {
						//alifErr_setString(_alifExcOverflowError_,
						//	"string too long for Alif bytes");
						return nullptr;
					}
					n = (AlifSizeT)m;
				}
				v = alifBytes_fromStringAndSize(str, n);
			}
			return v;
		}

		case 'N':
		case 'S':
		case 'O':
			if (**_pFormat == '&') {
				typedef AlifObject* (*converter)(void*);
				converter func = va_arg(*_pVa, converter);
				void* arg = va_arg(*_pVa, void*);
				++*_pFormat;
				return (*func)(arg);
			}
			else {
				AlifObject* v{};
				v = va_arg(*_pVa, AlifObject*);
				if (v != nullptr) {
					if (*(*_pFormat - 1) != 'N')
						ALIF_INCREF(v);
				}
				//else if (!alifErr_occurred())
				//	alifErr_setString(_alifExcSystemError_,
				//		"nullptr object passed to alif_buildValue");
				return v;
			}

		case ':':
		case ',':
		case ' ':
		case '\t':
			break;

		default:
			//alifErr_setString(_alifExcSystemError_,
			//	"bad format char passed to alif_buildValue");
			return nullptr;

		}
	}
}





AlifObject* alif_buildValue(const char* _format, ...) { // 475
	va_list va{};
	AlifObject* retVal{};
	va_start(va, _format);
	retVal = va_buildValue(_format, va);
	va_end(va);
	return retVal;
}




static AlifObject* va_buildValue(const char* _format, va_list _va) { // 509
	const char* f = _format;
	AlifSizeT n = count_format(f, '\0');
	va_list lva{};
	AlifObject* retVal{};

	if (n < 0) return nullptr;
	if (n == 0) {
		return ALIF_NONE;
	}
	va_copy(lva, _va);
	if (n == 1) {
		retVal = do_mkValue(&f, &lva);
	}
	else {
		retVal = do_mkTuple(&f, &lva, '\0', n);
	}
	va_end(lva);
	return retVal;
}

AlifObject** _alif_vaBuildStack(AlifObject** _smallStack, AlifSizeT _smallStackLen,
	const char* _format, va_list _va, AlifSizeT* _pNArgs) { // 533
	const char* f_{};
	AlifSizeT n_{};
	va_list lVa_{};
	AlifObject** stack{};
	AlifIntT res_{};

	n_ = count_format(_format, '\0');
	if (n_ < 0) {
		*_pNArgs = 0;
		return nullptr;
	}

	if (n_ == 0) {
		*_pNArgs = 0;
		return _smallStack;
	}

	if (n_ <= _smallStackLen) {
		stack = _smallStack;
	}
	else {
		stack = (AlifObject**)alifMem_dataAlloc(n_ * sizeof(stack[0]));
		if (stack == nullptr) {
			//alifErr_noMemory();
			return nullptr;
		}
	}

	va_copy(lVa_, _va);
	f_ = _format;
	res_ = do_mkStack(stack, &f_, &lVa_, '\0', n_);
	va_end(lVa_);

	if (res_ < 0) {
		if (stack != _smallStack) {
			alifMem_dataFree(stack);
		}
		return nullptr;
	}

	*_pNArgs = n_;
	return stack;
}



AlifIntT alifModule_addObjectRef(AlifObject* _mod,
	const char* _name, AlifObject* _value) { // 581
	if (!ALIFMODULE_CHECK(_mod)) {
		//alifErr_setString(_alifExcTypeError_,
		//	"alifModule_addObjectRef() first argument "
		//	"must be a module");
		return -1;
	}
	if (!_value) {
		if (!alifErr_occurred()) {
			//alifErr_setString(_alifExcSystemError_,
			//	"alifModule_addObjectRef() must be called "
			//	"with an exception raised if value is NULL");
		}
		return -1;
	}

	AlifObject* dict = alifModule_getDict(_mod);
	if (dict == nullptr) {
		/* Internal error -- modules must have a dict! */
		//alifErr_format(_alifExcSystemError_, "module '%s' has no __dict__",
		//	alifModule_getName(mod));
		return -1;
	}
	return alifDict_setItemString(dict, _name, _value);
}



AlifIntT alifModule_addType(AlifObject* _module, AlifTypeObject* _type) { // 639
	if (!alifType_isReady(_type) and alifType_ready(_type) < 0) {
		return -1;
	}

	const char* name = _alifType_name(_type);

	return alifModule_addObjectRef(_module, name, (AlifObject*)_type);
}
