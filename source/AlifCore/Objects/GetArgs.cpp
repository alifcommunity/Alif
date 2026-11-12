#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Dict.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_State.h"
#include "AlifCore_Tuple.h"
#include "AlifCore_Errors.h"


#define FLAG_COMPAT 1 // 23

typedef AlifIntT(*DestrT)(AlifObject*, void*); // 25

class FreeListEntryT { // 32
public:
	void* item{};
	DestrT destructor{};
};

class FreeListT { // 37
public:
	FreeListEntryT* entries{};
	AlifIntT first_available{};
	AlifIntT entries_malloced{};
};


#define STATIC_FREELIST_ENTRIES 8 // 43


static AlifIntT vGetArgs1(AlifObject*, const char*, va_list*, AlifIntT); // 48


static const char* convert_simple(AlifObject*, const char**, va_list*,
	AlifIntT, char*, AlifUSizeT, FreeListT*); // 54

static const char* convert_item(AlifObject*, const char**, va_list*, AlifIntT,
	AlifIntT*, char*, AlifUSizeT, FreeListT*); // 50

static AlifSizeT convert_buffer(AlifObject*, const void**, const char**); // 56
static AlifIntT get_buffer(AlifObject*, AlifBuffer*, const char**); // 57

static AlifIntT vGet_argsKeywords(AlifObject*, AlifObject*, const char*,
	const char * const*, va_list*, AlifIntT); // 59

static AlifIntT vGetArgsKeywordsFast_impl(AlifObject* const*, AlifSizeT,
	AlifObject*, AlifObject*, AlifArgParser*, va_list*, AlifIntT); // 63

static const char* skip_item(const char**, va_list*, AlifIntT); // 67


AlifIntT alifArg_parseTuple(AlifObject* _args, const char* _format, ...) { // 94
	AlifIntT retval{};
	va_list va{};

	va_start(va, _format);
	retval = vGetArgs1(_args, _format, &va, 0);
	va_end(va);
	return retval;
}

static AlifIntT cleanup_ptr(AlifObject* _self, void* _ptr) { // 160
	void** pptr = (void**)_ptr;
	alifMem_dataFree(*pptr);
	*pptr = nullptr;
	return 0;
}

static AlifIntT cleanup_buffer(AlifObject* _self, void* _ptr) { // 169
	AlifBuffer* buf = (AlifBuffer*)_ptr;
	if (buf) {
		alifBuffer_release(buf);
	}
	return 0;
}

static AlifIntT add_cleanup(void* _ptr, FreeListT* _freelist, DestrT _destructor) { // 179
	AlifIntT index{};

	index = _freelist->first_available;
	_freelist->first_available += 1;

	_freelist->entries[index].item = _ptr;
	_freelist->entries[index].destructor = _destructor;

	return 0;
}

static AlifIntT clean_return(AlifIntT _retval, FreeListT* _freelist) { // 193
	AlifIntT index{};

	if (_retval == 0) {
		for (index = 0; index < _freelist->first_available; ++index) {
			_freelist->entries[index].destructor(nullptr,
				_freelist->entries[index].item);
		}
	}
	if (_freelist->entries_malloced)
		alifMem_dataFree(_freelist->entries);
	return _retval;
}


static AlifIntT vGetArgs1_impl(AlifObject* _compatArgs,
	AlifObject* const* _stack, AlifSizeT _nargs, const char* _format,
	va_list* _pVa, AlifIntT _flags) { // 213
	char msgbuf[256]{};
	AlifIntT levels[32]{};
	const char* fname = nullptr;
	const char* message = nullptr;
	AlifIntT min = -1;
	AlifIntT max = 0;
	AlifIntT level = 0;
	AlifIntT endfmt = 0;
	const char* formatsave = _format;
	AlifSizeT i{};
	const char* msg;
	AlifIntT compat = _flags & FLAG_COMPAT;
	FreeListEntryT static_entries[STATIC_FREELIST_ENTRIES];
	FreeListT freelist;


	freelist.entries = static_entries;
	freelist.first_available = 0;
	freelist.entries_malloced = 0;

	_flags = _flags & ~FLAG_COMPAT;

	while (endfmt == 0) {
		AlifIntT c = *_format++;
		switch (c) {
		case '(':
			if (level == 0)
				max++;
			level++;
			if (level >= 30) { /*alif_fatalError("too many tuple nesting levels "
					"in argument format string");*/
			}
			break;
		case ')':
			if (level == 0) { /*alif_fatalError("excess ')' in getargs format");*/ }
			else
				level--;
			break;
		case '\0':
			endfmt = 1;
			break;
		case ':':
			fname = _format;
			endfmt = 1;
			break;
		case ';':
			message = _format;
			endfmt = 1;
			break;
		case '|':
			if (level == 0)
				min = max;
			break;
		default:
			if (level == 0) {
				if (ALIF_ISALPHA(c))
					if (c != 'e') /* skip encoded */
						max++;
			}
			break;
		}
	}

	//if (level != 0)
	//{ alif_fatalError(/* '(' */ "missing ')' in getargs format"); }

	if (min < 0)
		min = max;

	_format = formatsave;

	if (max > STATIC_FREELIST_ENTRIES) {
		freelist.entries = ((AlifUSizeT)max > ALIF_SIZET_MAX / sizeof(FreeListEntryT)) ? nullptr : \
			(FreeListEntryT*)alifMem_dataAlloc(max * sizeof(FreeListEntryT)); //* alif //* todo
		if (freelist.entries == nullptr) {
			//alifErr_noMemory();
			return 0;
		}
		freelist.entries_malloced = 1;
	}

	if (compat) {
		if (max == 0) {
			if (_compatArgs == nullptr)
				return 1;
			alifErr_format(_alifExcTypeError_,
				"%.200s%s لا يحتاج تمرير أي معاملات",
				fname == nullptr ? "الدالة " : fname,
				fname == nullptr ? "" : "()");
			return clean_return(0, &freelist);
		}
		else if (min == 1 and max == 1) {
			if (_compatArgs == nullptr) {
				alifErr_format(_alifExcTypeError_,
					"%.200s%s يحتاج على الأقل تمرير معامل واحد",
					fname == nullptr ? "دالة" : fname,
					fname == nullptr ? "" : "()");
				return clean_return(0, &freelist);
			}
			msg = convert_item(_compatArgs, &_format, _pVa, _flags, levels,
				msgbuf, sizeof(msgbuf), &freelist);
			if (msg == nullptr)
				return clean_return(1, &freelist);
			//seterror(levels[0], msg, levels + 1, fname, message);
			return clean_return(0, &freelist);
		}
		else {
			alifErr_setString(_alifExcSystemError_,
				"تنسيق قديم لجلب المعاملات يستخدم مميزات حديثة");
			return clean_return(0, &freelist);
		}
	}

	if (_nargs < min or max < _nargs) {
		if (message == nullptr)
			alifErr_format(_alifExcTypeError_,
				"%.150s%s تحتاج %s %d معامل%s (%zd المعطى)",
				fname == nullptr ? "دالة" : fname,
				fname == nullptr ? "" : "()",
				min == max ? "بالتحديد"
				: _nargs < min ? "على الأقل" : "على الأكثر",
				_nargs < min ? min : max,
				(_nargs < min ? min : max) == 1 ? "" : "ات",
				_nargs);
		else
			alifErr_setString(_alifExcTypeError_, message);
		return clean_return(0, &freelist);
	}

	for (i = 0; i < _nargs; i++) {
		if (*_format == '|')
			_format++;
		msg = convert_item(_stack[i], &_format, _pVa,
			_flags, levels, msgbuf,
			sizeof(msgbuf), &freelist);
		if (msg) {
			//seterror(i + 1, msg, levels, fname, message);
			return clean_return(0, &freelist);
		}
	}

	if (*_format != '\0' and !ALIF_ISALPHA(*_format) and
		*_format != '(' and
		*_format != '|' and *_format != ':' and *_format != ';') {
		alifErr_format(_alifExcSystemError_,
			"تنسيق نصي غير مناسب: %.200s", formatsave);
		return clean_return(0, &freelist);
	}

	return clean_return(1, &freelist);
}


static AlifIntT vGetArgs1(AlifObject* _args, const char* _format,
	va_list* _pVa, AlifIntT _flags) { // 370
	AlifObject** stack{};
	AlifSizeT nargs{};

	if (!(_flags & FLAG_COMPAT)) {

		if (!ALIFTUPLE_CHECK(_args)) {
			alifErr_setString(_alifExcSystemError_,
				"تنسيق جديد لجلب المعاملات ولكن المعامل ليس مترابطة");
			return 0;
		}

		stack = ALIFTUPLE_ITEMS(_args);
		nargs = ALIFTUPLE_GET_SIZE(_args);
	}
	else {
		stack = nullptr;
		nargs = 0;
	}

	return vGetArgs1_impl(_args, stack, nargs, _format, _pVa, _flags);
}



