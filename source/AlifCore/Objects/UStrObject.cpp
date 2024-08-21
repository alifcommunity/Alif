#include "alif.h"

#include "AlifCore_State.h"
#include "AlifCore_UStrObject.h"




#define MAX_UNICODE 0x10ffff // 106



// 129
#define ALIFUSTR_LENGTH(_op)                           \
    (ALIFASCIIOBJECT_CAST(_op)->length)
#define ALIFUSTR_STATE(_op)                            \
    (ALIFASCIIOBJECT_CAST(_op)->state)
#define ALIFUSTR_HASH(_op)                             \
    (ALIFASCIIOBJECT_CAST(_op)->hash)



#define LATIN1 ALIF_LATIN1_CHR // 180



static inline AlifObject* unicode_getEmpty() { // 214
	ALIF_DECLARE_STR(Empty, "");
	return &ALIF_STR(Empty);
}


// 360
#define ALIF_RETURN_UNICODE_EMPTY	return unicode_getEmpty();



AlifObject* alifUStr_new(AlifSizeT _size, AlifUCS4 _maxChar) { // 1282
	/* Optimization for empty strings */
	if (_size == 0) {
		return unicode_getEmpty();
	}

	AlifObject* obj{};
	AlifCompactUStrObject* unicode{};
	void* data{};
	AlifIntT kind{};
	AlifIntT isAscii{};
	AlifSizeT charSize{};
	AlifSizeT structSize{};

	isAscii = 0;
	structSize = sizeof(AlifCompactUStrObject);
	if (_maxChar < 128) {
		kind = AlifUStrKind_::AlifUStr_1Byte_Kind;
		charSize = 1;
		isAscii = 1;
		structSize = sizeof(AlifASCIIObject);
	}
	else if (_maxChar < 256) {
		kind = AlifUStrKind_::AlifUStr_1Byte_Kind;
		charSize = 1;
	}
	else if (_maxChar < 65536) {
		kind = AlifUStrKind_::AlifUStr_2Byte_Kind;
		charSize = 2;
	}
	else {
		if (_maxChar > MAX_UNICODE) {
			// error
			return nullptr;
		}
		kind = AlifUStrKind_::AlifUStr_4Byte_Kind;
		charSize = 4;
	}

	/* Ensure we won't overflow the size. */
	if (_size < 0) {
		// error
		return nullptr;
	}
	//if (size > ((ALIF_SIZET_MAX - struct_size) / char_size - 1)) {
	//	return alifErr_noMemory();

	obj = (AlifObject*)alifMem_objAlloc(structSize + (_size + 1) * charSize);
	//if (obj == nullptr) {
	//	return alifErr_noMemory();
	//}
	alifObject_init(obj, &_alifUStrType_);

	unicode = (AlifCompactUStrObject*)obj;
	if (isAscii)
		data = ((AlifASCIIObject*)obj) + 1;
	else
		data = unicode + 1;
	ALIFUSTR_LENGTH(unicode) = _size;
	ALIFUSTR_HASH(unicode) = -1;
	ALIFUSTR_STATE(unicode).interned = 0;
	ALIFUSTR_STATE(unicode).kind = kind;
	ALIFUSTR_STATE(unicode).compact = 1;
	ALIFUSTR_STATE(unicode).ascii = isAscii;
	ALIFUSTR_STATE(unicode).statically_allocated = 0;
	if (isAscii) {
		((char*)data)[_size] = 0;
	}
	else if (kind == AlifUStrKind_::AlifUStr_1Byte_Kind) {
		((char*)data)[_size] = 0;
		unicode->utf8 = nullptr;
		unicode->utf8Length = 0;
	}
	else {
		unicode->utf8 = nullptr;
		unicode->utf8Length = 0;
		if (kind == AlifUStrKind_::AlifUStr_2Byte_Kind)
			((AlifUCS2*)data)[_size] = 0;
		else
			((AlifUCS4*)data)[_size] = 0;
	}
	return obj;
}



static AlifObject* get_latin1Char(AlifUCS1 _ch) { // 1867
	AlifObject* obj = LATIN1(_ch);
	return obj;
}



AlifObject* alifUStr_fromString(const char* u) { // 2084
	AlifUSizeT size = strlen(u);
	if (size > ALIF_SIZET_MAX) {
		// error
		return nullptr;
	}
	return alifUStr_decodeUTF8Stateful(u, (AlifSizeT)size, nullptr, nullptr);
}









#include "StringLib/UCS2Lib.h" // 4954
#include "StringLib/Codecs.h"
#include "StringLib/Undef.h"

#include "StringLib/UCS4Lib.h" // 4958
#include "StringLib/Codecs.h"
#include "StringLib/Undef.h"


