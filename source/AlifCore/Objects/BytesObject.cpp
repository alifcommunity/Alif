#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_BytesMethods.h"
#include "AlifCore_BytesObject.h"
#include "AlifCore_Call.h"
#include "AlifCore_GlobalObjects.h"
#include "AlifCore_Long.h"
#include "AlifCore_Object.h"


#include "clinic/BytesObject.cpp.h"





#define ALIFBYTESOBJECT_SIZE (offsetof(AlifBytesObject, val) + 1) // 32

// 39
#define CHARACTERS ALIF_SINGLETON(bytesCharacters)
#define CHARACTER(ch) \
     ((AlifBytesObject *)&(CHARACTERS[ch]));
#define EMPTY (&ALIF_SINGLETON(bytesEmpty)) // 42


static inline AlifObject* bytes_getEmpty(void) { // 46
	AlifObject* empty = &EMPTY->objBase.objBase;
	return empty;
}



static AlifObject* _alifBytes_fromSize(AlifSizeT _size, AlifIntT _useCalloc) { // 76
	AlifBytesObject* op{};

	if (_size == 0) {
		return bytes_getEmpty();
	}

	if ((AlifUSizeT)_size > (AlifUSizeT)ALIF_SIZET_MAX - ALIFBYTESOBJECT_SIZE) {
		alifErr_setString(_alifExcOverflowError_,
			"نص نوع ثمانيات كبير جداً");
		return nullptr;
	}

	if (_useCalloc) {
		op = (AlifBytesObject*)alifMem_objAlloc(ALIFBYTESOBJECT_SIZE + _size); // this calloc and need review
	}
	else {
		op = (AlifBytesObject*)alifMem_objAlloc(ALIFBYTESOBJECT_SIZE + _size);
	}
	if (op == nullptr) {
		//return alifErr_noMemory();
		return nullptr; // temp
	}
	alifObject_initVar((AlifVarObject*)op, &_alifBytesType_, _size);
	op->hash = -1;
	if (!_useCalloc) {
		op->val[_size] = '\0';
	}
	return (AlifObject*)op;
}


AlifObject* alifBytes_fromStringAndSize(const char* _str, AlifSizeT _size) { // 111
	AlifBytesObject* op{};
	if (_size < 0) {
		alifErr_setString(_alifExcSystemError_,
			"تم تمرير حجم بالقيمة السالبة لـ alifBytes_fromStringAndSize");
		return nullptr;
	}
	if (_size == 1 and _str != nullptr) {
		op = CHARACTER(*_str & 255);
		return (AlifObject*)op;
	}
	if (_size == 0) {
		return bytes_getEmpty();
	}

	op = (AlifBytesObject*)_alifBytes_fromSize(_size, 0);
	if (op == nullptr)
		return nullptr;
	if (_str == nullptr)
		return (AlifObject*)op;

	memcpy(op->val, _str, _size);
	return (AlifObject*)op;
}


AlifObject* alifBytes_fromString(const char* _str) { // 139
	AlifUSizeT size{};
	AlifBytesObject* op{};

	size = strlen(_str);
	if (size > ALIF_SIZET_MAX - ALIFBYTESOBJECT_SIZE) {
		alifErr_setString(_alifExcOverflowError_,
			"نص نوع ثمانيات طويل جداً");
		return nullptr;
	}

	if (size == 0) {
		return bytes_getEmpty();
	}
	else if (size == 1) {
		op = CHARACTER(*_str & 255);
		return (AlifObject*)op;
	}

	/* Inline AlifObject_NewVar */
	op = (AlifBytesObject*)alifMem_objAlloc(ALIFBYTESOBJECT_SIZE + size);
	if (op == nullptr) {
		//return alifErr_noMemory();
		return nullptr; // temp
	}
	_alifObject_initVar((AlifVarObject*)op, &_alifBytesType_, size);
	op->hash = -1;
	memcpy(op->val, _str, size + 1);
	return (AlifObject*)op;
}










