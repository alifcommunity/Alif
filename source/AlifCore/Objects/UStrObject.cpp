#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Codecs.h"
#include "AlifCore_Object.h"
#include "AlifCore_State.h"
#include "AlifCore_UStrObject.h"
#include "AlifCore_BytesObject.h"
#include "AlifCore_CriticalSection.h"
#include "AlifCore_Format.h"


#include <Equal.h> // 61


// Forward Declaration
static inline AlifIntT alifUStrWriter_writeCharInline(AlifUStrWriter*, AlifUCS4);
static inline void alifUStrWriter_initWithBuffer(AlifUStrWriter*, AlifObject*);

#define MAX_UNICODE 0x10ffff // 106


// 114
#define _ALIFUSTR_UTF8(_op)                             \
    (ALIFCOMPACTUSTROBJECT_CAST(_op)->utf8)
#define ALIFUSTR_UTF8(_op) (                 \
     ALIFUSTR_IS_COMPACT_ASCII(_op) ?                   \
         ((char*)(ALIFASCIIOBJECT_CAST(_op) + 1)) :       \
         _ALIFUSTR_UTF8(_op))

#define _ALIFUSTR_UTF8_LENGTH(_op)                      \
    (ALIFCOMPACTUSTROBJECT_CAST(_op)->utf8Length)
#define ALIFUSTR_UTF8_LENGTH(_op) ALIFUSTR_IS_COMPACT_ASCII(_op) ?                   \
         ALIFASCIIOBJECT_CAST(_op)->length :              \
         _ALIFUSTR_UTF8_LENGTH(_op)

// 129
#define ALIFUSTR_LENGTH(_op)                           \
    (ALIFASCIIOBJECT_CAST(_op)->length)
#define ALIFUSTR_STATE(_op)                            \
    (ALIFASCIIOBJECT_CAST(_op)->state)
#define ALIFUSTR_HASH(_op)                             \
    (ALIFASCIIOBJECT_CAST(_op)->hash)					
#define ALIFUSTR_DATA_ANY(_op)                         \
    (ALIFUSTROBJECT_CAST(_op)->data.any)

// 144
#define ALIFUSTR_SHARE_UTF8(_op)                       \
     (ALIFUSTR_UTF8(_op) == ALIFUSTR_DATA(_op))        


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
   /* On Windows, overAllocate by 50% is the best factor */
#  define OVERALLOCATE_FACTOR 2
#else
   /* On Linux, overAllocate by 25% is the best factor */
#  define OVERALLOCATE_FACTOR 4
#endif


static AlifIntT uStr_decodeUTF8Writer(AlifUStrWriter*, const char*, AlifSizeT,
	AlifErrorHandler_, const char*, AlifSizeT*); // 203

static inline AlifObject* unicode_getEmpty() { // 214
	ALIF_DECLARE_STR(Empty, "");
	return &ALIF_STR(Empty);
}


static inline AlifObject* get_internedDict(AlifInterpreter* _interp) { // 223
	return ALIF_INTERP_CACHED_OBJECT(_interp, internedStrings);
}


#define INTERNED_STRINGS _alifDureRun_.cachedObjects.internedStrings // 231

static AlifHashT uStr_hash(AlifObject*); // 263
static AlifIntT uStr_compareEq(AlifObject*, AlifObject*);

static AlifUHashT hashTable_uStrHash(const void* _key) { // 266
	return uStr_hash((AlifObject*)_key);
}


static AlifIntT hashTable_uStrCompare(const void* _key1, const void* _key2) { // 272
	AlifObject* obj1 = (AlifObject*)_key1;
	AlifObject* obj2 = (AlifObject*)_key2;
	if (obj1 != nullptr and obj2 != nullptr) {
		return uStr_compareEq(obj1, obj2);
	}
	else {
		return obj1 == obj2;
	}
}



static bool hasShared_internDict(AlifInterpreter* _interp) { // 295
	AlifInterpreter* mainInterp = alifInterpreter_main();
	return _interp != mainInterp and _interp->featureFlags & ALIF_RTFLAGS_USE_ALIFMEM;
}

static AlifIntT init_internedDict(AlifInterpreter* interp) { // 302
	AlifObject* interned{};
	if (hasShared_internDict(interp)) {
		interned = get_internedDict(alifInterpreter_main());
		ALIF_INCREF(interned);
	}
	else {
		interned = alifDict_new();
		if (interned == nullptr) {
			return -1;
		}
	}
	ALIF_INTERP_CACHED_OBJECT(interp, internedStrings) = interned;
	return 0;
}


static AlifIntT initGlobal_internedStrings(AlifInterpreter* _interp) { // 308
	AlifHashTableAllocatorT hashTableAlloc = { alifMem_dataAlloc, alifMem_dataFree };

	INTERNED_STRINGS = alifHashTable_newFull(
		hashTable_uStrHash,
		hashTable_uStrCompare,
		nullptr,
		nullptr,
		&hashTableAlloc );
	if (INTERNED_STRINGS == nullptr) {
		//alifErr_clear();
		//return ALIFSTATUS_ERR("failed to create global interned dict");
		return -1;
	}

	//alifUStr_initStaticStrings(_interp);

	for (AlifIntT i = 0; i < 256; i++) {
		AlifObject* s = LATIN1(i);
		//alifUnicode_internStatic(_interp, &s);
	}
	return 1;
}


// 360
#define ALIF_RETURN_UNICODE_EMPTY return unicode_getEmpty();



static inline void uStr_fill(AlifIntT kind, void* data, AlifUCS4 value,
	AlifSizeT start, AlifSizeT length) { // 365
	switch (kind) {
	case AlifUStrKind_::AlifUStr_1Byte_Kind: {
		AlifUCS1 ch = (unsigned char)value;
		AlifUCS1* to = (AlifUCS1*)data + start;
		memset(to, ch, length);
		break;
	}
	case AlifUStrKind_::AlifUStr_2Byte_Kind: {
		AlifUCS2 ch = (AlifUCS2)value;
		AlifUCS2* to = (AlifUCS2*)data + start;
		const AlifUCS2* end = to + length;
		for (; to < end; ++to) *to = ch;
		break;
	}
	case AlifUStrKind_::AlifUStr_4Byte_Kind: {
		AlifUCS4 ch = value;
		AlifUCS4* to = (AlifUCS4*)data + start;
		const AlifUCS4* end = to + length;
		for (; to < end; ++to) *to = ch;
		break;
	}
	default: ALIF_UNREACHABLE();
	}
}




const unsigned char _alifASCIIWhitespace_[] = { // 400
	0, 0, 0, 0, 0, 0, 0, 0,
	/*     case 0x0009: * CHARACTER TABULATION */
	/*     case 0x000A: * LINE FEED */
	/*     case 0x000B: * LINE TABULATION */
	/*     case 0x000C: * FORM FEED */
	/*     case 0x000D: * CARRIAGE RETURN */
	0, 1, 1, 1, 1, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	/*     case 0x001C: * FILE SEPARATOR */
	/*     case 0x001D: * GROUP SEPARATOR */
	/*     case 0x001E: * RECORD SEPARATOR */
	/*     case 0x001F: * UNIT SEPARATOR */
	0, 0, 0, 0, 1, 1, 1, 1,
	/*     case 0x0020: * SPACE */
	1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};




static AlifIntT uStr_modifiable(AlifObject*); // 432

static AlifObject* uStr_encodeCallErrorhandler(const char*,
	AlifObject**, const char*, const char*, AlifObject*,
	AlifObject**, AlifSizeT, AlifSizeT, AlifSizeT*); // 443

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


static AlifErrorHandler_ get_errorHandlerWide(const wchar_t* _errors) { // 516
	if (_errors == nullptr or wcscmp(_errors, L"strict") == 0) {
		return AlifErrorHandler_::Alif_Error_Strict;
	}
	if (wcscmp(_errors, L"surrogateescape") == 0) {
		return AlifErrorHandler_::Alif_Error_SurrogateEscape;
	}
	if (wcscmp(_errors, L"replace") == 0) {
		return AlifErrorHandler_::Alif_Error_Replace;
	}
	if (wcscmp(_errors, L"ignore") == 0) {
		return AlifErrorHandler_::Alif_Error_Ignore;
	}
	if (wcscmp(_errors, L"backslashreplace") == 0) {
		return AlifErrorHandler_::Alif_Error_BackSlashReplace;
	}
	if (wcscmp(_errors, L"surrogatepass") == 0) {
		return AlifErrorHandler_::Alif_Error_SurrogatePass;
	}
	if (wcscmp(_errors, L"xmlcharrefreplace") == 0) {
		return AlifErrorHandler_::Alif_Error_XMLCharRefReplace;
	}
	return AlifErrorHandler_::Alif_Error_Other;
}


static AlifObject* uStr_result(AlifObject* _uStr) { // 727
	AlifSizeT length = ALIFUSTR_GET_LENGTH(_uStr);
	if (length == 0) {
		AlifObject* empty = unicode_getEmpty();
		if (_uStr != empty) {
			ALIF_DECREF(_uStr);
		}
		return empty;
	}

	if (length == 1) {
		AlifIntT kind = ALIFUSTR_KIND(_uStr);
		if (kind == AlifUStrKind_::AlifUStr_1Byte_Kind) {
			const AlifUCS1* data = ALIFUSTR_1BYTE_DATA(_uStr);
			AlifUCS1 ch_ = data[0];
			AlifObject* latin1_char = LATIN1(ch_);
			if (_uStr != latin1_char) {
				ALIF_DECREF(_uStr);
			}
			return latin1_char;
		}
	}

	return _uStr;
}

static AlifObject* uStr_resultUnchanged(AlifObject* _uStr) { // 758
	if (ALIFUSTR_CHECKEXACT(_uStr)) {
		return ALIF_NEWREF(_uStr);
	}
	else {
		/* Subtype -- return genuine unicode string with the same value. */
		return _alifUStr_copy(_uStr);
	}
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
			*_str++ = _alifHexDigits_[(ch_ >> 28) & 0xf];
			*_str++ = _alifHexDigits_[(ch_ >> 24) & 0xf];
			*_str++ = _alifHexDigits_[(ch_ >> 20) & 0xf];
			*_str++ = _alifHexDigits_[(ch_ >> 16) & 0xf];
			*_str++ = _alifHexDigits_[(ch_ >> 12) & 0xf];
			*_str++ = _alifHexDigits_[(ch_ >> 8) & 0xf];
		}
		else if (ch_ >= 0x100) {
			*_str++ = 'u';
			*_str++ = _alifHexDigits_[(ch_ >> 12) & 0xf];
			*_str++ = _alifHexDigits_[(ch_ >> 8) & 0xf];
		}
		else
			*_str++ = 'x';
		*_str++ = _alifHexDigits_[(ch_ >> 4) & 0xf];
		*_str++ = _alifHexDigits_[ch_ & 0xf];
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





static AlifIntT ensure_uStr(AlifObject* _obj) { // 960
	if (!ALIFUSTR_CHECK(_obj)) {
		//alifErr_format(_alifExcTypeError_,
		//	"must be str, not %.100s",
		//	ALIF_TYPE(_obj)->name);
		return -1;
	}
	return 0;
}



// 976
#include "StringLib/ASCIILib.h"
#include "StringLib/FastSearch.h"
#include "StringLib/Count.h"
#include "StringLib/Find.h"
#include "StringLib/FindMaxChar.h"
#include "StringLib/Undef.h"

#include "StringLib/UCS1Lib.h"
#include "StringLib/FastSearch.h"
#include "StringLib/Count.h"
#include "StringLib/Find.h"
#include "StringLib/Replace.h"
#include "StringLib/Repr.h"
#include "StringLib/FindMaxChar.h"
#include "StringLib/Undef.h"

#include "StringLib/UCS2Lib.h"
#include "StringLib/FastSearch.h"
#include "StringLib/Count.h"
#include "StringLib/Find.h"
#include "StringLib/Replace.h"
#include "StringLib/Repr.h"
#include "StringLib/FindMaxChar.h"
#include "StringLib/Undef.h"

#include "StringLib/UCS4Lib.h"
#include "StringLib/FastSearch.h"
#include "StringLib/Count.h"
#include "StringLib/Find.h"
#include "StringLib/Replace.h"
#include "StringLib/Repr.h"
#include "StringLib/FindMaxChar.h"
#include "StringLib/Undef.h"


static inline AlifSizeT findChar(const void* _s, AlifIntT _kind,
	AlifSizeT _size, AlifUCS4 _ch, AlifIntT _direction) { // 1023
	switch (_kind) {
	case AlifUStrKind_::AlifUStr_1Byte_Kind:
		if ((AlifUCS1)_ch != _ch)
			return -1;
		if (_direction > 0)
			return ucs1Lib_findChar((const AlifUCS1*)_s, _size, (AlifUCS1)_ch);
		else
			return ucs1Lib_rFindChar((const AlifUCS1*)_s, _size, (AlifUCS1)_ch);
	case AlifUStrKind_::AlifUStr_2Byte_Kind:
		if ((AlifUCS2)_ch != _ch)
			return -1;
		if (_direction > 0)
			return ucs2Lib_findChar((const AlifUCS2*)_s, _size, (AlifUCS2)_ch);
		else
			return ucs2Lib_rFindChar((const AlifUCS2*)_s, _size, (AlifUCS2)_ch);
	case AlifUStrKind_::AlifUStr_4Byte_Kind:
		if (_direction > 0)
			return ucs4Lib_findChar((const AlifUCS4*)_s, _size, _ch);
		else
			return ucs4Lib_rFindChar((const AlifUCS4*)_s, _size, _ch);
	default:
		ALIF_UNREACHABLE();
	}
}

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
		_ALIFUSTR_UTF8(_uStr) = nullptr;
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

static AlifIntT resize_inplace(AlifObject* _uStr, AlifSizeT _length) { // 1126
	AlifSizeT newSize{};
	AlifSizeT charSize{};
	AlifIntT shareUTF8{};
	void* data{};

	data = ALIFUSTR_DATA_ANY(_uStr);
	charSize = ALIFUSTR_KIND(_uStr);
	shareUTF8 = ALIFUSTR_SHARE_UTF8(_uStr);

	if (_length > (ALIF_SIZET_MAX / charSize - 1)) {
		//alifErr_noMemory();
		return -1;
	}
	newSize = (_length + 1) * charSize;

	if (!shareUTF8 and ALIFUSTR_HAS_UTF8_MEMORY(_uStr))
	{
		alifMem_objFree(_ALIFUSTR_UTF8(_uStr));
		_ALIFUSTR_UTF8(_uStr) = nullptr;
		ALIFUSTR_UTF8_LENGTH(_uStr) = 0;
	}

	data = (AlifObject*)alifMem_objRealloc(data, newSize);
	if (data == nullptr) {
		//alifErr_noMemory();
		return -1;
	}
	ALIFUSTR_DATA_ANY(_uStr) = data;
	if (shareUTF8) {
		_ALIFUSTR_UTF8(_uStr) = (char*)data;
		ALIFUSTR_UTF8_LENGTH(_uStr) = _length;
	}
	ALIFUSTR_LENGTH(_uStr) = _length;
	ALIFUSTR_WRITE(ALIFUSTR_KIND(_uStr), data, _length, 0);

	if (_length > ALIF_SIZET_MAX / (AlifSizeT)sizeof(wchar_t) - 1) {
		//alifErr_noMemory();
		return -1;
	}
	return 0;
}

