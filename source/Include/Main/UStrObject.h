#pragma once

using AlifUCS4 = uint32_t;
using AlifUCS2 = uint16_t;
using AlifUCS1 = uint8_t;

AlifObject* alifUnicode_decodeUTF8(const wchar_t*, int64_t, const wchar_t*);

AlifObject* alifUStr_fromString(const wchar_t*);

AlifObject* alifUnicode_internFromString(const wchar_t*);

enum UnicodeKind {
	UNICODE_2BYTE = 2,
	UNICODE_4BYTE = 4,
};

class AlifUStrObject {
public:

	AlifObject object;

	size_t length;

	size_t hash;

	uint8_t kind;

	void* UTF;
};

extern AlifInitObject _alifUStrType_;

#define ALIFUNICODE_CHECK_TYPE(_op) ALIF_IS_TYPE((_op), &_alifUStrType_)

#define ALIFUNICODE_CAST(unicode) ((AlifUStrObject*)(unicode))

#define ALIFUNICODE_KIND(_u) ((AlifUStrObject*)_u)->kind

#define ALIFUNICODE_GET_LENGTH(_op) ((AlifUStrObject*)(_op))->length


#define ALIF_SIZE_ROUND_DOWN(n, a) ((size_t)(n) & ~(size_t)((a) - 1))

static inline uint32_t alifUnicode_read_wchar(AlifObject* _unicode, int64_t _index)
{
	int kind_;

	kind_ = ALIFUNICODE_KIND(_unicode);

	if (kind_ == UNICODE_2BYTE) {
		return ((((uint16_t*)((AlifUStrObject*)_unicode)->UTF)[_index]));
	}
	return ((((uint32_t*)((AlifUStrObject*)_unicode)->UTF)[_index]));
}
#define ALIFUNICODE_READ_WCHAR(_unicode, _index) alifUnicode_read_wchar(_unicode, (_index))

// هنا يتم تحويل من utf16 الى utf32 والعكس 
#define ALIFUNICODE_CONVERT_BYTES(fromType, toType, begin, end, to) {   \
    toType *_to = (toType *)(to);                                      \
    const fromType *_iter = (const fromType *)(begin);                 \
    const fromType *_end = (const fromType *)(end);                    \
    SSIZE_T n = (_end) - (_iter);                                      \
    const fromType *_unrolledEnd = _iter + ALIF_SIZE_ROUND_DOWN(n, 4); \
    while (_iter < (_unrolledEnd)) {                                   \
        _to[0] = (toType) _iter[0];                                    \
        _to[1] = (toType) _iter[1];                                    \
        _to[2] = (toType) _iter[2];                                    \
        _to[3] = (toType) _iter[3];                                    \
        _iter += 4; _to += 4;                                          \
    }                                                                  \
    while (_iter < (_end)) { *_to++ = (toType) *_iter++; }             \
} 

AlifObject* alifNew_uStr(size_t, uint8_t);
AlifObject* alifNew_unicode(size_t, uint8_t);
AlifObject* alifUnicode_decodeStringToUTF8(const wchar_t*);
AlifSizeT alifUnicode_copyCharacters(AlifObject*, AlifSizeT, AlifObject*, AlifSizeT, AlifSizeT);
uint8_t find_maxChar(const wchar_t* str);
size_t hash_unicode(AlifObject*);
static AlifObject* alifUnicode_fromUint16(const uint16_t*, int64_t);
static AlifObject* alifUnicode_fromUint32(const uint32_t*, int64_t);
AlifObject* alifUStr_decodeUTF8Stateful(const wchar_t*, size_t, const wchar_t*, size_t*);
AlifObject* alifUStr_concat(AlifObject*, AlifObject*);
void alifUStr_append(AlifObject** , AlifObject* );
// in file eq.h
int unicode_eq(AlifObject*, AlifObject*);


AlifObject* alifUStr_objFromWChar(wchar_t*);

static inline void alifUnicode_write(int kind, void* data,
	int64_t index, uint32_t value)
{
	if (kind == UNICODE_2BYTE) {
		((uint16_t*)(data))[index] = ((uint16_t)(value));
	}
	else {
		((uint32_t*)(data))[index] = value;
	}
}
#define ALIFUNICODE_WRITE(kind, data, index, value) \
    alifUnicode_write(kind, ((void*)(data)), \
                    (index), ((uint32_t)(value)))

static inline uint32_t alifUnicode_read(int kind,
	const void* data, int64_t index)
{
	if (kind == UNICODE_2BYTE) {
		return ((const uint16_t*)data)[index];
	}
	return ((const uint32_t*)data)[index];
}
#define ALIFUNICODE_READ(kind, data, index) \
    alifUnicode_read(kind, \
                   ((const void*)data), \
                   index)

static inline uint32_t alifUnicode_maxCharValue(AlifObject* object)
{
	int kind = ((AlifUStrObject*)object)->kind;
	if (kind == UNICODE_2BYTE) {
		return 0xffffU;
	}
	return 0x10ffffU;
}
#define ALIFUNICODE_MAX_CHAR_VALUE(object) alifUnicode_maxCharValue(object)

int64_t alifUnicode_fill(AlifObject*, int64_t, int64_t, uint32_t);

class AlifSubUnicodeWriter {
public:
	AlifObject* buffer_;
	void* data_;
	int kind_;
	uint32_t maxChar;
	int64_t size_;
	int64_t pos_;

	/* minimum number of allocated characters (default: 0) */
	int64_t minLength;

	/* minimum character (default: 127, ASCII) */
	uint32_t minChar;

	/* If non-zero, overallocate the buffer (default: 0). */
	wchar_t overAllocate;

	/* If readonly is 1, buffer is a shared string (cannot be modified)
	   and size is set to 0. */
	wchar_t readonly_;
};

#define ALIFSUBUNICODEWRITER_PREPARE(_writer, _length, _maxChar)             \
    (((_maxChar) <= (_writer)->maxChar and (_length) <= (_writer)->size_ - (_writer)->pos_)                  \
     ? 0 : (((_length) == 0) ? 0 : alifSubUnicodeWriter_prepareInternal((_writer), (_length), (_maxChar))))


int alifSubUnicodeWriter_prepareInternal(AlifSubUnicodeWriter*, int64_t, uint32_t);

int alifSubUnicodeWriter_writeChar(AlifSubUnicodeWriter*, uint32_t);

int alifSubUnicodeWriter_writeStr(AlifSubUnicodeWriter*, AlifObject*);

const wchar_t* alifUnicode_asUTF8(AlifObject*);
