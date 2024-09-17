#include "alif.h"

#include "AlifCore_Object.h"
#include "AlifCore_State.h"
#include "AlifCore_UStrObject.h"
#include "AlifCore_BytesObject.h"


// Forward Declaration
static inline AlifIntT alifUStrWriter_writeCharInline(AlifUStrWriter*, AlifUCS4);
static inline void alifUStrWriter_initWithBuffer(AlifUStrWriter*, AlifObject*);

#define MAX_UNICODE 0x10ffff // 106

// 114
#define ALIFUSTR_UTF8(_op) ALIFCOMPACTUSTROBJECT_CAST(_op)->utf8

#define ALIFUSTR_UTF8_LENGTH(_op) ALIFCOMPACTUSTROBJECT_CAST(_op)->utf8Length

// 129
#define ALIFUSTR_LENGTH(_op)                           \
    (ALIFASCIIOBJECT_CAST(_op)->length)
#define ALIFUSTR_STATE(_op)                            \
    (ALIFASCIIOBJECT_CAST(_op)->state)
#define ALIFUSTR_HASH(_op)                             \
    (ALIFASCIIOBJECT_CAST(_op)->hash)					
#define ALIFUSTR_DATA_ANY(op)                         \
    (ALIFUSTROBJECT_CAST(op)->data.any)

// 151
#define ALIFUSTR_HAS_UTF8_MEMORY(_op)                  \
    !ALIFUSTR_IS_COMPACT_ASCII(_op) and ALIFUSTR_UTF8(_op)		\
	and ALIFUSTR_UTF8(_op) != ALIFUSTR_DATA(_op)


// 161
#define ALIFUSTR_CONVERT_BYTES(from_type, to_type, begin, end, to) \
    do {									\
        to_type* to_ = (to_type*)(to);                 \
        const from_type* _iter = (const from_type*)(begin);\
        const from_type* _end = (const from_type*)(end);\
        AlifSizeT n = (_end) - (_iter);                \
        const from_type *_unrolled_end =                \
            _iter + ALIF_SIZE_ROUND_DOWN(n, 4);          \
        while (_iter < (_unrolled_end)) {               \
            to_[0] = (to_type) _iter[0];                \
            to_[1] = (to_type) _iter[1];                \
            to_[2] = (to_type) _iter[2];                \
            to_[3] = (to_type) _iter[3];                \
            _iter += 4; to_ += 4;                       \
        }                                               \
        while (_iter < (_end)) *to_++ = (to_type) *_iter++;       \
    } while (0)


#define LATIN1 ALIF_LATIN1_CHR // 180

// 182
#ifdef _WINDOWS
   /* On Windows, overallocate by 50% is the best factor */
#  define OVERALLOCATE_FACTOR 2
#else
   /* On Linux, overallocate by 25% is the best factor */
#  define OVERALLOCATE_FACTOR 4
#endif


static inline AlifObject* unicode_getEmpty() { // 214
	ALIF_DECLARE_STR(Empty, "");
	return &ALIF_STR(Empty);
}


static inline AlifObject* get_internedDict(AlifInterpreter* _interp) { // 223
	return ALIF_INTERP_CACHED_OBJECT(_interp, internedStrings);
}


#define INTERNED_STRINGS _alifDureRun_.cachedObjects.internedStrings // 231


// 360
#define ALIF_RETURN_UNICODE_EMPTY	return unicode_getEmpty();






AlifErrorHandler_ alif_getErrorHandler(const char* _errors) { // 488
	if (_errors == nullptr or strcmp(_errors, "strict") == 0) {
		return AlifErrorHandler_::Alif_Error_Strict;
	}
	if (strcmp(_errors, "surrogateescape") == 0) {
		return AlifErrorHandler_::Alif_Error_SurrogateEscape;
	}
	if (strcmp(_errors, "replace") == 0) {
		return AlifErrorHandler_::Alif_Error_Replace;
	}
	if (strcmp(_errors, "ignore") == 0) {
		return AlifErrorHandler_::Alif_Error_Ignore;
	}
	if (strcmp(_errors, "backslashreplace") == 0) {
		return AlifErrorHandler_::Alif_Error_BackSlashReplace;
	}
	if (strcmp(_errors, "surrogatepass") == 0) {
		return AlifErrorHandler_::Alif_Error_SurrogatePass;
	}
	if (strcmp(_errors, "xmlcharrefreplace") == 0) {
		return AlifErrorHandler_::Alif_Error_XMLCharRefReplace;
	}
	return AlifErrorHandler_::Alif_Error_Other;
}



static AlifObject* ustr_result(AlifObject* _uStr) { // 727
	AlifSizeT length = ALIFUSTR_GET_LENGTH(_uStr);
	if (length == 0) {
		AlifObject* empty = unicode_getEmpty();
		if (_uStr != empty) {
			ALIF_DECREF(_uStr);
		}
		return empty;
	}

	if (length == 1) {
		int kind = ALIFUSTR_KIND(_uStr);
		if (kind == AlifUStrKind_::AlifUStr_1Byte_Kind) {
			const AlifUCS1* data = ALIFUSTR_1BYTE_DATA(_uStr);
			AlifUCS1 ch = data[0];
			AlifObject* latin1_char = LATIN1(ch);
			if (_uStr != latin1_char) {
				ALIF_DECREF(_uStr);
			}
			return latin1_char;
		}
	}

	return _uStr;
}