static const char* convert_tuple(AlifObject* _arg, const char** _pFormat,
	va_list* _pVa, AlifIntT _flags, AlifIntT* _levels, char* _msgbuf,
	AlifUSizeT _bufsize, FreeListT* _freelist) { // 458
	AlifIntT level = 0;
	AlifIntT n = 0;
	const char* format = *_pFormat;
	AlifIntT i{};
	AlifSizeT len{};

	for (;;) {
		AlifIntT c = *format++;
		if (c == '(') {
			if (level == 0)
				n++;
			level++;
		}
		else if (c == ')') {
			if (level == 0)
				break;
			level--;
		}
		else if (c == ':' or c == ';' or c == '\0')
			break;
		else if (level == 0 and ALIF_ISALPHA(c) and c != 'e')
			n++;
	}

	if (!alifSequence_check(_arg) or ALIFBYTES_CHECK(_arg)) {
		_levels[0] = 0;
		alifOS_snprintf(_msgbuf, _bufsize,
			"يجب أن يكون %d-عنصر تسلسل, وليس %.50s",
			n,
			_arg == ALIF_NONE ? "عدم" : ALIF_TYPE(_arg)->name);
		return _msgbuf;
	}

	len = alifSequence_size(_arg);
	if (len != n) {
		_levels[0] = 0;
		alifOS_snprintf(_msgbuf, _bufsize,
			"يجب أن يكون تسلسل بطول %d, وليس %zd",
			n, len);
		return _msgbuf;
	}

	format = *_pFormat;
	for (i = 0; i < n; i++) {
		const char* msg{};
		AlifObject* item{};
		item = alifSequence_getItem(_arg, i);
		if (item == nullptr) {
			alifErr_clear();
			_levels[0] = i + 1;
			_levels[1] = 0;
			strncpy(_msgbuf, "is not retrievable", _bufsize);
			return _msgbuf;
		}
		msg = convert_item(item, &format, _pVa, _flags, _levels + 1,
			_msgbuf, _bufsize, _freelist);
		ALIF_XDECREF(item);
		if (msg != nullptr) {
			_levels[0] = i + 1;
			return msg;
		}
	}

	*_pFormat = format;
	return nullptr;
}



static const char* convert_item(AlifObject* _arg, const char** _pFormat, va_list* _pVa, AlifIntT _flags,
	AlifIntT* _levels, char* _msgbuf, AlifUSizeT _bufsize, FreeListT* _freelist) { // 534
	const char* msg{};
	const char* format = *_pFormat;

	if (*format == '(' /* ')' */) {
		format++;
		msg = convert_tuple(_arg, &format, _pVa, _flags, _levels, _msgbuf,
			_bufsize, _freelist);
		if (msg == nullptr)
			format++;
	}
	else {
		msg = convert_simple(_arg, &format, _pVa, _flags,
			_msgbuf, _bufsize, _freelist);
		if (msg != nullptr)
			_levels[0] = 0;
	}
	if (msg == nullptr)
		*_pFormat = format;
	return msg;
}