AlifObject* _alifBytes_decodeEscape(const char* _str, AlifSizeT _len, const char* _errors,
	const char** _firstInvalidEscape) { // 1058
	AlifIntT c_{};
	char* p_{};
	const char* end{};
	AlifBytesWriter writer{};

	_alifBytesWriter_init(&writer);

	p_ = (char*)_alifBytesWriter_alloc(&writer, _len);
	if (p_ == nullptr)
		return nullptr;
	writer.overAllocate = 1;

	*_firstInvalidEscape = nullptr;

	end = _str + _len;
	while (_str < end) {
		if (*_str != '\\') {
			*p_++ = *_str++;
			continue;
		}

		_str++;
		if (_str == end) {
			//alifErr_setString(_alifExcValueError_,
			//	"Trailing \\ in string");
			goto failed;
		}

		switch (*_str++) {
			/* XXX This assumes ASCII! */
		case '\n': break;
		case '\\': *p_++ = '\\'; break;
		case '\'': *p_++ = '\''; break;
		case '\"': *p_++ = '\"'; break;
		case 'b': *p_++ = '\b'; break;
		case 'f': *p_++ = '\014'; break; /* FF */
		case 't': *p_++ = '\t'; break;
		case 'n': *p_++ = '\n'; break;
		case 'r': *p_++ = '\r'; break;
		case 'v': *p_++ = '\013'; break; /* VT */
		case 'a': *p_++ = '\007'; break; /* BEL, not classic C */
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
			c_ = _str[-1] - '0';
			if (_str < end and '0' <= *_str and *_str <= '7') {
				c_ = (c_ << 3) + *_str++ - '0';
				if (_str < end and '0' <= *_str and *_str <= '7')
					c_ = (c_ << 3) + *_str++ - '0';
			}
			if (c_ > 0377) {
				if (*_firstInvalidEscape == nullptr) {
					*_firstInvalidEscape = _str - 3; /* Back up 3 chars, since we've
													already incremented s. */
				}
			}
			*p_++ = c_;
			break;
		case 'x':
			if (_str + 1 < end) {
				int digit1, digit2;
				digit1 = _alifLongDigitValue_[ALIF_CHARMASK(_str[0])];
				digit2 = _alifLongDigitValue_[ALIF_CHARMASK(_str[1])];
				if (digit1 < 16 and digit2 < 16) {
					*p_++ = (unsigned char)((digit1 << 4) + digit2);
					_str += 2;
					break;
				}
			}
			/* invalid hexadecimal digits */

			if (!_errors or strcmp(_errors, "strict") == 0) {
				//alifErr_format(_alifExcValueError_,
				//	"invalid \\x escape at position %zd",
				//	_str - 2 - (end - _len));
				goto failed;
			}
			if (strcmp(_errors, "replace") == 0) {
				*p_++ = '?';
			}
			else if (strcmp(_errors, "ignore") == 0)
				/* do nothing */;
			else {
				//alifErr_format(_alifExcValueError_,
				//	"decoding error; unknown "
				//	"error handling code: %.400s",
				//	_errors);
				goto failed;
			}
			/* skip \x */
			if (_str < end and ALIF_ISXDIGIT(_str[0]))
				_str++; /* and a hexdigit */
			break;

		default:
			if (*_firstInvalidEscape == nullptr) {
				*_firstInvalidEscape = _str - 1; /* Back up one char, since we've
												already incremented s. */
			}
			*p_++ = '\\';
			_str--;
		}
	}

	return _alifBytesWriter_finish(&writer, p_);

failed:
	_alifBytesWriter_dealloc(&writer);
	return nullptr;
}


AlifSizeT alifBytes_size(AlifObject* op) { // 1211
	if (!ALIFBYTES_CHECK(op)) {
		alifErr_format(_alifExcTypeError_,
			"متوقع تمرير نوع ثمانيات, وليس %.200s", ALIF_TYPE(op)->name);
		return -1;
	}
	return ALIF_SIZE(op);
}


char* alifBytes_asString(AlifObject* _op) { // 1221
	if (!ALIFBYTES_CHECK(_op)) {
		alifErr_format(_alifExcTypeError_,
			"متوقع تمرير نوع ثمانيات, وليس %.200s", ALIF_TYPE(_op)->name);
		return nullptr;
	}
	return ((AlifBytesObject*)_op)->val;
}


AlifIntT alifBytes_asStringAndSize(AlifObject* _obj,
	char** _str, AlifSizeT* _len) { // 1232
	if (_str == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}

	if (!ALIFBYTES_CHECK(_obj)) {
		alifErr_format(_alifExcTypeError_,
			"متوقع تمرير نوع ثمانيات, وليس %.200s", ALIF_TYPE(_obj)->name);
		return -1;
	}

	*_str = ALIFBYTES_AS_STRING(_obj);
	if (_len != nullptr)
		*_len = ALIFBYTES_GET_SIZE(_obj);
	else if (strlen(*_str) != (AlifUSizeT)ALIFBYTES_GET_SIZE(_obj)) {
		alifErr_setString(_alifExcValueError_,
			"يوجد ثمانية فارغة");
		return -1;
	}
	return 0;
}


/* -------------------------------------------------------------------- */
/* Methods */

// 1259
#include "StringLib/StringDefs.h"
#define STRINGLIB_MUTABLE 0

#include "StringLib/FastSearch.h"
#include "StringLib/Count.h"
#include "StringLib/Find.h"

#include "StringLib/CppType.h"

#include "StringLib/Transmogrify.h"

#undef STRINGLIB_GET_EMPTY