static char* backSlash_replace(AlifBytesWriter* _writer, char* _str,
	AlifObject* _uStr, AlifSizeT _collStart, AlifSizeT _collEnd) { // 771
	AlifSizeT size{}, i_{};
	AlifUCS4 ch_{};
	AlifIntT kind{};
	const void* data{};

	kind = ALIFUSTR_KIND(_uStr);
	data = ALIFUSTR_DATA(_uStr);

	size = 0;
	/* determine replacement size */
	for (i_ = _collStart; i_ < _collEnd; ++i_) {
		AlifSizeT incr;

		ch_ = ALIFUSTR_READ(kind, data, i_);
		if (ch_ < 0x100)
			incr = 2 + 2;
		else if (ch_ < 0x10000)
			incr = 2 + 4;
		else {
			incr = 2 + 8;
		}
		if (size > ALIF_SIZET_MAX - incr) {
			//alifErr_setString(_alifExcOverflowError_,
			//	"encoded result is too long for a Alif string");
			return nullptr;
		}
		size += incr;
	}

	_str = (char*)alifBytesWriter_prepare(_writer, _str, size);
	if (_str == nullptr)
		return nullptr;

	/* generate replacement */
	for (i_ = _collStart; i_ < _collEnd; ++i_) {
		ch_ = ALIFUSTR_READ(kind, data, i_);
		*_str++ = '\\';
		if (ch_ >= 0x00010000) {
			*_str++ = 'U';
			*_str++ = _alifHexdigits_[(ch_ >> 28) & 0xf];
			*_str++ = _alifHexdigits_[(ch_ >> 24) & 0xf];
			*_str++ = _alifHexdigits_[(ch_ >> 20) & 0xf];
			*_str++ = _alifHexdigits_[(ch_ >> 16) & 0xf];
			*_str++ = _alifHexdigits_[(ch_ >> 12) & 0xf];
			*_str++ = _alifHexdigits_[(ch_ >> 8) & 0xf];
		}
		else if (ch_ >= 0x100) {
			*_str++ = 'u';
			*_str++ = _alifHexdigits_[(ch_ >> 12) & 0xf];
			*_str++ = _alifHexdigits_[(ch_ >> 8) & 0xf];
		}
		else
			*_str++ = 'x';
		*_str++ = _alifHexdigits_[(ch_ >> 4) & 0xf];
		*_str++ = _alifHexdigits_[ch_ & 0xf];
	}
	return _str;
}




static char* xmlCharRef_replace(AlifBytesWriter* _writer, char* _str,
	AlifObject* _uStr, AlifSizeT _collStart, AlifSizeT _collEnd) { // 837
	AlifSizeT size{}, i_{};
	AlifUCS4 ch_{};
	AlifIntT kind{};
	const void* data{};

	kind = ALIFUSTR_KIND(_uStr);
	data = ALIFUSTR_DATA(_uStr);

	size = 0;
	/* determine replacement size */
	for (i_ = _collStart; i_ < _collEnd; ++i_) {
		AlifSizeT incr{};

		ch_ = ALIFUSTR_READ(kind, data, i_);
		if (ch_ < 10)
			incr = 2 + 1 + 1;
		else if (ch_ < 100)
			incr = 2 + 2 + 1;
		else if (ch_ < 1000)
			incr = 2 + 3 + 1;
		else if (ch_ < 10000)
			incr = 2 + 4 + 1;
		else if (ch_ < 100000)
			incr = 2 + 5 + 1;
		else if (ch_ < 1000000)
			incr = 2 + 6 + 1;
		else {
			incr = 2 + 7 + 1;
		}
		if (size > ALIF_SIZET_MAX - incr) {
			//alifErr_setString(_alifExcOverflowError_,
			//	"encoded result is too long for a Alif string");
			return nullptr;
		}
		size += incr;
	}

	_str = (char*)alifBytesWriter_prepare(_writer, _str, size);
	if (_str == nullptr)
		return nullptr;

	/* generate replacement */
	for (i_ = _collStart; i_ < _collEnd; ++i_) {
		size = sprintf(_str, "&#%d;", ALIFUSTR_READ(kind, data, i_));
		if (size < 0) {
			return nullptr;
		}
		_str += size;
	}
	return _str;
}









// 976
#include "StringLib/ASCIILib.h"
#include "StringLib/FindMaxChar.h"
#include "StringLib/Undef.h"

#include "StringLib/UCS1Lib.h"
#include "StringLib/FindMaxChar.h"
#include "StringLib/Undef.h"

#include "StringLib/UCS2Lib.h"
#include "StringLib/FindMaxChar.h"
#include "StringLib/Undef.h"

#include "StringLib/UCS4Lib.h"
#include "StringLib/FindMaxChar.h"
#include "StringLib/Undef.h"