static const char* convert_simple(AlifObject* _arg, const char** _pFormat, va_list* _pVa,
	AlifIntT _flags, char* _mSGBuf, AlifUSizeT _bufSize, FreeListT* _freeList) { // 614

#define RETURN_ERR_OCCURRED return _mSGBuf

	const char* format = *_pFormat;
	char c = *format++;
	const char* sarg{};

	switch (c) {

	case 'b': { /* unsigned byte -- very short AlifIntT */
		unsigned char* p_ = va_arg(*_pVa, unsigned char*);
		long iVal = alifLong_asLong(_arg);
		if (iVal == -1 and alifErr_occurred())
			RETURN_ERR_OCCURRED;
		if (iVal < 0) {
			alifErr_setString(_alifExcOverflowError_,
				"ثمانية العدد الصحيح الذي لا يملك إشارة أقل من الحد الأدنى");
			RETURN_ERR_OCCURRED;
		}
		else if (iVal > UCHAR_MAX) {
			alifErr_setString(_alifExcOverflowError_,
				"ثمانية العدد الصحيح الذي لا يملك إشارة أكبر من الحد الأقصى");
			RETURN_ERR_OCCURRED;
		}
		else
			*p_ = (unsigned char)iVal;
		break;
	}

	case 'B': {
		unsigned char* p_ = va_arg(*_pVa, unsigned char*);
		unsigned long iVal = alifLong_asUnsignedLongMask(_arg);
		if (iVal == (unsigned long)-1 and alifErr_occurred())
			RETURN_ERR_OCCURRED;
		else
			*p_ = (unsigned char)iVal;
		break;
	}

	case 'h': {
		short* p_ = va_arg(*_pVa, short*);
		long iVal = alifLong_asLong(_arg);
		if (iVal == -1 and alifErr_occurred())
			RETURN_ERR_OCCURRED;
		if (iVal < SHRT_MIN) {
			alifErr_setString(_alifExcOverflowError_,
				"العدد الصحيح القصير الذي يملك إشارة أقل من الحد الأدنى");
			RETURN_ERR_OCCURRED;
		}
		else if (iVal > SHRT_MAX) {
			alifErr_setString(_alifExcOverflowError_,
				"العدد الصحيح القصير الذي يملك إشارة أكبر من الحد الأقصى");
			RETURN_ERR_OCCURRED;
		}
		else
			*p_ = (short)iVal;
		break;
	}

	case 'H': {
		unsigned short* p_ = va_arg(*_pVa, unsigned short*);
		unsigned long iVal = alifLong_asUnsignedLongMask(_arg);
		if (iVal == (unsigned long)-1 and alifErr_occurred())
			RETURN_ERR_OCCURRED;
		else
			*p_ = (unsigned short)iVal;
		break;
	}

	case 'i': {/* signed AlifIntT */
		AlifIntT* p_ = va_arg(*_pVa, AlifIntT*);
		long iVal = alifLong_asLong(_arg);
		if (iVal == -1 and alifErr_occurred())
			RETURN_ERR_OCCURRED;
		if (iVal > INT_MAX) {
			alifErr_setString(_alifExcOverflowError_,
				"العدد الصحيح الذي يملك إشارة أكبر من الحد الأقصى");
			RETURN_ERR_OCCURRED;
		}
		else if (iVal < INT_MIN) {
			alifErr_setString(_alifExcOverflowError_,
				"العدد الصحيح الذي يملك إشارة أقل من الحد الأدنى");
			RETURN_ERR_OCCURRED;
		}
		else
			*p_ = iVal;
		break;
	}

	case 'I': {
		unsigned int* p_ = va_arg(*_pVa, unsigned int*);
		unsigned long iVal = alifLong_asUnsignedLongMask(_arg);
		if (iVal == (unsigned long)-1 and alifErr_occurred())
			RETURN_ERR_OCCURRED;
		else
			*p_ = (unsigned int)iVal;
		break;
	}

	case 'n': /* AlifSizeT */
	{
		AlifObject* iObj{};
		AlifSizeT* p_ = va_arg(*_pVa, AlifSizeT*);
		AlifSizeT iVal = -1;
		iObj = _alifNumber_index(_arg);
		if (iObj != nullptr) {
			iVal = alifLong_asSizeT(iObj);
			ALIF_DECREF(iObj);
		}
		if (iVal == -1 and alifErr_occurred())
			RETURN_ERR_OCCURRED;
		*p_ = iVal;
		break;
	}
	case 'l': {/* long AlifIntT */
		long* p_ = va_arg(*_pVa, long*);
		long iVal = alifLong_asLong(_arg);
		if (iVal == -1 and alifErr_occurred())
			RETURN_ERR_OCCURRED;
		else
			*p_ = iVal;
		break;
	}

	case 'k': { /* long sized bitfield */
		unsigned long* p = va_arg(*_pVa, unsigned long*);
		unsigned long ival;
		if (ALIFLONG_CHECK(_arg)) { ival = alifLong_asUnsignedLongMask(_arg); }
		else { /*return convert_err("AlifIntT", _arg, _mSGBuf, _bufSize);*/ }
		*p = ival;
		break;
	}

	case 'L': {/* long long */
		long long* p = va_arg(*_pVa, long long*);
		long long ival = alifLong_asLongLong(_arg);
		if (ival == (long long)-1 and alifErr_occurred())
			RETURN_ERR_OCCURRED;
		else
			*p = ival;
		break;
	}

	case 'K': { /* long long sized bitfield */
		unsigned long long* p = va_arg(*_pVa, unsigned long long*);
		unsigned long long ival;
		if (ALIFLONG_CHECK(_arg)) { ival = alifLong_asUnsignedLongLongMask(_arg); }
		else {/*return convert_err("AlifIntT", _arg, _mSGBuf, _bufSize);*/ }
		*p = ival;
		break;
	}

	case 'f': {/* float */
		float* p = va_arg(*_pVa, float*);
		double dval = alifFloat_asDouble(_arg);
		if (dval == -1.0 and alifErr_occurred())
			RETURN_ERR_OCCURRED;
		else
			*p = (float)dval;
		break;
	}

	case 'd': {/* double */
		double* p = va_arg(*_pVa, double*);
		double dval = alifFloat_asDouble(_arg);
		if (dval == -1.0 and alifErr_occurred())
			RETURN_ERR_OCCURRED;
		else
			*p = dval;
		break;
	}

	case 'D': {/* complex double */
		AlifComplex* p = va_arg(*_pVa, AlifComplex*);
		AlifComplex cval{};
		cval = alifComplex_asCComplex(_arg);
		//if (alifErr_occurred())
		//	RETURN_ERR_OCCURRED;
		//else
		*p = cval;
		break;
	}

	case 'c': {/* char */
		char* p = va_arg(*_pVa, char*);
		if (ALIFBYTES_CHECK(_arg)) {
			if (ALIFBYTES_GET_SIZE(_arg) != 1) {
				//return convert_charErr("a byte string of length 1",
				//	"a bytes object", ALIFBYTES_GET_SIZE(_arg),
				//	_mSGBuf, _bufSize);
			}
			*p = ALIFBYTES_AS_STRING(_arg)[0];
		}
		else if (ALIFBYTEARRAY_CHECK(_arg)) {
			if (ALIFBYTEARRAY_GET_SIZE(_arg) != 1) {
				//return convert_charErr("a byte string of length 1",
				//	"a bytearray object", ALIFBYTEARRAY_GET_SIZE(_arg),
				//	_mSGBuf, _bufSize);
			}
			*p = ALIFBYTEARRAY_AS_STRING(_arg)[0];
		}
		else
			//return convert_err("a byte string of length 1", _arg, _mSGBuf, _bufSize);
			break;
	}

	case 'C': {/* unicode char */
		AlifIntT* p = va_arg(*_pVa, AlifIntT*);
		AlifIntT kind;
		const void* data;

		if (!ALIFUSTR_CHECK(_arg))
			//return convert_err("a unicode character", _arg, _mSGBuf, _bufSize);

			if (ALIFUSTR_GET_LENGTH(_arg) != 1) {
				//return convert_charErr("a unicode character",
				//	"a string", ALIFUSTR_GET_LENGTH(_arg),
				//	_mSGBuf, _bufSize);
			}

		kind = ALIFUSTR_KIND(_arg);
		data = ALIFUSTR_DATA(_arg);
		*p = ALIFUSTR_READ(kind, data, 0);
		break;
	}

	case 'p': {/* boolean *p*redicate */
		AlifIntT* p = va_arg(*_pVa, AlifIntT*);
		AlifIntT val = alifObject_isTrue(_arg);
		if (val > 0)
			*p = 1;
		else if (val == 0)
			*p = 0;
		else
			RETURN_ERR_OCCURRED;
		break;
	}
	case 'y': {/* any bytes-like object */
		void** p = (void**)va_arg(*_pVa, char**);
		const char* buf{};
		AlifSizeT count{};
		if (*format == '*') {
			if (get_buffer(_arg, (AlifBuffer*)p, &buf) < 0) { /*return convert_err(buf, _arg, _mSGBuf, _bufSize);*/ }
			format++;
			if (add_cleanup(p, _freeList, cleanup_buffer)) {
				//return convert_err(
				//	"(cleanup problem)",
				//	_arg, _mSGBuf, _bufSize);
			}
			break;
		}
		count = convert_buffer(_arg, (const void**)p, &buf);
		if (count < 0) {
			//return convert_err(buf, _arg, _mSGBuf, _bufSize);
		}
		if (*format == '#') {
			AlifSizeT* psize = va_arg(*_pVa, AlifSizeT*);
			*psize = count;
			format++;
		}
		else {
			if (strlen((const char*)*p) != (AlifUSizeT)count) { //* alif
				//alifErr_setString(_alifExcValueError_, "embedded null byte");
				RETURN_ERR_OCCURRED;
			}
		}
		break;
	}

	case 's': /* text string or bytes-like object */
	case 'z': /* text string, bytes-like object or None */
	{
		if (*format == '*') {
			/* "s*" or "z*" */
			AlifBuffer* p = (AlifBuffer*)va_arg(*_pVa, AlifBuffer*);

			if (c == 'z' and _arg == ALIF_NONE)
				alifBuffer_fillInfo(p, nullptr, nullptr, 0, 1, 0);
			else if (ALIFUSTR_CHECK(_arg)) {
				AlifSizeT len{};
				sarg = alifUStr_asUTF8AndSize(_arg, &len);
				//if (sarg == nullptr)
				//	return convert_err(CONV_UNICODE,
				//		_arg, _mSGBuf, _bufSize);
				//alifBuffer_fillInfo(p, _arg, (void*)sarg, len, 1, 0);
			}
			else { /* any bytes-like object */
				const char* buf{};
				if (get_buffer(_arg, p, &buf) < 0) {
					//return convert_err(buf, _arg, _mSGBuf, _bufSize);
				}
			}
			if (add_cleanup(p, _freeList, cleanup_buffer)) {
				//return convert_err(
				//	"(cleanup problem)",
				//	_arg, _mSGBuf, _bufSize);
			}
			format++;
		}
		else if (*format == '#') { /* a string or read-only bytes-like object */
			/* "s#" or "z#" */
			const void** p = (const void**)va_arg(*_pVa, const char**);
			AlifSizeT* psize = va_arg(*_pVa, AlifSizeT*);

			if (c == 'z' and _arg == ALIF_NONE) {
				*p = nullptr;
				*psize = 0;
			}
			else if (ALIFUSTR_CHECK(_arg)) {
				AlifSizeT len{};
				sarg = alifUStr_asUTF8AndSize(_arg, &len);
				//if (sarg == nullptr)
				//	return convert_err(CONV_UNICODE,
				//		_arg, _mSGBuf, _bufSize);
				*p = sarg;
				*psize = len;
			}
			else { /* read-only bytes-like object */
				/* XXX Really? */
				const char* buf{};
				AlifSizeT count = convert_buffer(_arg, p, &buf);
				if (count < 0) {
					//return convert_err(buf, _arg, _mSGBuf, _bufSize);
				}
				*psize = count;
			}
			format++;
		}
		else {
			/* "s" or "z" */
			const char** p = va_arg(*_pVa, const char**);
			AlifSizeT len{};
			sarg = nullptr;

			if (c == 'z' and _arg == ALIF_NONE)
				*p = nullptr;
			else if (ALIFUSTR_CHECK(_arg)) {
				sarg = alifUStr_asUTF8AndSize(_arg, &len);
				//if (sarg == nullptr)
				//	return convert_err(CONV_UNICODE,
				//		_arg, _mSGBuf, _bufSize);
				if (strlen(sarg) != (AlifUSizeT)len) {
					//alifErr_setString(_alifExcValueError_, "embedded null character");
					RETURN_ERR_OCCURRED;
				}
				*p = sarg;
			}
			//else
			//	return convert_err(c == 'z' ? "str or None" : "str",
			//		_arg, _mSGBuf, _bufSize);
		}
		break;
	}

	case 'e': {/* encoded string */
		char** buffer{};
		const char* encoding{};
		AlifObject* s{};
		AlifIntT recode_strings{};
		AlifSizeT size{};
		const char* ptr{};

		/* Get 'e' parameter: the encoding name */
		encoding = (const char*)va_arg(*_pVa, const char*);
		if (encoding == nullptr)
			encoding = alifUStr_getDefaultEncoding();

		/* Get output buffer parameter:
		   's' (recode all objects via Unicode) or
		   't' (only recode non-string objects)
		*/
		if (*format == 's')
			recode_strings = 1;
		else if (*format == 't')
			recode_strings = 0;
		//else
		//	return convert_err(
		//		"(unknown parser marker combination)",
		//		_arg, _mSGBuf, _bufSize);
		buffer = (char**)va_arg(*_pVa, char**);
		format++;
		//if (buffer == nullptr)
		//	return convert_err("(buffer is nullptr)",
		//		_arg, _mSGBuf, _bufSize);

		/* Encode object */
		if (!recode_strings and
			(ALIFBYTES_CHECK(_arg) or ALIFBYTEARRAY_CHECK(_arg))) {
			s = ALIF_NEWREF(_arg);
			if (ALIFBYTES_CHECK(_arg)) {
				size = ALIFBYTES_GET_SIZE(s);
				ptr = ALIFBYTES_AS_STRING(s);
			}
			else {
				size = ALIFBYTEARRAY_GET_SIZE(s);
				ptr = ALIFBYTEARRAY_AS_STRING(s);
			}
		}
		else if (ALIFUSTR_CHECK(_arg)) {
			/* Encode object; use default error handling */
			s = alifUStr_asEncodedString(_arg,
				encoding, nullptr);
			//if (s == nullptr)
			//	return convert_err("(encoding failed)",
			//		_arg, _mSGBuf, _bufSize);
			size = ALIFBYTES_GET_SIZE(s);
			ptr = ALIFBYTES_AS_STRING(s);
			if (ptr == nullptr)
				ptr = "";
		}
		else {
			//return convert_err(
			//	recode_strings ? "str" : "str, bytes or bytearray",
			//	_arg, _mSGBuf, _bufSize);
		}

		if (*format == '#') {

			AlifSizeT* psize = va_arg(*_pVa, AlifSizeT*);

			format++;
			if (psize == nullptr) {
				ALIF_DECREF(s);
				//return convert_err(
				//	"(buffer_len is nullptr)",
				//	_arg, _mSGBuf, _bufSize);
			}
			if (*buffer == nullptr) {
				*buffer = (((AlifUSizeT)(size + 1) > ALIF_SIZET_MAX / sizeof(char)) ? nullptr : \
					((char*)alifMem_dataAlloc((size + 1) * sizeof(char)))); //* alif
				if (*buffer == nullptr) {
					ALIF_DECREF(s);
					//alifErr_noMemory();
					RETURN_ERR_OCCURRED;
				}
				if (add_cleanup(buffer, _freeList, cleanup_ptr)) {
					ALIF_DECREF(s);
					//return convert_err(
					//	"(cleanup problem)",
					//	_arg, _mSGBuf, _bufSize);
				}
			}
			else {
				if (size + 1 > *psize) {
					ALIF_DECREF(s);
					//alifErr_format(_alifExcValueError_,
					//	"encoded string too long "
					//	"(%zd, maximum length %zd)",
					//	(AlifSizeT)size, (AlifSizeT)(*psize - 1));
					RETURN_ERR_OCCURRED;
				}
			}
			memcpy(*buffer, ptr, size + 1);

			*psize = size;
		}
		else {
			if ((AlifSizeT)strlen(ptr) != size) {
				ALIF_DECREF(s);
				//return convert_err(
				//	"encoded string without null bytes",
				//	_arg, _mSGBuf, _bufSize);
			}
			*buffer = (((AlifUSizeT)(size + 1) > ALIF_SIZET_MAX / sizeof(char)) ? nullptr : \
				((char*)alifMem_dataAlloc((size + 1) * sizeof(char)))); //* alif
			if (*buffer == nullptr) {
				ALIF_DECREF(s);
				//alifErr_noMemory();
				RETURN_ERR_OCCURRED;
			}
			if (add_cleanup(buffer, _freeList, cleanup_ptr)) {
				ALIF_DECREF(s);
				//return convert_err("(cleanup problem)",
				//	_arg, _mSGBuf, _bufSize);
			}
			memcpy(*buffer, ptr, size + 1);
		}
		ALIF_DECREF(s);
		break;
	}

	case 'S': { /* PyBytes object */
		AlifObject** p = va_arg(*_pVa, AlifObject**);
		if (ALIFBYTES_CHECK(_arg))
			*p = _arg;
		//else
		//	return convert_err("bytes", _arg, _mSGBuf, _bufSize);
		break;
	}

	case 'Y': { /* AlifByteArray object */
		AlifObject** p = va_arg(*_pVa, AlifObject**);
		if (ALIFBYTEARRAY_CHECK(_arg))
			*p = _arg;
		//else
		//	return convert_err("bytearray", _arg, _mSGBuf, _bufSize);
		break;
	}

	case 'U': { /* AlifUStr object */
		AlifObject** p = va_arg(*_pVa, AlifObject**);
		if (ALIFUSTR_CHECK(_arg)) {
			*p = _arg;
		}
		//else
		//	return convert_err("str", _arg, _mSGBuf, _bufSize);
		break;
	}

	case 'O': { /* object */
		AlifTypeObject* type{};
		AlifObject** p{};
		if (*format == '!') {
			type = va_arg(*_pVa, AlifTypeObject*);
			p = va_arg(*_pVa, AlifObject**);
			format++;
			if (alifType_isSubType(ALIF_TYPE(_arg), type))
				*p = _arg;
			//else
			//	return convert_err(type->name, _arg, _mSGBuf, _bufSize);

		}
		else if (*format == '&') {
			typedef AlifIntT(*converter)(AlifObject*, void*);
			converter convert = va_arg(*_pVa, converter);
			void* addr = va_arg(*_pVa, void*);
			AlifIntT res{};
			format++;
			if (!(res = (*convert)(_arg, addr))) {
				/*return convert_err("(unspecified)",
					_arg, _mSGBuf, _bufSize);*/
				return nullptr; //* alif
			}
			if (res == ALIF_CLEANUP_SUPPORTED and
				add_cleanup(addr, _freeList, convert) == -1) {
				//return convert_err("(cleanup problem)",
				//	_arg, _mSGBuf, _bufSize);
			}
		}
		else {
			p = va_arg(*_pVa, AlifObject**);
			*p = _arg;
		}
		break;
	}


	case 'w': { /* "w*": memory buffer, read-write access */
		void** p = va_arg(*_pVa, void**);

		if (*format != '*') {
			//return convert_err(
			//	"(invalid use of 'w' format character)",
			//	_arg, _mSGBuf, _bufSize);
		}
		format++;

		if (alifObject_getBuffer(_arg, (AlifBuffer*)p, ALIFBUF_WRITABLE) < 0) {
			alifErr_clear();
			//return convert_err("read-write bytes-like object",
			//	_arg, _mSGBuf, _bufSize);
		}
		if (add_cleanup(p, _freeList, cleanup_buffer)) {
			//return convert_err(
			//	"(cleanup problem)",
			//	_arg, _mSGBuf, _bufSize);
		}
		break;
	}

	default:
		//return convert_err("(impossible<bad format char>)", _arg, _mSGBuf, _bufSize);
		return nullptr; //* alif

	}

	*_pFormat = format;
	return nullptr;

#undef RETURN_ERR_OCCURRED
}