AlifObject* alifBytes_repr(AlifObject* _obj,
	AlifIntT _smartquotes) { // 1314
	AlifBytesObject* op = (AlifBytesObject*)_obj;
	AlifSizeT i{}, length = ALIF_SIZE(op);
	AlifSizeT newsize{}, squotes{}, dquotes{};
	AlifObject* v{};
	unsigned char quote{};
	const unsigned char* s{};
	AlifUCS1* p{};

	/* Compute size of output string */
	squotes = dquotes = 0;
	newsize = 3; /* b'' */
	s = (const unsigned char*)op->val;
	for (i = 0; i < length; i++) {
		AlifSizeT incr = 1;
		switch (s[i]) {
		case '\'': squotes++; break;
		case '"':  dquotes++; break;
		case '\\': case '\t': case '\n': case '\r':
			incr = 2; break; /* \C */
		default:
			if (s[i] < ' ' || s[i] >= 0x7f)
				incr = 4; /* \xHH */
		}
		if (newsize > ALIF_SIZET_MAX - incr)
			goto overflow;
		newsize += incr;
	}
	quote = '\'';
	if (_smartquotes and squotes and !dquotes)
		quote = '"';
	if (squotes and quote == '\'') {
		if (newsize > ALIF_SIZET_MAX - squotes)
			goto overflow;
		newsize += squotes;
	}

	v = alifUStr_new(newsize, 127);
	if (v == nullptr) {
		return nullptr;
	}
	p = ALIFUSTR_1BYTE_DATA(v);

	*p++ = 'b', *p++ = quote;
	for (i = 0; i < length; i++) {
		unsigned char c = op->val[i];
		if (c == quote || c == '\\')
			*p++ = '\\', *p++ = c;
		else if (c == '\t')
			*p++ = '\\', *p++ = 't';
		else if (c == '\n')
			*p++ = '\\', *p++ = 'n';
		else if (c == '\r')
			*p++ = '\\', *p++ = 'r';
		else if (c < ' ' || c >= 0x7f) {
			*p++ = '\\';
			*p++ = 'x';
			*p++ = _alifHexDigits_[(c & 0xf0) >> 4];
			*p++ = _alifHexDigits_[c & 0xf];
		}
		else
			*p++ = c;
	}
	*p++ = quote;
	return v;

overflow:
	alifErr_setString(_alifExcOverflowError_,
		"طول الثمانيات كبير جدا ليتم عرضه");
	return NULL;
}

static AlifObject* bytes_repr(AlifObject* op) { // 1389
	return alifBytes_repr(op, 1);
}

static AlifSizeT bytes_length(AlifObject* _self) { // 1407
	AlifBytesObject* a = ALIFBYTES_CAST(_self);
	return ALIF_SIZE(a);
}

static AlifObject* bytes_concat(AlifObject* _a, AlifObject* _b) { // 1414
	AlifBuffer va{}, vb{};
	AlifObject* result = nullptr;

	va.len = -1;
	vb.len = -1;
	if (alifObject_getBuffer(_a, &va, ALIFBUF_SIMPLE) != 0 or
		alifObject_getBuffer(_b, &vb, ALIFBUF_SIMPLE) != 0) {
		alifErr_format(_alifExcTypeError_, "لم يستطع دمج %.100s مع %.100s",
			ALIF_TYPE(_b)->name, ALIF_TYPE(_a)->name);
		goto done;
	}

	/* Optimize end cases */
	if (va.len == 0 and ALIFBYTES_CHECKEXACT(_b)) {
		result = ALIF_NEWREF(_b);
		goto done;
	}
	if (vb.len == 0 and ALIFBYTES_CHECKEXACT(_a)) {
		result = ALIF_NEWREF(_a);
		goto done;
	}

	if (va.len > ALIF_SIZET_MAX - vb.len) {
		//alifErr_noMemory();
		goto done;
	}

	result = alifBytes_fromStringAndSize(nullptr, va.len + vb.len);
	if (result != nullptr) {
		memcpy(ALIFBYTES_AS_STRING(result), va.buf, va.len);
		memcpy(ALIFBYTES_AS_STRING(result) + va.len, vb.buf, vb.len);
	}

done:
	if (va.len != -1)
		alifBuffer_release(&va);
	if (vb.len != -1)
		alifBuffer_release(&vb);
	return result;
}


static AlifIntT bytes_compareEq(AlifBytesObject* _a,
	AlifBytesObject* _b) { // 1516
	AlifIntT cmp{};
	AlifSizeT len{};

	len = ALIF_SIZE(_a);
	if (ALIF_SIZE(_b) != len)
		return 0;

	if (_a->val[0] != _b->val[0])
		return 0;

	cmp = memcmp(_a->val, _b->val, len);
	return (cmp == 0);
}


static AlifObject* bytes_richCompare(AlifObject* _aa,
	AlifObject* _bb, AlifIntT _op) { // 1533
	/* Make sure both arguments are strings. */
	if (!(ALIFBYTES_CHECK(_aa) and ALIFBYTES_CHECK(_bb))) {
		if (_alif_getConfig()->bytesWarning and (_op == ALIF_EQ or _op == ALIF_NE)) {
			if (ALIFUSTR_CHECK(_aa) or ALIFUSTR_CHECK(_bb)) {
				//if (alifErr_warnEx(_alifExcBytesWarning_,
				//	"Comparison between bytes and string", 1))
				//	return nullptr;
			}
			if (ALIFLONG_CHECK(_aa) or ALIFLONG_CHECK(_bb)) {
				//if (alifErr_warnEx(_alifExcBytesWarning_,
				//	"Comparison between bytes and int", 1))
				//	return nullptr;
			}
		}
		ALIF_RETURN_NOTIMPLEMENTED;
	}

	AlifBytesObject* a = ALIFBYTES_CAST(_aa);
	AlifBytesObject* b = ALIFBYTES_CAST(_bb);
	if (a == b) {
		switch (_op) {
		case ALIF_EQ:
		case ALIF_LE:
		case ALIF_GE:
			/* a byte string is equal to itself */
			ALIF_RETURN_TRUE;
		case ALIF_NE:
		case ALIF_LT:
		case ALIF_GT:
			ALIF_RETURN_FALSE;
		default:
			//alifErr_badArgument();
			return nullptr;
		}
	}
	else if (_op == ALIF_EQ or _op == ALIF_NE) {
		AlifIntT eq = bytes_compareEq(a, b);
		eq ^= (_op == ALIF_NE);
		return alifBool_fromLong(eq);
	}
	else {
		AlifSizeT lenA = ALIF_SIZE(a);
		AlifSizeT lenB = ALIF_SIZE(b);
		AlifSizeT min_len = ALIF_MIN(lenA, lenB);
		int c;
		if (min_len > 0) {
			c = ALIF_CHARMASK(*a->val) - ALIF_CHARMASK(*b->val);
			if (c == 0)
				c = memcmp(a->val, b->val, min_len);
		}
		else {
			c = 0;
		}
		if (c != 0) {
			ALIF_RETURN_RICHCOMPARE(c, 0, _op);
		}
		ALIF_RETURN_RICHCOMPARE(lenA, lenB, _op);
	}
}


