#include "alif.h"

#include "AlifCore_Object.h"
#include "AlifCore_State.h"
#include "AlifCore_UStrObject.h"
#include "AlifCore_BytesObject.h"


#include <Equal.h> // 61


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
   /* On Windows, overAllocate by 50% is the best factor */
#  define OVERALLOCATE_FACTOR 2
#else
   /* On Linux, overAllocate by 25% is the best factor */
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

static AlifHashT uStr_hash(AlifObject*); // 263
static AlifIntT uStrCompare_eq(AlifObject*, AlifObject*);

static AlifUHashT hashTable_uStrHash(const void* _key) { // 266
	return uStr_hash((AlifObject*)_key);
}


static AlifIntT hashTable_uStrCompare(const void* _key1, const void* _key2) { // 272
	AlifObject* obj1 = (AlifObject*)_key1;
	AlifObject* obj2 = (AlifObject*)_key2;
	if (obj1 != nullptr and obj2 != nullptr) {
		return uStrCompare_eq(obj1, obj2);
	}
	else {
		return obj1 == obj2;
	}
}


static AlifIntT init_internedDict(AlifInterpreter* interp) { // 285
	AlifObject* interned = interned = alifDict_new();
	if (interned == nullptr) {
		return -1;
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
#define ALIF_RETURN_UNICODE_EMPTY	return unicode_getEmpty();




static AlifIntT uStr_modifiable(AlifObject*); // 432



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
#include "StringLib/FastSearch.h"
#include "StringLib/FindMaxChar.h"
#include "StringLib/Undef.h"

#include "StringLib/UCS2Lib.h"
#include "StringLib/FastSearch.h"
#include "StringLib/FindMaxChar.h"
#include "StringLib/Undef.h"

#include "StringLib/UCS4Lib.h"
#include "StringLib/FastSearch.h"
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
			//f = uStr_fromFormatArg(_writer, f, &vargs2);
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





//static AlifIntT unicodeDecode_callErrorHandlerWriter(
//	const char* errors, AlifObject** errorHandler,
//	const char* encoding, const char* reason,
//	const char** input, const char** inend, AlifSizeT* startinpos,
//	AlifSizeT* endinpos, AlifObject** exceptionObject, const char** inptr,
//	AlifUStrWriter* writer) { // 4436
//	static const char* argparse = "Un;decoding error handler must return (str, AlifIntT) tuple";
//
//	AlifObject* restuple = nullptr;
//	AlifObject* repunicode = nullptr;
//	AlifSizeT insize{};
//	AlifSizeT newpos{};
//	AlifSizeT replen{};
//	AlifSizeT remain{};
//	AlifObject* inputobj = nullptr;
//	AlifIntT need_to_grow = 0;
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
//	if (newpos<0 or newpos>insize) {
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
                ALIFUSTR_WRITE(writer.kind, writer.data, writer.pos++, ch);  \
            } while(0)

#define WRITE_CHAR(_ch)                                                        \
            do {                                                              \
                if (ch <= writer.maxChar) {                                   \
                    ALIFUSTR_WRITE(writer.kind, writer.data, writer.pos++, ch); \
                }                                                             \
                else if (alifUStrWriter_writeCharInline(&writer, ch) < 0) { \
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
		//if (uStrDecode_callErrorhandlerWriter(
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
	case AlifUStr_1Byte_Kind:
	{
		switch (kind2) {
		case AlifUStr_1Byte_Kind:
		{
			AlifIntT cmp_ = memcmp(data1, data2, len);
			if (cmp_ < 0)
				return -1;
			if (cmp_ > 0)
				return 1;
			break;
		}
		case AlifUStr_2Byte_Kind:
			COMPARE(AlifUCS1, AlifUCS2);
			break;
		case AlifUStr_4Byte_Kind:
			COMPARE(AlifUCS1, AlifUCS4);
			break;
		default:
			ALIF_UNREACHABLE();
		}
		break;
	}
	case AlifUStr_2Byte_Kind:
	{
		switch (kind2) {
		case AlifUStr_1Byte_Kind:
			COMPARE(AlifUCS2, AlifUCS1);
			break;
		case AlifUStr_2Byte_Kind:
		{
			COMPARE(AlifUCS2, AlifUCS2);
			break;
		}
		case AlifUStr_4Byte_Kind:
			COMPARE(AlifUCS2, AlifUCS4);
			break;
		default:
			ALIF_UNREACHABLE();
		}
		break;
	}
	case AlifUStr_4Byte_Kind:
	{
		switch (kind2) {
		case AlifUStr_1Byte_Kind:
			COMPARE(AlifUCS4, AlifUCS1);
			break;
		case AlifUStr_2Byte_Kind:
			COMPARE(AlifUCS4, AlifUCS2);
			break;
		case AlifUStr_4Byte_Kind:
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




static AlifIntT uStrCompare_eq(AlifObject* _str1, AlifObject* _str2) { // 10963
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


AlifIntT alifUStr_eq(AlifObject* _aa, AlifObject* _bb) { // 11229
	return uStr_eq(_aa, _bb);
}


static AlifHashT uStr_hash(AlifObject* _self) { // 11663
	AlifUHashT x;  /* Unsigned for defined overflow behavior. */

	AlifHashT hash = alifAtomic_loadSizeRelaxed(&ALIFUSTR_HASH(_self));
	if (hash != -1) {
		return hash;
	}
	x = alif_hashBytes(ALIFUSTR_DATA(_self),
		ALIFUSTR_GET_LENGTH(_self) * ALIFUSTR_KIND(_self));

	alifAtomic_storeSizeRelaxed(&ALIFUSTR_HASH(_self), x);
	return x;
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
	if (_start >= length || _end < _start)
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

		str = alifUStr_fromASCII(_ascii, _len);
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
		AlifObject* str2;
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












AlifTypeObject _alifUStrType_ = { // 15235
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "نص",
	.basicSize = sizeof(AlifUStrObject),
	.itemSize = 0,
	.dealloc = ustr_dealloc,
	.hash = (HashFunc)uStr_hash,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
		ALIF_TPFLAGS_UNICODE_SUBCLASS |
		_ALIF_TPFLAGS_MATCH_SELF,
	.base = 0,
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

#if ALIF_GIL_DISABLED
	_immortalize = 1;
#endif

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