static AlifSizeT convert_buffer(AlifObject* arg,
	const void** p, const char** errmsg) { // 1237
	AlifBufferProcs* pb = ALIF_TYPE(arg)->asBuffer;
	AlifSizeT count{};
	AlifBuffer view{};

	*errmsg = nullptr;
	*p = nullptr;
	if (pb != nullptr and pb->releaseBuffer != nullptr) {
		*errmsg = "كائن ثمانيات للقراءة-فقط";
		return -1;
	}

	if (get_buffer(arg, &view, errmsg) < 0)
		return -1;
	count = view.len;
	*p = view.buf;
	alifBuffer_release(&view);
	return count;
}

static AlifIntT get_buffer(AlifObject* arg, AlifBuffer* view, const char** errmsg) { // 1259
	if (alifObject_getBuffer(arg, view, ALIFBUF_SIMPLE) != 0) {
		*errmsg = "كائن ثمانيات";
		return -1;
	}
	return 0;
}


AlifIntT alifArg_parseTupleAndKeywords(AlifObject* args,
	AlifObject* _keywords,
	const char* format,
	const char* const *kwlist, ...) { // 1275
	AlifIntT retval{};
	va_list va{};

	if ((args == nullptr or !ALIFTUPLE_CHECK(args)) or
		(_keywords != nullptr and !ALIFDICT_CHECK(_keywords)) or
		format == nullptr or
		kwlist == nullptr)
	{
		//ALIFERR_BADINTERNALCALL();
		return 0;
	}

	va_start(va, kwlist);
	retval = vGet_argsKeywords(args, _keywords, format, kwlist, &va, 0);
	va_end(va);
	return retval;
}


AlifIntT _alifArg_parseStackAndKeywords(AlifObject* const* _args,
	AlifSizeT _nargs, AlifObject* _kwnames, AlifArgParser* _parser, ...) { // 1402
	AlifIntT retval{};
	va_list va{};

	va_start(va, _parser);
	retval = vGetArgsKeywordsFast_impl(_args, _nargs, nullptr, _kwnames, _parser, &va, 0);
	va_end(va);
	return retval;
}

static AlifObject* new_kwTuple(const char * const *, AlifIntT, AlifIntT); // 1507

#define IS_END_OF_FORMAT(_c) (_c == '\0' or _c == ';' or _c == ':') // 1510