static AlifHashT bytes_hash(AlifObject* self) { // 1596
	AlifBytesObject* a = ALIFBYTES_CAST(self);
	ALIF_COMP_DIAG_PUSH
	ALIF_COMP_DIAG_IGNORE_DEPR_DECLS
		if (a->hash == -1) {
			/* Can't fail */
			a->hash = alif_hashBuffer(a->val, ALIF_SIZE(a));
		}
	return a->hash;
	ALIF_COMP_DIAG_POP
}

static AlifObject* bytes_subscript(AlifObject* op,
	AlifObject* item) { // 1610
	AlifBytesObject* self = ALIFBYTES_CAST(op);
	if (_alifIndex_check(item)) {
		AlifSizeT i = alifNumber_asSizeT(item, _alifExcIndexError_);
		if (i == -1 and alifErr_occurred())
			return nullptr;
		if (i < 0)
			i += ALIFBYTES_GET_SIZE(self);
		if (i < 0 || i >= ALIFBYTES_GET_SIZE(self)) {
			alifErr_setString(_alifExcIndexError_,
				"المؤشر خارج النطاق");
			return nullptr;
		}
		return _alifLong_fromUnsignedChar((unsigned char)self->val[i]);
	}
	else if (ALIFSLICE_CHECK(item)) {
		AlifSizeT start{}, stop{}, step{}, slicelength{}, i{};
		AlifUSizeT cur{};
		const char* source_buf{};
		char* result_buf{};
		AlifObject* result{};

		if (alifSlice_unpack(item, &start, &stop, &step) < 0) {
			return nullptr;
		}
		slicelength = alifSlice_adjustIndices(ALIFBYTES_GET_SIZE(self), &start,
			&stop, step);

		if (slicelength <= 0) {
			return alif_getConstant(ALIF_CONSTANT_EMPTY_BYTES);
		}
		else if (start == 0 and step == 1 and
			slicelength == ALIFBYTES_GET_SIZE(self) and
			ALIFBYTES_CHECKEXACT(self)) {
			return ALIF_NEWREF(self);
		}
		else if (step == 1) {
			return alifBytes_fromStringAndSize(
				ALIFBYTES_AS_STRING(self) + start,
				slicelength);
		}
		else {
			source_buf = ALIFBYTES_AS_STRING(self);
			result = alifBytes_fromStringAndSize(nullptr, slicelength);
			if (result == nullptr)
				return nullptr;

			result_buf = ALIFBYTES_AS_STRING(result);
			for (cur = start, i = 0; i < slicelength;
				cur += step, i++) {
				result_buf[i] = source_buf[cur];
			}

			return result;
		}
	}
	else {
		alifErr_format(_alifExcTypeError_,
			"مؤشر الثمانية يجب أن يكون عدد صحيح او قاطع, وليس %.200s",
			ALIF_TYPE(item)->name);
		return nullptr;
	}
}

static AlifIntT bytesBuffer_getBuffer(AlifObject* op,
	AlifBuffer* view, AlifIntT flags) { // 1676
	AlifBytesObject* self = ALIFBYTES_CAST(op);
	return alifBuffer_fillInfo(view, (AlifObject*)self, (void*)self->val, ALIF_SIZE(self),
		1, flags);
}

static AlifMappingMethods _bytesAsMapping_ = { // 1695
	.length = bytes_length,
	.subscript = bytes_subscript,
};

static AlifBufferProcs _bytesAsBuffer_ = { // 1701
	bytesBuffer_getBuffer,
	nullptr,
};






static AlifObject* bytes_replaceImpl(AlifBytesObject* _self, AlifBuffer* _old, AlifBuffer* _new,
	AlifSizeT _count) { // 2285
	return stringLib_replace((AlifObject*)_self,
		(const char*)_old->buf, _old->len,
		(const char*)_new->buf, _new->len, _count);
}



static AlifMethodDef _bytesMethods_[] = { // 2603
	{"حرف_صغير", stringLib_lower, METHOD_NOARGS},

	BYTES_REPLACE_METHODDEF

	{nullptr,     nullptr}
};



static AlifObject* bytes_subTypeNew(AlifTypeObject*, AlifObject*); // 2680

