#pragma once

using AlifUCS4 = uint32_t;
using AlifUCS2 = uint16_t;
using AlifUCS1 = uint8_t;

AlifObject* alifUStr_decodeUTF8(const wchar_t*, int64_t, const wchar_t*);

AlifObject* alifUStr_fromString(const wchar_t*);

AlifObject* alifUStr_internFromString(const wchar_t*);

void alifUStr_internInPlace(AlifObject**);

enum UStrKind {
	USTR_2BYTE = 2,
	USTR_4BYTE = 4,
};

class AlifUStrObject {
public:

	ALIFOBJECT_HEAD;

	size_t length_;

	size_t hash_;

	uint8_t kind_;

	void* UTF;
};

extern AlifInitObject _alifUStrType_;

#define ALIFUSTR_CHECK(_op) ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(_op), ALIFTPFLAGS_USTR_SUBCLASS)
#define ALIFUSTR_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifUStrType_)

#define ALIFUSTR_CAST(_uStr) ((AlifUStrObject*)(_uStr))

#define ALIFUSTR_KIND(_u) ((AlifUStrObject*)_u)->kind_

#define ALIFUSTR_GET_LENGTH(_op) ((AlifUStrObject*)(_op))->length_


#define ALIF_SIZE_ROUND_DOWN(_n, _a) ((size_t)(_n) & ~(size_t)((_a) - 1))





//#define ALIFASCIIOBJECT_CAST(op) \
//     ALIF_CAST(AlifASCIIObject*, (op)))
//
//
//static inline AlifUIntT alifUSgtr_checkInterned(AlifObject* op) {
//	return alifAsciiObject_cast(op)->state.interned;
//}
//#define ALIFUSTR_CHECK_INTERNED(op) alifUSgtr_checkInterned(ALIFOBJECT_CAST(op))






static inline uint32_t alifUStr_read_wchar(AlifObject* _uStr, int64_t _index)
{
	int kind_;

	kind_ = ALIFUSTR_KIND(_uStr);

	if (kind_ == USTR_2BYTE) {
		return ((((uint16_t*)((AlifUStrObject*)_uStr)->UTF)[_index]));
	}
	return ((((uint32_t*)((AlifUStrObject*)_uStr)->UTF)[_index]));
}
#define ALIFUSTR_READ_WCHAR(_uStr, _index) alifUStr_read_wchar(_uStr, (_index))



AlifObject* alifNew_uStr(size_t, uint8_t);
AlifObject* alifNew_uStr(size_t, uint8_t);
AlifObject* alifUStr_decodeStringToUTF8(const wchar_t*);
AlifSizeT alifUStr_copyCharacters(AlifObject*, AlifSizeT, AlifObject*, AlifSizeT, AlifSizeT);
uint8_t find_maxChar(const wchar_t* str);
size_t hash_uStr(AlifObject*);
static AlifObject* alifUStr_fromUint16(const uint16_t*, int64_t);
static AlifObject* alifUStr_fromUint32(const uint32_t*, int64_t);
AlifObject* alifUStr_decodeUTF8Stateful(const wchar_t*, size_t, const wchar_t*, size_t*);
AlifObject* alifUStr_concat(AlifObject*, AlifObject*);
void alifUStr_append(AlifObject** , AlifObject* );
// in file eq.h
int uStr_eq(AlifObject*, AlifObject*);


AlifObject* alifUStr_objFromWChar(wchar_t*);


//static inline unsigned int alifUStr_isCompact(AlifObject* op) {
//	return ((AlifUStrObject*)op)->state.compact;
//}
//#define ALIFUSTR_IS_COMPACT(op) alifUStr_isCompact(ALIFOBJECT_CAST(op))

static inline void* _alifUStr_nonCompactData(AlifObject* op) {
	void* data;
	data = ((AlifUStrObject*)op)->UTF;
	return data;
}

static inline void* alifUStr_data(AlifObject* op) {
	//if (ALIFUSTR_IS_COMPACT(op)) {
	//	return _ALIFUSTR_COMPACT_DATA(op);
	//}
	return _alifUStr_nonCompactData(op);
}
#define ALIFUSTR_DATA(op) alifUStr_data(ALIFOBJECT_CAST(op))


static inline void alifUStr_write(int kind, void* data,
	int64_t index, uint32_t value)
{
	if (kind == USTR_2BYTE) {
		((uint16_t*)(data))[index] = ((uint16_t)(value));
	}
	else {
		((uint32_t*)(data))[index] = value;
	}
}
#define ALIFUSTR_WRITE(kind, data, index, value) \
    alifUStr_write(kind, ((void*)(data)), \
                    (index), ((uint32_t)(value)))

static inline uint32_t alifUStr_read(int kind,
	const void* data, int64_t index)
{
	if (kind == USTR_2BYTE) {
		return ((const uint16_t*)data)[index];
	}
	return ((const uint32_t*)data)[index];
}
#define ALIFUSTR_READ(kind, data, index) \
    alifUStr_read(kind, \
                   ((const void*)data), \
                   index)

static inline uint32_t alifUStr_maxCharValue(AlifObject* object)
{
	int kind = ((AlifUStrObject*)object)->kind_;
	if (kind == USTR_2BYTE) {
		return 0xffffU;
	}
	return 0x10ffffU;
}
#define ALIFUSTR_MAX_CHAR_VALUE(object) alifUStr_maxCharValue(object)

int64_t alifUStr_fill(AlifObject*, int64_t, int64_t, uint32_t);

class AlifSubUStrWriter {
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

#define ALIFSUBUSTRWRITER_PREPARE(_writer, _length, _maxChar)             \
    (((_maxChar) <= (_writer)->maxChar and (_length) <= (_writer)->size_ - (_writer)->pos_)                  \
     ? 0 : (((_length) == 0) ? 0 : alifSubUStrWriter_prepareInternal((_writer), (_length), (_maxChar))))


int alifSubUStrWriter_prepareInternal(AlifSubUStrWriter*, int64_t, uint32_t);

int alifSubUStrWriter_writeChar(AlifSubUStrWriter*, uint32_t);

int alifSubUStrWriter_writeStr(AlifSubUStrWriter*, AlifObject*);

int alifUStr_fsConverter(AlifObject*, void*);

const wchar_t* alifUStr_asUTF8(AlifObject*);
AlifObject* alifUStr_join(AlifObject*, AlifObject*);