static AlifIntT vGet_argsKeywords(AlifObject* args, AlifObject* kwargs,
	const char* format, const char * const* kwlist,
	va_list* p_va, AlifIntT flags) { // 1512
	char msgbuf[512]{};
	int levels[32]{};
	const char *fname{}, *msg{}, *custom_msg{};
	AlifIntT min = INT_MAX;
	AlifIntT max = INT_MAX;
	AlifIntT i{}, pos{}, len{};
	AlifIntT skip = 0;
	AlifSizeT nargs{}, nkwargs{};
	FreeListEntryT static_entries[STATIC_FREELIST_ENTRIES]{};
	FreeListT freelist{};

	freelist.entries = static_entries;
	freelist.first_available = 0;
	freelist.entries_malloced = 0;

	/* grab the function name or custom error msg first (mutually exclusive) */
	fname = strchr(format, ':');
	if (fname) {
		fname++;
		custom_msg = nullptr;
	}
	else {
		custom_msg = strchr(format,';');
		if (custom_msg)
			custom_msg++;
	}

	/* scan kwlist and count the number of positional-only parameters */
	for (pos = 0; kwlist[pos] && !*kwlist[pos]; pos++) {
	}
	/* scan kwlist and get greatest possible nbr of args */
	for (len = pos; kwlist[len]; len++) {
		if (!*kwlist[len]) {
			alifErr_setString(_alifExcSystemError_,
				"اسم الكلمة المفتاحية للمعامل فارغ");
			return clean_return(0, &freelist);
		}
	}

	if (len > STATIC_FREELIST_ENTRIES) {
		freelist.entries = ((AlifUSizeT)len > ALIF_SIZET_MAX / sizeof(FreeListEntryT)) ? nullptr : \
			(FreeListEntryT*)alifMem_dataAlloc(len * sizeof(FreeListEntryT)); //* alif
		if (freelist.entries == nullptr) {
			//alifErr_noMemory();
			return 0;
		}
		freelist.entries_malloced = 1;
	}

	nargs = ALIFTUPLE_GET_SIZE(args);
	nkwargs = (kwargs == nullptr) ? 0 : ALIFDICT_GET_SIZE(kwargs);
	if (nargs + nkwargs > len) {
		alifErr_format(_alifExcTypeError_,
			"%.200s%s تحتاج على الأقل %d %sمعامل%s (المعطى %zd)",
			(fname == nullptr) ? "دالة" : fname,
			(fname == nullptr) ? "" : "()",
			len,
			(nargs == 0) ? "كلمة مفتاحية " : "",
			(len == 1) ? "" : "ات",
			nargs + nkwargs);
		return clean_return(0, &freelist);
	}

	/* convert tuple args and keyword args in same loop, using kwlist to drive process */
	for (i = 0; i < len; i++) {
		if (*format == '|') {
			if (min != INT_MAX) {
				alifErr_setString(_alifExcSystemError_,
					"تنسيق نصي غير صالح (| محدد مرتين)");
				return clean_return(0, &freelist);
			}

			min = i;
			format++;

			if (max != INT_MAX) {
				alifErr_setString(_alifExcSystemError_,
					"تنسيق نصي غير صالح ($ قبل |)");
				return clean_return(0, &freelist);
			}
		}
		if (*format == '$') {
			if (max != INT_MAX) {
				alifErr_setString(_alifExcSystemError_,
					"تنسيق نصي غير صالح ($ محدد مرتين)");
				return clean_return(0, &freelist);
			}

			max = i;
			format++;

			if (max < pos) {
				alifErr_setString(_alifExcSystemError_,
					"اسم معامل فارغ بعد $");
				return clean_return(0, &freelist);
			}
			if (skip) {
				/* Now we know the minimal and the maximal numbers of
				* positional arguments and can raise an exception with
				* informative message (see below). */
				break;
			}
			if (max < nargs) {
				if (max == 0) {
					alifErr_format(_alifExcTypeError_,
						"%.200s%s لا تحتاج إلى أي معاملات مكانية",
						(fname == nullptr) ? "دالة" : fname,
						(fname == nullptr) ? "" : "()");
				}
				else {
					alifErr_format(_alifExcTypeError_,
						"%.200s%s تحتاج %s %d معامل%s مكانية"
						" (المعطى %zd)",
						(fname == nullptr) ? "دالة" : fname,
						(fname == nullptr) ? "" : "()",
						(min != INT_MAX) ? "على الأقل" : "تماماً",
						max,
						max == 1 ? "" : "ات",
						nargs);
				}
				return clean_return(0, &freelist);
			}
		}
		if (IS_END_OF_FORMAT(*format)) {
			alifErr_format(_alifExcSystemError_,
				"إدخال مصفوفة كلمات مفتاحية (%d) اكبر من "
				"محددات التنسيق (%d)", len, i);
			return clean_return(0, &freelist);
		}
		if (!skip) {
			AlifObject* current_arg{};
			if (i < nargs) {
				current_arg = ALIF_NEWREF(ALIFTUPLE_GET_ITEM(args, i));
			}
			else if (nkwargs and i >= pos) {
				if (alifDict_getItemStringRef(kwargs, kwlist[i], &current_arg) < 0) {
					return clean_return(0, &freelist);
				}
				if (current_arg) {
					--nkwargs;
				}
			}
			else {
				current_arg = nullptr;
			}

			if (current_arg) {
				msg = convert_item(current_arg, &format, p_va, flags,
					levels, msgbuf, sizeof(msgbuf), &freelist);
				ALIF_DECREF(current_arg);
				if (msg) {
					//set_error(i+1, msg, levels, fname, custom_msg);
					return clean_return(0, &freelist);
				}
				continue;
			}

			if (i < min) {
				if (i < pos) {
					skip = 1;
				}
				else {
					alifErr_format(_alifExcTypeError_,  "%.200s%s معامل مطلوب "
						"مفقود '%s' (الموقع %d)",
						(fname == nullptr) ? "دالة" : fname,
						(fname == nullptr) ? "" : "()",
						kwlist[i], i+1);
					return clean_return(0, &freelist);
				}
			}
			if (!nkwargs and !skip) {
				return clean_return(1, &freelist);
			}
		}

		/* We are into optional args, skip through to any remaining
		* keyword args */
		msg = skip_item(&format, p_va, flags);
		if (msg) {
			alifErr_format(_alifExcSystemError_, "%s: '%s'", msg,
				format);
			return clean_return(0, &freelist);
		}
	}

	if (skip) {
		alifErr_format(_alifExcTypeError_,
			"%.200s%s تحتاج %s %d معامل%s مكاني"
			" (%zd المعطى)",
			(fname == nullptr) ? "دالة" : fname,
			(fname == nullptr) ? "" : "()",
			(ALIF_MIN(pos, min) < i) ? "على الأقل" : "تماماً",
			ALIF_MIN(pos, min),
			ALIF_MIN(pos, min) == 1 ? "" : "ات",
			nargs);
		return clean_return(0, &freelist);
	}

	if (!IS_END_OF_FORMAT(*format) && (*format != '|') && (*format != '$')) {
		alifErr_format(_alifExcSystemError_,
			"محددات المعاملات أكثر من إدخالات مصفوفة الكلمات المفتاحية "
			"(التنسيق المتبقي:'%s')", format);
		return clean_return(0, &freelist);
	}

	if (nkwargs > 0) {
		AlifObject* key{};
		AlifSizeT j{};
		/* make sure there are no arguments given by name and position */
		for (i = pos; i < nargs; i++) {
			AlifObject *current_arg;
			if (alifDict_getItemStringRef(kwargs, kwlist[i], &current_arg) < 0) {
				return clean_return(0, &freelist);
			}
			if (current_arg) {
				ALIF_DECREF(current_arg);
				/* arg present in tuple and in dict */
				alifErr_format(_alifExcTypeError_,
					"معامل لـ %.200s%s معطى بالأسم ('%s') "
					"والموقع (%d)",
					(fname == nullptr) ? "دالة" : fname,
					(fname == nullptr) ? "" : "()",
					kwlist[i], i+1);
				return clean_return(0, &freelist);
			}
		}
		/* make sure there are no extraneous keyword arguments */
		j = 0;
		while (alifDict_next(kwargs, &j, &key, nullptr)) {
			int match = 0;
			if (!ALIFUSTR_CHECK(key)) {
				alifErr_setString(_alifExcTypeError_,
					"الكلمات المفتاحية يجب أن تكون من نوع نص");
				return clean_return(0, &freelist);
			}
			for (i = pos; i < len; i++) {
				if (alifUStr_equalToUTF8(key, kwlist[i])) {
					match = 1;
					break;
				}
			}
			if (!match) {
				AlifObject *alifKWTuple = new_kwTuple(kwlist, len, pos);
				if (!alifKWTuple) {
					return clean_return(0, &freelist);
				}
				AlifObject *alifKWList = alifSequence_list(alifKWTuple);
				ALIF_DECREF(alifKWTuple);
				if (!alifKWList) {
					return clean_return(0, &freelist);
				}
				AlifObject* suggestionKeyword = _alif_calculateSuggestions(alifKWList, key);
				ALIF_DECREF(alifKWList);

				if (suggestionKeyword) {
					alifErr_format(_alifExcTypeError_,
						"%.200s%s حصلت على معامل كلمة مفتاحية غير متوقعة '%S'."
						" هل تقصد '%S'؟",
						(fname == nullptr) ? "هذه الدالة" : fname,
						(fname == nullptr) ? "" : "()",
						key,
						suggestionKeyword);
					ALIF_DECREF(suggestionKeyword);
				}
				else {
					alifErr_format(_alifExcTypeError_,
						"%.200s%s حصلت على معامل كلمة مفتاحية غير متوقعة '%S'",
						(fname == nullptr) ? "هذه الدالة" : fname,
						(fname == nullptr) ? "" : "()",
						key);
				}
				return clean_return(0, &freelist);
			}
		}
		/* Something wrong happened. There are extraneous keyword arguments,
		* but we don't know what. And we don't bother. */
		alifErr_format(_alifExcTypeError_,
			"معامل كلمة مفتاحية غير صالحة لـ %.200s%s",
			(fname == nullptr) ? "هذه الدالة" : fname,
			(fname == nullptr) ? "" : "()");
		return clean_return(0, &freelist);
	}

	return clean_return(1, &freelist);
}