static AlifObject* bytes_newImpl(AlifTypeObject* type,
	AlifObject* x, const char* encoding,
	const char* errors) { // 2692
	AlifObject* bytes{};
	AlifObject* func{};
	AlifSizeT size{};

	if (x == nullptr) {
		if (encoding != nullptr || errors != nullptr) {
			alifErr_setString(_alifExcTypeError_,
				encoding != nullptr ?
				"ترميز بدون وسيط نصي" :
				"اخطاء بدون وسيط نصي");
			return nullptr;
		}
		bytes = alifBytes_fromStringAndSize(nullptr, 0);
	}
	else if (encoding != nullptr) {
		/* Encode via the codec registry */
		if (!ALIFUSTR_CHECK(x)) {
			alifErr_setString(_alifExcTypeError_,
				"ترميز بدون وسيط نصي");
			return nullptr;
		}
		bytes = alifUStr_asEncodedString(x, encoding, errors);
	}
	else if (errors != nullptr) {
		alifErr_setString(_alifExcTypeError_,
			ALIFUSTR_CHECK(x) ?
			"وسيط نصي بدون ترميز" :
			"اخطاء بدون وسيط نصي");
		return nullptr;
	}
	else if ((func = _alifObject_lookupSpecial(x, &ALIF_STR(__bytes__))) != nullptr) {
		bytes = _alifObject_callNoArgs(func);
		ALIF_DECREF(func);
		if (bytes == nullptr)
			return nullptr;
		if (!ALIFBYTES_CHECK(bytes)) {
			alifErr_format(_alifExcTypeError_,
				"__ثمانيات__ ارجعت ليس-ثمانيات (نوع %.200s)",
				ALIF_TYPE(bytes)->name);
			ALIF_DECREF(bytes);
			return nullptr;
		}
	}
	else if (alifErr_occurred())
		return nullptr;
	else if (ALIFUSTR_CHECK(x)) {
		alifErr_setString(_alifExcTypeError_,
			"وسيط نصي بدون ترميز");
		return nullptr;
	}
	/* Is it an integer? */
	else if (_alifIndex_check(x)) {
		size = alifNumber_asSizeT(x, _alifExcOverflowError_);
		if (size == -1 and alifErr_occurred()) {
			if (!alifErr_exceptionMatches(_alifExcTypeError_))
				return nullptr;
			alifErr_clear();  /* fall through */
			bytes = alifBytes_fromObject(x);
		}
		else {
			if (size < 0) {
				alifErr_setString(_alifExcValueError_, "عد بالسالب");
				return nullptr;
			}
			bytes = _alifBytes_fromSize(size, 1);
		}
	}
	else {
		bytes = alifBytes_fromObject(x);
	}

	if (bytes != nullptr and type != &_alifBytesType_) {
		ALIF_SETREF(bytes, bytes_subTypeNew(type, bytes));
	}

	return bytes;
}

static AlifObject* _alifBytes_fromBuffer(AlifObject* x) { // 2778
	AlifObject* new_{};
	AlifBuffer view{};

	if (alifObject_getBuffer(x, &view, ALIFBUF_FULL_RO) < 0)
		return nullptr;

	new_ = alifBytes_fromStringAndSize(nullptr, view.len);
	if (!new_)
		goto fail;
	if (alifBuffer_toContiguous(((AlifBytesObject*)new_)->val,
		&view, view.len, 'C') < 0)
		goto fail;
	alifBuffer_release(&view);
	return new_;

fail:
	ALIF_XDECREF(new_);
	alifBuffer_release(&view);
	return nullptr;
}

static AlifObject* _alifBytes_fromList(AlifObject* x) { // 2802
	AlifSizeT i{}, size = ALIFLIST_GET_SIZE(x);
	AlifSizeT value{};
	char* str{};
	AlifObject* item{};
	AlifBytesWriter writer{};

	_alifBytesWriter_init(&writer);
	str = (char*)_alifBytesWriter_alloc(&writer, size);
	if (str == nullptr)
		return nullptr;
	writer.overAllocate = 1;
	size = writer.allocated;

	for (i = 0; i < ALIFLIST_GET_SIZE(x); i++) {
		item = ALIFLIST_GET_ITEM(x, i);
		ALIF_INCREF(item);
		value = alifNumber_asSizeT(item, nullptr);
		ALIF_DECREF(item);
		if (value == -1 and alifErr_occurred())
			goto error;

		if (value < 0 or value >= 256) {
			alifErr_setString(_alifExcValueError_,
				"الثمانية يجب أن تكون في المدى(0, 256)");
			goto error;
		}

		if (i >= size) {
			str = (char*)_alifBytesWriter_resize(&writer, str, size + 1);
			if (str == nullptr)
				return nullptr;
			size = writer.allocated;
		}
		*str++ = (char)value;
	}
	return _alifBytesWriter_finish(&writer, str);

error:
	_alifBytesWriter_dealloc(&writer);
	return nullptr;
}