static AlifObject* resize_compact(AlifObject* _uStr, AlifSizeT _length) { // 1071
	AlifSizeT charSize{};
	AlifSizeT structSize{};
	AlifSizeT newSize{};
	AlifObject* newUStr{};

	charSize = ALIFUSTR_KIND(_uStr);
	if (ALIFUSTR_IS_ASCII(_uStr))
		structSize = sizeof(AlifASCIIObject);
	else
		structSize = sizeof(AlifCompactUStrObject);

	if (_length > ((ALIF_SIZET_MAX - structSize) / charSize - 1)) {
		//alifErr_noMemory();
		return nullptr;
	}
	newSize = (structSize + (_length + 1) * charSize);

	if (ALIFUSTR_HAS_UTF8_MEMORY(_uStr)) {
		alifMem_dataFree(ALIFUSTR_UTF8(_uStr));
		ALIFUSTR_UTF8(_uStr) = nullptr;
		ALIFUSTR_UTF8_LENGTH(_uStr) = 0;
	}

	newUStr = (AlifObject*)alifMem_objRealloc(_uStr, newSize);
	if (newUStr == nullptr) {
		alif_newReferenceNoTotal(_uStr);
		//alifErr_noMemory();
		return nullptr;
	}
	_uStr = newUStr;
	alif_newReferenceNoTotal(_uStr);

	ALIFUSTR_LENGTH(_uStr) = _length;
	ALIFUSTR_WRITE(ALIFUSTR_KIND(_uStr), ALIFUSTR_DATA(_uStr), _length, 0);
	return _uStr;
}


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
	if (_size > ((ALIF_SIZET_MAX - structSize) / charSize - 1)) {
		//return alifErr_noMemory();
		return nullptr;
	}
	obj = (AlifObject*)alifMem_objAlloc(structSize + (_size + 1) * charSize);
	if (obj == nullptr) {
		//return alifErr_noMemory();
		return nullptr;
	}

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
	ALIFUSTR_STATE(unicode).staticallyAllocated = 0;
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

static AlifIntT copy_characters(AlifObject* _to, AlifSizeT _toStart, AlifObject* _from,
	AlifSizeT _fromStart, AlifSizeT _howMany, AlifIntT _checkMaxChar) { // 1389
	AlifIntT fromKind{}, toKind{};
	const void* fromData{};
	void* toData{};

	if (_howMany == 0) return 0;

	fromKind = ALIFUSTR_KIND(_from);
	fromData = ALIFUSTR_DATA(_from);
	toKind = ALIFUSTR_KIND(_to);
	toData = ALIFUSTR_DATA(_to);

	if (fromKind == toKind) {
		if (_checkMaxChar
			and !ALIFUSTR_IS_ASCII(_from) && ALIFUSTR_IS_ASCII(_to)) {
			AlifUCS4 max_char;
			max_char = ucs1Lib_findMaxChar((AlifUCS1*)fromData, (const AlifUCS1*)fromData + _howMany);
			if (max_char >= 128) return -1;
		}
		memcpy((char*)toData + toKind * _toStart,
			(const char*)fromData + fromKind * _fromStart,
			toKind * _howMany);
	}
	else if (fromKind == AlifUStrKind_::AlifUStr_1Byte_Kind
		and toKind == AlifUStrKind_::AlifUStr_2Byte_Kind) {
		ALIFUSTR_CONVERT_BYTES(
			AlifUCS1, AlifUCS2,
			ALIFUSTR_1BYTE_DATA(_from) + _fromStart,
			ALIFUSTR_1BYTE_DATA(_from) + _fromStart + _howMany,
			ALIFUSTR_2BYTE_DATA(_to) + _toStart
		);
	}
	else if (fromKind == AlifUStrKind_::AlifUStr_1Byte_Kind
		and toKind == AlifUStrKind_::AlifUStr_4Byte_Kind) {
		ALIFUSTR_CONVERT_BYTES(
			AlifUCS1, AlifUCS4,
			ALIFUSTR_1BYTE_DATA(_from) + _fromStart,
			ALIFUSTR_1BYTE_DATA(_from) + _fromStart + _howMany,
			ALIFUSTR_4BYTE_DATA(_to) + _toStart
		);
	}
	else if (fromKind == AlifUStrKind_::AlifUStr_2Byte_Kind
		and toKind == AlifUStrKind_::AlifUStr_4Byte_Kind) {
		ALIFUSTR_CONVERT_BYTES(
			AlifUCS2, AlifUCS4,
			ALIFUSTR_2BYTE_DATA(_from) + _fromStart,
			ALIFUSTR_2BYTE_DATA(_from) + _fromStart + _howMany,
			ALIFUSTR_4BYTE_DATA(_to) + _toStart
		);
	}
	else {
		if (!_checkMaxChar) {
			if (fromKind == AlifUStrKind_::AlifUStr_2Byte_Kind
				and toKind == AlifUStrKind_::AlifUStr_1Byte_Kind)
			{
				ALIFUSTR_CONVERT_BYTES(
					AlifUCS2, AlifUCS1,
					ALIFUSTR_2BYTE_DATA(_from) + _fromStart,
					ALIFUSTR_2BYTE_DATA(_from) + _fromStart + _howMany,
					ALIFUSTR_1BYTE_DATA(_to) + _toStart
				);
			}
			else if (fromKind == AlifUStrKind_::AlifUStr_4Byte_Kind
				and toKind == AlifUStrKind_::AlifUStr_1Byte_Kind)
			{
				ALIFUSTR_CONVERT_BYTES(
					AlifUCS4, AlifUCS1,
					ALIFUSTR_4BYTE_DATA(_from) + _fromStart,
					ALIFUSTR_4BYTE_DATA(_from) + _fromStart + _howMany,
					ALIFUSTR_1BYTE_DATA(_to) + _toStart
				);
			}
			else if (fromKind == AlifUStrKind_::AlifUStr_4Byte_Kind
				and toKind == AlifUStrKind_::AlifUStr_2Byte_Kind) {
				ALIFUSTR_CONVERT_BYTES(
					AlifUCS4, AlifUCS2,
					ALIFUSTR_4BYTE_DATA(_from) + _fromStart,
					ALIFUSTR_4BYTE_DATA(_from) + _fromStart + _howMany,
					ALIFUSTR_2BYTE_DATA(_to) + _toStart
				);
			}
			else {
				ALIF_UNREACHABLE();
			}
		}
		else {
			const AlifUCS4 to_maxchar = ALIFUSTR_MAX_CHAR_VALUE(_to);
			AlifUCS4 ch_{};
			AlifSizeT i{};

			for (i = 0; i < _howMany; i++) {
				ch_ = ALIFUSTR_READ(fromKind, fromData, _fromStart + i);
				if (ch_ > to_maxchar) return -1;
				ALIFUSTR_WRITE(toKind, toData, _toStart + i, ch_);
			}
		}
	}
	return 0;
}