static AlifIntT scan_keywords(const char* const* _keywords,
	AlifIntT* _ptotal, AlifIntT* _pposonly) { // 1820
	AlifIntT i{};
	for (i = 0; _keywords[i] and !*_keywords[i]; i++) {
	}
	*_pposonly = i;

	for (; _keywords[i]; i++) {
		if (!*_keywords[i]) {
			alifErr_setString(_alifExcSystemError_,
				"اسم كلمة مفتاحية فارغ");
			return -1;
		}
	}
	*_ptotal = i;
	return 0;
}

static AlifIntT parse_format(const char* _format, AlifIntT _total,
	AlifIntT _npos, const char** _pfname, const char** _pCustomMsg,
	AlifIntT* _pmin, AlifIntT* _pmax) { // 1841
	const char* custommsg{};
	const char* fname = strchr(_format, ':');
	if (fname) {
		fname++;
		custommsg = nullptr;
	}
	else {
		custommsg = strchr(_format, ';');
		if (custommsg) {
			custommsg++;
		}
	}

	AlifIntT min = INT_MAX;
	AlifIntT max = INT_MAX;
	for (AlifIntT i = 0; i < _total; i++) {
		if (*_format == '|') {
			if (min != INT_MAX) {
				alifErr_setString(_alifExcSystemError_,
					"تنسيق نصي غير مناسب (| محددة مرتين)");
				return -1;
			}
			if (max != INT_MAX) {
				alifErr_setString(_alifExcSystemError_,
					"تنسيق نصي غير مناسب ($ قبل |)");
				return -1;
			}
			min = i;
			_format++;
		}
		if (*_format == '$') {
			if (max != INT_MAX) {
				alifErr_setString(_alifExcSystemError_,
					"تنسيق نصي غير مناسب ($ محددة مرتين)");
				return -1;
			}
			if (i < _npos) {
				alifErr_setString(_alifExcSystemError_,
					"اسم معامل فارغ بعد $");
				return -1;
			}
			max = i;
			_format++;
		}
		if (IS_END_OF_FORMAT(*_format)) {
			alifErr_format(_alifExcSystemError_,
				"مصفوفة كلمات مفتاحية مدخلة (%d) اكثر من "
				"محددات التنسيق (%d)", _total, i);
			return -1;
		}

		const char* msg = skip_item(&_format, nullptr, 0);
		if (msg) {
			alifErr_format(_alifExcSystemError_, "%s: '%s'", msg,
				_format);
			return -1;
		}
	}
	min = ALIF_MIN(min, _total);
	max = ALIF_MIN(max, _total);

	if (!IS_END_OF_FORMAT(*_format) and (*_format != '|') and (*_format != '$')) {
		alifErr_format(_alifExcSystemError_,
			"محددات معاملات اكثر من مصفوفة الكلمات المفتاحية المدخلة "
			"(النسق المتبقي:'%s')", _format);
		return -1;
	}

	*_pfname = fname;
	*_pCustomMsg = custommsg;
	*_pmin = min;
	*_pmax = max;
	return 0;
}


static AlifObject* new_kwTuple(const char * const* _keywords,
	AlifIntT total, AlifIntT pos) { // 1922
	AlifIntT nkw = total - pos;
	AlifObject* kwtuple = alifTuple_new(nkw);
	if (kwtuple == nullptr) {
		return nullptr;
	}
	_keywords += pos;
	for (AlifIntT i = 0; i < nkw; i++) {
		AlifObject* str = alifUStr_fromString(_keywords[i]);
		if (str == nullptr) {
			ALIF_DECREF(kwtuple);
			return nullptr;
		}
		AlifInterpreter* interp = _alifInterpreter_get();
		_alifUStr_internImmortal(interp, &str);
		ALIFTUPLE_SET_ITEM(kwtuple, i, str);
	}
	return kwtuple;
}


static AlifIntT _parser_init(void* _arg) { // 1944
	AlifArgParser* parser = (AlifArgParser*)_arg;
	const char* const* keywords = parser->keywords;

	AlifIntT len{}, pos{};
	if (scan_keywords(keywords, &len, &pos) < 0) {
		return -1;
	}

	const char* fname, * custommsg = nullptr;
	AlifIntT min = 0, max = 0;
	if (parser->format) {
		if (parse_format(parser->format, len, pos,
			&fname, &custommsg, &min, &max) < 0) {
			return -1;
		}
	}
	else {
		fname = parser->fname;
	}

	AlifIntT owned{};
	AlifObject* kwtuple = parser->kwTuple;
	if (kwtuple == nullptr) {
		AlifThread* saveThread = nullptr;
		AlifThread* tempThread = nullptr;
		if (!alif_isMainInterpreter(alifInterpreter_get())) {
			tempThread = _alifThreadState_new(_alifInterpreterState_main());
			if (tempThread == nullptr) {
				return -1;
			}
		//	saveThread = alifThreadState_swap(tempThread);
		}
		kwtuple = new_kwTuple(keywords, len, pos);
		if (tempThread != nullptr) {
		//	alifThreadState_clear(tempThread);
		//	(void)alifThreadState_swap(saveThread);
		//	alifThreadState_delete(tempThread);
		}
		if (kwtuple == nullptr) {
			return -1;
		}
		owned = 1;
	}
	else {
		owned = 0;
	}

	parser->pos = pos;
	parser->fname = fname;
	parser->customMsg = custommsg;
	parser->min = min;
	parser->max = max;
	parser->kwTuple = kwtuple;
	parser->isKwTupleOwned = owned;

	parser->next = (AlifArgParser*)alifAtomic_loadPtr(&_alifRuntime_.getArgs.staticParsers); //* alif
	do {
		// compare-exchange updates parser->next on failure
	}
	while (!alifAtomic_compareExchangePtr(&_alifRuntime_.getArgs.staticParsers,
		&parser->next, parser));
	return 0;
}


static AlifIntT parser_init(AlifArgParser* _parser) { // 2021
	return _alifOnceFlag_callOnce(&_parser->once, &_parser_init, _parser);
}


static AlifObject* find_keyword(AlifObject* kwnames,
	AlifObject* const* kwstack, AlifObject* key) { // 2048
	AlifSizeT i{}, nkwargs{};

	nkwargs = ALIFTUPLE_GET_SIZE(kwnames);
	for (i = 0; i < nkwargs; i++) {
		AlifObject* kwname = ALIFTUPLE_GET_ITEM(kwnames, i);
		if (kwname == key) {
			return ALIF_NEWREF(kwstack[i]);
		}
	}

	for (i = 0; i < nkwargs; i++) {
		AlifObject* kwname = ALIFTUPLE_GET_ITEM(kwnames, i);
		if (_alifUStr_equal(kwname, key)) {
			return ALIF_NEWREF(kwstack[i]);
		}
	}
	return nullptr;
}