static AlifObject* _alifBytes_fromTuple(AlifObject* x) { // 2847
	AlifObject* bytes{};
	AlifSizeT i{}, size = ALIFTUPLE_GET_SIZE(x);
	AlifSizeT value{};
	char* str{};
	AlifObject* item{};

	bytes = alifBytes_fromStringAndSize(nullptr, size);
	if (bytes == nullptr)
		return nullptr;
	str = ((AlifBytesObject*)bytes)->val;

	for (i = 0; i < size; i++) {
		item = ALIFTUPLE_GET_ITEM(x, i);
		value = alifNumber_asSizeT(item, nullptr);
		if (value == -1 and alifErr_occurred())
			goto error;

		if (value < 0 or value >= 256) {
			alifErr_setString(_alifExcValueError_,
				"الثمانية يجب أن تكون في المدى(0, 256)");
			goto error;
		}
		*str++ = (char)value;
	}
	return bytes;

error:
	ALIF_DECREF(bytes);
	return nullptr;
}

static AlifObject* _alifBytes_fromIterator(AlifObject* it,
	AlifObject* x) { // 2881
	char* str{};
	AlifSizeT i{}, size{};
	AlifBytesWriter writer{};

	/* For iterator version, create a bytes object and resize as needed */
	size = alifObject_lengthHint(x, 64);
	if (size == -1 and alifErr_occurred())
		return nullptr;

	_alifBytesWriter_init(&writer);
	str = (char*)_alifBytesWriter_alloc(&writer, size);
	if (str == nullptr)
		return nullptr;
	writer.overAllocate = 1;
	size = writer.allocated;

	/* Run the iterator to exhaustion */
	for (i = 0; ; i++) {
		AlifObject* item{};
		AlifSizeT value{};

		/* Get the next item */
		item = alifIter_next(it);
		if (item == nullptr) {
			if (alifErr_occurred())
				goto error;
			break;
		}

		/* Interpret it as an int (__index__) */
		value = alifNumber_asSizeT(item, nullptr);
		ALIF_DECREF(item);
		if (value == -1 and alifErr_occurred())
			goto error;

		/* Range check */
		if (value < 0 or value >= 256) {
			alifErr_setString(_alifExcValueError_,
				"الثمانية يجب أن تكون في المدى(0, 256)");
			goto error;
		}

		/* Append the byte */
		if (i >= size) {
			str = (char*)_alifBytesWriter_resize(&writer, str, size + 1);
			if (str == nullptr)
				return nullptr;
			size = writer.allocated;
		}
		*str++ = (char)value;
	}

	return _alifBytesWriter_finish(&writer, str);

error:
	_alifBytesWriter_dealloc(&writer);
	return nullptr;
}

AlifObject* alifBytes_fromObject(AlifObject* _x) { // 2943
	AlifObject* it{}, * result{};

	if (_x == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}

	if (ALIFBYTES_CHECKEXACT(_x)) {
		return ALIF_NEWREF(_x);
	}

	/* Use the modern buffer interface */
	if (alifObject_checkBuffer(_x))
		return _alifBytes_fromBuffer(_x);

	if (ALIFLIST_CHECKEXACT(_x))
		return _alifBytes_fromList(_x);

	if (ALIFTUPLE_CHECKEXACT(_x))
		return _alifBytes_fromTuple(_x);

	if (!ALIFUSTR_CHECK(_x)) {
		it = alifObject_getIter(_x);
		if (it != nullptr) {
			result = _alifBytes_fromIterator(it, _x);
			ALIF_DECREF(it);
			return result;
		}
		if (!alifErr_exceptionMatches(_alifExcTypeError_)) {
			return nullptr;
		}
	}

	alifErr_format(_alifExcTypeError_,
		"لا يمكن تحويل الكائن '%.200s' إلى نوع ثمانيات",
		ALIF_TYPE(_x)->name);
	return nullptr;
}


static AlifObject* bytes_alloc(AlifTypeObject* _self,
	AlifSizeT _nitems) { // 2990
	AlifBytesObject* obj = (AlifBytesObject*)alifType_genericAlloc(_self, _nitems);
	if (obj == nullptr) {
		return nullptr;
	}
	ALIF_COMP_DIAG_PUSH
	ALIF_COMP_DIAG_IGNORE_DEPR_DECLS
		obj->hash = -1;
	ALIF_COMP_DIAG_POP
	return (AlifObject*)obj;
}

static AlifObject* bytes_subTypeNew(AlifTypeObject* _type,
	AlifObject* _tmp) { // 3004
	AlifObject* pnew{};
	AlifSizeT n{};

	n = ALIFBYTES_GET_SIZE(_tmp);
	pnew = _type->alloc(_type, n);
	if (pnew != nullptr) {
		memcpy(ALIFBYTES_AS_STRING(pnew),
			ALIFBYTES_AS_STRING(_tmp), n + 1);
		ALIF_COMP_DIAG_PUSH
		ALIF_COMP_DIAG_IGNORE_DEPR_DECLS
		((AlifBytesObject*)pnew)->hash =
			((AlifBytesObject*)_tmp)->hash;
		ALIF_COMP_DIAG_POP
	}
	return pnew;
}

AlifTypeObject _alifBytesType_ = { // 3028
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "ثمانيات",
	.basicSize = ALIFBYTESOBJECT_SIZE,
	.itemSize = sizeof(char),
	.repr = bytes_repr,
	.asMapping = &_bytesAsMapping_,
	.hash = bytes_hash,
	.getAttro = alifObject_genericGetAttr,
	.asBuffer = &_bytesAsBuffer_,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
		ALIF_TPFLAGS_BYTES_SUBCLASS | _ALIF_TPFLAGS_MATCH_SELF,
	.richCompare = (RichCmpFunc)bytes_richCompare,
	.methods = _bytesMethods_,
	.alloc = bytes_alloc,
	.new_ = bytes_new,
	.free = alifMem_objFree,
	.versionTag = _ALIF_TYPE_VERSION_BYTES,
};