static AlifObject* resize_copy(AlifObject* _uStr, AlifSizeT _length) { // 1182
	AlifSizeT copyLength{};
	AlifObject* copy{};

	copy = alifUStr_new(_length, ALIFUSTR_MAX_CHAR_VALUE(_uStr));
	if (copy == nullptr) return nullptr;

	copyLength = ALIF_MIN(_length, ALIFUSTR_GET_LENGTH(_uStr));
	alifUStr_fastCopyCharacters(copy, 0, _uStr, 0, copyLength);
	return copy;
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

static AlifIntT uStr_checkModifiable(AlifObject* _unicode) { // 1378
	if (!uStr_modifiable(_unicode)) {
		//alifErr_setString(_alifExcSystemError_,
		//	"Cannot modify a string currently used");
		return -1;
	}
	return 0;
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
			and !ALIFUSTR_IS_ASCII(_from) and ALIFUSTR_IS_ASCII(_to)) {
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

AlifSizeT alifUStr_copyCharacters(AlifObject* _to, AlifSizeT _toStart,
	AlifObject* _from, AlifSizeT _fromStart, AlifSizeT _howMany) { // 1537
	AlifIntT err{};

	if (!ALIFUSTR_CHECK(_from) or !ALIFUSTR_CHECK(_to)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}

	if ((AlifUSizeT)_fromStart > (AlifUSizeT)ALIFUSTR_GET_LENGTH(_from)) {
		//alifErr_setString(_alifExcIndexError_, "string index out of range");
		return -1;
	}
	if ((AlifUSizeT)_toStart > (AlifUSizeT)ALIFUSTR_GET_LENGTH(_to)) {
		//alifErr_setString(_alifExcIndexError_, "string index out of range");
		return -1;
	}
	if (_howMany < 0) {
		//alifErr_setString(_alifExcSystemError_, "how_many cannot be negative");
		return -1;
	}
	_howMany = ALIF_MIN(ALIFUSTR_GET_LENGTH(_from) - _fromStart, _howMany);
	if (_toStart + _howMany > ALIFUSTR_GET_LENGTH(_to)) {
		//alifErr_format(_alifExcSystemError_,
		//	"Cannot write %zi characters at %zi "
		//	"in a string of %zi characters",
		//	_howMany, _toStart, ALIFUSTR_GET_LENGTH(_to));
		return -1;
	}

	if (_howMany == 0)
		return 0;

	if (uStr_checkModifiable(_to))
		return -1;

	err = copy_characters(_to, _toStart, _from, _fromStart, _howMany, 1);
	if (err) {
		//alifErr_format(_alifExcSystemError_,
		//	"Cannot copy %s characters "
		//	"into a string of %s characters",
		//	unicode_kindName(_from),
		//	unicode_kindName(_to));
		return -1;
	}
	return _howMany;
}


static AlifIntT find_maxCharSurrogates(const wchar_t* _begin, const wchar_t* _end,
	AlifUCS4* _maxChar, AlifSizeT* _numSurrogates) { // 1593

	const wchar_t* iter{};
	AlifUCS4 ch_{};

	*_numSurrogates = 0;
	*_maxChar = 0;

	for (iter = _begin; iter < _end; ) {
#if SIZEOF_WCHAR_T == 2
		if (alifUnicode_isHighSurrogate(iter[0])
			and (iter + 1) < _end
			and alifUnicode_isLowSurrogate(iter[1]))
		{
			ch_ = alifUnicode_joinSurrogates(iter[0], iter[1]);
			++(*_numSurrogates);
			iter += 2;
		}
		else
#endif
		{
			ch_ = *iter;
			iter++;
		}
		if (ch_ > *_maxChar) {
			*_maxChar = ch_;
			if (*_maxChar > MAX_UNICODE) {
				//alifErr_format(_alifExcValueError_,
				//	"character U+%x is not in range [U+0000; U+%x]",
				//	ch, MAX_UNICODE);
				return -1;
			}
		}
	}
	return 0;
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
	//	AlifInterpreter* interp = _alifInterpreter_get();
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

static AlifIntT uStr_modifiable(AlifObject* _unicode) { // 1738
	if (ALIF_REFCNT(_unicode) != 1)
		return 0;
	if (alifAtomic_loadSizeRelaxed(&ALIFUSTR_HASH(_unicode)) != -1)
		return 0;
	if (ALIFUSTR_CHECK_INTERNED(_unicode))
		return 0;
	if (!ALIFUSTR_CHECKEXACT(_unicode))
		return 0;
	return 1;
}

static AlifIntT uStr_resize(AlifObject** _pUStr, AlifSizeT _length) { // 1758
	AlifObject* uStr{};
	AlifSizeT oldLength{};

	uStr = *_pUStr;

	oldLength = ALIFUSTR_GET_LENGTH(uStr);
	if (oldLength == _length)
		return 0;

	if (_length == 0) {
		AlifObject* empty = unicode_getEmpty();
		ALIF_SETREF(*_pUStr, empty);
		return 0;
	}

	if (!uStr_modifiable(uStr)) {
		AlifObject* copy = resize_copy(uStr, _length);
		if (copy == nullptr)
			return -1;
		ALIF_SETREF(*_pUStr, copy);
		return 0;
	}

	if (ALIFUSTR_IS_COMPACT(uStr)) {
		AlifObject* newUStr = resize_compact(uStr, _length);
		if (newUStr == nullptr)
			return -1;
		*_pUStr = newUStr;
		return 0;
	}
	return resize_inplace(uStr, _length);
}


static AlifObject* get_latin1Char(AlifUCS1 _ch) { // 1867
	AlifObject* obj = LATIN1(_ch);
	return obj;
}

static AlifObject* uStr_char(AlifUCS4 _ch) { // 1874
	AlifObject* unicode{};

	if (_ch < 256) {
		return get_latin1Char(_ch);
	}

	unicode = alifUStr_new(1, _ch);
	if (unicode == nullptr) return nullptr;

	if (ALIFUSTR_KIND(unicode) == AlifUStrKind_::AlifUStr_2Byte_Kind) {
		ALIFUSTR_2BYTE_DATA(unicode)[0] = (AlifUCS2)_ch;
	}
	else {
		ALIFUSTR_4BYTE_DATA(unicode)[0] = _ch;
	}
	return unicode;
}

static inline void uStrWrite_wideChar(AlifIntT _kind, void* _data,
	const wchar_t* _u, AlifSizeT _size, AlifSizeT _numSurrogates) { // 1901
	switch (_kind) {
	case AlifUStrKind_::AlifUStr_1Byte_Kind:
		ALIFUSTR_CONVERT_BYTES(wchar_t, unsigned char, _u, _u + _size, _data);
		break;

	case AlifUStrKind_::AlifUStr_2Byte_Kind:
#if SIZEOF_WCHAR_T == 2
		memcpy(_data, _u, _size * 2);
#else
		ALIFUSTR_CONVERT_BYTES(wchar_t, AlifUCS2, _u, _u + _size, _data);
#endif
		break;

	case AlifUStrKind_::AlifUStr_4Byte_Kind:
	{
#if SIZEOF_WCHAR_T == 2
		// Convert a 16-bits wchar_t representation to UCS4, this will decode
		// surrogate pairs.
		const wchar_t* end = _u + _size;
		AlifUCS4* ucs4_out = (AlifUCS4*)_data;
		for (const wchar_t* iter = _u; iter < end; ) {
			if (alifUnicode_isHighSurrogate(iter[0])
				and (iter + 1) < end
				and alifUnicode_isLowSurrogate(iter[1]))
			{
				*ucs4_out++ = alifUnicode_joinSurrogates(iter[0], iter[1]);
				iter += 2;
			}
			else {
				*ucs4_out++ = *iter;
				iter++;
			}
		}
#else
		memcpy(_data, _u, _size * 4);
#endif
		break;
	}
	default:
		ALIF_UNREACHABLE();
	}
}

AlifObject* alifUStr_fromWideChar(const wchar_t* _u, AlifSizeT _size) { // 1956
	AlifObject* unicode{};
	AlifUCS4 maxchar = 0;
	AlifSizeT num_surrogates{};

	if (_u == nullptr and _size != 0) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}

	if (_size == -1) {
		_size = wcslen(_u);
	}

	/* If the Unicode data is known at construction time, we can apply
	   some optimizations which share commonly used objects. */

	   /* Optimization for empty strings */
	if (_size == 0)
		ALIF_RETURN_UNICODE_EMPTY

	if (_size == 1 and (AlifUCS4)*_u < 256)
		return get_latin1Char((unsigned char)*_u);

	/* If not empty and not single character, copy the Unicode data
	   into the new object */
	if (find_maxCharSurrogates(_u, _u + _size,
		&maxchar, &num_surrogates) == -1)
		return nullptr;

	unicode = alifUStr_new(_size - num_surrogates, maxchar);
	if (!unicode)
		return nullptr;

	uStrWrite_wideChar(ALIFUSTR_KIND(unicode), ALIFUSTR_DATA(unicode),
		_u, _size, num_surrogates);

	return uStr_result(unicode);
}


AlifIntT alifUStrWriter_writeWideChar(AlifUStrWriter* pub_writer,
	const wchar_t* str, AlifSizeT size) { // 2015
	AlifUStrWriter* writer = (AlifUStrWriter*)pub_writer;

	if (size < 0) {
		size = wcslen(str);
	}

	if (size == 0) {
		return 0;
	}

#ifdef HAVE_NON_UNICODE_WCHAR_T_REPRESENTATION
	/* Oracle Solaris uses non-Unicode internal wchar_t form for
	   non-Unicode locales and hence needs conversion to UCS-4 first. */
	if (_alif_localeUsesNonUnicodeWchar()) {
		wchar_t* converted = _alif_decodeNonUnicodeWchar(str, size);
		if (!converted) {
			return -1;
		}

		AlifIntT res = alifUStrWriter_writeUCS4(pub_writer, converted, size);
		alifMem_dataFree(converted);
		return res;
	}
#endif

	AlifUCS4 maxchar = 0;
	AlifSizeT num_surrogates{};
	if (find_maxCharSurrogates(str, str + size,
		&maxchar, &num_surrogates) == -1) {
		return -1;
	}

	if (ALIFUSTRWRITER_PREPARE(writer, size - num_surrogates, maxchar) < 0) {
		return -1;
	}

	AlifIntT kind = writer->kind;
	void* data = (AlifUCS1*)writer->data + writer->pos * kind;
	uStrWrite_wideChar(kind, data, str, size, num_surrogates);

	writer->pos += size - num_surrogates;
	return 0;
}


AlifObject* alifUStr_fromStringAndSize(const char* _u, AlifSizeT _size) { // 2065
	if (_size < 0) {
		//alifErr_setString(_alifExcSystemError_,
		//	"Negative size passed to alifUStr_fromStringAndSize");
		return nullptr;
	}
	if (_u != nullptr) {
		return alifUStr_decodeUTF8Stateful(_u, _size, nullptr, nullptr);
	}
	if (_size > 0) {
		//alifErr_setString(_alifExcSystemError_,
		//	"nullptr string with positive size with nullptr passed to alifUStr_fromStringAndSize");
		return nullptr;
	}
	return unicode_getEmpty();
}


AlifObject* alifUStr_fromString(const char* u) { // 2084
	AlifUSizeT size = strlen(u);
	if (size > ALIF_SIZET_MAX) {
		// error
		return nullptr;
	}
	return alifUStr_decodeUTF8Stateful(u, (AlifSizeT)size, nullptr, nullptr);
}



AlifObject* alifUStr_fromASCII(const char* _buffer, AlifSizeT _size) { // 2179
	const unsigned char* s_ = (const unsigned char*)_buffer;
	AlifObject* uStr{};
	if (_size == 1) {
		return get_latin1Char(s_[0]);
	}
	uStr = alifUStr_new(_size, 127);
	if (!uStr)
		return nullptr;
	memcpy(ALIFUSTR_1BYTE_DATA(uStr), s_, _size);
	return uStr;
}

static AlifObject* _alifUStr_fromUCS1(const AlifUCS1* _u, AlifSizeT _size) { // 2213
	AlifObject* res{};
	unsigned char maxChar{};

	if (_size == 0) {
		ALIF_RETURN_UNICODE_EMPTY;
	}
	if (_size == 1) {
		return get_latin1Char(_u[0]);
	}

	maxChar = ucs1Lib_findMaxChar(_u, _u + _size);
	res = alifUStr_new(_size, maxChar);
	if (!res) return nullptr;
	memcpy(ALIFUSTR_1BYTE_DATA(res), _u, _size);
	return res;
}

static AlifObject* _alifUStr_fromUCS2(const AlifUCS2* _u, AlifSizeT _size) { // 2236
	AlifObject* res{};
	AlifUCS2 maxChar{};

	if (_size == 0) ALIF_RETURN_UNICODE_EMPTY;
	if (_size == 1) return uStr_char(_u[0]);

	maxChar = ucs2Lib_findMaxChar(_u, _u + _size);
	res = alifUStr_new(_size, maxChar);
	if (!res)
		return nullptr;
	if (maxChar >= 256)
		memcpy(ALIFUSTR_2BYTE_DATA(res), _u, sizeof(AlifUCS2) * _size);
	else {
		ALIFUSTR_CONVERT_BYTES(AlifUCS2, AlifUCS1,
			_u, _u + _size, ALIFUSTR_1BYTE_DATA(res));
	}
	return res;
}

static AlifObject* _alifUStr_fromUCS4(const AlifUCS4* _u, AlifSizeT _size) { // 2262
	AlifObject* res{};
	AlifUCS4 maxChar{};

	if (_size == 0)
		ALIF_RETURN_UNICODE_EMPTY;
	if (_size == 1)
		return uStr_char(_u[0]);

	maxChar = ucs4Lib_findMaxChar(_u, _u + _size);
	res = alifUStr_new(_size, maxChar);
	if (!res) return nullptr;
	if (maxChar < 256)
		ALIFUSTR_CONVERT_BYTES(AlifUCS4, AlifUCS1, _u, _u + _size,
			ALIFUSTR_1BYTE_DATA(res));
	else if (maxChar < 0x10000)
		ALIFUSTR_CONVERT_BYTES(AlifUCS4, AlifUCS2, _u, _u + _size,
			ALIFUSTR_2BYTE_DATA(res));
	else
		memcpy(ALIFUSTR_4BYTE_DATA(res), _u, sizeof(AlifUCS4) * _size);
	return res;
}

AlifObject* alifUStr_fromKindAndData(AlifIntT _kind,
	const void* _buffer, AlifSizeT _size) { // 2335
	if (_size < 0) {
		//alifErr_setString(_alifExcValueError_, "size must be positive");
		return nullptr;
	}
	switch (_kind) {
	case AlifUStrKind_::AlifUStr_1Byte_Kind:
		return _alifUStr_fromUCS1((const AlifUCS1*)_buffer, _size);
	case AlifUStrKind_::AlifUStr_2Byte_Kind:
		return _alifUStr_fromUCS2((const AlifUCS2*)_buffer, _size);
	case AlifUStrKind_::AlifUStr_4Byte_Kind:
		return _alifUStr_fromUCS4((const AlifUCS4*)_buffer, _size);
	default:
		//alifErr_setString(_alifExcSystemError_, "invalid kind");
		return nullptr;
	}
}

AlifUCS4 alifUStr_findMaxChar(AlifObject* unicode, AlifSizeT start, AlifSizeT end) { // 2355
	AlifIntT kind{};
	const void* startptr{}, * endptr{};

	if (start == 0 and end == ALIFUSTR_GET_LENGTH(unicode))
		return ALIFUSTR_MAX_CHAR_VALUE(unicode);

	if (start == end)
		return 127;

	if (ALIFUSTR_IS_ASCII(unicode))
		return 127;

	kind = ALIFUSTR_KIND(unicode);
	startptr = ALIFUSTR_DATA(unicode);
	endptr = (char*)startptr + end * kind;
	startptr = (char*)startptr + start * kind;
	switch (kind) {
	case AlifUStrKind_::AlifUStr_1Byte_Kind:
		return ucs1Lib_findMaxChar((const AlifUCS1*)startptr, (const AlifUCS1*)endptr);
	case AlifUStrKind_::AlifUStr_2Byte_Kind:
		return ucs2Lib_findMaxChar((const AlifUCS2*)startptr, (const AlifUCS2*)endptr);
	case AlifUStrKind_::AlifUStr_4Byte_Kind:
		return ucs4Lib_findMaxChar((const AlifUCS4*)startptr, (const AlifUCS4*)endptr);
	default:
		ALIF_UNREACHABLE();
	}
}

static void uStr_adjustMaxChar(AlifObject** _pUStr) { // 2393
	AlifObject* unicode{}, * copy{};
	AlifUCS4 maxChar{};
	AlifSizeT len{};
	AlifIntT kind{};

	unicode = *_pUStr;
	if (ALIFUSTR_IS_ASCII(unicode))
		return;

	len = ALIFUSTR_GET_LENGTH(unicode);
	kind = ALIFUSTR_KIND(unicode);
	if (kind == AlifUStrKind_::AlifUStr_1Byte_Kind) {
		const AlifUCS1* u = ALIFUSTR_1BYTE_DATA(unicode);
		maxChar = ucs1Lib_findMaxChar(u, u + len);
		if (maxChar >= 128)
			return;
	}
	else if (kind == AlifUStrKind_::AlifUStr_2Byte_Kind) {
		const AlifUCS2* u = ALIFUSTR_2BYTE_DATA(unicode);
		maxChar = ucs2Lib_findMaxChar(u, u + len);
		if (maxChar >= 256)
			return;
	}
	else if (kind == AlifUStrKind_::AlifUStr_4Byte_Kind) {
		const AlifUCS4* u = ALIFUSTR_4BYTE_DATA(unicode);
		maxChar = ucs4Lib_findMaxChar(u, u + len);
		if (maxChar >= 0x10000)
			return;
	}
	else ALIF_UNREACHABLE();

	copy = alifUStr_new(len, maxChar);
	if (copy != nullptr)
		alifUStr_fastCopyCharacters(copy, 0, unicode, 0, len);
	ALIF_DECREF(unicode);
	*_pUStr = copy;
}

AlifObject* _alifUStr_copy(AlifObject* _uStr) { // 2436
	AlifSizeT length{};
	AlifObject* copy{};

	if (!ALIFUSTR_CHECK(_uStr)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}

	length = ALIFUSTR_GET_LENGTH(_uStr);
	copy = alifUStr_new(length, ALIFUSTR_MAX_CHAR_VALUE(_uStr));
	if (!copy)
		return nullptr;

	memcpy(ALIFUSTR_DATA(copy), ALIFUSTR_DATA(_uStr),
		length * ALIFUSTR_KIND(_uStr));
	return copy;
}


static void* unicode_asKind(AlifIntT _sKind, void const* _data,
	AlifSizeT _len, AlifIntT _kind) { // 2463
	void* result{};

	switch (_kind) {
	case AlifUStrKind_::AlifUStr_2Byte_Kind:
		result = alifMem_dataAlloc(_len * sizeof(AlifUCS2)); //* alif
		if (!result) {
			//return alifErr_noMemory();
			return nullptr; //* alif
		}
		ALIFUSTR_CONVERT_BYTES(
			AlifUCS1, AlifUCS2,
			(const AlifUCS1*)_data,
			((const AlifUCS1*)_data) + _len,
			result);
		return result;
	case AlifUStrKind_::AlifUStr_4Byte_Kind:
		result = alifMem_dataAlloc(_len * sizeof(AlifUCS4)); //* alif
		if (!result) {
			//return alifErr_noMemory();
			return nullptr; //* alif
		}
		if (_sKind == AlifUStrKind_::AlifUStr_2Byte_Kind) {
			ALIFUSTR_CONVERT_BYTES(
				AlifUCS2, AlifUCS4,
				(const AlifUCS2*)_data,
				((const AlifUCS2*)_data) + _len,
				result);
		}
		else {
			ALIFUSTR_CONVERT_BYTES(
				AlifUCS1, AlifUCS4,
				(const AlifUCS1*)_data,
				((const AlifUCS1*)_data) + _len,
				result);
		}
		return result;
	default:
		ALIF_UNREACHABLE();
		return nullptr;
	}
}


#define MAX_INTMAX_CHARS (5 + (sizeof(intmax_t)*8-1) / 3) // 2576

static AlifIntT uStr_fromFormatWriteStr(AlifUStrWriter* writer, AlifObject* str,
	AlifSizeT width, AlifSizeT precision, AlifIntT flags) { // 2578
	AlifSizeT length{}, fill{}, arglen{};
	AlifUCS4 maxchar{};

	length = ALIFUSTR_GET_LENGTH(str);
	if ((precision == -1 or precision >= length)
		and width <= length)
		return alifUStrWriter_writeStr(writer, str);

	if (precision != -1)
		length = ALIF_MIN(precision, length);

	arglen = ALIF_MAX(length, width);
	if (ALIFUSTR_MAX_CHAR_VALUE(str) > writer->maxChar)
		maxchar = alifUStr_findMaxChar(str, 0, length);
	else
		maxchar = writer->maxChar;

	if (ALIFUSTRWRITER_PREPARE(writer, arglen, maxchar) == -1)
		return -1;

	fill = ALIF_MAX(width - length, 0);
	if (fill and !(flags & F_LJUST)) {
		if (alifUStr_fill(writer->buffer, writer->pos, fill, ' ') == -1)
			return -1;
		writer->pos += fill;
	}

	alifUStr_fastCopyCharacters(writer->buffer, writer->pos,
		str, 0, length);
	writer->pos += length;

	if (fill and (flags & F_LJUST)) {
		if (alifUStr_fill(writer->buffer, writer->pos, fill, ' ') == -1)
			return -1;
		writer->pos += fill;
	}

	return 0;
}




static AlifIntT uStr_fromFormatWriteUTF8(AlifUStrWriter* writer, const char* str,
	AlifSizeT width, AlifSizeT precision, AlifIntT flags) { // 2622
	/* UTF-8 */
	AlifSizeT* pconsumed = nullptr;
	AlifSizeT length;
	if (precision == -1) {
		length = strlen(str);
	}
	else {
		length = 0;
		while (length < precision && str[length]) {
			length++;
		}
		if (length == precision) {
			pconsumed = &length;
		}
	}

	if (width < 0) {
		return uStr_decodeUTF8Writer(writer, str, length,
			AlifErrorHandler_::Alif_Error_Replace, "replace", pconsumed);
	}

	AlifObject* unicode = alifUStr_decodeUTF8Stateful(str, length,
		"replace", pconsumed);
	if (unicode == nullptr)
		return -1;

	AlifIntT res = uStr_fromFormatWriteStr(writer, unicode,
		width, -1, flags);
	ALIF_DECREF(unicode);
	return res;
}



static AlifIntT uStr_fromFormatWriteWCStr(AlifUStrWriter* _writer, const wchar_t* _str,
	AlifIntT _width, AlifSizeT _precision, AlifIntT _flags) { // 2663
	AlifSizeT length{};
	if (_precision == -1) {
		length = wcslen(_str);
	}
	else {
		length = 0;
		while (length < _precision and _str[length]) {
			length++;
		}
	}

	if (_width < 0) {
		return alifUStrWriter_writeWideChar((AlifUStrWriter*)_writer,
			_str, length);
	}

	AlifObject* unicode = alifUStr_fromWideChar(_str, length);
	if (unicode == nullptr)
		return -1;

	AlifIntT res = uStr_fromFormatWriteStr(_writer, unicode, _width, -1, _flags);
	ALIF_DECREF(unicode);
	return res;
}










 // 2692
#define F_LONG 1
#define F_LONGLONG 2
#define F_SIZE 3
#define F_PTRDIFF 4
#define F_INTMAX 5


static const char* uStr_fromFormatArg(AlifUStrWriter* _writer,
	const char* _f, va_list* _vargs) { // 2703
	const char* p{};
	AlifSizeT len{};
	AlifIntT flags = 0;
	AlifSizeT width{};
	AlifSizeT precision{};

	p = _f;
	_f++;
	if (*_f == '%') {
		if (alifUStrWriter_writeCharInline(_writer, '%') < 0)
			return nullptr;
		_f++;
		return _f;
	}

	while (1) {
		switch (*_f++) {
		case '-': flags |= F_LJUST; continue;
		case '0': flags |= F_ZERO; continue;
		case '#': flags |= F_ALT; continue;
		}
		_f--;
		break;
	}

	width = -1;
	if (*_f == '*') {
		width = va_arg(*_vargs, AlifIntT);
		if (width < 0) {
			flags |= F_LJUST;
			width = -width;
		}
		_f++;
	}
	else if (ALIF_ISDIGIT((unsigned)*_f)) {
		width = *_f - '0';
		_f++;
		while (ALIF_ISDIGIT((unsigned)*_f)) {
			if (width > (ALIF_SIZET_MAX - ((AlifIntT)*_f - '0')) / 10) {
				//alifErr_setString(_alifExcValueError_,
				//	"width too big");
				return nullptr;
			}
			width = (width * 10) + (*_f - '0');
			_f++;
		}
	}
	precision = -1;
	if (*_f == '.') {
		_f++;
		if (*_f == '*') {
			precision = va_arg(*_vargs, AlifIntT);
			if (precision < 0) {
				precision = -2;
			}
			_f++;
		}
		else if (ALIF_ISDIGIT((unsigned)*_f)) {
			precision = (*_f - '0');
			_f++;
			while (ALIF_ISDIGIT((unsigned)*_f)) {
				if (precision > (ALIF_SIZET_MAX - ((int)*_f - '0')) / 10) {
					//alifErr_setString(_alifExcValueError_,
					//	"precision too big");
					return nullptr;
				}
				precision = (precision * 10) + (*_f - '0');
				_f++;
			}
		}
	}

	AlifIntT sizemod = 0;
	if (*_f == 'l') {
		if (_f[1] == 'l') {
			sizemod = F_LONGLONG;
			_f += 2;
		}
		else {
			sizemod = F_LONG;
			++_f;
		}
	}
	else if (*_f == 'z') {
		sizemod = F_SIZE;
		++_f;
	}
	else if (*_f == 't') {
		sizemod = F_PTRDIFF;
		++_f;
	}
	else if (*_f == 'j') {
		sizemod = F_INTMAX;
		++_f;
	}
	if (_f[0] != '\0' and _f[1] == '\0')
		_writer->overAllocate = 0;

	switch (*_f) {
	case 'd': case 'i': case 'o': case 'u': case 'x': case 'X':
		break;
	case 'c': case 'p':
		if (sizemod or width >= 0 or precision >= 0) goto invalid_format;
		break;
	case 's':
	case 'V':
		if (sizemod and sizemod != F_LONG) goto invalid_format;
		break;
	default:
		if (sizemod) goto invalid_format;
		break;
	}

	switch (*_f) {
	case 'c':
	{
		AlifIntT ordinal = va_arg(*_vargs, AlifIntT);
		if (ordinal < 0 or ordinal > MAX_UNICODE) {
			//alifErr_setString(_alifExcOverflowError_,
			//	"character argument not in range(0x110000)");
			return nullptr;
		}
		if (alifUStrWriter_writeCharInline(_writer, ordinal) < 0)
			return nullptr;
		break;
	}

	case 'd': case 'i':
	case 'o': case 'u': case 'x': case 'X':
	{
		char buffer[MAX_INTMAX_CHARS];

		// Fill buffer using sprinf, with one of many possible format
		// strings, like "%llX" for `long long` in hexadecimal.
		// The type/size is in `sizemod`; the format is in `*f`.

		// Use macros with nested switches to keep the sprintf format strings
		// as compile-time literals, avoiding warnings and maybe allowing
		// optimizations.

		// `SPRINT` macro does one sprintf
		// Example usage: SPRINT("l", "X", unsigned long) expands to
		// sprintf(buffer, "%" "l" "X", va_arg(*vargs, unsigned long))
#define SPRINT(SIZE_SPEC, FMT_CHAR, TYPE) \
            sprintf(buffer, "%" SIZE_SPEC FMT_CHAR, va_arg(*_vargs, TYPE))

		// One inner switch to handle all format variants
#define DO_SPRINTS(SIZE_SPEC, SIGNED_TYPE, UNSIGNED_TYPE)             \
            switch (*_f) {                                                     \
                case 'o': len = SPRINT(SIZE_SPEC, "o", UNSIGNED_TYPE); break; \
                case 'u': len = SPRINT(SIZE_SPEC, "u", UNSIGNED_TYPE); break; \
                case 'x': len = SPRINT(SIZE_SPEC, "x", UNSIGNED_TYPE); break; \
                case 'X': len = SPRINT(SIZE_SPEC, "X", UNSIGNED_TYPE); break; \
                default:  len = SPRINT(SIZE_SPEC, "d", SIGNED_TYPE); break;   \
		}
		switch (sizemod) {
		case F_LONG:     DO_SPRINTS("l", long, unsigned long); break;
		case F_LONGLONG: DO_SPRINTS("ll", long long, unsigned long long); break;
		case F_SIZE:     DO_SPRINTS("z", AlifSizeT, AlifUSizeT); break;
		case F_PTRDIFF:  DO_SPRINTS("t", ptrdiff_t, ptrdiff_t); break;
		case F_INTMAX:   DO_SPRINTS("j", intmax_t, uintmax_t); break;
		default:         DO_SPRINTS("", AlifIntT, AlifUIntT); break;
		}
#undef SPRINT
#undef DO_SPRINTS

		AlifIntT sign = (buffer[0] == '-');
		len -= sign;

		precision = ALIF_MAX(precision, len);
		width = ALIF_MAX(width, precision + sign);
		if ((flags & F_ZERO) and !(flags & F_LJUST)) {
			precision = width - sign;
		}

		AlifSizeT spacepad = ALIF_MAX(width - precision - sign, 0);
		AlifSizeT zeropad = ALIF_MAX(precision - len, 0);

		if (ALIFUSTRWRITER_PREPARE(_writer, width, 127) == -1)
			return nullptr;

		if (spacepad and !(flags & F_LJUST)) {
			if (alifUStr_fill(_writer->buffer, _writer->pos, spacepad, ' ') == -1)
				return nullptr;
			_writer->pos += spacepad;
		}

		if (sign) {
			if (alifUStrWriter_writeChar(_writer, '-') == -1)
				return nullptr;
		}

		if (zeropad) {
			if (alifUStr_fill(_writer->buffer, _writer->pos, zeropad, '0') == -1)
				return nullptr;
			_writer->pos += zeropad;
		}

		if (alifUStrWriter_writeASCIIString(_writer, &buffer[sign], len) < 0)
			return nullptr;

		if (spacepad and (flags & F_LJUST)) {
			if (alifUStr_fill(_writer->buffer, _writer->pos, spacepad, ' ') == -1)
				return nullptr;
			_writer->pos += spacepad;
		}
		break;
	}

	//case 'p':
	//{
	//	char number[MAX_INTMAX_CHARS]{};

	//	len = sprintf(number, "%p", va_arg(*_vargs, void*));

	//	/* %p is ill-defined:  ensure leading 0x. */
	//	if (number[1] == 'X')
	//		number[1] = 'x';
	//	else if (number[1] != 'x') {
	//		memmove(number + 2, number,
	//			strlen(number) + 1);
	//		number[0] = '0';
	//		number[1] = 'x';
	//		len += 2;
	//	}

	//	if (alifUStrWriter_writeASCIIString(_writer, number, len) < 0)
	//		return nullptr;
	//	break;
	//}

	case 's':
	{
		if (sizemod) {
			const wchar_t* s = va_arg(*_vargs, const wchar_t*);
			if (uStr_fromFormatWriteWCStr(_writer, s, width, precision, flags) < 0)
				return nullptr;
		}
		else {
			/* UTF-8 */
			const char* s = va_arg(*_vargs, const char*);
			if (uStr_fromFormatWriteUTF8(_writer, s, width, precision, flags) < 0)
				return nullptr;
		}
		break;
	}

	//case 'U':
	//{
	//	AlifObject* obj = va_arg(*_vargs, AlifObject*);

	//	if (uStr_fromFormatWriteStr(_writer, obj, width, precision, flags) == -1)
	//		return nullptr;
	//	break;
	//}

	//case 'V':
	//{
	//	AlifObject* obj = va_arg(*_vargs, AlifObject*);
	//	const char* str{};
	//	const wchar_t* wstr{};
	//	if (sizemod) {
	//		wstr = va_arg(*_vargs, const wchar_t*);
	//	}
	//	else {
	//		str = va_arg(*_vargs, const char*);
	//	}
	//	if (obj) {
	//		if (unicode_fromformat_write_str(_writer, obj, width, precision, flags) == -1)
	//			return nullptr;
	//	}
	//	else if (sizemod) {
	//		if (unicode_fromformat_write_wcstr(_writer, wstr, width, precision, flags) < 0)
	//			return nullptr;
	//	}
	//	else {
	//		if (unicode_fromformat_write_utf8(_writer, str, width, precision, flags) < 0)
	//			return nullptr;
	//	}
	//	break;
	//}

	//case 'S':
	//{
	//	AlifObject* obj = va_arg(*_vargs, AlifObject*);
	//	AlifObject* str;
	//	str = alifObject_str(obj);
	//	if (!str)
	//		return nullptr;
	//	if (unicode_fromFormatWriteStr(_writer, str, width, precision, flags) == -1) {
	//		ALIF_DECREF(str);
	//		return nullptr;
	//	}
	//	ALIF_DECREF(str);
	//	break;
	//}

	case 'R':
	{
		AlifObject* obj = va_arg(*_vargs, AlifObject*);
		AlifObject* repr;
		repr = alifObject_repr(obj);
		if (!repr)
			return nullptr;
		if (uStr_fromFormatWriteStr(_writer, repr, width, precision, flags) == -1) {
			ALIF_DECREF(repr);
			return nullptr;
		}
		ALIF_DECREF(repr);
		break;
	}

	//case 'A':
	//{
	//	AlifObject* obj = va_arg(*_vargs, AlifObject*);
	//	AlifObject* ascii;
	//	ascii = alifObject_ascii(obj);
	//	if (!ascii)
	//		return nullptr;
	//	if (unicode_fromFormatWriteStr(_writer, ascii, width, precision, flags) == -1) {
	//		ALIF_DECREF(ascii);
	//		return nullptr;
	//	}
	//	ALIF_DECREF(ascii);
	//	break;
	//}

	//case 'T':
	//{
	//	AlifObject* obj = va_arg(*_vargs, AlifObject*);
	//	AlifTypeObject* type = (AlifTypeObject*)ALIF_NEWREF(ALIF_TYPE(obj));

	//	AlifObject* type_name{};
	//	if (flags & F_ALT) {
	//		type_name = _alifType_getFullyQualifiedName(type, ':');
	//	}
	//	else {
	//		type_name = alifType_getFullyQualifiedName(type);
	//	}
	//	ALIF_DECREF(type);
	//	if (!type_name) {
	//		return nullptr;
	//	}

	//	if (unicode_fromFormatWriteStr(_writer, type_name,
	//		width, precision, flags) == -1) {
	//		ALIF_DECREF(type_name);
	//		return nullptr;
	//	}
	//	ALIF_DECREF(type_name);
	//	break;
	//}

	//case 'N':
	//{
	//	AlifObject* typeRaw = va_arg(*_vargs, AlifObject*);

	//	if (!ALIFTYPE_CHECK(typeRaw)) {
	//		//alifErr_setString(_alifExcTypeError_, "%N argument must be a type");
	//		return nullptr;
	//	}
	//	AlifTypeObject* type = (AlifTypeObject*)typeRaw;

	//	AlifObject* typeName{};
	//	if (flags & F_ALT) {
	//		typeName = _alifType_getFullyQualifiedName(type, ':');
	//	}
	//	else {
	//		typeName = alifType_getFullyQualifiedName(type);
	//	}
	//	if (!typeName) {
	//		return nullptr;
	//	}
	//	if (unicode_fromFormatWriteStr(_writer, typeName,
	//		width, precision, flags) == -1) {
	//		ALIF_DECREF(typeName);
	//		return nullptr;
	//	}
	//	ALIF_DECREF(typeName);
	//	break;
	//}
	//
	default:
	invalid_format:
		//alifErr_format(_alifExcSystemError_, "invalid format string: %s", p);
		return nullptr;
	}

	_f++;
	return _f;
}


//* alif //* review //* todo
static AlifIntT uStr_fromFormatForError(AlifUStrWriter* _writer,
	const char* _format, va_list _vargs) { //* alif

	va_list vargs{};
	va_copy(vargs, _vargs);

	char buffer[256]{};

	AlifIntT len = vsprintf(buffer, _format, vargs);
	if (len < 0) {
		goto fail;
	}
	_writer->pos = len;


	if (ALIFUSTRWRITER_PREPARE(_writer, len, 127) == -1) {
		goto fail;
	}

	memcpy(_writer->data, buffer, len);

	va_end(vargs);
	return 0;

fail:
	va_end(vargs);
	return -1;
}
//* alif //* review //* todo
AlifObject* alifUStr_fromFormatVFroError(const char* _format, va_list _vargs) { //* alif
	AlifUStrWriter writer{};
	alifUStrWriter_init(&writer);

	if (uStr_fromFormatForError(&writer, _format, _vargs) < 0) {
		alifUStrWriter_dealloc(&writer);
		return nullptr;
	}
	return alifUStrWriter_finish(&writer);
}


static AlifIntT uStr_fromFormat(AlifUStrWriter* _writer,
	const char* _format, va_list _vargs) { // 3121
	AlifSizeT len = strlen(_format);
	_writer->minLength += len + 100;
	_writer->overAllocate = 1;

	va_list vargs2{};
	va_copy(vargs2, _vargs);

	AlifIntT isAscii = (ucs1Lib_findMaxChar((AlifUCS1*)_format, (AlifUCS1*)_format + len) < 128);
	if (!isAscii) {
		AlifSizeT i{};
		for (i = 0; i < len and (unsigned char)_format[i] <= 127; i++);
		//alifErr_format(_alifExcValueError_,
		//	"alifUStr_fromFormatV() expects an ASCII-encoded format "
		//	"string, got a non-ASCII byte: 0x%02x",
		//	(unsigned char)_format[i]);
		goto fail;
	}

	for (const char* f = _format; *f; ) {
		if (*f == '%') {
			f = uStr_fromFormatArg(_writer, f, &vargs2);
			if (f == nullptr)
				goto fail;
		}
		else {
			const char* p = strchr(f, '%');
			if (p != nullptr) {
				len = p - f;
			}
			else {
				len = strlen(f);
				_writer->overAllocate = 0;
			}

			if (alifUStrWriter_writeASCIIString(_writer, f, len) < 0) {
				goto fail;
			}
			f += len;
		}
	}
	va_end(vargs2);
	return 0;

fail:
	va_end(vargs2);
	return -1;
}


AlifObject* alifUStr_fromFormatV(const char* _format, va_list _vargs) { // 3175
	AlifUStrWriter writer{};
	alifUStrWriter_init(&writer);

	if (uStr_fromFormat(&writer, _format, _vargs) < 0) {
		alifUStrWriter_dealloc(&writer);
		return nullptr;
	}
	return alifUStrWriter_finish(&writer);
}

AlifObject* alifUStr_fromFormat(const char* _format, ...) { // 3188
	AlifObject* ret{};
	va_list vargs{};

	va_start(vargs, _format);
	ret = alifUStr_fromFormatV(_format, vargs);
	va_end(vargs);
	return ret;
}

static AlifSizeT uStr_getWideCharSize(AlifObject* _uStr) { // 3218
	AlifSizeT res_{};
	res_ = ALIFUSTR_LENGTH(_uStr);
#if SIZEOF_WCHAR_T == 2
	if (ALIFUSTR_KIND(_uStr) == AlifUStrKind_::AlifUStr_4Byte_Kind) {
		const AlifUCS4* s_ = ALIFUSTR_4BYTE_DATA(_uStr);
		const AlifUCS4* end_ = s_ + res_;
		for (; s_ < end_; ++s_) {
			if (*s_ > 0xFFFF) {
				++res_;
			}
		}
	}
#endif
	return res_;
}

static void uStr_copyAsWideChar(AlifObject* _uStr, wchar_t* _w, AlifSizeT _size) { // 3241

	if (ALIFUSTR_KIND(_uStr) == sizeof(wchar_t)) {
		memcpy(_w, ALIFUSTR_DATA(_uStr), _size * sizeof(wchar_t));
		return;
	}

	if (ALIFUSTR_KIND(_uStr) == AlifUStrKind_::AlifUStr_1Byte_Kind) {
		const AlifUCS1* s_ = ALIFUSTR_1BYTE_DATA(_uStr);
		for (; _size--; ++s_, ++_w) {
			*_w = *s_;
		}
	}
	else {
#if SIZEOF_WCHAR_T == 4
		const AlifUCS2* s_ = ALIFUSTR_2BYTE_DATA(_uStr);
		for (; _size--; ++s_, ++_w) {
			*_w = *s_;
		}
#else
		const AlifUCS4* s_ = ALIFUSTR_4BYTE_DATA(_uStr);
		for (; _size--; ++s_, ++_w) {
			AlifUCS4 ch_ = *s_;
			if (ch_ > 0xFFFF) {
				*_w++ = alifUnicode_highSurrogate(ch_);
				if (!_size--)
					break;
				*_w = alifUnicode_lowSurrogate(ch_);
			}
			else {
				*_w = ch_;
			}
		}
#endif
	}
}

AlifSizeT alifUStr_asWideChar(AlifObject* _uStr, wchar_t* _w, AlifSizeT _size) { // 3296
	AlifSizeT res_{};

	if (_uStr == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	if (!ALIFUSTR_CHECK(_uStr)) {
		//alifErr_badArgument();
		return -1;
	}

	res_ = uStr_getWideCharSize(_uStr);
	if (_w == nullptr) {
		return res_ + 1;
	}

	if (_size > res_) {
		_size = res_ + 1;
	}
	else {
		res_ = _size;
	}
	uStr_copyAsWideChar(_uStr, _w, _size);

#ifdef HAVE_NON_UNICODE_WCHAR_T_REPRESENTATION
	if (alif_localeUsesNonUnicodeWchar()) {
		if (alif_encodeNonUnicodeWcharInPlace(_w, _size) < 0) {
			return -1;
		}
	}
#endif

	return res_;
}


wchar_t* alifUStr_asWideCharString(AlifObject* _uStr, AlifSizeT* _size) { // 3337
	wchar_t* buffer{};
	AlifSizeT buflen{};

	if (_uStr == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	if (!ALIFUSTR_CHECK(_uStr)) {
		//alifErr_badArgument();
		return nullptr;
	}

	buflen = uStr_getWideCharSize(_uStr);
	buffer = (wchar_t*)alifMem_dataAlloc((buflen + 1) * sizeof(wchar_t)); // ALIFMEM_NEW
	if (buffer == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}
	uStr_copyAsWideChar(_uStr, buffer, buflen + 1);

	if (_size != nullptr) {
		*_size = buflen;
	}
	else if (wcslen(buffer) != (AlifUSizeT)buflen) {
		alifMem_dataFree(buffer);
		//alifErr_setString(_alifExcValueError_,
		//	"embedded null character");
		return nullptr;
	}
	return buffer;
}

AlifObject* alifUStr_fromOrdinal(AlifIntT _ordinal) { // 3434
	if (_ordinal < 0 or _ordinal > MAX_UNICODE) {
		//alifErr_setString(_alifExcValueError_,
			//"chr() arg not in range(0x110000)");
		return nullptr;
	}

	return uStr_char((AlifUCS4)_ordinal);
}

AlifObject* alifUStr_fromObject(AlifObject* _obj) { // 3446
	if (ALIFUSTR_CHECKEXACT(_obj)) {
		return ALIF_NEWREF(_obj);
	}
	if (ALIFUSTR_CHECK(_obj)) {
		/* For a Unicode subtype that's not a Unicode object,
		   return a true Unicode object with the same data. */
		return _alifUStr_copy(_obj);
	}
	//alifErr_format(_alifExcTypeError_,
		//"Can't convert '%.100s' object to str implicitly",
		//ALIF_TYPE(obj)->name);
	return nullptr;
}


AlifIntT _alif_normalizeEncoding(const char* encoding,
	char* lower, AlifUSizeT lower_len) { // 3520
	const char* e{};
	char* l{};
	char* l_end{};
	int punct{};

	e = encoding;
	l = lower;
	l_end = &lower[lower_len - 1];
	punct = 0;
	while (1) {
		char c = *e;
		if (c == 0) {
			break;
		}

		if (ALIF_ISALNUM(c) or c == '.') {
			if (punct and l != lower) {
				if (l == l_end) {
					return 0;
				}
				*l++ = '_';
			}
			punct = 0;

			if (l == l_end) {
				return 0;
			}
			//*l++ = ALIF_TOLOWER(c);
			*l++ = c; //* alif
		}
		else {
			punct = 1;
		}

		e++;
	}
	*l = '\0';
	return 1;
}


AlifObject* alifUStr_decode(const char* s, AlifSizeT size,
	const char* encoding, const char* errors) { // 3566
	AlifObject* buffer = nullptr, * unicode{};
	AlifBuffer info{};
	char buflower[11]{};

	//if (uStCheck_encodingErrors(encoding, errors) < 0) {
	//	return nullptr;
	//}

	if (size == 0) {
		ALIF_RETURN_UNICODE_EMPTY;
	}

	if (encoding == nullptr) {
		return alifUStr_decodeUTF8Stateful(s, size, errors, nullptr);
	}

	/* Shortcuts for common default encodings */
	if (_alif_normalizeEncoding(encoding, buflower, sizeof(buflower))) {
		char* lower = buflower;

		/* Fast paths */
		if (lower[0] == 'u' and lower[1] == 't' and lower[2] == 'f') {
			lower += 3;
			if (*lower == '_') {
				/* Match "utf8" and "utf_8" */
				lower++;
			}

			if (lower[0] == '8' && lower[1] == 0) {
				return alifUStr_decodeUTF8Stateful(s, size, errors, nullptr);
			}
			else if (lower[0] == '1' and lower[1] == '6' and lower[2] == 0) {
				return alifUStr_decodeUTF16(s, size, errors, 0);
			}
			else if (lower[0] == '3' and lower[1] == '2' and lower[2] == 0) {
				return alifUStr_decodeUTF32(s, size, errors, 0);
			}
		}
		else {
			if (strcmp(lower, "ascii") == 0
				or strcmp(lower, "us_ascii") == 0) {
				//return alifUStr_decodeASCII(s, size, errors);
			}
#ifdef _WINDOWS
			else if (strcmp(lower, "mbcs") == 0) {
				//return alifUStr_decodeMBCS(s, size, errors);
			}
#endif
			else if (strcmp(lower, "latin1") == 0
				or strcmp(lower, "latin_1") == 0
				or strcmp(lower, "iso_8859_1") == 0
				or strcmp(lower, "iso8859_1") == 0) {
				//return alifUStr_decodeLatin1(s, size, errors);
			}
		}
	}

	/* Decode via the codec registry */
	buffer = nullptr;
	//if (alifBuffer_fillInfo(&info, nullptr, (void*)s, size, 1, ALIFBUF_FULL_RO) < 0)
	//	goto onError;
	//buffer = alifMemoryView_fromBuffer(&info);
	//if (buffer == nullptr)
	//	goto onError;
	unicode = _alifCodec_decodeText(buffer, encoding, errors);
	if (unicode == nullptr)
		goto onError;
	if (!ALIFUSTR_CHECK(unicode)) {
		//alifErr_format(_alifExcTypeError_,
		//	"'%.400s' decoder returned '%.400s' instead of 'str'; "
		//	"use codecs.decode() to decode to arbitrary types",
		//	encoding, ALIF_TYPE(unicode)->name);
		ALIF_DECREF(unicode);
		goto onError;
	}
	ALIF_DECREF(buffer);
	return uStr_result(unicode);

onError:
	ALIF_XDECREF(buffer);
	return nullptr;
}


static AlifObject* uStr_encodeLocale(AlifObject* _uStr,
	AlifErrorHandler_ _errorHandler, AlifIntT _currentLocale) { // 3749
	AlifSizeT wlen{};
	wchar_t* wstr = alifUStr_asWideCharString(_uStr, &wlen);
	if (wstr == nullptr) {
		return nullptr;
	}

	if ((AlifUSizeT)wlen != wcslen(wstr)) {
		//alifErr_setString(_alifExcValueError_, "embedded null character");
		alifMem_dataFree(wstr);
		return nullptr;
	}

	char* str{};
	AlifUSizeT error_pos{};
	const char* reason{};
	AlifIntT res = _alif_encodeLocaleEx(wstr, &str, &error_pos, &reason,
		_currentLocale, _errorHandler);
	alifMem_dataFree(wstr);

	if (res != 0) {
		if (res == -2) {
			//AlifObject* exc{};
			//exc = alifObject_callFunction(_alifExcUStrEncodeError_, "sOnns",
			//	"locale", unicode,
			//	(AlifSizeT)error_pos,
			//	(AlifSizeT)(error_pos + 1),
			//	reason);
			//if (exc != nullptr) {
			//	alifCodec_strictErrors(exc);
			//	ALIF_DECREF(exc);
			//}
		}
		else if (res == -3) {
			//alifErr_setString(_alifExcValueError_, "unsupported error handler");
		}
		else {
			//alifErr_noMemory();
		}
		return nullptr;
	}

	AlifObject* bytes = alifBytes_fromString(str);
	free(str); // need review
	return bytes;
}


AlifObject* alifUStr_encodeFSDefault(AlifObject* _uStr) { // 3806
	AlifInterpreter* interp = _alifInterpreter_get();


	const AlifConfig* config = alifInterpreter_getConfig(interp);
	//const wchar_t* fileSystemErrors = config->fileSystemErrors;
	const wchar_t* fileSystemErrors = nullptr;
	AlifErrorHandler_ errors = get_errorHandlerWide(fileSystemErrors);
	return uStr_encodeLocale(_uStr, errors, 0);
}


AlifObject* alifUStr_asEncodedString(AlifObject* unicode,
	const char* encoding, const char* errors) { // 3840
	AlifObject* v{};
	char buflower[11]{};

	if (!ALIFUSTR_CHECK(unicode)) {
		//alifErr_badArgument();
		return nullptr;
	}

	//if (uStr_checkEncodingErrors(encoding, errors) < 0) {
	//	return nullptr;
	//}

	if (encoding == nullptr) {
		return _alifUStr_asUTF8String(unicode, errors);
	}

	/* Shortcuts for common default encodings */
	if (_alif_normalizeEncoding(encoding, buflower, sizeof(buflower))) {
		char* lower = buflower;

		/* Fast paths */
		if (lower[0] == 'u' and lower[1] == 't' and lower[2] == 'f') {
			lower += 3;
			if (*lower == '_') {
				/* Match "utf8" and "utf_8" */
				lower++;
			}

			if (lower[0] == '8' and lower[1] == 0) {
				return _alifUStr_asUTF8String(unicode, errors);
			}
			else if (lower[0] == '1' and lower[1] == '6' and lower[2] == 0) {
				return _alifUStr_encodeUTF16(unicode, errors, 0);
			}
			else if (lower[0] == '3' and lower[1] == '2' and lower[2] == 0) {
				return _alifUStr_encodeUTF32(unicode, errors, 0);
			}
		}
		else {
			if (strcmp(lower, "ascii") == 0
				or strcmp(lower, "us_ascii") == 0) {
				return _alifUStr_asASCIIString(unicode, errors);
			}
#ifdef _WINDOWS
			else if (strcmp(lower, "mbcs") == 0) {
				return alifUStr_encodeCodePage(CP_ACP, unicode, errors);
			}
#endif
			else if (strcmp(lower, "latin1") == 0 or
				strcmp(lower, "latin_1") == 0 or
				strcmp(lower, "iso_8859_1") == 0 or
				strcmp(lower, "iso8859_1") == 0) {
				return _alifUStr_asLatin1String(unicode, errors);
			}
		}
	}

	/* Encode via the codec registry */
	v = _alifCodec_encodeText(unicode, encoding, errors);
	if (v == nullptr)
		return nullptr;

	/* The normal path */
	if (ALIFBYTES_CHECK(v))
		return v;

	/* If the codec returns a buffer, raise a warning and convert to bytes */
	if (ALIFBYTEARRAY_CHECK(v)) {
		AlifIntT error{};
		AlifObject* b{};

		//error = alifErr_warnFormat(_alifExcRuntimeWarning_, 1,
		//	"encoder %s returned bytearray instead of bytes; "
		//	"use codecs.encode() to encode to arbitrary types",
		//	encoding);
		if (error) {
			ALIF_DECREF(v);
			return nullptr;
		}

		b = alifBytes_fromStringAndSize(ALIFBYTEARRAY_AS_STRING(v),
			ALIFBYTEARRAY_GET_SIZE(v));
		ALIF_DECREF(v);
		return b;
	}

	//alifErr_format(_alifExcTypeError_,
	//	"'%.400s' encoder returned '%.400s' instead of 'bytes'; "
	//	"use codecs.encode() to encode to arbitrary types",
	//	encoding,
	//	ALIF_TYPE(v)->name);
	ALIF_DECREF(v);
	return nullptr;
}




AlifIntT alifUStr_fsConverter(AlifObject* _arg, void* _addr) { // 4079
	AlifObject* path = nullptr;
	AlifObject* output = nullptr;
	AlifSizeT size{};
	const char* data{};
	if (_arg == nullptr) {
		ALIF_DECREF(*(AlifObject**)_addr);
		*(AlifObject**)_addr = nullptr;
		return 1;
	}
	//path = alifOS_fsPath(_arg);
	//if (path == nullptr) {
	//	return 0;
	//}
	//if (ALIFBYTES_CHECK(path)) {
	//	output = path;
	//}
	//else {  // alifOS_fsPath() guarantees its returned value is bytes or str.
		//output = alifUStr_encodeFSDefault(path);
		output = alifUStr_encodeFSDefault(_arg);
		//ALIF_DECREF(path);
		if (!output) {
			return 0;
		}
	//}

	size = ALIFBYTES_GET_SIZE(output);
	data = ALIFBYTES_AS_STRING(output);
	if ((AlifUSizeT)size != strlen(data)) {
		//alifErr_setString(_alifExcValueError_, "embedded null byte");
		ALIF_DECREF(output);
		return 0;
	}
	*(AlifObject**)_addr = output;
	return ALIF_CLEANUP_SUPPORTED;
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


const char* alifUStr_getDefaultEncoding(void) { // 4277
	return "utf-8";
}


static AlifIntT uStrDecode_callErrorHandlerWriter(
	const char* _errors, AlifObject** _errorHandler,
	const char* _encoding, const char* _reason,
	const char** _input, const char** _inend, AlifSizeT* _startinpos,
	AlifSizeT* _endinpos, AlifObject** _exceptionObject, const char** _inptr,
	AlifUStrWriter* _writer) { // 4436
	static const char* argparse = "Un;decoding error handler must return (str, AlifIntT) tuple";

	AlifObject* resTuple = nullptr;
	AlifObject* repUnicode = nullptr;
	AlifSizeT inSize{};
	AlifSizeT newPos{};
	AlifSizeT replen{};
	AlifSizeT remain{};
	AlifObject* inputObj = nullptr;
	AlifIntT needToGrow = 0;
	const char* newInptr{};

	if (*_errorHandler == nullptr) {
		//*_errorHandler = alifCodec_lookupError(_errors); //* todo
		if (*_errorHandler == nullptr)
			goto onError;
	}

	//make_decodeException(_exceptionObject, _encoding, *_input,
	//	*_inend - *_input, *_startinpos, *_endinpos, _reason); //* todo
	if (*_exceptionObject == nullptr)
		goto onError;

	resTuple = alifObject_callOneArg(*_errorHandler, *_exceptionObject);
	if (resTuple == nullptr)
		goto onError;
	if (!ALIFTUPLE_CHECK(resTuple)) {
		//alifErr_setString(alifExcTypeError, &argparse[3]);
		goto onError;
	}
	if (!alifArg_parseTuple(resTuple, argparse, &repUnicode, &newPos))
		goto onError;

	//inputObj = alifUStrDecodeError_getObject(*_exceptionObject);
	//if (!inputObj)
	//	goto onError;
	remain = *_inend - *_input - *_endinpos;
	*_input = ALIFBYTES_AS_STRING(inputObj);
	inSize = ALIFBYTES_GET_SIZE(inputObj);
	*_inend = *_input + inSize;

	ALIF_DECREF(inputObj);

	if (newPos < 0)
		newPos = inSize + newPos;
	if (newPos<0 or newPos>inSize) {
		//alifErr_format(alifExcIndexError, "position %zd from error handler out of bounds", newpos);
		goto onError;
	}

	replen = ALIFUSTR_GET_LENGTH(repUnicode);
	if (replen > 1) {
		_writer->minLength += replen - 1;
		needToGrow = 1;
	}
	newInptr = *_input + newPos;
	if (*_inend - newInptr > remain) {
		_writer->minLength += *_inend - newInptr - remain;
		needToGrow = 1;
	}
	if (needToGrow) {
		_writer->overAllocate = 1;
		if (ALIFUSTRWRITER_PREPARE(_writer, _writer->minLength - _writer->pos,
			ALIFUSTR_MAX_CHAR_VALUE(repUnicode)) == -1)
			goto onError;
	}
	if (alifUStrWriter_writeStr(_writer, repUnicode) == -1)
		goto onError;

	*_endinpos = newPos;
	*_inptr = newInptr;

	/* we made it! */
	ALIF_DECREF(resTuple);
	return 0;

onError:
	ALIF_XDECREF(resTuple);
	return -1;
}



/* ------------------------------------------ UTF-8 Codec ------------------------------------------ */

AlifObject* alifUStr_decodeUTF8(const char* _str,
	AlifSizeT _size, const char* _errors) { // 4938
	return alifUStr_decodeUTF8Stateful(_str, _size, _errors, nullptr);
}

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
			AlifUSizeT value = *(const AlifUSizeT*)_p;
			if (value & ASCII_CHAR_MASK)
				break;
			*((AlifUSizeT*)q) = value;
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
				AlifUSizeT value = *(const AlifUSizeT*)_p;
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
	const char* _s, const char* end, AlifErrorHandler_ error_handler,
	const char* errors, AlifSizeT* consumed) { // 5028

	AlifSizeT startInPos{}, endInpos{};
	const char* errMsg = "";
	AlifObject* errorHandlerObj = nullptr;
	AlifObject* exc = nullptr;

	while (_s < end) {
		AlifUCS4 ch_{};
		AlifIntT kind = writer->kind;

		if (kind == AlifUStrKind_::AlifUStr_1Byte_Kind) {
			if (ALIFUSTR_IS_ASCII(writer->buffer))
				ch_ = asciiLib_utf8Decode(&_s, end, (AlifUCS1*)writer->data, &writer->pos);
			else
				ch_ = ucs1Lib_utf8Decode(&_s, end, (AlifUCS1*)writer->data, &writer->pos);
		}
		else if (kind == AlifUStrKind_::AlifUStr_2Byte_Kind) {
			ch_ = ucs2Lib_utf8Decode(&_s, end, (AlifUCS2*)writer->data, &writer->pos);
		}
		else {
			ch_ = ucs4Lib_utf8Decode(&_s, end, (AlifUCS4*)writer->data, &writer->pos);
		}

		switch (ch_) {
		case 0:
			if (_s == end or consumed)
				goto End;
			errMsg = "نهاية بيانات غير صحيحة";
			startInPos = _s - starts;
			endInpos = end - starts;
			break;
		case 1:
			errMsg = "خطأ في بايت البداية";
			startInPos = _s - starts;
			endInpos = startInPos + 1;
			break;
		case 2:
			if (consumed and (unsigned char)_s[0] == 0xED and end - _s == 2
				and (unsigned char)_s[1] >= 0xA0 and (unsigned char)_s[1] <= 0xBF)
			{
				/* Truncated surrogate code in range D800-DFFF */
				goto End;
			}
			//ALIF_FALLTHROUGH;
		case 3:
		case 4:
			errMsg = "امتداد بايت غير صحيح";
			startInPos = _s - starts;
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
			_s += (endInpos - startInPos);
			break;

		case AlifErrorHandler_::Alif_Error_Replace:
			if (alifUStrWriter_writeCharInline(writer, 0xfffd) < 0)
				goto onError;
			_s += (endInpos - startInPos);
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
			_s += (endInpos - startInPos);
			break;
		}

		default:
			//if (unicodeDecode_callErrorHandlerWriter(
			//	errors, &errorHandlerObj,
			//	"utf-8", errMsg,
			//	&starts, &end, &startInPos, &endInpos, &exc, &_s,
			//	writer)) {
			//	goto onError;
			//}

			if (ALIFUSTRWRITER_PREPARE(writer, end - _s, 127) < 0) {
				return -1;
			}
		}
	}

End:
	if (consumed)
		*consumed = _s - starts;

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



static AlifIntT uStr_decodeUTF8Writer(AlifUStrWriter* writer, const char* s,
	AlifSizeT size, AlifErrorHandler_ error_handler,
	const char* errors, AlifSizeT* consumed) { // 5204
	if (size == 0) {
		if (consumed) {
			*consumed = 0;
		}
		return 0;
	}

	// fast path: try ASCII string.
	if (ALIFUSTRWRITER_PREPARE(writer, size, 127) < 0) {
		return -1;
	}

	const char* starts = s;
	const char* end = s + size;
	AlifSizeT decoded = 0;
	AlifUCS1* dest = (AlifUCS1*)writer->data + writer->pos * writer->kind;
	if (writer->kind == AlifUStrKind_::AlifUStr_1Byte_Kind) {
		decoded = ascii_decode(s, end, dest);
		writer->pos += decoded;

		if (decoded == size) {
			if (consumed) {
				*consumed = size;
			}
			return 0;
		}
		s += decoded;
		size -= decoded;
	}

	return unicode_decodeUTF8Impl(writer, starts, s, end,
		error_handler, errors, consumed);
}



AlifObject* alifUStr_decodeUTF8Stateful(const char* _s, AlifSizeT size,
	const char* errors, AlifSizeT* consumed) { // 5245
	return unicode_decodeUTF8(_s, size, AlifErrorHandler_::Alif_Error_Unknown, errors, consumed);
}


AlifIntT alif_decodeUTF8Ex(const char* _s, AlifSizeT size, wchar_t** wstr, AlifUSizeT* wlen,
	const char** reason, AlifErrorHandler_ errors) { // 5267
	const char* origs = _s;
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
	e_ = _s + size;
	outPos = 0;
	while (_s < e_) {
		AlifUCS4 ch_;
#if SIZEOF_WCHAR_T == 4
		ch_ = ucs4Lib_utf8Decode(&_s, e_, (AlifUCS4*)uStr, &outPos);
#else
		ch_ = ucs2Lib_utf8Decode(&_s, e_, (AlifUCS2*)uStr, &outPos);
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
			if (!ch_ and _s == e_) {
				break;
			}

			if (surrogateescape) {
				uStr[outPos++] = 0xDC00 + (unsigned char)*_s++;
			}
			else {
				/* Is it a valid three-byte code? */
				if (surrogatepass
					and (e_ - _s) >= 3
					and (_s[0] & 0xf0) == 0xe0
					and (_s[1] & 0xc0) == 0x80
					and (_s[2] & 0xc0) == 0x80)
				{
					ch_ = ((_s[0] & 0x0f) << 12) + ((_s[1] & 0x3f) << 6) + (_s[2] & 0x3f);
					_s += 3;
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
						*wlen = _s - origs;
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


static AlifObject* uStr_encodeUtf8(AlifObject* unicode,
	AlifErrorHandler_ error_handler, const char* errors) { // 5538
	if (!ALIFUSTR_CHECK(unicode)) {
		//alifErr_badArgument();
		return nullptr;
	}

	if (ALIFUSTR_UTF8(unicode))
		return alifBytes_fromStringAndSize(ALIFUSTR_UTF8(unicode),
			ALIFUSTR_UTF8_LENGTH(unicode));

	AlifIntT kind = ALIFUSTR_KIND(unicode);
	const void* data = ALIFUSTR_DATA(unicode);
	AlifSizeT size = ALIFUSTR_GET_LENGTH(unicode);

	AlifBytesWriter writer{};
	char* end{};

	switch (kind) {
	default:
		ALIF_UNREACHABLE();
	case AlifUStrKind_::AlifUStr_1Byte_Kind:
		end = ucs1Lib_utf8Encoder(&writer, unicode, (AlifUCS1*)data, size, error_handler, errors);
		break;
	case AlifUStrKind_::AlifUStr_2Byte_Kind:
		end = ucs2Lib_utf8Encoder(&writer, unicode, (AlifUCS2*)data, size, error_handler, errors);
		break;
	case AlifUStrKind_::AlifUStr_4Byte_Kind:
		end = ucs4Lib_utf8Encoder(&writer, unicode, (AlifUCS4*)data, size, error_handler, errors);
		break;
	}

	if (end == nullptr) {
		alifBytesWriter_dealloc(&writer);
		return nullptr;
	}
	return alifBytesWriter_finish(&writer, end);
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
	_ALIFUSTR_UTF8(_uStr) = cache;
	ALIFUSTR_UTF8_LENGTH(_uStr) = len;
	memcpy(cache, start, len);
	cache[len] = '\0';
	alifBytesWriter_dealloc(&writer);
	return 0;
}



AlifObject* _alifUStr_asUTF8String(AlifObject* unicode, const char* errors) { // 5633
	return uStr_encodeUtf8(unicode, AlifErrorHandler_::Alif_Error_Unknown, errors);
}



AlifObject* _alifUStr_encodeUTF32(AlifObject* str,
	const char* errors, AlifIntT byteorder) { // 5802
	AlifIntT kind{};
	const void* data{};
	AlifSizeT len{};
	AlifObject* v{};
	uint32_t* out;
#if ALIF_LITTLE_ENDIAN
	AlifIntT native_ordering = byteorder <= 0;
#else
	AlifIntT native_ordering = byteorder >= 0;
#endif
	const char* encoding{};
	AlifSizeT nsize{}, pos{};
	AlifObject* errorHandler = nullptr;
	AlifObject* exc = nullptr;
	AlifObject* rep = nullptr;

	if (!ALIFUSTR_CHECK(str)) {
		//alifErr_badArgument();
		return nullptr;
	}
	kind = ALIFUSTR_KIND(str);
	data = ALIFUSTR_DATA(str);
	len = ALIFUSTR_GET_LENGTH(str);

	if (len > ALIF_SIZET_MAX / 4 - (byteorder == 0))
	{
		//return alifErr_noMemory();
	}
	nsize = len + (byteorder == 0);
	v = alifBytes_fromStringAndSize(nullptr, nsize * 4);
	if (v == nullptr)
		return nullptr;

	/* output buffer is 4-bytes aligned */
	out = (uint32_t*)ALIFBYTES_AS_STRING(v);
	if (byteorder == 0)
		*out++ = 0xFEFF;
	if (len == 0)
		goto done;

	if (byteorder == -1)
		encoding = "utf-32-le";
	else if (byteorder == 1)
		encoding = "utf-32-be";
	else
		encoding = "utf-32";

	if (kind == AlifUStrKind_::AlifUStr_1Byte_Kind) {
		ucs1Lib_utf32Encode((const AlifUCS1*)data, len, &out, native_ordering);
		goto done;
	}

	pos = 0;
	while (pos < len) {
		AlifSizeT newpos{}, repsize{}, moreunits{};

		if (kind == AlifUStrKind_::AlifUStr_2Byte_Kind) {
			pos += ucs2Lib_utf32Encode((const AlifUCS2*)data + pos, len - pos,
				&out, native_ordering);
		}
		else {
			pos += ucs4Lib_utf32Encode((const AlifUCS4*)data + pos, len - pos,
				&out, native_ordering);
		}
		if (pos == len)
			break;

		rep = uStr_encodeCallErrorhandler(
			errors, &errorHandler,
			encoding, "surrogates not allowed",
			str, &exc, pos, pos + 1, &newpos);
		if (!rep)
			goto error;

		if (ALIFBYTES_CHECK(rep)) {
			repsize = ALIFBYTES_GET_SIZE(rep);
			if (repsize & 3) {
				//raise_encodeException(&exc, encoding,
				//	str, pos, pos + 1,
				//	"surrogates not allowed");
				goto error;
			}
			moreunits = repsize / 4;
		}
		else {
			moreunits = repsize = ALIFUSTR_GET_LENGTH(rep);
			if (!ALIFUSTR_IS_ASCII(rep)) {
				//raise_encodeException(&exc, encoding,
				//	str, pos, pos + 1,
				//	"surrogates not allowed");
				goto error;
			}
		}
		moreunits += pos - newpos;
		pos = newpos;

		/* four bytes are reserved for each surrogate */
		if (moreunits > 0) {
			AlifSizeT outpos = out - (uint32_t*)ALIFBYTES_AS_STRING(v);
			if (moreunits >= (ALIF_SIZET_MAX - ALIFBYTES_GET_SIZE(v)) / 4) {
				/* integer overflow */
				//alifErr_noMemory();
				goto error;
			}
			if (alifBytes_resize(&v, ALIFBYTES_GET_SIZE(v) + 4 * moreunits) < 0)
				goto error;
			out = (uint32_t*)ALIFBYTES_AS_STRING(v) + outpos;
		}

		if (ALIFBYTES_CHECK(rep)) {
			memcpy(out, ALIFBYTES_AS_STRING(rep), repsize);
			out += repsize / 4;
		}
		else /* rep is unicode */ {
			ucs1Lib_utf32Encode(ALIFUSTR_1BYTE_DATA(rep), repsize,
				&out, native_ordering);
		}

		ALIF_CLEAR(rep);
	}

	nsize = (unsigned char*)out - (unsigned char*)ALIFBYTES_AS_STRING(v);
	if (nsize != ALIFBYTES_GET_SIZE(v))
		alifBytes_resize(&v, nsize);
	ALIF_XDECREF(errorHandler);
	ALIF_XDECREF(exc);
done:
	return v;
error:
	ALIF_XDECREF(rep);
	ALIF_XDECREF(errorHandler);
	ALIF_XDECREF(exc);
	ALIF_XDECREF(v);
	return nullptr;
}





AlifObject* alifUStr_decodeUTF32(const char* s, AlifSizeT size,
	const char* errors, AlifIntT* byteorder) { // 5648
	return alifUStr_decodeUTF32Stateful(s, size, errors, byteorder, nullptr);
}

AlifObject* alifUStr_decodeUTF32Stateful(const char* _s, AlifSizeT _size,
	const char* _errors, AlifIntT* _byteOrder, AlifSizeT* _consumed) { // 5658
	const char* starts = _s;
	AlifSizeT startInPos{};
	AlifSizeT endInPos{};
	AlifUStrWriter writer{};
	const unsigned char* q_{}, * e{};
	AlifIntT le_, bo_ = 0;       /* assume native ordering by default */
	const char* encoding{};
	const char* errmsg = "";
	AlifObject* errorHandler = nullptr;
	AlifObject* exc_ = nullptr;

	q_ = (const unsigned char*)_s;
	e = q_ + _size;

	if (_byteOrder)
		bo_ = *_byteOrder;
	if (bo_ == 0 and _size >= 4) {
		AlifUCS4 bom_ = ((unsigned int)q_[3] << 24) | (q_[2] << 16) | (q_[1] << 8) | q_[0];
		if (bom_ == 0x0000FEFF) {
			bo_ = -1;
			q_ += 4;
		}
		else if (bom_ == 0xFFFE0000) {
			bo_ = 1;
			q_ += 4;
		}
		if (_byteOrder)
			*_byteOrder = bo_;
	}

	if (q_ == e) {
		if (_consumed)
			*_consumed = _size;
			ALIF_RETURN_UNICODE_EMPTY;
	}

#ifdef WORDS_BIGENDIAN
	le_ = bo_ < 0;
#else
	le_ = bo_ <= 0;
#endif
	encoding = le_ ? "utf-32-le" : "utf-32-be";

	alifUStrWriter_init(&writer);
	writer.minLength = (e - q_ + 3) / 4;
	if (ALIFUSTRWRITER_PREPARE(&writer, writer.minLength, 127) == -1)
		goto onError;

	while (1) {
		AlifUCS4 ch_ = 0;
		AlifUCS4 maxCh = ALIFUSTR_MAX_CHAR_VALUE(writer.buffer);

		if (e - q_ >= 4) {
			AlifIntT kind = writer.kind;
			void* data = writer.data;
			const unsigned char* last = e - 4;
			AlifSizeT pos_ = writer.pos;
			if (le_) {
				do {
					ch_ = ((unsigned int)q_[3] << 24) | (q_[2] << 16) | (q_[1] << 8) | q_[0];
					if (ch_ > maxCh)
						break;
					if (kind != AlifUStrKind_::AlifUStr_1Byte_Kind and
						alifUnicode_isSurrogate(ch_))
						break;
					ALIFUSTR_WRITE(kind, data, pos_++, ch_);
					q_ += 4;
				} while (q_ <= last);
			}
			else {
				do {
					ch_ = ((unsigned int)q_[0] << 24) | (q_[1] << 16) | (q_[2] << 8) | q_[3];
					if (ch_ > maxCh)
						break;
					if (kind != AlifUStrKind_::AlifUStr_1Byte_Kind and
						alifUnicode_isSurrogate(ch_))
						break;
					ALIFUSTR_WRITE(kind, data, pos_++, ch_);
					q_ += 4;
				} while (q_ <= last);
			}
			writer.pos = pos_;
		}

		if (alifUnicode_isSurrogate(ch_)) {
			errmsg = "code point in surrogate code point range(0xd800, 0xe000)";
			startInPos = ((const char*)q_) - starts;
			endInPos = startInPos + 4;
		}
		else if (ch_ <= maxCh) {
			if (q_ == e or _consumed)
				break;
			errmsg = "truncated data";
			startInPos = ((const char*)q_) - starts;
			endInPos = ((const char*)e) - starts;
		}
		else {
			if (ch_ < 0x110000) {
				if (alifUStrWriter_writeCharInline(&writer, ch_) < 0)
					goto onError;
				q_ += 4;
				continue;
			}
			errmsg = "code point not in range(0x110000)";
			startInPos = ((const char*)q_) - starts;
			endInPos = startInPos + 4;
		}

		if (uStrDecode_callErrorHandlerWriter(
			_errors, &errorHandler,
			encoding, errmsg,
			&starts, (const char**)&e, &startInPos, &endInPos, &exc_, (const char**)&q_,
			&writer))
			goto onError;
	}

	if (_consumed)
		*_consumed = (const char*)q_ - starts;

	ALIF_XDECREF(errorHandler);
	ALIF_XDECREF(exc_);
	return alifUStrWriter_finish(&writer);

onError:
	alifUStrWriter_dealloc(&writer);
	ALIF_XDECREF(errorHandler);
	ALIF_XDECREF(exc_);
	return nullptr;
}






AlifObject* alifUStr_decodeUTF16(const char* s, AlifSizeT size,
	const char* errors, AlifIntT* byteorder) { // 5955
	return alifUStr_decodeUTF16Stateful(s, size, errors, byteorder, nullptr);
}

AlifObject* alifUStr_decodeUTF16Stateful(const char* s, AlifSizeT size,
	const char* errors, AlifIntT* byteorder, AlifSizeT* consumed) { // 5964
	const char* starts = s;
	AlifSizeT startinpos{};
	AlifSizeT endinpos{};
	AlifUStrWriter writer{};
	const unsigned char* q{}, * e{};
	AlifIntT bo = 0;       /* assume native ordering by default */
	AlifIntT native_ordering;
	const char* errmsg = "";
	AlifObject* errorHandler = nullptr;
	AlifObject* exc = nullptr;
	const char* encoding{};

	q = (const unsigned char*)s;
	e = q + size;

	if (byteorder)
		bo = *byteorder;

	if (bo == 0 and size >= 2) {
		const AlifUCS4 bom = (q[1] << 8) | q[0];
		if (bom == 0xFEFF) {
			q += 2;
			bo = -1;
		}
		else if (bom == 0xFFFE) {
			q += 2;
			bo = 1;
		}
		if (byteorder)
			*byteorder = bo;
	}

	if (q == e) {
		if (consumed)
			*consumed = size;
		ALIF_RETURN_UNICODE_EMPTY;
	}

#if ALIF_LITTLE_ENDIAN
	native_ordering = bo <= 0;
	encoding = bo <= 0 ? "utf-16-le" : "utf-16-be";
#else
	native_ordering = bo >= 0;
	encoding = bo >= 0 ? "utf-16-be" : "utf-16-le";
#endif

	alifUStrWriter_init(&writer);
	writer.minLength = (e - q + 1) / 2;
	if (ALIFUSTRWRITER_PREPARE(&writer, writer.minLength, 127) == -1)
		goto onError;

	while (1) {
		AlifUCS4 ch = 0;
		if (e - q >= 2) {
			int kind = writer.kind;
			if (kind == AlifUStrKind_::AlifUStr_1Byte_Kind) {
				if (ALIFUSTR_IS_ASCII(writer.buffer))
					ch = asciiLib_utf16Decode(&q, e,
						(AlifUCS1*)writer.data, &writer.pos,
						native_ordering);
				else
					ch = ucs1Lib_utf16Decode(&q, e,
						(AlifUCS1*)writer.data, &writer.pos,
						native_ordering);
			}
			else if (kind == AlifUStrKind_::AlifUStr_2Byte_Kind) {
				ch = ucs2Lib_utf16Decode(&q, e,
					(AlifUCS2*)writer.data, &writer.pos,
					native_ordering);
			}
			else {
				ch = ucs4Lib_utf16Decode(&q, e,
					(AlifUCS4*)writer.data, &writer.pos,
					native_ordering);
			}
		}

		switch (ch)
		{
		case 0:
			/* remaining byte at the end? (size should be even) */
			if (q == e or consumed)
				goto End;
			errmsg = "truncated data";
			startinpos = ((const char*)q) - starts;
			endinpos = ((const char*)e) - starts;
			break;
			/* The remaining input chars are ignored if the callback
			   chooses to skip the input */
		case 1:
			q -= 2;
			if (consumed)
				goto End;
			errmsg = "unexpected end of data";
			startinpos = ((const char*)q) - starts;
			endinpos = ((const char*)e) - starts;
			break;
		case 2:
			errmsg = "illegal encoding";
			startinpos = ((const char*)q) - 2 - starts;
			endinpos = startinpos + 2;
			break;
		case 3:
			errmsg = "illegal UTF-16 surrogate";
			startinpos = ((const char*)q) - 4 - starts;
			endinpos = startinpos + 2;
			break;
		default:
			if (alifUStrWriter_writeCharInline(&writer, ch) < 0)
				goto onError;
			continue;
		}

		if (uStrDecode_callErrorHandlerWriter(
			errors, &errorHandler, encoding, errmsg, &starts,
			(const char**)&e, &startinpos, &endinpos,
			&exc, (const char**)&q, &writer))
			goto onError;
	}

End:
	if (consumed)
		*consumed = (const char*)q - starts;

	ALIF_XDECREF(errorHandler);
	ALIF_XDECREF(exc);
	return alifUStrWriter_finish(&writer);

onError:
	alifUStrWriter_dealloc(&writer);
	ALIF_XDECREF(errorHandler);
	ALIF_XDECREF(exc);
	return nullptr;
}



AlifObject* _alifUStr_encodeUTF16(AlifObject* str,
	const char* errors, AlifIntT byteorder) { // 6119
	AlifIntT kind{};
	const void* data{};
	AlifSizeT len{};
	AlifObject* v{};
	unsigned short* out{};
	AlifSizeT pairs{};
#if ALIF_BIG_ENDIAN
	AlifIntT native_ordering = byteorder >= 0;
#else
	AlifIntT native_ordering = byteorder <= 0;
#endif
	const char* encoding{};
	AlifSizeT nsize{}, pos{};
	AlifObject* errorHandler = nullptr;
	AlifObject* exc = nullptr;
	AlifObject* rep = nullptr;

	if (!ALIFUSTR_CHECK(str)) {
		//alifErr_badArgument();
		return nullptr;
	}
	kind = ALIFUSTR_KIND(str);
	data = ALIFUSTR_DATA(str);
	len = ALIFUSTR_GET_LENGTH(str);

	pairs = 0;
	if (kind == AlifUStrKind_::AlifUStr_4Byte_Kind) {
		const AlifUCS4* in = (const AlifUCS4*)data;
		const AlifUCS4* end = in + len;
		while (in < end) {
			if (*in++ >= 0x10000) {
				pairs++;
			}
		}
	}
	if (len > ALIF_SIZET_MAX / 2 - pairs - (byteorder == 0)) {
		//return alifErr_noMemory();
	}
	nsize = len + pairs + (byteorder == 0);
	v = alifBytes_fromStringAndSize(nullptr, nsize * 2);
	if (v == nullptr) {
		return nullptr;
	}

	/* output buffer is 2-bytes aligned */
	out = (unsigned short*)ALIFBYTES_AS_STRING(v);
	if (byteorder == 0) {
		*out++ = 0xFEFF;
	}
	if (len == 0) {
		goto done;
	}

	if (kind == AlifUStrKind_::AlifUStr_1Byte_Kind) {
		ucs1Lib_utf16Encode((const AlifUCS1*)data, len, &out, native_ordering);
		goto done;
	}

	if (byteorder < 0) {
		encoding = "utf-16-le";
	}
	else if (byteorder > 0) {
		encoding = "utf-16-be";
	}
	else {
		encoding = "utf-16";
	}

	pos = 0;
	while (pos < len) {
		AlifSizeT newpos{}, repsize{}, moreunits{};

		if (kind == AlifUStrKind_::AlifUStr_2Byte_Kind) {
			pos += ucs2Lib_utf16Encode((const AlifUCS2*)data + pos, len - pos,
				&out, native_ordering);
		}
		else {
			pos += ucs4Lib_utf16Encode((const AlifUCS4*)data + pos, len - pos,
				&out, native_ordering);
		}
		if (pos == len)
			break;

		rep = uStr_encodeCallErrorhandler(
			errors, &errorHandler,
			encoding, "surrogates not allowed",
			str, &exc, pos, pos + 1, &newpos);
		if (!rep)
			goto error;

		if (ALIFBYTES_CHECK(rep)) {
			repsize = ALIFBYTES_GET_SIZE(rep);
			if (repsize & 1) {
				//raise_encodeException(&exc, encoding,
				//	str, pos, pos + 1,
				//	"surrogates not allowed");
				goto error;
			}
			moreunits = repsize / 2;
		}
		else {
			moreunits = repsize = ALIFUSTR_GET_LENGTH(rep);
			if (!ALIFUSTR_IS_ASCII(rep)) {
				//raise_encodeException(&exc, encoding,
				//	str, pos, pos + 1,
				//	"surrogates not allowed");
				goto error;
			}
		}
		moreunits += pos - newpos;
		pos = newpos;

		/* two bytes are reserved for each surrogate */
		if (moreunits > 0) {
			AlifSizeT outpos = out - (unsigned short*)ALIFBYTES_AS_STRING(v);
			if (moreunits >= (ALIF_SIZET_MAX - ALIFBYTES_GET_SIZE(v)) / 2) {
				/* integer overflow */
				//alifErr_noMemory();
				goto error;
			}
			if (alifBytes_resize(&v, ALIFBYTES_GET_SIZE(v) + 2 * moreunits) < 0)
				goto error;
			out = (unsigned short*)ALIFBYTES_AS_STRING(v) + outpos;
		}

		if (ALIFBYTES_CHECK(rep)) {
			memcpy(out, ALIFBYTES_AS_STRING(rep), repsize);
			out += repsize / 2;
		}
		else /* rep is unicode */ {
			ucs1Lib_utf16Encode(ALIFUSTR_1BYTE_DATA(rep), repsize,
				&out, native_ordering);
		}

		ALIF_CLEAR(rep);
	}

	nsize = (unsigned char*)out - (unsigned char*)ALIFBYTES_AS_STRING(v);
	if (nsize != ALIFBYTES_GET_SIZE(v))
		alifBytes_resize(&v, nsize);
	ALIF_XDECREF(errorHandler);
	ALIF_XDECREF(exc);
done:
	return v;
error:
	ALIF_XDECREF(rep);
	ALIF_XDECREF(errorHandler);
	ALIF_XDECREF(exc);
	ALIF_XDECREF(v);
	return nullptr;
#undef STORECHAR
}


/* --- Unicode Escape Codec ----------------------------------------------- */

AlifObject* alifUStr_decodeUStrEscapeInternal(const char* _str, AlifSizeT _size,
	const char* _errors, AlifSizeT* _consumed, const char** _firstInvalidEscape) { // 6308
	const char* starts = _str;
	AlifUStrWriter writer{};
	const char* end{};
	AlifObject* errorHandler = nullptr;
	AlifObject* exc = nullptr;
	//AlifUStrNameCAPI* ucnhash_capi{};

	// so we can remember if we've seen an invalid escape char or not
	*_firstInvalidEscape = nullptr;

	if (_size == 0) {
		if (_consumed) {
			*_consumed = 0;
		}
		ALIF_RETURN_UNICODE_EMPTY
	}
	/* Escaped strings will always be longer than the resulting
	   Unicode string, so we start with size here and then reduce the
	   length after conversion to the true value.
	   (but if the error callback returns a long replacement string
	   we'll have to allocate more space) */
	alifUStrWriter_init(&writer);
	writer.minLength = _size;
	if (ALIFUSTRWRITER_PREPARE(&writer, _size, 127) < 0) {
		goto onError;
	}

	end = _str + _size;
	while (_str < end) {
		unsigned char c = (unsigned char)*_str++;
		AlifUCS4 ch{};
		AlifIntT count{};
		const char* message{};

#define WRITE_ASCII_CHAR(_ch)                                                  \
            do {                                                              \
                ALIFUSTR_WRITE(writer.kind, writer.data, writer.pos++, _ch);  \
            } while(0)

#define WRITE_CHAR(_ch)                                                        \
            do {                                                              \
                if (ch <= writer.maxChar) {                                   \
                    ALIFUSTR_WRITE(writer.kind, writer.data, writer.pos++, _ch); \
                }                                                             \
                else if (alifUStrWriter_writeCharInline(&writer, _ch) < 0) { \
                    goto onError;                                             \
                }                                                             \
            } while(0)

		/* Non-escape characters are interpreted as Unicode ordinals */
		if (c != '\\') {
			WRITE_CHAR(c);
			continue;
		}

		AlifSizeT startinpos = _str - starts - 1;
		/* \ - Escapes */
		if (_str >= end) {
			message = "\\ at end of string";
			goto incomplete;
		}
		c = (unsigned char)*_str++;

		switch (c) {

			/* \x escapes */
		case '\n': continue;
		case '\\': WRITE_ASCII_CHAR('\\'); continue;
		case '\'': WRITE_ASCII_CHAR('\''); continue;
		case '\"': WRITE_ASCII_CHAR('\"'); continue;
		case 'b': WRITE_ASCII_CHAR('\b'); continue;
			/* FF */
		case 'f': WRITE_ASCII_CHAR('\014'); continue;
		case 't': WRITE_ASCII_CHAR('\t'); continue;
		case 'n': WRITE_ASCII_CHAR('\n'); continue;
		case 'r': WRITE_ASCII_CHAR('\r'); continue;
			/* VT */
		case 'v': WRITE_ASCII_CHAR('\013'); continue;
			/* BEL, not classic C */
		case 'a': WRITE_ASCII_CHAR('\007'); continue;

			/* \OOO (octal) escapes */
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
			ch = c - '0';
			if (_str < end and '0' <= *_str and *_str <= '7') {
				ch = (ch << 3) + *_str++ - '0';
				if (_str < end and '0' <= *_str and *_str <= '7') {
					ch = (ch << 3) + *_str++ - '0';
				}
			}
			if (ch > 0377) {
				if (*_firstInvalidEscape == nullptr) {
					*_firstInvalidEscape = _str - 3; /* Back up 3 chars, since we've
													already incremented _s. */
				}
			}
			WRITE_CHAR(ch);
			continue;

			/* hex escapes */
			/* \xXX */
		case 'x':
			count = 2;
			message = "truncated \\xXX escape";
			goto hexescape;

			/* \uXXXX */
		case 'u':
			count = 4;
			message = "truncated \\uXXXX escape";
			goto hexescape;

			/* \UXXXXXXXX */
		case 'U':
			count = 8;
			message = "truncated \\UXXXXXXXX escape";
		hexescape:
			for (ch = 0; count; ++_str, --count) {
				if (_str >= end) {
					goto incomplete;
				}
				c = (unsigned char)*_str;
				ch <<= 4;
				if (c >= '0' and c <= '9') {
					ch += c - '0';
				}
				else if (c >= 'a' and c <= 'f') {
					ch += c - ('a' - 10);
				}
				else if (c >= 'A' and c <= 'F') {
					ch += c - ('A' - 10);
				}
				else {
					goto error;
				}
			}

			/* when we get here, ch is a 32-bit unicode character */
			if (ch > MAX_UNICODE) {
				message = "illegal Unicode character";
				goto error;
			}

			WRITE_CHAR(ch);
			continue;

			/* \N{name} */
		case 'N':
			//ucnhash_capi = _alifUStr_getNameCAPI();
			//if (ucnhash_capi == nullptr) {
			//	alifErr_setString(
			//		_alifExcUnicodeError_,
			//		"\\N escapes not supported (can't load unicodedata module)"
			//	);
			//	goto onError;
			//}

			message = "malformed \\N character escape";
			if (_str >= end) {
				goto incomplete;
			}
			if (*_str == '{') {
				const char* start = ++_str;
				size_t namelen;
				/* look for the closing brace */
				while (_str < end and *_str != '}')
					_str++;
				if (_str >= end) {
					goto incomplete;
				}
				namelen = _str - start;
				if (namelen) {
					/* found a name.  look it up in the unicode database */
					_str++;
					ch = 0xffffffff; /* in case 'getcode' messes up */
					if (namelen <= INT_MAX ) {
						WRITE_CHAR(ch);
						continue;
					}
					message = "unknown Unicode character name";
				}
			}
			goto error;

		default:
			if (*_firstInvalidEscape == nullptr) {
				*_firstInvalidEscape = _str - 1; /* Back up one char, since we've
												already incremented _s. */
			}
			WRITE_ASCII_CHAR('\\');
			WRITE_CHAR(c);
			continue;
		}

	incomplete:
		if (_consumed) {
			*_consumed = startinpos;
			break;
		}
	error:;
		AlifSizeT endinpos = _str - starts;
		writer.minLength = end - _str + writer.pos;
		//if (uStr_decodeCallErrorhandlerWriter(
		//	_errors, &errorHandler,
		//	"unicodeescape", message,
		//	&starts, &end, &startinpos, &endinpos, &exc, &_str,
		//	&writer)) {
		//	goto onError;
		//}

#undef WRITE_ASCII_CHAR
#undef WRITE_CHAR
	}

	ALIF_XDECREF(errorHandler);
	ALIF_XDECREF(exc);
	return alifUStrWriter_finish(&writer);

onError:
	alifUStrWriter_dealloc(&writer);
	ALIF_XDECREF(errorHandler);
	ALIF_XDECREF(exc);
	return nullptr;
}



static AlifObject* uStr_encodeCallErrorhandler(const char* errors,
	AlifObject** errorHandler, const char* encoding, const char* reason,
	AlifObject* unicode, AlifObject** exceptionObject, AlifSizeT startpos,
	AlifSizeT endpos, AlifSizeT* newpos) { // 6988
	static const char* argparse = "On;encoding error handler must return (str/bytes, int) tuple";
	AlifSizeT len{};
	AlifObject* restuple{};
	AlifObject* resunicode{};

	if (*errorHandler == nullptr) {
		//*errorHandler = alifCodec_lookupError(errors);
		if (*errorHandler == nullptr)
			return nullptr;
	}

	len = ALIFUSTR_GET_LENGTH(unicode);

	//make_encodeException(exceptionObject,
	//	encoding, unicode, startpos, endpos, reason);
	if (*exceptionObject == nullptr)
		return nullptr;

	restuple = alifObject_callOneArg(*errorHandler, *exceptionObject);
	if (restuple == nullptr)
		return nullptr;
	if (!ALIFTUPLE_CHECK(restuple)) {
		//alifErr_setString(_alifExcTypeError_, &argparse[3]);
		ALIF_DECREF(restuple);
		return nullptr;
	}
	if (!alifArg_parseTuple(restuple, argparse,
		&resunicode, newpos)) {
		ALIF_DECREF(restuple);
		return nullptr;
	}
	if (!ALIFUSTR_CHECK(resunicode) and !ALIFBYTES_CHECK(resunicode)) {
		//alifErr_setString(_alifExcTypeError_, &argparse[3]);
		ALIF_DECREF(restuple);
		return nullptr;
	}
	if (*newpos < 0)
		*newpos = len + *newpos;
	if (*newpos<0 || *newpos>len) {
		//alifErr_format(_alifExcIndexError_, "position %zd from error handler out of bounds", *newpos);
		ALIF_DECREF(restuple);
		return nullptr;
	}
	ALIF_INCREF(resunicode);
	ALIF_DECREF(restuple);
	return resunicode;
}



static AlifObject* uStr_encodeUcs1(AlifObject* unicode,
	const char* errors, const AlifUCS4 limit) { // 7044
	/* input state */
	AlifSizeT pos = 0, size{};
	AlifIntT kind{};
	const void* data{};
	/* pointer into the output */
	char* str{};
	const char* encoding = (limit == 256) ? "latin-1" : "ascii";
	const char* reason = (limit == 256) ? "ordinal not in range(256)" : "ordinal not in range(128)";
	AlifObject* error_handler_obj = nullptr;
	AlifObject* exc = nullptr;
	AlifErrorHandler_ error_handler = AlifErrorHandler_::Alif_Error_Unknown;
	AlifObject* rep = nullptr;
	/* output object */
	AlifBytesWriter writer{};

	size = ALIFUSTR_GET_LENGTH(unicode);
	kind = ALIFUSTR_KIND(unicode);
	data = ALIFUSTR_DATA(unicode);
	/* allocate enough for a simple encoding without
	   replacements, if we need more, we'll resize */
	if (size == 0)
		return alifBytes_fromStringAndSize(nullptr, 0);

	alifBytesWriter_init(&writer);
	str = (char*)alifBytesWriter_alloc(&writer, size);
	if (str == nullptr)
		return nullptr;

	while (pos < size) {
		AlifUCS4 ch = ALIFUSTR_READ(kind, data, pos);

		/* can we encode this? */
		if (ch < limit) {
			/* no overflow check, because we know that the space is enough */
			*str++ = (char)ch;
			++pos;
		}
		else {
			AlifSizeT newpos{}, i{};
			/* startpos for collecting unencodable chars */
			AlifSizeT collstart = pos;
			AlifSizeT collend = collstart + 1;
			/* find all unecodable characters */

			while ((collend < size) and (ALIFUSTR_READ(kind, data, collend) >= limit))
				++collend;

			/* Only overallocate the buffer if it's not the last write */
			writer.overAllocate = (collend < size);

			/* cache callback name lookup (if not done yet, i.e. it's the first error) */
			if (error_handler == AlifErrorHandler_::Alif_Error_Unknown)
				error_handler = alif_getErrorHandler(errors);

			switch (error_handler) {
			case AlifErrorHandler_::Alif_Error_Strict:
				//raise_encodeException(&exc, encoding, unicode, collstart, collend, reason);
				goto onError;

			case AlifErrorHandler_::Alif_Error_Replace:
				memset(str, '?', collend - collstart);
				str += (collend - collstart);
				ALIF_FALLTHROUGH;
			case AlifErrorHandler_::Alif_Error_Ignore:
				pos = collend;
				break;

			case AlifErrorHandler_::Alif_Error_BackSlashReplace:
				/* subtract preallocated bytes */
				writer.minSize -= (collend - collstart);
				str = backSlash_replace(&writer, str,
					unicode, collstart, collend);
				if (str == nullptr)
					goto onError;
				pos = collend;
				break;

			case AlifErrorHandler_::Alif_Error_XMLCharRefReplace:
				/* subtract preallocated bytes */
				writer.minSize -= (collend - collstart);
				str = xmlCharRef_replace(&writer, str,
					unicode, collstart, collend);
				if (str == nullptr)
					goto onError;
				pos = collend;
				break;

			case AlifErrorHandler_::Alif_Error_SurrogateEscape:
				for (i = collstart; i < collend; ++i) {
					ch = ALIFUSTR_READ(kind, data, i);
					if (ch < 0xdc80 || 0xdcff < ch) {
						/* Not a UTF-8b surrogate */
						break;
					}
					*str++ = (char)(ch - 0xdc00);
					++pos;
				}
				if (i >= collend)
					break;
				collstart = pos;
				ALIF_FALLTHROUGH;

			default:
				rep = uStr_encodeCallErrorhandler(errors, &error_handler_obj,
					encoding, reason, unicode, &exc,
					collstart, collend, &newpos);
				if (rep == nullptr)
					goto onError;

				if (newpos < collstart) {
					writer.overAllocate = 1;
					str = (char*)alifBytesWriter_prepare(&writer, str,
						collstart - newpos);
					if (str == nullptr)
						goto onError;
				}
				else {
					/* subtract preallocated bytes */
					writer.minSize -= newpos - collstart;
					/* Only overallocate the buffer if it's not the last write */
					writer.overAllocate = (newpos < size);
				}

				if (ALIFBYTES_CHECK(rep)) {
					/* Directly copy bytes result to output. */
					str = (char*)alifBytesWriter_writeBytes(&writer, str,
						ALIFBYTES_AS_STRING(rep),
						ALIFBYTES_GET_SIZE(rep));
				}
				else {

					if (limit == 256 ?
						ALIFUSTR_KIND(rep) != AlifUStrKind_::AlifUStr_1Byte_Kind :
						!ALIFUSTR_IS_ASCII(rep))
					{
						/* Not all characters are smaller than limit */
						//raise_encodeException(&exc, encoding, unicode,
						//	collstart, collend, reason);
						goto onError;
					}
					str = (char*)alifBytesWriter_writeBytes(&writer, str,
						ALIFUSTR_DATA(rep),
						ALIFUSTR_GET_LENGTH(rep));
				}
				if (str == nullptr)
					goto onError;

				pos = newpos;
				ALIF_CLEAR(rep);
			}
		}
	}

	ALIF_XDECREF(error_handler_obj);
	ALIF_XDECREF(exc);
	return alifBytesWriter_finish(&writer, str);

onError:
	ALIF_XDECREF(rep);
	alifBytesWriter_dealloc(&writer);
	ALIF_XDECREF(error_handler_obj);
	ALIF_XDECREF(exc);
	return nullptr;
}


AlifObject* _alifUStr_asLatin1String(AlifObject* unicode, const char* errors) { // 7221
	if (!ALIFUSTR_CHECK(unicode)) {
		//alifErr_badArgument();
		return nullptr;
	}
	if (ALIFUSTR_KIND(unicode) == AlifUStrKind_::AlifUStr_1Byte_Kind)
		return alifBytes_fromStringAndSize((const char*)ALIFUSTR_DATA(unicode),
			ALIFUSTR_GET_LENGTH(unicode));
	return uStr_encodeUcs1(unicode, errors, 256);
}





AlifObject* _alifUStr_asASCIIString(AlifObject* unicode, const char* errors) { // 7345
	if (!ALIFUSTR_CHECK(unicode)) {
		//alifErr_badArgument();
		return nullptr;
	}
	/* Fast path: if it is an ASCII-only string, construct bytes object
	   directly. Else defer to above function to raise the exception. */
	if (ALIFUSTR_IS_ASCII(unicode))
		return alifBytes_fromStringAndSize((const char*)ALIFUSTR_DATA(unicode),
			ALIFUSTR_GET_LENGTH(unicode));
	return uStr_encodeUcs1(unicode, errors, 128);
}



#ifdef _WINDOWS // 7366

/* --- MBCS codecs for Windows -------------------------------------------- */


 // 7370
#if SIZEOF_INT < SIZEOF_SIZE_T
#define NEED_RETRY
#endif

 // 7378
#define DECODING_CHUNK_SIZE (INT_MAX/4)




static DWORD encode_codePageFlags(UINT code_page, const char* errors) { // 7684
	if (code_page == CP_UTF8) {
		return WC_ERR_INVALID_CHARS;
	}
	else if (code_page == CP_UTF7) {
		/* CP_UTF7 only supports flags=0 */
		return 0;
	}
	else {
		if (errors != nullptr and strcmp(errors, "replace") == 0)
			return 0;
		else
			return WC_NO_BEST_FIT_CHARS;
	}
}


static AlifIntT encode_codePageStrict(UINT code_page, AlifObject** outbytes,
	AlifObject* unicode, AlifSizeT offset, AlifIntT len, const char* errors) { // 7712
	BOOL usedDefaultChar = FALSE;
	BOOL* pusedDefaultChar = &usedDefaultChar;
	AlifIntT outsize{};
	wchar_t* p{};
	AlifSizeT size;
	const DWORD flags = encode_codePageFlags(code_page, nullptr);
	char* out{};
	AlifObject* substring{};
	AlifIntT ret = -1;


	if (code_page != CP_UTF8 && code_page != CP_UTF7)
		pusedDefaultChar = &usedDefaultChar;
	else
		pusedDefaultChar = nullptr;

	substring = alifUStr_subString(unicode, offset, offset + len);
	if (substring == nullptr)
		return -1;
	p = alifUStr_asWideCharString(substring, &size);
	ALIF_CLEAR(substring);
	if (p == nullptr) {
		return -1;
	}

	/* First get the size of the result */
	outsize = WideCharToMultiByte(code_page, flags,
		p, (int)size,
		nullptr, 0,
		nullptr, pusedDefaultChar);
	if (outsize <= 0)
		goto error;
	/* If we used a default char, then we failed! */
	if (pusedDefaultChar and *pusedDefaultChar) {
		ret = -2;
		goto done;
	}

	if (*outbytes == nullptr) {
		/* Create string object */
		*outbytes = alifBytes_fromStringAndSize(nullptr, outsize);
		if (*outbytes == nullptr) {
			goto done;
		}
		out = ALIFBYTES_AS_STRING(*outbytes);
	}
	else {
		/* Extend string object */
		const AlifSizeT n = alifBytes_size(*outbytes);
		if (outsize > ALIF_SIZET_MAX - n) {
			//alifErr_noMemory();
			goto done;
		}
		if (alifBytes_resize(outbytes, n + outsize) < 0) {
			goto done;
		}
		out = ALIFBYTES_AS_STRING(*outbytes) + n;
	}

	/* Do the conversion */
	outsize = WideCharToMultiByte(code_page, flags,
		p, (int)size,
		out, outsize,
		nullptr, pusedDefaultChar);
	if (outsize <= 0)
		goto error;
	if (pusedDefaultChar && *pusedDefaultChar) {
		ret = -2;
		goto done;
	}
	ret = 0;

done:
	alifMem_dataFree(p);
	return ret;

error:
	if (GetLastError() == ERROR_NO_UNICODE_TRANSLATION) {
		ret = -2;
		goto done;
	}
	//alifErr_setFromWindowsErr(0);
	goto done;
}


static AlifObject* encode_codePage(AlifIntT code_page,
	AlifObject* unicode, const char* errors) { // 7992
	AlifSizeT len{};
	AlifObject* outbytes = nullptr;
	AlifSizeT offset{};
	AlifIntT chunk_len{}, ret{}, done{};

	if (!ALIFUSTR_CHECK(unicode)) {
		//alifErr_badArgument();
		return nullptr;
	}

	len = ALIFUSTR_GET_LENGTH(unicode);

	if (code_page < 0) {
		//alifErr_setString(_alifExcValueError_, "invalid code page number");
		return nullptr;
	}

	if (len == 0)
		return alifBytes_fromStringAndSize(nullptr, 0);

	offset = 0;
	do
	{
#ifdef NEED_RETRY
		if (len > DECODING_CHUNK_SIZE) {
			chunk_len = DECODING_CHUNK_SIZE;
			done = 0;
		}
		else
#endif
		{
			chunk_len = (int)len;
			done = 1;
		}

		ret = encode_codePageStrict(code_page, &outbytes,
			unicode, offset, chunk_len,
			errors);
		//if (ret == -2)
		//	ret = encode_codePageErrors(code_page, &outbytes,
		//		unicode, offset,
		//		chunk_len, errors);
		if (ret < 0) {
			ALIF_XDECREF(outbytes);
			return nullptr;
		}

		offset += chunk_len;
		len -= chunk_len;
	} while (!done);

	return outbytes;
}





AlifObject* alifUStr_encodeCodePage(AlifIntT code_page,
	AlifObject* unicode, const char* errors) { // 8051
	return encode_codePage(code_page, unicode, errors);
}




#endif /* _WINDOWS */ // 8067








AlifObject* _alifUStr_transformDecimalAndSpaceToASCII(AlifObject* unicode) { // 9285
	if (!ALIFUSTR_CHECK(unicode)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	if (ALIFUSTR_CHECK(unicode) //* alif //* review //* todo
		or ALIFUSTR_IS_ASCII(unicode)) {
		/* If the string is already ASCII, just return the same string */
		return ALIF_NEWREF(unicode);
	}

	AlifSizeT len = ALIFUSTR_GET_LENGTH(unicode);
	AlifObject* result = alifUStr_new(len, 127);
	if (result == nullptr) {
		return nullptr;
	}

	AlifUCS1* out = ALIFUSTR_1BYTE_DATA(result);
	AlifIntT kind = ALIFUSTR_KIND(unicode);
	const void* data = ALIFUSTR_DATA(unicode);
	AlifSizeT i{};
	for (i = 0; i < len; ++i) {
		AlifUCS4 ch = ALIFUSTR_READ(kind, data, i);
		if (ch < 127) {
			out[i] = ch;
		}
		else if (alifUStr_isSpace(ch)) {
			out[i] = ' ';
		}
		else {
			AlifIntT decimal = ALIF_USTR_TODECIMAL(ch);
			if (decimal < 0) {
				out[i] = '?';
				out[i + 1] = '\0';
				ALIFUSTR_LENGTH(result) = i + 1;
				break;
			}
			out[i] = '0' + decimal;
		}
	}

	return result;
}




// 9318
#define ADJUST_INDICES(_start, _end, _len) \
    do {                                \
        if (_end > _len) {                \
            _end = _len;                  \
        }                               \
        else if (_end < 0) {             \
            _end += _len;                 \
            if (_end < 0) {              \
                _end = 0;                \
            }                           \
        }                               \
        if (_start < 0) {                \
            _start += _len;               \
            if (_start < 0) {            \
                _start = 0;              \
            }                           \
        }                               \
    } while (0)

AlifSizeT alifUStr_findChar(AlifObject* _str, AlifUCS4 _ch,
	AlifSizeT _start, AlifSizeT _end, AlifIntT _direction) { // 9582
	AlifIntT kind{};
	AlifSizeT len_{}, result{};
	len_ = ALIFUSTR_GET_LENGTH(_str);
	ADJUST_INDICES(_start, _end, len_);
	if (_end - _start < 1)
		return -1;
	kind = ALIFUSTR_KIND(_str);
	result = findChar(ALIFUSTR_1BYTE_DATA(_str) + kind * _start,
		kind, _end - _start, _ch, _direction);
	if (result == -1)
		return -1;
	else
		return _start + result;
}


AlifObject* alifUStr_join(AlifObject* _separator, AlifObject* _seq) { // 9910
	AlifObject* res_{};
	AlifObject* fSeq{};
	AlifSizeT seqLen{};
	AlifObject** items{};

	fSeq = alifSequence_fast(_seq, "can only join an iterable");
	if (fSeq == nullptr) {
		return nullptr;
	}

	ALIF_BEGIN_CRITICAL_SECTION_SEQUENCE_FAST(_seq);

	items = ALIFSEQUENCE_FAST_ITEMS(fSeq);
	seqLen = ALIFSEQUENCE_FAST_GET_SIZE(fSeq);
	res_ = alifUStr_joinArray(_separator, items, seqLen);

	ALIF_END_CRITICAL_SECTION_SEQUENCE_FAST();

	ALIF_DECREF(fSeq);
	return res_;
}

AlifObject* alifUStr_joinArray(AlifObject* _separator,
	AlifObject* const* _items, AlifSizeT _seqLen) { // 9935
	AlifObject* res_ = nullptr; /* the result */
	AlifObject* sep_ = nullptr;
	AlifSizeT sepLen{};
	AlifObject* item{};
	AlifSizeT sz_{}, i_{}, resOffset{};
	AlifUCS4 maxChar{};
	AlifUCS4 itemMaxChar{};
	AlifIntT useMemCpy{};
	unsigned char* resData = nullptr, * sepData = nullptr;
	AlifObject* lastObj{};
	AlifIntT kind = 0;

	if (_seqLen == 0) {
		ALIF_RETURN_UNICODE_EMPTY;
	}

	lastObj = nullptr;
	if (_seqLen == 1) {
		if (ALIFUSTR_CHECKEXACT(_items[0])) {
			res_ = _items[0];
			return ALIF_NEWREF(res_);
		}
		sepLen = 0;
		maxChar = 0;
	}
	else {
		if (_separator == nullptr) {
			sep_ = alifUStr_fromOrdinal(' ');
			if (!sep_)
				goto onError;
			sepLen = 1;
			maxChar = 32;
		}
		else {
			if (!ALIFUSTR_CHECK(_separator)) {
				//alifErr_format(_alifExcTypeError_,
					//"separator: expected str instance,"
					//" %.80s found",
					//ALIF_TYPE(_separator)->name);
				goto onError;
			}
			sep_ = _separator;
			sepLen = ALIFUSTR_GET_LENGTH(_separator);
			maxChar = ALIFUSTR_MAX_CHAR_VALUE(_separator);

			ALIF_INCREF(sep_);
		}
		lastObj = sep_;
	}

	sz_ = 0;
	useMemCpy = 1;
	for (i_ = 0; i_ < _seqLen; i_++) {
		AlifUSizeT addSZ{};
		item = _items[i_];
		if (!ALIFUSTR_CHECK(item)) {
			//alifErr_format(_alifExcTypeError_,
				//"sequence item %zd: expected str instance,"
				//" %.80s found",
				//i, ALIF_TYPE(item)->name);
			goto onError;
		}
		addSZ = ALIFUSTR_GET_LENGTH(item);
		itemMaxChar = ALIFUSTR_MAX_CHAR_VALUE(item);
		maxChar = ALIF_MAX(maxChar, itemMaxChar);
		if (i_ != 0) {
			addSZ += sepLen;
		}
		if (addSZ > (AlifUSizeT)(ALIF_SIZET_MAX - sz_)) {
			//alifErr_setString(_alifExcOverflowError_,
				//"join() result is too long for a Alif string");
			goto onError;
		}
		sz_ += addSZ;
		if (useMemCpy and lastObj != nullptr) {
			if (ALIFUSTR_KIND(lastObj) != ALIFUSTR_KIND(item))
				useMemCpy = 0;
		}
		lastObj = item;
	}

	res_ = alifUStr_new(sz_, maxChar);
	if (res_ == nullptr)
		goto onError;

	/* Catenate everything. */
	if (useMemCpy) {
		resData = ALIFUSTR_1BYTE_DATA(res_);
		kind = ALIFUSTR_KIND(res_);
		if (sepLen != 0)
			sepData = ALIFUSTR_1BYTE_DATA(sep_);
	}

	if (useMemCpy) {
		for (i_ = 0; i_ < _seqLen; ++i_) {
			AlifSizeT itemlen{};
			item = _items[i_];

			/* Copy item, and maybe the separator. */
			if (i_ and sepLen != 0) {
				memcpy(resData,
					sepData,
					kind * sepLen);
				resData += kind * sepLen;
			}

			itemlen = ALIFUSTR_GET_LENGTH(item);
			if (itemlen != 0) {
				memcpy(resData,
					ALIFUSTR_DATA(item),
					kind * itemlen);
				resData += kind * itemlen;
			}
		}
	}
	else {
		for (i_ = 0, resOffset = 0; i_ < _seqLen; ++i_) {
			AlifSizeT itemlen{};
			item = _items[i_];

			if (i_ and sepLen != 0) {
				alifUStr_fastCopyCharacters(res_, resOffset, sep_, 0, sepLen);
				resOffset += sepLen;
			}

			itemlen = ALIFUSTR_GET_LENGTH(item);
			if (itemlen != 0) {
				alifUStr_fastCopyCharacters(res_, resOffset, item, 0, itemlen);
				resOffset += itemlen;
			}
		}
	}

	ALIF_XDECREF(sep_);
	return res_;

onError:
	ALIF_XDECREF(sep_);
	ALIF_XDECREF(res_);
	return nullptr;
}



void _alifUStr_fastFill(AlifObject* _unicode, AlifSizeT _start,
	AlifSizeT _length, AlifUCS4 _fillChar) { // 10101
	const AlifIntT kind = ALIFUSTR_KIND(_unicode);
	void* data = ALIFUSTR_DATA(_unicode);
	uStr_fill(kind, data, _fillChar, _start, _length);
}

AlifSizeT alifUStr_fill(AlifObject* _unicode, AlifSizeT _start,
	AlifSizeT _length, AlifUCS4 _fillChar) { // 10114
	AlifSizeT maxlen{};

	if (!ALIFUSTR_CHECK(_unicode)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	if (uStr_checkModifiable(_unicode))
		return -1;

	if (_start < 0) {
		//alifErr_setString(_alifExcIndexError_, "string index out of range");
		return -1;
	}
	if (_fillChar > ALIFUSTR_MAX_CHAR_VALUE(_unicode)) {
		//alifErr_setString(_alifExcValueError_,
		//	"fill character is bigger than "
		//	"the string maximum character");
		return -1;
	}

	maxlen = ALIFUSTR_GET_LENGTH(_unicode) - _start;
	_length = ALIF_MIN(maxlen, _length);
	if (_length <= 0)
		return 0;

	_alifUStr_fastFill(_unicode, _start, _length, _fillChar);
	return _length;
}


static AlifSizeT anyLib_find(AlifIntT _kind, AlifObject* _str1,
	const void* _buf1, AlifSizeT _len1, AlifObject* _str2,
	const void* _buf2, AlifSizeT _len2, AlifSizeT _offset) { // 10407
	switch (_kind) {
	case AlifUStrKind_::AlifUStr_1Byte_Kind:
		if (ALIFUSTR_IS_ASCII(_str1) and ALIFUSTR_IS_ASCII(_str2))
			return asciiLib_find((const AlifUCS1*)_buf1, _len1, (const AlifUCS1*)_buf2, _len2, _offset);
		else
			return ucs1Lib_find((const AlifUCS1*)_buf1, _len1, (const AlifUCS1*)_buf2, _len2, _offset);
	case AlifUStrKind_::AlifUStr_2Byte_Kind:
		return ucs2Lib_find((const AlifUCS2*)_buf1, _len1, (const AlifUCS2*)_buf2, _len2, _offset);
	case AlifUStrKind_::AlifUStr_4Byte_Kind:
		return ucs4Lib_find((const AlifUCS4*)_buf1, _len1, (const AlifUCS4*)_buf2, _len2, _offset);
	}
	ALIF_UNREACHABLE();
}

static AlifSizeT anyLib_count(AlifIntT _kind, AlifObject* _sstr,
	const void* _sbuf, AlifSizeT _slen, AlifObject* _str1,
	const void* _buf1, AlifSizeT _len1, AlifSizeT _maxCount) { // 10425
	switch (_kind) {
	case AlifUStrKind_::AlifUStr_1Byte_Kind:
		return ucs1Lib_count((const AlifUCS1*)_sbuf, _slen, (const AlifUCS1*)_buf1, _len1, _maxCount);
	case AlifUStrKind_::AlifUStr_2Byte_Kind:
		return ucs2Lib_count((const AlifUCS2*)_sbuf, _slen, (const AlifUCS2*)_buf1, _len1, _maxCount);
	case AlifUStrKind_::AlifUStr_4Byte_Kind:
		return ucs4Lib_count((const AlifUCS4*)_sbuf, _slen, (const AlifUCS4*)_buf1, _len1, _maxCount);
	}
	ALIF_UNREACHABLE();
}

static void replace_1charInplace(AlifObject* _u, AlifSizeT _pos,
	AlifUCS4 _u1, AlifUCS4 _u2, AlifSizeT _maxCount) { // 10440
	AlifIntT kind = ALIFUSTR_KIND(_u);
	void* data = ALIFUSTR_DATA(_u);
	AlifSizeT len = ALIFUSTR_GET_LENGTH(_u);
	if (kind == AlifUStrKind_::AlifUStr_1Byte_Kind) {
		ucs1Lib_replace1CharInplace((AlifUCS1*)data + _pos,
			(AlifUCS1*)data + len,
			_u1, _u2, _maxCount);
	}
	else if (kind == AlifUStrKind_::AlifUStr_2Byte_Kind) {
		ucs2Lib_replace1CharInplace((AlifUCS2*)data + _pos,
			(AlifUCS2*)data + len,
			_u1, _u2, _maxCount);
	}
	else {
		ucs4Lib_replace1CharInplace((AlifUCS4*)data + _pos,
			(AlifUCS4*)data + len,
			_u1, _u2, _maxCount);
	}
}


static AlifObject* replace(AlifObject* _self, AlifObject* _str1,
	AlifObject* _str2, AlifSizeT _maxCount) { // 10465

	AlifObject* u{};
	const char* sbuf = (const char*)ALIFUSTR_DATA(_self);
	const void* buf1 = ALIFUSTR_DATA(_str1);
	const void* buf2 = ALIFUSTR_DATA(_str2);
	AlifIntT srelease = 0, release1 = 0, release2 = 0;
	AlifIntT skind = ALIFUSTR_KIND(_self);
	AlifIntT kind1 = ALIFUSTR_KIND(_str1);
	AlifIntT kind2 = ALIFUSTR_KIND(_str2);
	AlifSizeT slen = ALIFUSTR_GET_LENGTH(_self);
	AlifSizeT len1 = ALIFUSTR_GET_LENGTH(_str1);
	AlifSizeT len2 = ALIFUSTR_GET_LENGTH(_str2);
	AlifIntT mayshrink{};
	AlifUCS4 maxchar{}, maxchar_str1{}, maxchar_str2{};

	if (slen < len1)
		goto nothing;

	if (_maxCount < 0)
		_maxCount = ALIF_SIZET_MAX;
	else if (_maxCount == 0)
		goto nothing;

	if (_str1 == _str2)
		goto nothing;

	maxchar = ALIFUSTR_MAX_CHAR_VALUE(_self);
	maxchar_str1 = ALIFUSTR_MAX_CHAR_VALUE(_str1);
	if (maxchar < maxchar_str1)
		/* substring too wide to be present */
		goto nothing;
	maxchar_str2 = ALIFUSTR_MAX_CHAR_VALUE(_str2);
	/* Replacing str1 with str2 may cause a maxchar reduction in the
	   result string. */
	mayshrink = (maxchar_str2 < maxchar_str1) and (maxchar == maxchar_str1);
	maxchar = ALIF_MAX(maxchar, maxchar_str2);

	if (len1 == len2) {
		/* same length */
		if (len1 == 0)
			goto nothing;
		if (len1 == 1) {
			/* replace characters */
			AlifUCS4 u1{}, u2{};
			AlifSizeT pos{};

			u1 = ALIFUSTR_READ(kind1, buf1, 0);
			pos = findChar(sbuf, skind, slen, u1, 1);
			if (pos < 0)
				goto nothing;
			u2 = ALIFUSTR_READ(kind2, buf2, 0);
			u = alifUStr_new(slen, maxchar);
			if (!u)
				goto error;

			alifUStr_fastCopyCharacters(u, 0, _self, 0, slen);
			replace_1charInplace(u, pos, u1, u2, _maxCount);
		}
		else {
			AlifIntT rkind = skind;
			char* res{};
			AlifSizeT i{};

			if (kind1 < rkind) {
				/* widen substring */
				buf1 = unicode_asKind(kind1, buf1, len1, rkind);
				if (!buf1) goto error;
				release1 = 1;
			}
			i = anyLib_find(rkind, _self, sbuf, slen, _str1, buf1, len1, 0);
			if (i < 0)
				goto nothing;
			if (rkind > kind2) {
				/* widen replacement */
				buf2 = unicode_asKind(kind2, buf2, len2, rkind);
				if (!buf2) goto error;
				release2 = 1;
			}
			else if (rkind < kind2) {
				/* widen self and buf1 */
				rkind = kind2;
				if (release1) {
					alifMem_dataFree((void*)buf1);
					buf1 = ALIFUSTR_DATA(_str1);
					release1 = 0;
				}
				sbuf = (const char*)unicode_asKind(skind, sbuf, slen, rkind);
				if (!sbuf) goto error;
				srelease = 1;
				buf1 = unicode_asKind(kind1, buf1, len1, rkind);
				if (!buf1) goto error;
				release1 = 1;
			}
			u = alifUStr_new(slen, maxchar);
			if (!u)
				goto error;
			res = (char*)ALIFUSTR_DATA(u);

			memcpy(res, sbuf, rkind * slen);
			/* change everything in-place, starting with this one */
			memcpy(res + rkind * i,
				buf2,
				rkind * len2);
			i += len1;

			while (--_maxCount > 0) {
				i = anyLib_find(rkind, _self,
					sbuf + rkind * i, slen - i,
					_str1, buf1, len1, i);
				if (i == -1)
					break;
				memcpy(res + rkind * i,
					buf2,
					rkind * len2);
				i += len1;
			}
		}
	}
	else {
		AlifSizeT n, i, j, ires;
		AlifSizeT new_size;
		AlifIntT rkind = skind;
		char* res;

		if (kind1 < rkind) {
			/* widen substring */
			buf1 = unicode_asKind(kind1, buf1, len1, rkind);
			if (!buf1) goto error;
			release1 = 1;
		}
		n = anyLib_count(rkind, _self, sbuf, slen, _str1, buf1, len1, _maxCount);
		if (n == 0)
			goto nothing;
		if (kind2 < rkind) {
			/* widen replacement */
			buf2 = unicode_asKind(kind2, buf2, len2, rkind);
			if (!buf2) goto error;
			release2 = 1;
		}
		else if (kind2 > rkind) {
			/* widen self and buf1 */
			rkind = kind2;
			sbuf = (const char*)unicode_asKind(skind, sbuf, slen, rkind);
			if (!sbuf) goto error;
			srelease = 1;
			if (release1) {
				alifMem_dataFree((void*)buf1);
				buf1 = ALIFUSTR_DATA(_str1);
				release1 = 0;
			}
			buf1 = unicode_asKind(kind1, buf1, len1, rkind);
			if (!buf1) goto error;
			release1 = 1;
		}
		/* new_size = ALIFUSTR_GET_LENGTH(self) + n * (ALIFUSTR_GET_LENGTH(str2) -
		   ALIFUSTR_GET_LENGTH(str1)); */
		if (len1 < len2 and len2 - len1 >(ALIF_SIZET_MAX - slen) / n) {
			//alifErr_setString(_alifExcOverflowError_,
			//	"replace string is too long");
			goto error;
		}
		new_size = slen + n * (len2 - len1);
		if (new_size == 0) {
			u = unicode_getEmpty();
			goto done;
		}
		if (new_size > (ALIF_SIZET_MAX / rkind)) {
			//alifErr_setString(_alifExcOverflowError_,
			//	"replace string is too long");
			goto error;
		}
		u = alifUStr_new(new_size, maxchar);
		if (!u)
			goto error;
		res = (char*)ALIFUSTR_DATA(u);
		ires = i = 0;
		if (len1 > 0) {
			while (n-- > 0) {
				/* look for next match */
				j = anyLib_find(rkind, _self,
					sbuf + rkind * i, slen - i,
					_str1, buf1, len1, i);
				if (j == -1)
					break;
				else if (j > i) {
					/* copy unchanged part [i:j] */
					memcpy(res + rkind * ires,
						sbuf + rkind * i,
						rkind * (j - i));
					ires += j - i;
				}
				/* copy substitution string */
				if (len2 > 0) {
					memcpy(res + rkind * ires,
						buf2,
						rkind * len2);
					ires += len2;
				}
				i = j + len1;
			}
			if (i < slen)
				/* copy tail [i:] */
				memcpy(res + rkind * ires,
					sbuf + rkind * i,
					rkind * (slen - i));
		}
		else {
			/* interleave */
			while (n > 0) {
				memcpy(res + rkind * ires,
					buf2,
					rkind * len2);
				ires += len2;
				if (--n <= 0)
					break;
				memcpy(res + rkind * ires,
					sbuf + rkind * i,
					rkind);
				ires++;
				i++;
			}
			memcpy(res + rkind * ires,
				sbuf + rkind * i,
				rkind * (slen - i));
		}
	}

	if (mayshrink) {
		uStr_adjustMaxChar(&u);
		if (u == nullptr)
			goto error;
	}

done:
	if (srelease)
		alifMem_dataFree((void*)sbuf);
	if (release1)
		alifMem_dataFree((void*)buf1);
	if (release2)
		alifMem_dataFree((void*)buf2);
	return u;

nothing:
	/* nothing to replace; return original string (when possible) */
	if (srelease)
		alifMem_dataFree((void*)sbuf);
	if (release1)
		alifMem_dataFree((void*)buf1);
	if (release2)
		alifMem_dataFree((void*)buf2);
	return uStr_resultUnchanged(_self);

error:
	if (srelease)
		alifMem_dataFree((void*)sbuf);
	if (release1)
		alifMem_dataFree((void*)buf1);
	if (release2)
		alifMem_dataFree((void*)buf2);
	return nullptr;
}



static AlifIntT uStr_compare(AlifObject* _str1, AlifObject* _str2) { // 10847
#define COMPARE(_type1, _type2) \
    do { \
        _type1* p1 = (_type1 *)data1; \
        _type2* p2 = (_type2 *)data2; \
        _type1* end = p1 + len; \
        AlifUCS4 c1, c2; \
        for (; p1 != end; p1++, p2++) { \
            c1 = *p1; \
            c2 = *p2; \
            if (c1 != c2) \
                return (c1 < c2) ? -1 : 1; \
        } \
    } \
    while (0)

	AlifIntT kind1{}, kind2{};
	const void* data1{}, * data2{};
	AlifSizeT len1{}, len2{}, len{};

	kind1 = ALIFUSTR_KIND(_str1);
	kind2 = ALIFUSTR_KIND(_str2);
	data1 = ALIFUSTR_DATA(_str1);
	data2 = ALIFUSTR_DATA(_str2);
	len1 = ALIFUSTR_GET_LENGTH(_str1);
	len2 = ALIFUSTR_GET_LENGTH(_str2);
	len = ALIF_MIN(len1, len2);

	switch (kind1) {
	case AlifUStrKind_::AlifUStr_1Byte_Kind:
	{
		switch (kind2) {
		case AlifUStrKind_::AlifUStr_1Byte_Kind:
		{
			AlifIntT cmp_ = memcmp(data1, data2, len);
			if (cmp_ < 0)
				return -1;
			if (cmp_ > 0)
				return 1;
			break;
		}
		case AlifUStrKind_::AlifUStr_2Byte_Kind:
			COMPARE(AlifUCS1, AlifUCS2);
			break;
		case AlifUStrKind_::AlifUStr_4Byte_Kind:
			COMPARE(AlifUCS1, AlifUCS4);
			break;
		default:
			ALIF_UNREACHABLE();
		}
		break;
	}
	case AlifUStrKind_::AlifUStr_2Byte_Kind:
	{
		switch (kind2) {
		case AlifUStrKind_::AlifUStr_1Byte_Kind:
			COMPARE(AlifUCS2, AlifUCS1);
			break;
		case AlifUStrKind_::AlifUStr_2Byte_Kind:
		{
			COMPARE(AlifUCS2, AlifUCS2);
			break;
		}
		case AlifUStrKind_::AlifUStr_4Byte_Kind:
			COMPARE(AlifUCS2, AlifUCS4);
			break;
		default:
			ALIF_UNREACHABLE();
		}
		break;
	}
	case AlifUStrKind_::AlifUStr_4Byte_Kind:
	{
		switch (kind2) {
		case AlifUStrKind_::AlifUStr_1Byte_Kind:
			COMPARE(AlifUCS4, AlifUCS1);
			break;
		case AlifUStrKind_::AlifUStr_2Byte_Kind:
			COMPARE(AlifUCS4, AlifUCS2);
			break;
		case AlifUStrKind_::AlifUStr_4Byte_Kind:
		{
#if defined(HAVE_WMEMCMP) and SIZEOF_WCHAR_T == 4
			AlifIntT cmp_ = wmemcmp((wchar_t*)data1, (wchar_t*)data2, len);
			if (cmp_ < 0)
				return -1;
			if (cmp_ > 0)
				return 1;
#else
			COMPARE(AlifUCS4, AlifUCS4);
#endif
			break;
		}
		default:
			ALIF_UNREACHABLE();
		}
		break;
	}
	default:
		ALIF_UNREACHABLE();
	}

	if (len1 == len2)
		return 0;
	if (len1 < len2)
		return -1;
	else
		return 1;

#undef COMPARE
}



static AlifIntT uStr_compareEq(AlifObject* _str1, AlifObject* _str2) { // 10963
	AlifIntT kind{};
	const void* data1{}, * data2{};
	AlifSizeT len_{};
	AlifIntT cmp_{};

	len_ = ALIFUSTR_GET_LENGTH(_str1);
	if (ALIFUSTR_GET_LENGTH(_str2) != len_)
		return 0;
	kind = ALIFUSTR_KIND(_str1);
	if (ALIFUSTR_KIND(_str2) != kind)
		return 0;
	data1 = ALIFUSTR_DATA(_str1);
	data2 = ALIFUSTR_DATA(_str2);

	cmp_ = memcmp(data1, data2, len_ * kind);
	return (cmp_ == 0);
}


AlifIntT _alifUStr_equal(AlifObject* _str1, AlifObject* _str2) { // 10984
	if (_str1 == _str2) {
		return 1;
	}
	return uStr_compareEq(_str1, _str2);
}


AlifIntT alifUStr_compare(AlifObject* _left, AlifObject* _right) { // 10996
	if (ALIFUSTR_CHECK(_left) and ALIFUSTR_CHECK(_right)) {
		if (_left == _right)
			return 0;

		return uStr_compare(_left, _right);
	}
	//alifErr_format(_alifExcTypeError_,
		//"Can't compare %.100s and %.100s",
		//ALIF_TYPE(_left)->name,
		//ALIF_TYPE(right)->name);
	return -1;
}

AlifIntT alifUStr_compareWithASCIIString(AlifObject* _uni, const char* _str) { // 11013
	AlifSizeT i_{};
	AlifIntT kind{};
	AlifUCS4 chr{};

	kind = ALIFUSTR_KIND(_uni);
	if (kind == AlifUStrKind_::AlifUStr_1Byte_Kind) {
		const void* data = ALIFUSTR_1BYTE_DATA(_uni);
		AlifUSizeT len1 = (AlifUSizeT)ALIFUSTR_GET_LENGTH(_uni);
		AlifUSizeT len, len2 = strlen(_str);
		AlifIntT cmp_{};

		len = ALIF_MIN(len1, len2);
		cmp_ = memcmp(data, _str, len);
		if (cmp_ != 0) {
			if (cmp_ < 0)
				return -1;
			else
				return 1;
		}
		if (len1 > len2)
			return 1; /* _uni is longer */
		if (len1 < len2)
			return -1; /* _str is longer */
		return 0;
	}
	else {
		const void* data = ALIFUSTR_DATA(_uni);
		/* Compare Unicode string and source character set string */
		for (i_ = 0; (chr = ALIFUSTR_READ(kind, data, i_)) and _str[i_]; i_++)
			if (chr != (unsigned char)_str[i_])
				return (chr < (unsigned char)(_str[i_])) ? -1 : 1;
		if (ALIFUSTR_GET_LENGTH(_uni) != i_ or chr)
			return 1; /* _uni is longer */
		if (_str[i_])
			return -1; /* _str is longer */
		return 0;
	}
}

AlifIntT alifUStr_equalToUTF8(AlifObject* _uStr, const char* _str) { // 11058
	return alifUStr_equalToUTF8AndSize(_uStr, _str, strlen(_str));
}

AlifIntT alifUStr_equalToUTF8AndSize(AlifObject* _uStr,
	const char* _str, AlifSizeT _size) { // 11064

	if (ALIFUSTR_IS_ASCII(_uStr)) {
		AlifSizeT len = ALIFUSTR_GET_LENGTH(_uStr);
		return _size == len and
			memcmp(ALIFUSTR_1BYTE_DATA(_uStr), _str, len) == 0;
	}
	if (ALIFUSTR_UTF8(_uStr) != nullptr) {
		AlifSizeT len = ALIFUSTR_UTF8_LENGTH(_uStr);
		return _size == len and
			memcmp(ALIFUSTR_UTF8(_uStr), _str, len) == 0;
	}

	AlifSizeT len = ALIFUSTR_GET_LENGTH(_uStr);
	if ((AlifUSizeT)len >= (AlifUSizeT)_size or (AlifUSizeT)len < (AlifUSizeT)_size / 4) {
		return 0;
	}
	const unsigned char* s_ = (const unsigned char*)_str;
	const unsigned char* ends = s_ + (AlifUSizeT)_size;
	AlifIntT kind = ALIFUSTR_KIND(_uStr);
	const void* data = ALIFUSTR_DATA(_uStr);
	/* Compare Unicode string and UTF-8 string */
	for (AlifSizeT i = 0; i < len; i++) {
		AlifUCS4 ch = ALIFUSTR_READ(kind, data, i);
		if (ch < 0x80) {
			if (ends == s_ or s_[0] != ch) {
				return 0;
			}
			s_ += 1;
		}
		else if (ch < 0x800) {
			if ((ends - s_) < 2 or
				s_[0] != (0xc0 | (ch >> 6)) or
				s_[1] != (0x80 | (ch & 0x3f)))
			{
				return 0;
			}
			s_ += 2;
		}
		else if (ch < 0x10000) {
			if (alifUnicode_isSurrogate(ch) or
				(ends - s_) < 3 or
				s_[0] != (0xe0 | (ch >> 12)) or
				s_[1] != (0x80 | ((ch >> 6) & 0x3f)) or
				s_[2] != (0x80 | (ch & 0x3f)))
			{
				return 0;
			}
			s_ += 3;
		}
		else {
			if ((ends - s_) < 4 or
				s_[0] != (0xf0 | (ch >> 18)) or
				s_[1] != (0x80 | ((ch >> 12) & 0x3f)) or
				s_[2] != (0x80 | ((ch >> 6) & 0x3f)) or
				s_[3] != (0x80 | (ch & 0x3f)))
			{
				return 0;
			}
			s_ += 4;
		}
	}
	return s_ == ends;
}

AlifIntT alifUStr_equalToASCIIString(AlifObject* _uStr, const char* _str) { // 11135
	AlifUSizeT len_{};
	if (!ALIFUSTR_IS_ASCII(_uStr))
		return 0;
	len_ = (AlifUSizeT)ALIFUSTR_GET_LENGTH(_uStr);
	return strlen(_str) == len_ and
		memcmp(ALIFUSTR_1BYTE_DATA(_uStr), _str, len_) == 0;
}


AlifObject* alifUStr_richCompare(AlifObject* _left, AlifObject* _right, AlifIntT _op) { // 11193
	AlifIntT result{};

	if (!ALIFUSTR_CHECK(_left) or !ALIFUSTR_CHECK(_right))
		return ALIF_NOTIMPLEMENTED;

	if (_left == _right) {
		switch (_op) {
		case ALIF_EQ:
		case ALIF_LE:
		case ALIF_GE:
			/* a string is equal to itself */
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
		result = uStr_compareEq(_left, _right);
		result ^= (_op == ALIF_NE);
		return alifBool_fromLong(result);
	}
	else {
		result = uStr_compare(_left, _right);
		ALIF_RETURN_RICHCOMPARE(result, 0, _op);
	}
}



AlifIntT alifUStr_eq(AlifObject* _aa, AlifObject* _bb) { // 11229
	return uStr_eq(_aa, _bb);
}



AlifObject* alifUStr_concat(AlifObject* _left, AlifObject* _right) { // 11295
	AlifObject* result{};
	AlifUCS4 maxChar{}, maxChar2{};
	AlifSizeT leftLen{}, rightLen{}, newLen{};

	if (ensure_uStr(_left) < 0)
		return nullptr;

	if (!ALIFUSTR_CHECK(_right)) {
		//alifErr_format(_alifExcTypeError_,
		//	"can only concatenate str (not \"%.200s\") to str",
		//	ALIF_TYPE(_right)->name);
		return nullptr;
	}

	/* Shortcuts */
	AlifObject* empty = unicode_getEmpty();  // Borrowed reference
	if (_left == empty) {
		return alifUStr_fromObject(_right);
	}
	if (_right == empty) {
		return alifUStr_fromObject(_left);
	}

	leftLen = ALIFUSTR_GET_LENGTH(_left);
	rightLen = ALIFUSTR_GET_LENGTH(_right);
	if (leftLen > ALIF_SIZET_MAX - rightLen) {
		//alifErr_setString(_alifExcOverflowError_,
		//	"strings are too large to concat");
		return nullptr;
	}
	newLen = leftLen + rightLen;

	maxChar = ALIFUSTR_MAX_CHAR_VALUE(_left);
	maxChar2 = ALIFUSTR_MAX_CHAR_VALUE(_right);
	maxChar = ALIF_MAX(maxChar, maxChar2);

	/* Concat the two Unicode strings */
	result = alifUStr_new(newLen, maxChar);
	if (result == nullptr) return nullptr;
	alifUStr_fastCopyCharacters(result, 0, _left, 0, leftLen);
	alifUStr_fastCopyCharacters(result, leftLen, _right, 0, rightLen);
	return result;
}

void alifUStr_append(AlifObject** _pLeft, AlifObject* _right) { // 11344
	AlifObject* left{}, * res{};
	AlifUCS4 maxChar{}, maxChar2{};
	AlifSizeT leftLen{}, rightLen{}, newLen{};
	AlifObject* empty{};

	if (_pLeft == nullptr) {
		//if (!alifErr_occurred())
		//	ALIFERR_BADINTERNALCALL();
		return;
	}
	left = *_pLeft;
	if (_right == nullptr or left == nullptr
		or !ALIFUSTR_CHECK(left) or !ALIFUSTR_CHECK(_right)) {
		//if (!alifErr_occurred())
		//	ALIFERR_BADINTERNALCALL();
		goto error;
	}

	/* Shortcuts */
	empty = unicode_getEmpty();  // Borrowed reference
	if (left == empty) {
		ALIF_DECREF(left);
		*_pLeft = ALIF_NEWREF(_right);
		return;
	}
	if (_right == empty) {
		return;
	}

	leftLen = ALIFUSTR_GET_LENGTH(left);
	rightLen = ALIFUSTR_GET_LENGTH(_right);
	if (leftLen > ALIF_SIZET_MAX - rightLen) {
		//alifErr_setString(_alifExcOverflowError_,
		//	"strings are too large to concat");
		goto error;
	}
	newLen = leftLen + rightLen;

	if (uStr_modifiable(left)
		and ALIFUSTR_CHECKEXACT(_right)
		and ALIFUSTR_KIND(_right) <= ALIFUSTR_KIND(left)
		and !(ALIFUSTR_IS_ASCII(left) and !ALIFUSTR_IS_ASCII(_right)))
	{
		/* append inplace */
		if (uStr_resize(_pLeft, newLen) != 0)
			goto error;

		/* copy 'right' into the newly allocated area of 'left' */
		alifUStr_fastCopyCharacters(*_pLeft, leftLen, _right, 0, rightLen);
	}
	else {
		maxChar = ALIFUSTR_MAX_CHAR_VALUE(left);
		maxChar2 = ALIFUSTR_MAX_CHAR_VALUE(_right);
		maxChar = ALIF_MAX(maxChar, maxChar2);

		/* Concat the two Unicode strings */
		res = alifUStr_new(newLen, maxChar);
		if (res == nullptr) goto error;
		alifUStr_fastCopyCharacters(res, 0, left, 0, leftLen);
		alifUStr_fastCopyCharacters(res, leftLen, _right, 0, rightLen);
		ALIF_DECREF(left);
		*_pLeft = res;
	}
	return;

error:
	ALIF_CLEAR(*_pLeft);
}


static AlifHashT uStr_hash(AlifObject* _self) { // 11663
	AlifUHashT x{};  /* Unsigned for defined overflow behavior. */

	AlifHashT hash = alifAtomic_loadSizeRelaxed(&ALIFUSTR_HASH(_self));
	if (hash != -1) {
		return hash;
	}
	x = alif_hashBuffer(ALIFUSTR_DATA(_self),
		ALIFUSTR_GET_LENGTH(_self) * ALIFUSTR_KIND(_self));

	alifAtomic_storeSizeRelaxed(&ALIFUSTR_HASH(_self), x);
	return x;
}



AlifSizeT _alifUStr_scanIdentifier(AlifObject* _self) { // 12090
	AlifSizeT i{};
	AlifSizeT len = ALIFUSTR_GET_LENGTH(_self);
	if (len == 0) {
		/* an empty string is not a valid identifier */
		return 0;
	}

	int kind = ALIFUSTR_KIND(_self);
	const void* data = ALIFUSTR_DATA(_self);
	AlifUCS4 ch = ALIFUSTR_READ(kind, data, 0);
	if (!_alifUStr_isXIDStart(ch) and ch != 0x5F /* LOW LINE */) {
		return 0;
	}

	for (i = 1; i < len; i++) {
		ch = ALIFUSTR_READ(kind, data, i);
		if (!_alifUStr_isXIDContinue(ch)) {
			return i;
		}
	}
	return i;
}


AlifIntT alifUStr_isIdentifier(AlifObject* _self) { // 12124
	AlifSizeT i = _alifUStr_scanIdentifier(_self);
	AlifSizeT len = ALIFUSTR_GET_LENGTH(_self);
	/* an empty string is not a valid identifier */
	return len and i == len;
}



AlifObject* alifUStr_subString(AlifObject* _self,
	AlifSizeT _start, AlifSizeT _end) { // 12304
	const unsigned char* data{};
	AlifIntT kind{};
	AlifSizeT length{};

	length = ALIFUSTR_GET_LENGTH(_self);
	_end = ALIF_MIN(_end, length);

	if (_start == 0 and _end == length)
		return uStr_resultUnchanged(_self);

	if (_start < 0 or _end < 0) {
		//alifErr_setString(_alifExcIndexError_, "string index out of range");
		return nullptr;
	}
	if (_start >= length or _end < _start)
		ALIF_RETURN_UNICODE_EMPTY;

	length = _end - _start;
	if (ALIFUSTR_IS_ASCII(_self)) {
		data = ALIFUSTR_1BYTE_DATA(_self);
		return alifUStr_fromASCII((const char*)(data + _start), length);
	}
	else {
		kind = ALIFUSTR_KIND(_self);
		data = ALIFUSTR_1BYTE_DATA(_self);
		return alifUStr_fromKindAndData(kind,
			data + kind * _start, length);
	}
}




AlifObject* alifUStr_replace(AlifObject* _str,
	AlifObject* _subStr, AlifObject* _replstr, AlifSizeT _maxCount) { // 12530
	if (ensure_uStr(_str) < 0 or ensure_uStr(_subStr) < 0 or
		ensure_uStr(_replstr) < 0)
		return nullptr;
	return replace(_str, _subStr, _replstr, _maxCount);
}








static AlifObject* uStr_repr(AlifObject* _UStr) { // 12621
	AlifSizeT isize = ALIFUSTR_GET_LENGTH(_UStr);
	const void* idata = ALIFUSTR_DATA(_UStr);

	/* Compute length of output, quote characters, and
	   maximum character */
	AlifSizeT osize = 0;
	AlifUCS4 maxch = 127;
	AlifSizeT squote = 0;
	AlifSizeT dquote = 0;
	AlifIntT ikind = ALIFUSTR_KIND(_UStr);
	for (AlifSizeT i = 0; i < isize; i++) {
		AlifUCS4 ch = ALIFUSTR_READ(ikind, idata, i);
		AlifSizeT incr = 1;
		switch (ch) {
		case '\'': squote++; break;
		case '"':  dquote++; break;
		case '\\': case '\t': case '\r': case '\n':
			incr = 2;
			break;
		default:
			/* Fast-path ASCII */
			if (ch < ' ' || ch == 0x7f)
				incr = 4; /* \xHH */
			else if (ch < 0x7f)
				;
			else if (ALIF_USTR_ISPRINTABLE(ch))
				maxch = (ch > maxch) ? ch : maxch;
			else if (ch < 0x100)
				incr = 4; /* \xHH */
			else if (ch < 0x10000)
				incr = 6; /* \uHHHH */
			else
				incr = 10; /* \uHHHHHHHH */
		}
		if (osize > ALIF_SIZET_MAX - incr) {
			//alifErr_setString(_alifExcOverflowError_,
			//	"string is too long to generate repr");
			return nullptr;
		}
		osize += incr;
	}

	AlifUCS4 quote = '\'';
	AlifIntT changed = (osize != isize);
	if (squote) {
		changed = 1;
		if (dquote)
			/* Both squote and dquote present. Use squote,
			   and escape them */
			osize += squote;
		else
			quote = '"';
	}
	osize += 2;   /* quotes */

	AlifObject* repr = alifUStr_new(osize, maxch);
	if (repr == nullptr)
		return nullptr;
	AlifIntT okind = ALIFUSTR_KIND(repr);
	void* odata = ALIFUSTR_DATA(repr);

	if (!changed) {
		ALIFUSTR_WRITE(okind, odata, 0, quote);

		alifUStr_fastCopyCharacters(repr, 1,
			_UStr, 0,
			isize);

		ALIFUSTR_WRITE(okind, odata, osize - 1, quote);
	}
	else {
		switch (okind) {
		case AlifUStrKind_::AlifUStr_1Byte_Kind:
			ucs1Lib_repr(_UStr, quote, (AlifUCS1*)odata);
			break;
		case AlifUStrKind_::AlifUStr_2Byte_Kind:
			ucs2Lib_repr(_UStr, quote, (AlifUCS2*)odata);
			break;
		default:
			ucs4Lib_repr(_UStr, quote, (AlifUCS4*)odata);
		}
	}

	return repr;
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


void alifUStrWriter_init(AlifUStrWriter* _writer) { // 13388
	memset(_writer, 0, sizeof(*_writer));
	/* ASCII is the bare minimum */
	_writer->minChar = 127;
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
			and newlen <= (ALIF_SIZET_MAX - newlen / OVERALLOCATE_FACTOR)) {
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

AlifIntT alifUStrWriter_writeChar(AlifUStrWriter* _writer,
	AlifUCS4 _ch) { // 13558
	return alifUStrWriter_writeCharInline(_writer, _ch);
}


AlifIntT alifUStrWriter_writeStr(AlifUStrWriter* _writer, AlifObject* _str) { // 13576
	AlifUCS4 maxChar{};
	AlifSizeT len{};

	len = ALIFUSTR_GET_LENGTH(_str);
	if (len == 0) return 0;
	maxChar = ALIFUSTR_MAX_CHAR_VALUE(_str);
	if (maxChar > _writer->maxChar or len > _writer->size - _writer->pos) {
		if (_writer->buffer == nullptr and !_writer->overAllocate) {
			_writer->readOnly = 1;
			_writer->buffer = ALIF_NEWREF(_str);
			alifUStrWriter_update(_writer);
			_writer->pos += len;
			return 0;
		}
		if (alifUStrWriter_prepareInternal(_writer, len, maxChar) == -1)
			return -1;
	}
	alifUStr_fastCopyCharacters(_writer->buffer, _writer->pos, _str, 0, len);
	_writer->pos += len;
	return 0;
}



AlifIntT alifUStrWriter_writeASCIIString(AlifUStrWriter* _writer,
	const char* _ascii, AlifSizeT _len) { // 13690
	if (_len == -1)
		_len = strlen(_ascii);

	if (_writer->buffer == nullptr and !_writer->overAllocate) {
		AlifObject* str;

		//str = alifUStr_fromASCII(_ascii, _len);
		str = alifUStr_fromString(_ascii); //* alif //* review
		if (str == nullptr)
			return -1;

		_writer->readOnly = 1;
		_writer->buffer = str;
		alifUStrWriter_update(_writer);
		_writer->pos += _len;
		return 0;
	}

	if (ALIFUSTRWRITER_PREPARE(_writer, _len, 127) == -1)
		return -1;

	switch (_writer->kind)
	{
	case AlifUStrKind_::AlifUStr_1Byte_Kind:
	{
		const AlifUCS1* str = (const AlifUCS1*)_ascii;
		AlifUCS1* data = (AlifUCS1*)_writer->data;

		memcpy(data + _writer->pos, str, _len);
		break;
	}
	case AlifUStrKind_::AlifUStr_2Byte_Kind:
	{
		ALIFUSTR_CONVERT_BYTES(
			AlifUCS1, AlifUCS2,
			_ascii, _ascii + _len,
			(AlifUCS2*)_writer->data + _writer->pos);
		break;
	}
	case AlifUStrKind_::AlifUStr_4Byte_Kind:
	{
		ALIFUSTR_CONVERT_BYTES(
			AlifUCS1, AlifUCS4,
			_ascii, _ascii + _len,
			(AlifUCS4*)_writer->data + _writer->pos);
		break;
	}
	default:
		ALIF_UNREACHABLE();
	}

	_writer->pos += _len;
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
		AlifObject* str2{};
		str2 = resize_compact(str, _writer->pos);
		if (str2 == nullptr) {
			ALIF_DECREF(str);
			return nullptr;
		}
		str = str2;
	}

	return uStr_result(str);
}



void alifUStrWriter_dealloc(AlifUStrWriter* _writer) { // 13852
	ALIF_CLEAR(_writer->buffer);
}




//static AlifObject* uStr_mod(AlifObject* v, AlifObject* w) { // 13999
//	if (!ALIFUSTR_CHECK(v))
//		return ALIF_NOTIMPLEMENTED;
//	return alifUStr_format(v, w);
//}

static AlifNumberMethods _uStrAsNumber_ = { // 14007
	0,              /*add*/
	0,              /*subtract*/
	0,              /*multiply*/
	//uStr_mod,            /*remainder*/
};





AlifTypeObject _alifUStrType_ = { // 15235
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "نص",
	.basicSize = sizeof(AlifUStrObject),
	.itemSize = 0,
	.dealloc = ustr_dealloc,
	.repr = uStr_repr,
	.asNumber = &_uStrAsNumber_,
	.hash = (HashFunc)uStr_hash,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
		ALIF_TPFLAGS_UNICODE_SUBCLASS |
		_ALIF_TPFLAGS_MATCH_SELF,
	.richCompare = alifUStr_richCompare,
	.free = alifMem_objFree,
};


AlifIntT alifUStr_initGlobalObjects(AlifInterpreter* _interp) { // 15318
	if (alif_isMainInterpreter(_interp)) {
		AlifIntT status = initGlobal_internedStrings(_interp);
		if (status < 1) {
			return status;
		}
	}

	if (init_internedDict(_interp)) {
		//alifErr_clear();
		//return ALIFSTATUS_ERR("failed to create interned dict");
		return -1;
	}
	 
	return 1;
}


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

	_immortalize = 1;

	if (ALIF_ISIMMORTAL(_s)) {
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


void alifUStr_internImmortal(AlifInterpreter* _interp, AlifObject** _p) { // 15553
	*_p = intern_common(_interp, *_p, 1);
}

void alifUStr_internMortal(AlifInterpreter* _interp, AlifObject** _p) { // 15561
	*_p = intern_common(_interp, *_p, 0);
}

AlifObject* alifUStr_internFromString(const char* _cp) { // 15592
	AlifObject* s_ = alifUStr_fromString(_cp);
	if (s_ == nullptr) {
		return nullptr;
	}
	AlifInterpreter* interp = _alifInterpreter_get();
	alifUStr_internMortal(interp, &s_);
	return s_;
}