static AlifIntT vGetArgsKeywordsFast_impl(AlifObject* const* args, AlifSizeT nargs,
	AlifObject* kwargs, AlifObject* kwnames, AlifArgParser* parser,
	va_list* p_va, AlifIntT flags) { // 2074
	AlifObject* kwtuple{};
	char msgbuf[512]{};
	AlifIntT levels[32]{};
	const char* format{};
	const char* msg{};
	AlifObject* keyword{};
	AlifSizeT i{};
	AlifIntT pos{}, len{};
	AlifSizeT nkwargs{};
	FreeListEntryT static_entries[STATIC_FREELIST_ENTRIES]{};
	FreeListT freelist{};
	AlifObject* const* kwstack = nullptr;

	freelist.entries = static_entries;
	freelist.first_available = 0;
	freelist.entries_malloced = 0;

	if (parser == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return 0;
	}

	if (kwnames != nullptr and !ALIFTUPLE_CHECK(kwnames)) {
		//ALIFERR_BADINTERNALCALL();
		return 0;
	}

	if (parser_init(parser) < 0) {
		return 0;
	}

	kwtuple = parser->kwTuple;
	pos = parser->pos;
	len = pos + (AlifIntT)ALIFTUPLE_GET_SIZE(kwtuple);

	if (len > STATIC_FREELIST_ENTRIES) {
		freelist.entries = (AlifUSizeT)(len) > ALIF_SIZET_MAX / sizeof(FreeListEntryT)
			? nullptr : (FreeListEntryT*)alifMem_dataAlloc(len * sizeof(FreeListEntryT)); //* alif
		if (freelist.entries == nullptr) {
			//alifErr_noMemory();
			return 0;
		}
		freelist.entries_malloced = 1;
	}

	if (kwargs != nullptr) {
		nkwargs = ALIFDICT_GET_SIZE(kwargs);
	}
	else if (kwnames != nullptr) {
		nkwargs = ALIFTUPLE_GET_SIZE(kwnames);
		kwstack = args + nargs;
	}
	else {
		nkwargs = 0;
	}
	if (nargs + nkwargs > len) {
		//alifErr_format(_alifExcTypeError_,
		//	"%.200s%s takes at most %d %sargument%s (%zd given)",
		//	(parser->fname == nullptr) ? "function" : parser->fname,
		//	(parser->fname == nullptr) ? "" : "()",
		//	len,
		//	(nargs == 0) ? "keyword " : "",
		//	(len == 1) ? "" : "s",
		//	nargs + nkwargs);
		return clean_return(0, &freelist);
	}
	if (parser->max < nargs) {
		if (parser->max == 0) {
			//alifErr_format(_alifExcTypeError_,
			//	"%.200s%s takes no positional arguments",
			//	(parser->fname == nullptr) ? "function" : parser->fname,
			//	(parser->fname == nullptr) ? "" : "()");
		}
		else {
			//alifErr_format(_alifExcTypeError_,
			//	"%.200s%s takes %s %d positional argument%s (%zd given)",
			//	(parser->fname == nullptr) ? "function" : parser->fname,
			//	(parser->fname == nullptr) ? "" : "()",
			//	(parser->min < parser->max) ? "at most" : "exactly",
			//	parser->max,
			//	parser->max == 1 ? "" : "s",
			//	nargs);
		}
		return clean_return(0, &freelist);
	}

	format = parser->format;
	for (i = 0; i < len; i++) {
		if (*format == '|') {
			format++;
		}
		if (*format == '$') {
			format++;
		}

		AlifObject* current_arg{};
		if (i < nargs) {
			current_arg = ALIF_NEWREF(args[i]);
		}
		else if (nkwargs and i >= pos) {
			keyword = ALIFTUPLE_GET_ITEM(kwtuple, i - pos);
			if (kwargs != nullptr) {
				if (alifDict_getItemRef(kwargs, keyword, &current_arg) < 0) {
					return clean_return(0, &freelist);
				}
			}
			else {
				current_arg = find_keyword(kwnames, kwstack, keyword);
			}
			if (current_arg) {
				--nkwargs;
			}
		}
		else {
			current_arg = nullptr;
		}

		if (current_arg) {
			msg = convert_item(current_arg, &format, p_va, flags,
				levels, msgbuf, sizeof(msgbuf), &freelist);
			ALIF_DECREF(current_arg);
			if (msg) {
				//set_error(i + 1, msg, levels, parser->fname, parser->customMsg);
				return clean_return(0, &freelist);
			}
			continue;
		}

		if (i < parser->min) {
			if (i < pos) {
				AlifSizeT min = ALIF_MIN(pos, parser->min);
				//alifErr_format(_alifExcTypeError_,
				//	"%.200s%s takes %s %d positional argument%s"
				//	" (%zd given)",
				//	(parser->fname == nullptr) ? "function" : parser->fname,
				//	(parser->fname == nullptr) ? "" : "()",
				//	min < parser->max ? "at least" : "exactly",
				//	min,
				//	min == 1 ? "" : "s",
				//	nargs);
			}
			else {
				keyword = ALIFTUPLE_GET_ITEM(kwtuple, i - pos);
				//alifErr_format(_alifExcTypeError_, "%.200s%s missing required "
				//	"argument '%U' (pos %d)",
				//	(parser->fname == nullptr) ? "function" : parser->fname,
				//	(parser->fname == nullptr) ? "" : "()",
				//	keyword, i + 1);
			}
			return clean_return(0, &freelist);
		}
		if (!nkwargs) {
			return clean_return(1, &freelist);
		}

		msg = skip_item(&format, p_va, flags);
	}

	if (nkwargs > 0) {
		for (i = pos; i < nargs; i++) {
			AlifObject* current_arg{};
			keyword = ALIFTUPLE_GET_ITEM(kwtuple, i - pos);
			if (kwargs != nullptr) {
				if (alifDict_getItemRef(kwargs, keyword, &current_arg) < 0) {
					return clean_return(0, &freelist);
				}
			}
			else {
				current_arg = find_keyword(kwnames, kwstack, keyword);
			}
			if (current_arg) {
				ALIF_DECREF(current_arg);
				/* arg present in tuple and in dict */
				//alifErr_format(_alifExcTypeError_,
				//	"argument for %.200s%s given by name ('%U') "
				//	"and position (%d)",
				//	(parser->fname == nullptr) ? "function" : parser->fname,
				//	(parser->fname == nullptr) ? "" : "()",
				//	keyword, i + 1);
				return clean_return(0, &freelist);
			}
		}

		//error_unexpectedKeywordArg(kwargs, kwnames, kwtuple, parser->fname);
		return clean_return(0, &freelist);
	}

	return clean_return(1, &freelist);
}



AlifObject* const* _alifArg_unpackKeywords(AlifObject* const* args, AlifSizeT nargs,
	AlifObject* kwargs, AlifObject* kwnames, AlifArgParser* parser, AlifIntT minpos, AlifIntT maxpos,
	AlifIntT minkw, AlifIntT varpos, AlifObject** buf) { // 2313
	AlifObject* kwtuple{};
	AlifObject* keyword{};
	AlifIntT i{}, posonly{}, minposonly{}, maxargs{};
	AlifIntT reqlimit = minkw ? maxpos + minkw : minpos;
	AlifSizeT nkwargs{};
	AlifObject* const* kwstack = nullptr;

	if (parser == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}

	if (kwnames != nullptr and !ALIFTUPLE_CHECK(kwnames)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}

	if (args == nullptr and nargs == 0) {
		args = buf;
	}

	if (parser_init(parser) < 0) {
		return nullptr;
	}

	kwtuple = parser->kwTuple;
	posonly = parser->pos;
	minposonly = ALIF_MIN(posonly, minpos);
	maxargs = posonly + (AlifIntT)ALIFTUPLE_GET_SIZE(kwtuple);

	if (kwargs != nullptr) {
		nkwargs = ALIFDICT_GET_SIZE(kwargs);
	}
	else if (kwnames != nullptr) {
		nkwargs = ALIFTUPLE_GET_SIZE(kwnames);
		kwstack = args + nargs;
	}
	else {
		nkwargs = 0;
	}
	if (nkwargs == 0 and minkw == 0 and minpos <= nargs and (varpos || nargs <= maxpos)) {
		/* Fast path. */
		return args;
	}
	if (!varpos and nargs + nkwargs > maxargs) {
		//alifErr_format(_alifExcTypeError_,
		//	"%.200s%s takes at most %d %sargument%s (%zd given)",
		//	(parser->fname == nullptr) ? "function" : parser->fname,
		//	(parser->fname == nullptr) ? "" : "()",
		//	maxargs,
		//	(nargs == 0) ? "keyword " : "",
		//	(maxargs == 1) ? "" : "s",
		//	nargs + nkwargs);
		return nullptr;
	}
	if (!varpos and nargs > maxpos) {
		if (maxpos == 0) {
			//alifErr_format(_alifExcTypeError_,
			//	"%.200s%s takes no positional arguments",
			//	(parser->fname == nullptr) ? "function" : parser->fname,
			//	(parser->fname == nullptr) ? "" : "()");
		}
		else {
			//alifErr_format(_alifExcTypeError_,
			//	"%.200s%s takes %s %d positional argument%s (%zd given)",
			//	(parser->fname == nullptr) ? "function" : parser->fname,
			//	(parser->fname == nullptr) ? "" : "()",
			//	(minpos < maxpos) ? "at most" : "exactly",
			//	maxpos,
			//	(maxpos == 1) ? "" : "s",
			//	nargs);
		}
		return nullptr;
	}
	if (nargs < minposonly) {
		//alifErr_format(_alifExcTypeError_,
		//	"%.200s%s takes %s %d positional argument%s"
		//	" (%zd given)",
		//	(parser->fname == nullptr) ? "function" : parser->fname,
		//	(parser->fname == nullptr) ? "" : "()",
		//	(varpos or minposonly < maxpos) ? "at least" : "exactly",
		//	minposonly,
		//	minposonly == 1 ? "" : "s",
		//	nargs);
		return nullptr;
	}

	if (varpos) {
		nargs = ALIF_MIN(maxpos, nargs);
	}

	/* copy tuple args */
	for (i = 0; i < nargs; i++) {
		buf[i] = args[i];
	}

	/* copy keyword args using kwtuple to drive process */
	for (i = ALIF_MAX((int)nargs, posonly); i < maxargs; i++) {
		AlifObject* current_arg{};
		if (nkwargs) {
			keyword = ALIFTUPLE_GET_ITEM(kwtuple, i - posonly);
			if (kwargs != nullptr) {
				if (alifDict_getItemRef(kwargs, keyword, &current_arg) < 0) {
					return nullptr;
				}
			}
			else {
				current_arg = find_keyword(kwnames, kwstack, keyword);
			}
		}
		else if (i >= reqlimit) {
			break;
		}
		else {
			current_arg = nullptr;
		}

		buf[i] = current_arg;

		if (current_arg) {
			ALIF_DECREF(current_arg);
			--nkwargs;
		}
		else if (i < minpos or (maxpos <= i and i < reqlimit)) {
			/* Less arguments than required */
			keyword = ALIFTUPLE_GET_ITEM(kwtuple, i - posonly);
			//alifErr_format(_alifExcTypeError_, "%.200s%s missing required "
			//	"argument '%U' (pos %d)",
			//	(parser->fname == nullptr) ? "function" : parser->fname,
			//	(parser->fname == nullptr) ? "" : "()",
			//	keyword, i + 1);
			return nullptr;
		}
	}

	if (nkwargs > 0) {
		/* make sure there are no arguments given by name and position */
		for (i = posonly; i < nargs; i++) {
			AlifObject* current_arg{};
			keyword = ALIFTUPLE_GET_ITEM(kwtuple, i - posonly);
			if (kwargs != nullptr) {
				if (alifDict_getItemRef(kwargs, keyword, &current_arg) < 0) {
					return nullptr;
				}
			}
			else {
				current_arg = find_keyword(kwnames, kwstack, keyword);
			}
			if (current_arg) {
				ALIF_DECREF(current_arg);
				/* arg present in tuple and in dict */
				//alifErr_format(_alifExcTypeError_,
				//	"argument for %.200s%s given by name ('%U') "
				//	"and position (%d)",
				//	(parser->fname == nullptr) ? "function" : parser->fname,
				//	(parser->fname == nullptr) ? "" : "()",
				//	keyword, i + 1);
				return nullptr;
			}
		}

		//errorUnexpected_keywordArg(kwargs, kwnames, kwtuple, parser->fname);
		return nullptr;
	}

	return buf;
}