void alifBytes_concat(AlifObject** _pv, AlifObject* _w) { // 3072
	if (*_pv == nullptr) return;
	if (_w == nullptr) {
		ALIF_CLEAR(*_pv);
		return;
	}

	if (ALIF_REFCNT(*_pv) == 1 and ALIFBYTES_CHECKEXACT(*_pv)) {
		/* Only one reference, so we can resize in place */
		AlifSizeT oldsize{};
		AlifBuffer wb{};

		if (alifObject_getBuffer(_w, &wb, ALIFBUF_SIMPLE) != 0) {
			alifErr_format(_alifExcTypeError_, "لم يستطع دمج %.100s مع %.100s",
				ALIF_TYPE(_w)->name, ALIF_TYPE(*_pv)->name);
			ALIF_CLEAR(*_pv);
			return;
		}

		oldsize = ALIFBYTES_GET_SIZE(*_pv);
		if (oldsize > ALIF_SIZET_MAX - wb.len) {
			//alifErr_noMemory();
			goto error;
		}
		if (_alifBytes_resize(_pv, oldsize + wb.len) < 0)
			goto error;

		memcpy(ALIFBYTES_AS_STRING(*_pv) + oldsize, wb.buf, wb.len);
		alifBuffer_release(&wb);
		return;

error:
		alifBuffer_release(&wb);
		ALIF_CLEAR(*_pv);
		return;
	}

	else {
		/* Multiple references, need to create new object */
		AlifObject* v{};
		v = bytes_concat(*_pv, _w);
		ALIF_SETREF(*_pv, v);
	}
}