void alifUStr_fastCopyCharacters(AlifObject* _to, AlifSizeT _toStart,
	AlifObject* _from, AlifSizeT _fromStart, AlifSizeT _howMany) { // 1529
	(void)copy_characters(_to, _toStart, _from, _fromStart, _howMany, 0);
}

static void ustr_dealloc(AlifObject* _uStr) { // 1633
	if (ALIFUSTR_STATE(_uStr).staticallyAllocated) {
		alif_setImmortal(_uStr);
		return;
	}
	switch (ALIFUSTR_STATE(_uStr).interned) {
	case SSTATE_NOT_INTERNED:
		break;
	//case SSTATE_INTERNED_MORTAL:
	//	ALIF_SET_REFCNT(_uStr, 2);
	//	AlifInterpreter* interp = alifInterpreter_get();
	//	AlifObject* interned = get_internedDict(interp);
	//	AlifObject* popped{};
	//	AlifIntT r = alifDict_pop(interned, _uStr, &popped);
	//	if (r == -1) {
	//		//alifErr_writeUnraisable(unicode);
	//		alif_setImmortal(_uStr);
	//		ALIFUSTR_STATE(_uStr).interned = SSTATE_INTERNED_IMMORTAL;
	//		return;
	//	}
	//	if (r == 0) {
	//		alif_setImmortal(_uStr);
	//		return;
	//	}
	//	ALIF_SET_REFCNT(_uStr, 0);
	//	break;
	default:
		alif_setImmortal(_uStr);
		return;
	}
	if (ALIFUSTR_HAS_UTF8_MEMORY(_uStr)) {
		alifMem_objFree(ALIFUSTR_UTF8(_uStr));
	}
	if (!ALIFUSTR_IS_COMPACT(_uStr) and ALIFUSTR_DATA_ANY(_uStr)) {
		alifMem_objFree(ALIFUSTR_DATA_ANY(_uStr));
	}

	ALIF_TYPE(_uStr)->free(_uStr);
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



static AlifIntT uStr_fillUTF8(AlifObject*); // 4164

const char* alifUStr_asUTF8AndSize(AlifObject* _uStr, AlifSizeT* _pSize) { // 4167
	if (!ALIFUSTR_CHECK(_uStr)) {
		//alifErr_badArgument();
		if (_pSize) {
			*_pSize = -1;
		}
		return nullptr;
	}

	if (ALIFUSTR_UTF8(_uStr) == nullptr) {
		if (uStr_fillUTF8(_uStr) == -1) {
			if (_pSize) {
				*_pSize = -1;
			}
			return nullptr;
		}
	}

	if (_pSize) {
		*_pSize = ALIFUSTR_UTF8_LENGTH(_uStr);
	}
	return ALIFUSTR_UTF8(_uStr);
}

const char* alifUStr_asUTF8(AlifObject* _uStr) { // 4193
	return alifUStr_asUTF8AndSize(_uStr, nullptr);
}





//static AlifIntT unicodeDecode_callErrorHandlerWriter(
//	const char* errors, AlifObject** errorHandler,
//	const char* encoding, const char* reason,
//	const char** input, const char** inend, AlifSizeT* startinpos,
//	AlifSizeT* endinpos, AlifObject** exceptionObject, const char** inptr,
//	AlifUStrWriter* writer) { // 4436
//	static const char* argparse = "Un;decoding error handler must return (str, int) tuple";
//
//	AlifObject* restuple = nullptr;
//	AlifObject* repunicode = nullptr;
//	AlifSizeT insize{};
//	AlifSizeT newpos{};
//	AlifSizeT replen{};
//	AlifSizeT remain{};
//	AlifObject* inputobj = nullptr;
//	int need_to_grow = 0;
//	const char* new_inptr{};
//
//	if (*errorHandler == nullptr) {
//		*errorHandler = alifCodec_lookupError(errors);
//		if (*errorHandler == nullptr)
//			goto onError;
//	}
//
//	make_decodeException(exceptionObject,
//		encoding,
//		*input, *inend - *input,
//		*startinpos, *endinpos,
//		reason);
//	if (*exceptionObject == nullptr)
//		goto onError;
//
//	restuple = alifObject_callOneArg(*errorHandler, *exceptionObject);
//	if (restuple == nullptr)
//		goto onError;
//	if (!ALIFTUPLE_CHECK(restuple)) {
//		//alifErr_setString(alifExcTypeError, &argparse[3]);
//		goto onError;
//	}
//	if (!alifArg_parseTuple(restuple, argparse, &repunicode, &newpos))
//		goto onError;
//
//	inputobj = alifUStrDecodeError_getObject(*exceptionObject);
//	if (!inputobj)
//		goto onError;
//	remain = *inend - *input - *endinpos;
//	*input = ALIFBYTES_AS_STRING(inputobj);
//	insize = ALIFBYTES_GET_SIZE(inputobj);
//	*inend = *input + insize;
//
//	ALIF_DECREF(inputobj);
//
//	if (newpos < 0)
//		newpos = insize + newpos;
//	if (newpos<0 || newpos>insize) {
//		//alifErr_format(alifExcIndexError, "position %zd from error handler out of bounds", newpos);
//		goto onError;
//	}
//
//	replen = ALIFUSTR_GET_LENGTH(repunicode);
//	if (replen > 1) {
//		writer->minLength += replen - 1;
//		need_to_grow = 1;
//	}
//	new_inptr = *input + newpos;
//	if (*inend - new_inptr > remain) {
//		writer->minLength += *inend - new_inptr - remain;
//		need_to_grow = 1;
//	}
//	if (need_to_grow) {
//		writer->overAllocate = 1;
//		if (ALIFUSTRWRITER_PREPARE(writer, writer->minLength - writer->pos,
//			ALIFUSTR_MAX_CHAR_VALUE(repunicode)) == -1)
//			goto onError;
//	}
//	if (alifUStrWriter_writeStr(writer, repunicode) == -1)
//		goto onError;
//
//	*endinpos = newpos;
//	*inptr = new_inptr;
//
//	/* we made it! */
//	ALIF_DECREF(restuple);
//	return 0;
//
//onError:
//	ALIF_XDECREF(restuple);
//	return -1;
//}



/* ------------------------------------------ UTF-8 Codec ------------------------------------------ */

#include "StringLib/ASCIILib.h" // 4946
#include "StringLib/Codecs.h"
#include "StringLib/Undef.h"

#include "StringLib/UCS1Lib.h" // 4950
#include "StringLib/Codecs.h"
#include "StringLib/Undef.h"

#include "StringLib/UCS2Lib.h" // 4954
#include "StringLib/Codecs.h"
#include "StringLib/Undef.h"

#include "StringLib/UCS4Lib.h" // 4958
#include "StringLib/Codecs.h"
#include "StringLib/Undef.h"


// 4964
#if (SIZEOF_SIZE_T == 8)
# define ASCII_CHAR_MASK 0x8080808080808080ULL
#elif (SIZEOF_SIZE_T == 4)
# define ASCII_CHAR_MASK 0x80808080U
#else
# error CPP 'size_t' size should be either 4 or 8!
#endif


static AlifSizeT ascii_decode(const char* _start, const char* _end, AlifUCS1* _dest) { // 4972
	const char* p = _start;

#if SIZEOF_SIZE_T <= SIZEOF_VOID_P
	if (ALIF_IS_ALIGNED(p, ALIGNOF_SIZE_T)
		and ALIF_IS_ALIGNED(_dest, ALIGNOF_SIZE_T))
	{
		const char* _p = p;
		AlifUCS1* q = _dest;
		while (_p + SIZEOF_SIZE_T <= _end) {
			size_t value = *(const size_t*)_p;
			if (value & ASCII_CHAR_MASK)
				break;
			*((size_t*)q) = value;
			_p += SIZEOF_SIZE_T;
			q += SIZEOF_SIZE_T;
		}
		p = _p;
		while (p < _end) {
			if ((unsigned char)*p & 0x80)
				break;
			*q++ = *p++;
		}
		return p - _start;
	}
#endif
	while (p < _end) {
		if (ALIF_IS_ALIGNED(p, ALIGNOF_SIZE_T)) {
			const char* _p = p;
			while (_p + SIZEOF_SIZE_T <= _end) {
				size_t value = *(const size_t*)_p;
				if (value & ASCII_CHAR_MASK)
					break;
				_p += SIZEOF_SIZE_T;
			}
			p = _p;
			if (_p == _end)
				break;
		}
		if ((unsigned char)*p & 0x80)
			break;
		++p;
	}
	memcpy(_dest, _start, p - _start);
	return p - _start;
}

static AlifIntT unicode_decodeUTF8Impl(AlifUStrWriter* writer, const char* starts,
	const char* s, const char* end, AlifErrorHandler_ error_handler,
	const char* errors, AlifSizeT* consumed) { // 5028

	AlifSizeT startInPos{}, endInpos{};
	const char* errMsg = "";
	AlifObject* errorHandlerObj = nullptr;
	AlifObject* exc = nullptr;

	while (s < end) {
		AlifUCS4 ch_{};
		AlifIntT kind = writer->kind;

		if (kind == AlifUStrKind_::AlifUStr_1Byte_Kind) {
			if (ALIFUSTR_IS_ASCII(writer->buffer))
				ch_ = asciiLib_utf8Decode(&s, end, (AlifUCS1*)writer->data, &writer->pos);
			else
				ch_ = ucs1Lib_utf8Decode(&s, end, (AlifUCS1*)writer->data, &writer->pos);
		}
		else if (kind == AlifUStrKind_::AlifUStr_2Byte_Kind) {
			ch_ = ucs2Lib_utf8Decode(&s, end, (AlifUCS2*)writer->data, &writer->pos);
		}
		else {
			ch_ = ucs4Lib_utf8Decode(&s, end, (AlifUCS4*)writer->data, &writer->pos);
		}

		switch (ch_) {
		case 0:
			if (s == end or consumed)
				goto End;
			errMsg = "نهاية بيانات غير صحيحة";
			startInPos = s - starts;
			endInpos = end - starts;
			break;
		case 1:
			errMsg = "خطأ في بايت البداية";
			startInPos = s - starts;
			endInpos = startInPos + 1;
			break;
		case 2:
			if (consumed && (unsigned char)s[0] == 0xED && end - s == 2
				and (unsigned char)s[1] >= 0xA0 && (unsigned char)s[1] <= 0xBF)
			{
				/* Truncated surrogate code in range D800-DFFF */
				goto End;
			}
			//ALIF_FALLTHROUGH;
		case 3:
		case 4:
			errMsg = "امتداد بايت غير صحيح";
			startInPos = s - starts;
			endInpos = startInPos + ch_ - 1;
			break;
		default:
			if (alifUStrWriter_writeCharInline(writer, ch_) < 0)
				goto onError;
			continue;
		}

		if (error_handler == AlifErrorHandler_::Alif_Error_Unknown)
			error_handler = alif_getErrorHandler(errors);

		switch (error_handler) {
		case AlifErrorHandler_::Alif_Error_Ignore:
			s += (endInpos - startInPos);
			break;

		case AlifErrorHandler_::Alif_Error_Replace:
			if (alifUStrWriter_writeCharInline(writer, 0xfffd) < 0)
				goto onError;
			s += (endInpos - startInPos);
			break;

		case AlifErrorHandler_::Alif_Error_SurrogateEscape:
		{
			AlifSizeT i;

			if (ALIFUSTRWRITER_PREPAREKIND(writer, AlifUStrKind_::AlifUStr_2Byte_Kind) < 0)
				goto onError;
			for (i = startInPos; i < endInpos; i++) {
				ch_ = (AlifUCS4)(unsigned char)(starts[i]);
				alifUStr_write(writer->kind, writer->data, writer->pos, ch_ + 0xdc00);
				writer->pos++;
			}
			s += (endInpos - startInPos);
			break;
		}

		default:
			//if (unicodeDecode_callErrorHandlerWriter(
			//	errors, &errorHandlerObj,
			//	"utf-8", errMsg,
			//	&starts, &end, &startInPos, &endInpos, &exc, &s,
			//	writer)) {
			//	goto onError;
			//}

			if (ALIFUSTRWRITER_PREPARE(writer, end - s, 127) < 0) {
				return -1;
			}
		}
	}

End:
	if (consumed)
		*consumed = s - starts;

	ALIF_XDECREF(errorHandlerObj);
	ALIF_XDECREF(exc);
	return 0;

onError:
	ALIF_XDECREF(errorHandlerObj);
	ALIF_XDECREF(exc);
	return -1;
}

static AlifObject* unicode_decodeUTF8(const char* _str, AlifSizeT _size,
	AlifErrorHandler_ _errorHandler, const char* _errors, AlifSizeT* _consumed) { // 5151
	if (_size == 0) {
		if (_consumed) {
			*_consumed = 0;
		}
		ALIF_RETURN_UNICODE_EMPTY;
	}

	if (_size == 1 and (unsigned char)_str[0] < 128) {
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
	const char* e_{};
	wchar_t* uStr{};
	AlifSizeT outPos{};

	AlifIntT surrogateescape = 0;
	AlifIntT surrogatepass = 0;
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

	if (ALIF_SIZET_MAX / (AlifSizeT)sizeof(wchar_t) - 1 < size) {
		return -1;
	}

	uStr = (wchar_t*)alifMem_dataAlloc((size + 1) * sizeof(wchar_t));
	if (!uStr) {
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
		ch_ = ucs2Lib_utf8Decode(&s, e_, (AlifUCS2*)uStr, &outPos);
#endif
		if (ch_ > 0xFF) {
#if SIZEOF_WCHAR_T == 4
			ALIF_UNREACHABLE();
#else
			/* write a surrogate pair */
			uStr[outPos++] = (wchar_t)alifUnicode_highSurrogate(ch_);
			uStr[outPos++] = (wchar_t)alifUnicode_lowSurrogate(ch_);
#endif
		}
		else {
			if (!ch_ && s == e_) {
				break;
			}

			if (surrogateescape) {
				uStr[outPos++] = 0xDC00 + (unsigned char)*s++;
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
					uStr[outPos++] = ch_;
				}
				else {
					alifMem_dataFree(uStr);
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
	uStr[outPos] = L'\0';
	if (wlen) {
		*wlen = outPos;
	}
	*wstr = uStr;
	return 0;
}



static AlifIntT uStr_fillUTF8(AlifObject* _uStr) { // 5582

	AlifIntT kind = ALIFUSTR_KIND(_uStr);
	const void* data = ALIFUSTR_DATA(_uStr);
	AlifSizeT size = ALIFUSTR_GET_LENGTH(_uStr);

	AlifBytesWriter writer{};
	char* end{};

	switch (kind) {
	default:
		ALIF_UNREACHABLE();
	case AlifUStrKind_::AlifUStr_1Byte_Kind:
		end = ucs1Lib_utf8Encoder(&writer, _uStr, (AlifUCS1*)data, size,
			AlifErrorHandler_::Alif_Error_Strict, nullptr);
		break;
	case AlifUStrKind_::AlifUStr_2Byte_Kind:
		end = ucs2Lib_utf8Encoder(&writer, _uStr, (AlifUCS2*)data, size,
			AlifErrorHandler_::Alif_Error_Strict, nullptr);
		break;
	case AlifUStrKind_::AlifUStr_4Byte_Kind:
		end = ucs4Lib_utf8Encoder(&writer, _uStr, (AlifUCS4*)data, size,
			AlifErrorHandler_::Alif_Error_Strict, nullptr);
		break;
	}
	if (end == nullptr) {
		alifBytesWriter_dealloc(&writer);
		return -1;
	}

	const char* start = writer.useSmallBuffer ? writer.smallBuffer :
		ALIFBYTES_AS_STRING(writer.buffer);
	AlifSizeT len = end - start;

	char* cache = (char*)alifMem_objAlloc(len + 1);
	if (cache == nullptr) {
		alifBytesWriter_dealloc(&writer);
		//alifErr_noMemory();
		return -1;
	}
	ALIFUSTR_UTF8(_uStr) = cache;
	ALIFUSTR_UTF8_LENGTH(_uStr) = len;
	memcpy(cache, start, len);
	cache[len] = '\0';
	alifBytesWriter_dealloc(&writer);
	return 0;
}










































static inline void alifUStrWriter_update(AlifUStrWriter* _writer) { // 13364
	_writer->maxChar = ALIFUSTR_MAX_CHAR_VALUE(_writer->buffer);
	_writer->data = ALIFUSTR_DATA(_writer->buffer);

	if (!_writer->readOnly) {
		_writer->kind = ALIFUSTR_KIND(_writer->buffer);
		_writer->size = ALIFUSTR_GET_LENGTH(_writer->buffer);
	}
	else {
		_writer->kind = 0;
		_writer->size = 0;
	}
}




static inline void alifUStrWriter_initWithBuffer(AlifUStrWriter* _writer,
	AlifObject* _buffer) { // 13439
	memset(_writer, 0, sizeof(*_writer));
	_writer->buffer = _buffer;
	alifUStrWriter_update(_writer);
	_writer->minLength = _writer->size;
}

AlifIntT alifUStrWriter_prepareInternal(AlifUStrWriter* _writer,
	AlifSizeT _length, AlifUCS4 _maxChar) { // 13448

	AlifSizeT newlen{};
	AlifObject* newbuffer{};

	if (_length > ALIF_SIZET_MAX - _writer->pos) {
		//alifErr_noMemory();
		return -1;
	}
	newlen = _writer->pos + _length;

	_maxChar = ALIF_MAX(_maxChar, _writer->minChar);

	if (_writer->buffer == nullptr) {
		if (_writer->overAllocate
			and newlen <= (ALIF_SIZET_MAX - newlen / OVERALLOCATE_FACTOR)) {
			newlen += newlen / OVERALLOCATE_FACTOR;
		}
		if (newlen < _writer->minLength)
			newlen = _writer->minLength;

		_writer->buffer = alifUStr_new(newlen, _maxChar);
		if (_writer->buffer == nullptr)
			return -1;
	}
	else if (newlen > _writer->size) {
		if (_writer->overAllocate
			&& newlen <= (ALIF_SIZET_MAX - newlen / OVERALLOCATE_FACTOR)) {
			newlen += newlen / OVERALLOCATE_FACTOR;
		}
		if (newlen < _writer->minLength)
			newlen = _writer->minLength;

		if (_maxChar > _writer->maxChar or _writer->readOnly) {
			/* resize + widen */
			_maxChar = ALIF_MAX(_maxChar, _writer->maxChar);
			newbuffer = alifUStr_new(newlen, _maxChar);
			if (newbuffer == nullptr)
				return -1;
			alifUStr_fastCopyCharacters(newbuffer, 0,
				_writer->buffer, 0, _writer->pos);
			ALIF_DECREF(_writer->buffer);
			_writer->readOnly = 0;
		}
		else {
			newbuffer = resize_compact(_writer->buffer, newlen);
			if (newbuffer == nullptr)
				return -1;
		}
		_writer->buffer = newbuffer;
	}
	else if (_maxChar > _writer->maxChar) {
		newbuffer = alifUStr_new(_writer->size, _maxChar);
		if (newbuffer == nullptr) return -1;

		alifUStr_fastCopyCharacters(newbuffer, 0, _writer->buffer, 0, _writer->pos);
		ALIF_SETREF(_writer->buffer, newbuffer);
	}
	alifUStrWriter_update(_writer);
	return 0;

#undef OVERALLOCATE_FACTOR
}

AlifIntT alifUStrWriter_prepareKindInternal(AlifUStrWriter* _writer, AlifIntT _kind) { // 13526
	AlifUCS4 maxChar{};

	switch (_kind)
	{
	case AlifUStrKind_::AlifUStr_1Byte_Kind: maxChar = 0xff; break;
	case AlifUStrKind_::AlifUStr_2Byte_Kind: maxChar = 0xffff; break;
	case AlifUStrKind_::AlifUStr_4Byte_Kind: maxChar = MAX_UNICODE; break;
	default:
		ALIF_UNREACHABLE();
	}

	return alifUStrWriter_prepareInternal(_writer, 0, maxChar);
}

static inline AlifIntT alifUStrWriter_writeCharInline(AlifUStrWriter* _writer, AlifUCS4 _ch) { // 13547
	if (ALIFUSTRWRITER_PREPARE(_writer, 1, _ch) < 0)
		return -1;
	ALIFUSTR_WRITE(_writer->kind, _writer->data, _writer->pos, _ch);
	_writer->pos++;
	return 0;
}






AlifObject* alifUStrWriter_finish(AlifUStrWriter* _writer) { // 13809
	AlifObject* str{};

	if (_writer->pos == 0) {
		ALIF_CLEAR(_writer->buffer);
		ALIF_RETURN_UNICODE_EMPTY;
	}

	str = _writer->buffer;
	_writer->buffer = nullptr;

	if (_writer->readOnly) {
		return str;
	}

	if (ALIFUSTR_GET_LENGTH(str) != _writer->pos) {
		AlifObject* str2;
		str2 = resize_compact(str, _writer->pos);
		if (str2 == nullptr) {
			ALIF_DECREF(str);
			return nullptr;
		}
		str = str2;
	}

	return ustr_result(str);
}



void alifUStrWriter_dealloc(AlifUStrWriter* _writer) { // 13852
	ALIF_CLEAR(_writer->buffer);
}












AlifTypeObject _alifUStrType_ = { // 15235
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "نص",
	.basicSize = sizeof(AlifUStrObject),
	.itemSize = 0,
	.dealloc = ustr_dealloc,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
		ALIF_TPFLAGS_UNICODE_SUBCLASS |
		_ALIF_TPFLAGS_MATCH_SELF,
	.base = 0,
	.free = alifMem_objFree,
};




static void immortalize_interned(AlifObject* _s) { // 15405

	ALIFUSTR_STATE(_s).interned = SSTATE_INTERNED_IMMORTAL;
	alif_setImmortal(_s);
}


static AlifObject* intern_common(AlifInterpreter* _interp,
	AlifObject* _s, bool _immortalize) { // 15423

	if (_s == nullptr or !ALIFUSTR_CHECK(_s)) {
		return _s;
	}

	if (!ALIFUSTR_CHECKEXACT(_s)) {
		return _s;
	}

	switch (ALIFUSTR_CHECK_INTERNED(_s)) {
	case SSTATE_NOT_INTERNED:
		break;
	case SSTATE_INTERNED_MORTAL:
		if (_immortalize) {
			immortalize_interned(_s);
		}
		return _s;
	default:
		return _s;
	}

#if ALIF_GIL_DISABLED
	_immortalize = 1;
#endif

	if (alif_isImmortal(_s)) {
		_immortalize = 1;
	}

	if (ALIFUSTR_GET_LENGTH(_s) == 1 and
		ALIFUSTR_KIND(_s) == AlifUStrKind_::AlifUStr_1Byte_Kind) {
		AlifObject* r = LATIN1(*(unsigned char*)ALIFUSTR_DATA(_s));
		ALIF_DECREF(_s);
		return r;
	}

	{
		AlifObject* r = (AlifObject*)alifHashTable_get(INTERNED_STRINGS, _s);
		if (r != nullptr) {
			ALIF_DECREF(_s);
			return ALIF_NEWREF(r);
		}
	}

	AlifObject* interned = get_internedDict(_interp);

	AlifObject* t{};
	{
		AlifIntT res = alifDict_setDefaultRef(interned, _s, _s, &t);
		if (res < 0) {
			//alifErr_clear();
			return _s;
		}
		else if (res == 1) {
			ALIF_DECREF(_s);
			if (_immortalize and
				ALIFUSTR_CHECK_INTERNED(t) == SSTATE_INTERNED_MORTAL) {
				immortalize_interned(t);
			}
			return t;
		}
		else {
			ALIF_DECREF(t);
		}
	}
	if (!ALIF_ISIMMORTAL(_s)) {
		ALIF_SET_REFCNT(_s, ALIF_REFCNT(_s) - 2);
	}
	ALIFUSTR_STATE(_s).interned = SSTATE_INTERNED_MORTAL;


	if (_immortalize) {
		immortalize_interned(_s);
	}

	return _s;
}


void alifUStr_internMortal(AlifInterpreter* _interp, AlifObject** _p) { // 15561
	*_p = intern_common(_interp, *_p, 0);
}

AlifObject* alifUStr_internFromString(const char* _cp) { // 15592
	AlifObject* s = alifUStr_fromString(_cp);
	if (s == nullptr) {
		return nullptr;
	}
	AlifInterpreter* interp = alifInterpreter_get();
	alifUStr_internMortal(interp, &s);
	return s;
}