static AlifObject* unicode_decodeUTF8(const char* _str, AlifSizeT _size,
	AlifErrorHandler_ _errorHandler, const char* _errors, AlifSizeT* _consumed) { // 5151
	if (_size == 0) {
		if (_consumed) {
			*_consumed = 0;
		}
		ALIF_RETURN_UNICODE_EMPTY;
	}

	if (_size == 1 && (unsigned char)_str[0] < 128) {
		if (_consumed) {
			*_consumed = 1;
		}
		return get_latin1Char((unsigned char)_str[0]);
	}

	const char* starts = _str;
	const char* end = _str + _size;
	AlifObject* u = alifUStr_new(_size, 127);
	if (u == nullptr) {
		return nullptr;
	}
	AlifSizeT decoded = ascii_decode(_str, end, ALIFUSTR_1BYTE_DATA(u));
	if (decoded == _size) {
		if (_consumed) {
			*_consumed = _size;
		}
		return u;
	}
	_str += decoded;
	_size -= decoded;

	AlifUStrWriter writer{};
	alifUStrWriter_initWithBuffer(&writer, u);
	writer.pos = decoded;

	if (unicode_decodeUTF8Impl(&writer, starts, _str, end,
		_errorHandler, _errors, _consumed) < 0) {
		alifUStrWriter_dealloc(&writer);
		return nullptr;
	}
	return alifUStrWriter_finish(&writer);
}


AlifObject* alifUStr_decodeUTF8Stateful(const char* s, AlifSizeT size,
	const char* errors, AlifSizeT* consumed) { // 5245
	return unicode_decodeUTF8(s, size, AlifErrorHandler_::Alif_Error_Unknown, errors, consumed);
}


AlifIntT alif_decodeUTF8Ex(const char* s, AlifSizeT size, wchar_t** wstr, AlifUSizeT* wlen,
	const char** reason, AlifErrorHandler_ errors) { // 5267
	const char* origs = s;
	const char* e_;
	wchar_t* unicode;
	AlifSizeT outPos;

	int surrogateescape = 0;
	int surrogatepass = 0;
	switch (errors)
	{
	case AlifErrorHandler_::Alif_Error_Strict:
		break;
	case AlifErrorHandler_::Alif_Error_SurrogateEscape:
		surrogateescape = 1;
		break;
	case AlifErrorHandler_::Alif_Error_SurrogatePass:
		surrogatepass = 1;
		break;
	default:
		return -3;
	}

	/* Note: size will always be longer than the resulting Unicode
	   character count */
	if (ALIF_SIZET_MAX / (AlifSizeT)sizeof(wchar_t) - 1 < size) {
		return -1;
	}

	unicode = (wchar_t*)alifMem_dataAlloc((size + 1) * sizeof(wchar_t));
	if (!unicode) {
		return -1;
	}

	/* Unpack UTF-8 encoded data */
	e_ = s + size;
	outPos = 0;
	while (s < e_) {
		AlifUCS4 ch_;
#if SIZEOF_WCHAR_T == 4
		ch_ = ucs4Lib_utf8Decode(&s, e_, (AlifUCS4*)unicode, &outPos);
#else
		ch_ = ucs2Lib_utf8Decode(&s, e_, (AlifUCS2*)unicode, &outPos);
#endif
		if (ch_ > 0xFF) {
#if SIZEOF_WCHAR_T == 4
			ALIF_UNREACHABLE();
#else
			/* write a surrogate pair */
			unicode[outPos++] = (wchar_t)alifUnicode_highSurrogate(ch_);
			unicode[outPos++] = (wchar_t)alifUnicode_lowSurrogate(ch_);
#endif
		}
		else {
			if (!ch_ && s == e_) {
				break;
			}

			if (surrogateescape) {
				unicode[outPos++] = 0xDC00 + (unsigned char)*s++;
			}
			else {
				/* Is it a valid three-byte code? */
				if (surrogatepass
					&& (e_ - s) >= 3
					&& (s[0] & 0xf0) == 0xe0
					&& (s[1] & 0xc0) == 0x80
					&& (s[2] & 0xc0) == 0x80)
				{
					ch_ = ((s[0] & 0x0f) << 12) + ((s[1] & 0x3f) << 6) + (s[2] & 0x3f);
					s += 3;
					unicode[outPos++] = ch_;
				}
				else {
					alifMem_dataFree(unicode);
					if (reason != nullptr) {
						switch (ch_) {
						case 0:
							*reason = "unexpected end of data";
							break;
						case 1:
							*reason = "invalid start byte";
							break;
							/* 2, 3, 4 */
						default:
							*reason = "invalid continuation byte";
							break;
						}
					}
					if (wlen != nullptr) {
						*wlen = s - origs;
					}
					return -2;
				}
			}
		}
	}
	unicode[outPos] = L'\0';
	if (wlen) {
		*wlen = outPos;
	}
	*wstr = unicode;
	return 0;
}