AlifIntT _alifBytes_resize(AlifObject** _pv, AlifSizeT _newSize) { // 3141
	AlifObject* v{};
	AlifBytesObject* sv{};
	v = *_pv;
	if (!ALIFBYTES_CHECK(v) or _newSize < 0) {
		*_pv = 0;
		ALIF_DECREF(v);
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	AlifSizeT oldsize = ALIFBYTES_GET_SIZE(v);
	if (oldsize == _newSize) {
		/* return early if newsize equals to v->size */
		return 0;
	}
	if (oldsize == 0) {
		*_pv = _alifBytes_fromSize(_newSize, 0);
		ALIF_DECREF(v);
		return (*_pv == nullptr) ? -1 : 0;
	}
	if (_newSize == 0) {
		*_pv = bytes_getEmpty();
		ALIF_DECREF(v);
		return 0;
	}
	if (ALIF_REFCNT(v) != 1) {
		if (oldsize < _newSize) {
			*_pv = _alifBytes_fromSize(_newSize, 0);
			if (*_pv) {
				memcpy(ALIFBYTES_AS_STRING(*_pv), ALIFBYTES_AS_STRING(v), oldsize);
			}
		}
		else {
			*_pv = alifBytes_fromStringAndSize(ALIFBYTES_AS_STRING(v), _newSize);
		}
		ALIF_DECREF(v);
		return (*_pv == nullptr) ? -1 : 0;
	}

	_ALIFREFTRACERTRACK(v, AlifRefTracerEvent_::Alif_RefTracer_Destroy);
	*_pv = (AlifObject*)
		alifMem_objRealloc(v, ALIFBYTESOBJECT_SIZE + _newSize);
	if (*_pv == nullptr) {
		alifMem_objFree(v);
		//alifErr_noMemory();
		return -1;
	}
	_alif_newReferenceNoTotal(*_pv);
	sv = (AlifBytesObject*)*_pv;
	ALIF_SET_SIZE(sv, _newSize);
	sv->val[_newSize] = '\0';
	ALIF_COMP_DIAG_PUSH;
	ALIF_COMP_DIAG_IGNORE_DEPR_DECLS;
	sv->hash = -1;          /* invalidate cached hash value */
	ALIF_COMP_DIAG_POP;
	return 0;
}





/* AlifBytesWriter API */

#ifdef _WINDOWS // 3363
   /* On Windows, overallocate by 50% is the best factor */
#  define OVERALLOCATE_FACTOR 2
#else
   /* On Linux, overallocate by 25% is the best factor */
#  define OVERALLOCATE_FACTOR 4
#endif


void _alifBytesWriter_init(AlifBytesWriter* _writer) { // 3371
	memset(_writer, 0, offsetof(AlifBytesWriter, smallBuffer));
}



void _alifBytesWriter_dealloc(AlifBytesWriter* _writer) { // 3382
	ALIF_CLEAR(_writer->buffer);
}

ALIF_LOCAL_INLINE(char*)
alifBytesWriter_asString(AlifBytesWriter* _writer) { // 3388
	if (_writer->useSmallBuffer) {
		return _writer->smallBuffer;
	}
	else if (_writer->useByteArray) {
		return ALIFBYTEARRAY_AS_STRING(_writer->buffer);
	}
	else {
		return ALIFBYTES_AS_STRING(_writer->buffer);
	}
}

ALIF_LOCAL_INLINE(AlifSizeT)
alifBytesWriter_getSize(AlifBytesWriter* _writer, char* _str) { // 3405
	const char* start = alifBytesWriter_asString(_writer);
	return _str - start;
}

void* _alifBytesWriter_resize(AlifBytesWriter* _writer, void* _str, AlifSizeT _size) { // 3452
	AlifSizeT allocated{}, pos{};

	allocated = _size;
	if (_writer->overAllocate
		and allocated <= (ALIF_SIZET_MAX - allocated / OVERALLOCATE_FACTOR)) {
		/* overallocate to limit the number of realloc() */
		allocated += allocated / OVERALLOCATE_FACTOR;
	}

	pos = alifBytesWriter_getSize(_writer, (char*)_str);
	if (!_writer->useSmallBuffer) {
		if (_writer->useByteArray) {
			if (alifByteArray_resize(_writer->buffer, allocated))
				goto error;
		}
		else {
			if (_alifBytes_resize(&_writer->buffer, allocated))
				goto error;
		}
	}
	else {
		/* convert from stack buffer to bytes object buffer */

		if (_writer->useByteArray)
			_writer->buffer = alifByteArray_fromStringAndSize(nullptr, allocated);
		else
			_writer->buffer = alifBytes_fromStringAndSize(nullptr, allocated);
		if (_writer->buffer == nullptr)
			goto error;

		if (pos != 0) {
			char* dest;
			if (_writer->useByteArray)
				dest = ALIFBYTEARRAY_AS_STRING(_writer->buffer);
			else
				dest = ALIFBYTES_AS_STRING(_writer->buffer);
			memcpy(dest,
				_writer->smallBuffer,
				pos);
		}

		_writer->useSmallBuffer = 0;
	}
	_writer->allocated = allocated;

	_str = alifBytesWriter_asString(_writer) + pos;
	return _str;

error:
	_alifBytesWriter_dealloc(_writer);
	return nullptr;
}




void* _alifBytesWriter_prepare(AlifBytesWriter* _writer, void* _str, AlifSizeT _size) { // 3522
	AlifSizeT newMinSize{};

	if (_size == 0) {
		/* nothing to do */
		return _str;
	}

	if (_writer->minSize > ALIF_SIZET_MAX - _size) {
		//alifErr_noMemory();
		_alifBytesWriter_dealloc(_writer);
		return nullptr;
	}
	newMinSize = _writer->minSize + _size;

	if (newMinSize > _writer->allocated)
		_str = _alifBytesWriter_resize(_writer, _str, newMinSize);

	_writer->minSize = newMinSize;
	return _str;
}


void* _alifBytesWriter_alloc(AlifBytesWriter* writer, AlifSizeT size) { // 3552
	writer->useSmallBuffer = 1;
	writer->allocated = sizeof(writer->smallBuffer);
	return _alifBytesWriter_prepare(writer, writer->smallBuffer, size);
}



AlifObject* _alifBytesWriter_finish(AlifBytesWriter* _writer, void* _str) { // 3582
	AlifSizeT size{};
	AlifObject* result{};

	size = alifBytesWriter_getSize(_writer, (char*)_str);
	if (size == 0 and !_writer->useByteArray) {
		ALIF_CLEAR(_writer->buffer);
		/* Get the empty byte string singleton */
		result = alifBytes_fromStringAndSize(nullptr, 0);
	}
	else if (_writer->useSmallBuffer) {
		if (_writer->useByteArray) {
			result = alifByteArray_fromStringAndSize(_writer->smallBuffer, size);
		}
		else {
			result = alifBytes_fromStringAndSize(_writer->smallBuffer, size);
		}
	}
	else {
		result = _writer->buffer;
		_writer->buffer = nullptr;

		if (size != _writer->allocated) {
			if (_writer->useByteArray) {
				if (alifByteArray_resize(result, size)) {
					ALIF_DECREF(result);
					return nullptr;
				}
			}
			else {
				if (_alifBytes_resize(&result, size)) {
					return nullptr;
				}
			}
		}
	}
	return result;
}





void* _alifBytesWriter_writeBytes(AlifBytesWriter* _writer, void* _ptr,
	const void* _bytes, AlifSizeT _size) { // 3626
	char* str = (char*)_ptr;

	str = (char*)_alifBytesWriter_prepare(_writer, str, _size);
	if (str == nullptr)
		return nullptr;

	memcpy(str, _bytes, _size);
	str += _size;

	return str;
}




void _alifBytes_repeat(char* _dest, AlifSizeT _lenDest,
	const char* _src, AlifSizeT _lenSrc) { // 3657
	if (_lenDest == 0) {
		return;
	}
	if (_lenSrc == 1) {
		memset(_dest, _src[0], _lenDest);
	}
	else {
		if (_src != _dest) {
			memcpy(_dest, _src, _lenSrc);
		}
		AlifSizeT copied = _lenSrc;
		while (copied < _lenDest) {
			AlifSizeT bytesToCopy = ALIF_MIN(copied, _lenDest - copied);
			memcpy(_dest + copied, _dest, bytesToCopy);
			copied += bytesToCopy;
		}
	}
}