static const char* skip_item(const char** p_format,
	va_list* p_va, AlifIntT flags) { // 2640
	const char* format = *p_format;
	char c = *format++;

	switch (c) {

		/*
		 * codes that take a single data pointer as an argument
		 * (the type of the pointer is irrelevant)
		 */

	case 'b': /* byte -- very short int */
	case 'B': /* byte as bitfield */
	case 'h': /* short int */
	case 'H': /* short int as bitfield */
	case 'i': /* int */
	case 'I': /* int sized bitfield */
	case 'l': /* long int */
	case 'k': /* long int sized bitfield */
	case 'L': /* long long */
	case 'K': /* long long sized bitfield */
	case 'n': /* AlifSizeT */
	case 'f': /* float */
	case 'd': /* double */
	case 'D': /* complex double */
	case 'c': /* char */
	case 'C': /* unicode char */
	case 'p': /* boolean predicate */
	case 'S': /* string object */
	case 'Y': /* string object */
	case 'U': /* unicode string object */
	{
		if (p_va != nullptr) {
			(void)va_arg(*p_va, void*);
		}
		break;
	}

	/* string codes */

	case 'e': /* string with encoding */
	{
		if (p_va != nullptr) {
			(void)va_arg(*p_va, const char*);
		}
		if (!(*format == 's' || *format == 't'))
			/* after 'e', only 's' and 't' is allowed */
			goto err;
		format++;
	}
	ALIF_FALLTHROUGH;

	case 's': /* string */
	case 'z': /* string or None */
	case 'y': /* bytes */
	case 'w': /* buffer, read-write */
	{
		if (p_va != nullptr) {
			(void)va_arg(*p_va, char**);
		}
		if (c == 'w' and *format != '*') {
			/* after 'w', only '*' is allowed */
			goto err;
		}
		if (*format == '#') {
			if (p_va != nullptr) {
				(void)va_arg(*p_va, AlifSizeT*);
			}
			format++;
		}
		else if ((c == 's' or c == 'z' or c == 'y' or c == 'w')
			and *format == '*') {
			format++;
		}
		break;
	}

	case 'O': /* object */
	{
		if (*format == '!') {
			format++;
			if (p_va != nullptr) {
				(void)va_arg(*p_va, AlifTypeObject*);
				(void)va_arg(*p_va, AlifObject**);
			}
		}
		else if (*format == '&') {
			typedef int (*converter)(AlifObject*, void*);
			if (p_va != nullptr) {
				(void)va_arg(*p_va, converter);
				(void)va_arg(*p_va, void*);
			}
			format++;
		}
		else {
			if (p_va != nullptr) {
				(void)va_arg(*p_va, AlifObject**);
			}
		}
		break;
	}

	case '(':           /* bypass tuple, not handled at all previously */
	{
		const char* msg;
		for (;;) {
			if (*format == ')')
				break;
			if (IS_END_OF_FORMAT(*format))
				return "Unmatched left paren in format "
				"string";
			msg = skip_item(&format, p_va, flags);
			if (msg)
				return msg;
		}
		format++;
		break;
	}

	case ')':
		return "Unmatched right paren in format string";

	default:
err:
		return "impossible<bad format char>";

	}

	*p_format = format;
	return nullptr;
}


AlifIntT _alifArg_checkPositional(const char* name, AlifSizeT nargs,
	AlifSizeT min, AlifSizeT max) { // 2778
	if (nargs < min) {
		//if (name != nullptr)
		//	alifErr_format(
		//		_alifExcTypeError_,
		//		"%.200s expected %s%zd argument%s, got %zd",
		//		name, (min == max ? "" : "at least "), min, min == 1 ? "" : "s", nargs);
		//else
		//	alifErr_format(
		//		_alifExcTypeError_,
		//		"unpacked tuple should have %s%zd element%s,"
		//		" but has %zd",
		//		(min == max ? "" : "at least "), min, min == 1 ? "" : "s", nargs);
		return 0;
	}

	if (nargs == 0) {
		return 1;
	}

	if (nargs > max) {
		//if (name != nullptr)
		//	alifErr_format(
		//		_alifExcTypeError_,
		//		"%.200s expected %s%zd argument%s, got %zd",
		//		name, (min == max ? "" : "at most "), max, max == 1 ? "" : "s", nargs);
		//else
		//	alifErr_format(
		//		_alifExcTypeError_,
		//		"unpacked tuple should have %s%zd element%s,"
		//		" but has %zd",
		//		(min == max ? "" : "at most "), max, max == 1 ? "" : "s", nargs);
		return 0;
	}

	return 1;
}


static AlifIntT unpack_stack(AlifObject* const* args,
	AlifSizeT nargs, const char* name,
	AlifSizeT min, AlifSizeT max, va_list vargs) { // 2822
	AlifSizeT i{};
	AlifObject** o{};

	if (!_alifArg_checkPositional(name, nargs, min, max)) {
		return 0;
	}

	for (i = 0; i < nargs; i++) {
		o = va_arg(vargs, AlifObject**);
		*o = args[i];
	}
	return 1;
}

AlifIntT alifArg_unpackTuple(AlifObject* args,
	const char* name, AlifSizeT min, AlifSizeT max, ...) { // 2840
	AlifObject** stack{};
	AlifSizeT nargs{};
	AlifIntT retval{};
	va_list vargs{};

	if (!ALIFTUPLE_CHECK(args)) {
		alifErr_setString(_alifExcSystemError_,
			"alifArg_unpackTuple() مصفوفة معاملات ليست من نوع مترابطة");
		return 0;
	}
	stack = ALIFTUPLE_ITEMS(args);
	nargs = ALIFTUPLE_GET_SIZE(args);

	va_start(vargs, max);
	retval = unpack_stack(stack, nargs, name, min, max, vargs);
	va_end(vargs);
	return retval;
}

AlifIntT _alifArg_unpackStack(AlifObject* const* _args,
	AlifSizeT _nargs, const char* _name,
	AlifSizeT _min, AlifSizeT _max, ...) { // 2714
	AlifIntT retval{};
	va_list vargs{};

	va_start(vargs, _max);
	retval = unpack_stack(_args, _nargs, _name, _min, _max, vargs);
	va_end(vargs);
	return retval;
}

// 2728
//#undef _ALIFARG_NOKEYWORDS
//#undef _ALIFARG_NOKWNAMES
//#undef _ALIFARG_NOPOSITIONAL

AlifIntT _alifArg_noKeywords(const char* funcname, AlifObject* kwargs) { // 2885
	if (kwargs == nullptr) {
		return 1;
	}
	if (!ALIFDICT_CHECKEXACT(kwargs)) {
		//ALIFERR_BADINTERNALCALL();
		return 0;
	}
	if (ALIFDICT_GET_SIZE(kwargs) == 0) {
		return 1;
	}

	//alifErr_format(_alifExcTypeError_, "%.200s() takes no keyword arguments",
	//	funcname);
	return 0;
}


AlifIntT _alifArg_noKwnames(const char* funcname, AlifObject* kwnames) { // 2921
	if (kwnames == nullptr) {
		return 1;
	}

	if (ALIFTUPLE_GET_SIZE(kwnames) == 0) {
		return 1;
	}

	//alifErr_format(_alifExcTypeError_, "%s() takes no keyword arguments", funcname);
	return 0;
}
