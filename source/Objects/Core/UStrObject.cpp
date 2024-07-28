#include "alif.h"

#include "AlifCore_Object.h"
#include "AlifCore_AlifCycle.h"
#include "AlifCore_UString.h"
#include "AlifCore_Memory.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_Abstract.h"
#include "AlifCore_AlifEval.h"
#include "AlifCore_GlobalString.h"

// make this include comment to know the error
#include "AlifCore_ModSupport.h" // this is temp and should transfer to UStrObject.c.h or .h

#ifdef _WINDOWS
#include "windows.h"
#endif // _WINDOWS


// هنا يتم تحويل من utf16 الى utf32 والعكس 
#define ALIFSUBUSTR_CONVERT_BYTES(_fromType, _toType, _begin, _end, _to) {   \
    _toType *to_ = (_toType *)(_to);                                      \
    const _fromType *iter_ = (const _fromType *)(_begin);                 \
    const _fromType *end_ = (const _fromType *)(_end);                    \
    AlifSizeT n_ = (end_) - (iter_);                                      \
    const _fromType *unrolledEnd = iter_ + ALIF_SIZE_ROUND_DOWN(n_, 4); \
    while (iter_ < (unrolledEnd)) {                                   \
        to_[0] = (_toType) iter_[0];                                    \
        to_[1] = (_toType) iter_[1];                                    \
        to_[2] = (_toType) iter_[2];                                    \
        to_[3] = (_toType) iter_[3];                                    \
        iter_ += 4; to_ += 4;                                          \
    }                                                                  \
    while (iter_ < (end_)) { *to_++ = (_toType) *iter_++; }             \
} 

// Forward
static inline int alifSubUStrWriter_writeWcharInline(AlifSubUStrWriter*, uint32_t);
static int alifSubUStr_fastCopyCharacters(AlifObject*, int64_t , AlifObject* , int64_t , int64_t , int );


//static inline AlifObject* uStr_get_empty(void)
//{
//	ALIFSUB_DECLARE_STR(empty, "");
//	return &ALIFSUB_STR(empty);
//}

static inline void alifUStr_fill(int _kind, void* _data, uint32_t _value,
	int64_t _start, int64_t _length)
{
	switch (_kind) {

	case USTR_2BYTE: {
		uint16_t ch_ = (uint16_t)_value;
		uint16_t* to_ = (uint16_t*)_data + _start;
		const uint16_t* end_ = to_ + _length;
		for (; to_ < end_; ++to_) *to_ = ch_;
		break;
	}
	case USTR_4BYTE: {
		uint32_t ch_ = _value;
		uint32_t* to_ = (uint32_t*)_data + _start;
		const uint32_t* end_ = to_ + _length;
		for (; to_ < end_; ++to_) *to_ = ch_;
		break;
	}
	//default: UNREACHABLE();
	}
}

static AlifObject* uStr_result(AlifObject* _uStr)
{
	int64_t length_ = ALIFUSTR_GET_LENGTH(_uStr);
	if (length_ == 0) {
		//AlifObject* empty = uStr_get_empty();
		//if (_uStr != empty) {
			//ALIF_DECREF(_uStr);
		//}
		//return empty;
	}

	return _uStr;
}

static AlifObject* uStr_result_unchanged(AlifObject* _uStr)
{
	if ((_uStr->type_ == &_alifUStrType_)) {
		return ALIF_NEWREF(_uStr);
	}
	else
		return alifSubUStr_copy(_uStr);
}

// in file bytearrayobject.c in 1111 and should call _from there only 
#define STRINGLIB_FAST_MEMCHR memchr

#include "FastSearch.h"
#include "Split.h"
#include "Find.h"

// in file count.h 
template <typename STRINGLIB_CHAR>
int64_t count(const STRINGLIB_CHAR* _str, int64_t _strLen,
	const STRINGLIB_CHAR* _sub, int64_t _subLen,
	int64_t _maxCount)
{
	int64_t count_;

	if (_strLen < 0)
		return 0; /* start_ > len_(_uStr) */
	if (_subLen == 0)
		return (_strLen < _maxCount) ? _strLen + 1 : _maxCount;

	count_ = fastSearch(_str, _strLen, _sub, _subLen, _maxCount, FAST_COUNT);

	if (count_ < 0)
		return 0; 

	return count_;
}

static inline int64_t findChar(const void* _s, int _kind,
	int64_t _size, uint32_t _ch,
	int _direction)
{
	switch (_kind) {
	case USTR_2BYTE:
		if ((uint16_t)_ch != _ch)
			return -1;
		if (_direction > 0)
			return find_char((const uint16_t*)_s, _size, (uint16_t)_ch);
		else
			return rfind_char((const uint16_t*)_s, _size, (uint16_t)_ch);
	case USTR_4BYTE:
		if (_direction > 0)
			return find_char((const uint32_t*)_s, _size, _ch);
		else
			return rfind_char((const uint32_t*)_s, _size, _ch);
	//default:
		//UNREACHABLE();
	}
}

static int resize_inplace(AlifObject* _uStr, int64_t _length)
{

	int64_t newSize;
	int64_t charSize;
	int shareUTF8;
	void* data_;
#ifdef ALIF_DEBUG
	int64_t oldLength = ALIFUSTR_GET_LENGTH(_uStr);
#endif

	data_ = ALIFUSTR_CAST(_uStr)->UTF;
	charSize = ALIFUSTR_KIND(_uStr);

	shareUTF8 = 1;

	if (_length > (LLONG_MAX / charSize - 1)) {
		return -1;
	}
	newSize = (_length + 1) * charSize;

	if (!shareUTF8
		// &&
		//ALIFSUBUSTR_HAS_UTF8_MEMORY(_uStr)
		 )
	{
		alifMem_objFree(ALIFUSTR_CAST(_uStr)->UTF);
		ALIFUSTR_CAST(_uStr)->UTF = nullptr;
		ALIFUSTR_GET_LENGTH(_uStr) = 0;
	}

	data_ = (AlifObject*)alifMem_dataRealloc(data_, newSize);
	if (data_ == nullptr) {
		return -1;
	}
	ALIFUSTR_CAST(_uStr)->UTF = data_;
	if (shareUTF8) {
		ALIFUSTR_CAST(_uStr)->UTF = data_;
		ALIFUSTR_GET_LENGTH(_uStr) = _length;
	}
	ALIFUSTR_GET_LENGTH(_uStr) = _length;
	ALIFUSTR_WRITE(ALIFUSTR_KIND(_uStr), data_, _length, 0);
#ifdef ALIF_DEBUG
	uStr_fill_invalid(_uStr, old_length);
#endif

	if (_length > LLONG_MAX / (int64_t)sizeof(wchar_t) - 1) {
		return -1;
	}
	return 0;
}

static AlifObject*
resize_copy(AlifObject* _uStr, int64_t _length)
{
	int64_t copyLength;
	AlifObject* copy_;

	copy_ = alifNew_uStr(_length, ALIFUSTR_MAX_CHAR_VALUE(_uStr));
	if (copy_ == nullptr)
		return nullptr;

	copyLength = ALIF_MIN(_length, ALIFUSTR_GET_LENGTH(_uStr));
	alifSubUStr_fastCopyCharacters(copy_, 0, _uStr, 0, copyLength, 0);
	return copy_;
}

// من هنا يتم انشاء كائن نصي
AlifObject* alifNew_uStr(size_t _size, uint8_t _maxChar) { /// M
	AlifObject* obj_{};
	AlifUStrObject* uStr{};
	void* data_{};
	uint8_t kind_{};
	uint8_t charSize{};
	size_t structSize = sizeof(AlifUStrObject);


	if (_maxChar == 2) {
		kind_ = USTR_2BYTE;
		charSize = 2;
	}
	else if (_maxChar == 4) {
		kind_ = USTR_4BYTE;
		charSize = 4;
	}
	else {
		std::wcout << L"ترميز الحروف غير صحيح\n" << std::endl;
		exit(-1);
	}

	obj_ = (AlifObject*)alifMem_objAlloc(structSize + (_size + 1) * charSize);

	alifSubObject_init(obj_, &_alifUStrType_);

	uStr = (AlifUStrObject*)obj_;

	data_ = uStr + 1;

	uStr->length_ = _size;
	uStr->kind_ = kind_;
	uStr->hash_ = 0;
	uStr->UTF = data_;

	if (kind_ == USTR_2BYTE) {
		((uint16_t*)data_)[_size] = 0;
	}
	else if (kind_ == USTR_4BYTE) {
		((uint32_t*)data_)[_size] = 0;
	}

	return obj_;
}

static int
uStr_modifiable(AlifObject* _uStr)
{
	if (ALIF_REFCNT(_uStr) != 1)
		return 0;
	if (ALIFUSTR_CAST(_uStr)->hash_ != -1)
		return 0;
	//if (USTR_CHECK_INTERNED(_uStr)) // من الممكن ان يضاف
	//	return 0;
	if (!(_uStr->type_ == &_alifUStrType_))
		return 0;
	return 1;
}

static int uStr_check_modifiable(AlifObject* _uStr)
{
	if (!uStr_modifiable(_uStr)) {
		return -1;
	}
	return 0;
}

static int alifSubUStr_fastCopyCharacters(AlifObject* _to, int64_t _toStart,
	AlifObject* _from, int64_t _fromStart,
	int64_t _howMany, int _checkMaxChar)
{
	int fromKind, toKind;
	const void* fromData;
	void* toData;

	if (_howMany == 0)
		return 0;

	fromKind = ALIFUSTR_CAST(_from)->kind_;
	fromData = ALIFUSTR_CAST(_from)->UTF;
	toKind = ALIFUSTR_CAST(_to)->kind_;
	toData = ALIFUSTR_CAST(_to)->UTF;

#ifdef DEBUG
	if (!_checkMaxChar
		&& ALIFUSTR_MAX_CHAR_VALUE(_from) > ALIFUSTR_MAX_CHAR_VALUE(_to))
	{
		uint32_t toMaxChar = ALIFUSTR_MAX_CHAR_VALUE(_to);
		uint32_t ch_;
		int64_t i_;
		for (i_ = 0; i_ < _howMany; i_++) {
			ch_ = USTR_READ(fromKind, fromData, _fromStart + i_);
			assert(ch_ <= toMaxChar);
		}
	}
#endif

	if (fromKind == toKind) {
		memcpy((wchar_t*)toData + toKind * _toStart,
			(const wchar_t*)fromData + fromKind * _fromStart,
			toKind * _howMany);
	}
	else if (fromKind == USTR_2BYTE
		&& toKind == USTR_4BYTE)
	{
		ALIFSUBUSTR_CONVERT_BYTES(
			uint16_t, uint32_t,
			((uint16_t*)(ALIFUSTR_CAST(_from))->UTF) + _fromStart,
			((uint16_t*)(ALIFUSTR_CAST(_from))->UTF) + _fromStart + _howMany,
			((uint32_t*)(ALIFUSTR_CAST(_to))->UTF) + _toStart
		);
	}
	else {

		if (!_checkMaxChar) {
			 if (fromKind == USTR_4BYTE
				&& toKind == USTR_2BYTE)
			{
				 ALIFSUBUSTR_CONVERT_BYTES(
					 uint32_t, uint16_t,
					 ((uint32_t*)(ALIFUSTR_CAST(_from))->UTF) + _fromStart,
					 ((uint32_t*)(ALIFUSTR_CAST(_from))->UTF) + _fromStart + _howMany,
					 ((uint16_t*)(ALIFUSTR_CAST(_to))->UTF) + _toStart
				);
			}
			else {
				//UNREACHABLE();
			}
		}
		else {
			const uint32_t toMaxChar = ALIFUSTR_MAX_CHAR_VALUE(_to);
			uint32_t ch_;
			int64_t i_;

			for (i_ = 0; i_ < _howMany; i_++) {
				ch_ = ALIFUSTR_READ(fromKind, fromData, _fromStart + i_);
				if (ch_ > toMaxChar)
					return -1;
				ALIFUSTR_WRITE(toKind, toData, _toStart + i_, ch_);
			}
		}
	}
	return 0;
}

AlifSizeT alifUStr_copyCharacters(AlifObject* _to, AlifSizeT _toStart,
	AlifObject* _from, AlifSizeT _fromStart, AlifSizeT _howMany)
{
	int err_{};

	_howMany = ALIF_MIN(ALIFUSTR_GET_LENGTH(_from) - _fromStart, _howMany);
	if (_toStart + _howMany > ALIFUSTR_GET_LENGTH(_to)) {
		return -1;
	}

	if (_howMany == 0)
		return 0;

	if (uStr_check_modifiable(_to))
		return -1;

	err_ = alifSubUStr_fastCopyCharacters(_to, _toStart, _from, _fromStart, _howMany, 1);
	if (err_) {
		return -1;
	}
	return _howMany;
}

// هنا يتم ايجاد اعلى ترميز في النص لتحديد نوع ترميز النص
uint8_t find_maxChar(const wchar_t* _str) {

	while (*_str != L'\0') {
		if (*_str >= 0xD800 && *_str <= 0xDBFF) {
			_str++;
			return 4;
		}
		_str++;
	}
	return 2;
}

static int uStr_resize(AlifObject** _pUStr, int64_t _length)
{
	AlifObject* uStr;
	int64_t oldLength;

	uStr = *_pUStr;
	oldLength = ALIFUSTR_GET_LENGTH(uStr);
	if (oldLength == _length)
		return 0;

	if (_length == 0) {
		//AlifObject* empty = uStr_get_empty();
		//ALIF_SETREF(*_pUStr, empty);
		return 0;
	}

	if (!uStr_modifiable(uStr)) {
		AlifObject* copy_ = resize_copy(uStr, _length);
		if (copy_ == nullptr)
			return -1;
		ALIF_SETREF(*_pUStr, copy_);
		return 0;
	}
	return resize_inplace(uStr, _length);
}

AlifObject* alifUStr_fromWideChar(const wchar_t* _u, int64_t _size)
{
	AlifObject* uStr{};
	AlifUCS4 maxChar = 0;
	int64_t numSurrogates{};

	if (_u == nullptr && _size != 0) {
		return nullptr;
	}

	if (_size == -1) {
		_size = wcslen(_u);
	}

	//if (size == 0)
		//ALIFSUB_RETURN_USTR_EMPTY();

	if ((maxChar = find_maxChar(_u)) == -1)
		return nullptr;

	uStr = alifNew_uStr(_size - numSurrogates, maxChar);
	if (!uStr)
		return nullptr;

	switch (ALIFUSTR_KIND(uStr)) {
	case USTR_2BYTE:
#if ALIF_uStr_SIZE == 2
		memcpy(AlifUSTR_2BYTE_DATA(_uStr), u_, size * 2);
#else
		ALIFSUBUSTR_CONVERT_BYTES(wchar_t, AlifUCS2,
			_u, _u + _size, ALIFUSTR_CAST(uStr)->UTF);
#endif
		break;
	case USTR_4BYTE:
#if SIZEOF_WCHAR_T == 2
		/* This is the only case which has to process surrogates, thus
		   a simple copy loop is not enough and we need a function. */
		uStr_convert_wchar_to_ucs4(u_, u_ + size, _uStr);
#else
		memcpy(((uint32_t*)(ALIFUSTR_CAST(uStr))->UTF), _u, _size * 4);
#endif
		break;
	default:
		//ALIF_UNREACHABLE();
		return nullptr; // temp
	}

	return uStr_result(uStr);
}

AlifObject* alifSubUStr_copy(AlifObject* _uStr)
{
	int64_t length_;
	AlifObject* copy_;

	if (!(_uStr->type_ == &_alifUStrType_)) {
		return nullptr;
	}

	length_ = ALIFUSTR_CAST(_uStr)->length_;
	copy_ = alifNew_uStr(length_, ALIFUSTR_MAX_CHAR_VALUE(_uStr));
	if (!copy_)
		return nullptr;

	memcpy(ALIFUSTR_CAST(copy_)->UTF, ALIFUSTR_CAST(_uStr)->UTF,
		length_ * ALIFUSTR_CAST(_uStr)->kind_);
	return copy_;
}

// هنا يتم عد الحروف بترميز utf32 الموجودة في النص فقط
size_t count_4byteCharacters(const wchar_t* _uStr) {
	size_t count4byte = 0;
	while (*_uStr != L'\0') {
		if (*_uStr >= 0xD800 && *_uStr <= 0xDBFF) {
			_uStr++;
			count4byte++;
		}
		_uStr++;
	}

	return count4byte;
}

// هنا يتم عد الحروف بترميز utf16 الموجودة في النص فقط
size_t count_2byteCharacters(const wchar_t* _uStr) {
	size_t count2byte = 0;
	while (*_uStr != L'\0') {
		count2byte++;
		if (*_uStr >= 0xD800 && *_uStr <= 0xDBFF) {
			_uStr++;
			count2byte--;
		}
		_uStr++;
	}

	return count2byte;
}

// من هنا يتم عد الحروف الموجودة في النص
size_t count_characters(const wchar_t* _uStr) {
	size_t count = 0;
	while (*_uStr != L'\0') {
		count++;
		if (*_uStr >= 0xD800 && *_uStr <= 0xDBFF) {
			_uStr++;
		}
		_uStr++;
	}

	return count;
}

static void uStrConvert_wCharToUCS4(const wchar_t* _begin, const wchar_t* _end, AlifObject* _uStr) { /// M
	const wchar_t* iter_{};
	uint32_t* uCS4{};

	uCS4 = (uint32_t*)ALIFUSTR_CAST(_uStr)->UTF;

	for (iter_ = _begin; iter_ < _end; ) {
		if ((0xD800 <= iter_[0] && iter_[0] <= 0xDBFF)
			&& (iter_ + 1) < _end
			&& (0xDC00 <= iter_[1] && iter_[1] <= 0xDFFF))
		{
			*uCS4++ = 0x10000 + (((iter_[0] & 0x03FF) << 10) | (iter_[1] & 0x03FF));
			iter_ += 2;
		}
		else {
			*uCS4++ = *iter_;
			iter_++;
		}
	}
}

AlifObject* alifUStr_objFromWChar(wchar_t* _buffer) { /// M
	AlifObject* strObj{};
	size_t size_ = wcslen(_buffer);
	uint8_t maxChar = find_maxChar(_buffer);

	strObj = alifNew_uStr(size_, maxChar);
	
	if (ALIFUSTR_CAST(strObj)->kind_ == USTR_2BYTE) {
#if SIZEOF_WCHART == 2
		memcpy(ALIFUSTR_CAST(strObj)->UTF, _buffer, size_ * 2);
#else
		ALIFUSTR_CONVERT_BYTES(wchar_t, uint16_t,
			_buffer, _buffer + size_, ALIFUSTR_CAST(strObj)->UTF);
#endif
	}
	else if (ALIFUSTR_CAST(strObj)->kind_ == USTR_4BYTE) {
#if SIZEOF_WCHART == 2
		uStrConvert_wCharToUCS4(_buffer, _buffer + size_, strObj);
#else
		memcpy(ALIFUSTR_CAST(strObj)->UTF, _buffer, size_ * 4);
#endif
	}

	return strObj;
}

void copy_string(AlifObject* _object, const wchar_t* _uStr, AlifSizeT _length, AlifSizeT _maxChar) {

	if (_maxChar == 2) {

		memcpy(ALIFUSTR_CAST(_object)->UTF, _uStr, (_length * _maxChar));

	}
	else if (_maxChar == 4) {

		uint32_t* UTF = (uint32_t*)(ALIFUSTR_CAST(_object))->UTF;
		size_t len_ = 0;
		uint32_t point_, surrogate_;

		while (*_uStr != L'\0') {
			point_ = (uint32_t)(*_uStr);
			if (point_ >= 0xD800 && point_ <= 0xDBFF) {
				++_uStr; 
				surrogate_ = (uint32_t)(*_uStr);
				point_ = (point_ - 0xD800) * 0x400 + (surrogate_ - 0xDC00) + 0x10000;
			}

			UTF[len_++] = point_;
			++_uStr; 
		}

	}
	else {
		std::wcout << L"ترميز الحروف غير صحيح\n" << std::endl;
		exit(-1);
	}

}

static AlifObject* uStr_wchar_t(uint32_t _ch)
{
	AlifObject* uStr;

	uStr = alifNew_uStr(1, _ch);
	if (uStr == nullptr)
		return nullptr;

	if (ALIFUSTR_CAST(uStr)->kind_ == USTR_2BYTE) {
		((uint16_t*)ALIFUSTR_CAST(uStr)->UTF)[0] = (uint32_t)_ch;
	}
	else {
		((uint32_t*)ALIFUSTR_CAST(uStr)->UTF)[0] = _ch;
	}
	return uStr;
}

static uint32_t kind_maxChar_limit(int _kind)
{
	switch (_kind) {
	case USTR_2BYTE:
		return 0x100;
	case USTR_4BYTE:
		return 0x10000;
	//default:
		//UNREACHABLE();
	}
}

static AlifObject*
alifUStr_fromUint16(const uint16_t* _u, int64_t _size)
{
	AlifObject* res_;
	uint16_t maxChar;

	//if (size_ == 0)
	//	_RETURN_uStr_EMPTY();
	if (_size == 1)
		return uStr_wchar_t(_u[0]);

	maxChar = find_maxChar((const wchar_t*)_u);
	res_ = alifNew_uStr(_size, maxChar);
	if (!res_)
		return nullptr;
	if (maxChar >= 256)
		memcpy(((uint16_t*)ALIFUSTR_CAST(res_)->UTF), _u, sizeof(uint16_t) * _size);
	return res_;
}

static AlifObject* alifUStr_fromUint32(const uint32_t* _u, int64_t _size)
{
	AlifObject* res_;
	uint32_t maxChar;

	//if (size_ == 0)
		//_RETURN_uStr_EMPTY();
	if (_size == 1)
		return uStr_wchar_t(_u[0]);

	maxChar = find_maxChar((const wchar_t*)_u);
	res_ = alifNew_uStr(_size, maxChar);
	if (!res_)
		return nullptr;
	 if (maxChar < 0x10000)
	 {
		 ALIFSUBUSTR_CONVERT_BYTES(uint32_t, uint16_t, _u, _u + _size,
			 ((uint16_t*)ALIFUSTR_CAST(res_)->UTF));
	 }
	else
	 {
		 memcpy(((uint32_t*)ALIFUSTR_CAST(res_)->UTF), _u, sizeof(uint32_t) * _size);
	 }
	return res_;
}

void combine_string(AlifObject* _result, AlifSizeT start_, AlifObject* _from, AlifSizeT _fromStart, AlifSizeT _length, uint8_t _maxChar) {

	AlifSizeT fromKind = (ALIFUSTR_CAST(_from))->kind_;
	AlifSizeT toKind = (ALIFUSTR_CAST(_result))->kind_;
	if (fromKind == toKind) {

		void* fromData = ((wchar_t*)(ALIFUSTR_CAST(_from))->UTF + (_fromStart * fromKind));
		void* toData = ((wchar_t*)(ALIFUSTR_CAST(_result))->UTF + (start_ * toKind));

		memcpy(toData, fromData, _length * _maxChar);

	}
	else if (fromKind == 2 && toKind == 4) {

		uint16_t* fromData = ((uint16_t*)(ALIFUSTR_CAST(_from))->UTF);
		uint32_t* toData = ((uint32_t*)(ALIFUSTR_CAST(_result))->UTF);

		ALIFSUBUSTR_CONVERT_BYTES(uint16_t, uint32_t,
			fromData + _fromStart,
			fromData + _fromStart + _length, 
			toData + start_);

	}
	else if (fromKind == 4 && toKind == 2) {
	
		uint32_t* fromData = ((uint32_t*)(ALIFUSTR_CAST(_from))->UTF);
		uint16_t* toData = ((uint16_t*)(ALIFUSTR_CAST(_result))->UTF);

		ALIFSUBUSTR_CONVERT_BYTES(uint32_t, uint16_t,
			fromData + _fromStart,
			fromData + _fromStart + _length,
			toData + start_);
	}
	else {
		std::wcout << L"ترميز الحروف غير صحيح\n" << std::endl;
		exit(-1);
	}

}

static AlifObject* uStr_wChar(uint32_t _ch)
{
	AlifObject* uStr = alifNew_uStr(1, _ch);
	if (uStr == nullptr)
		return nullptr;

	if (ALIFUSTR_CAST(uStr)->kind_ == USTR_2BYTE) {
		((uint16_t*)(ALIFUSTR_CAST(uStr)->UTF))[0] = (uint16_t)_ch;
	}
	else {
		((uint32_t*)(ALIFUSTR_CAST(uStr)->UTF))[0] = _ch;
	}
	return uStr;
}


AlifObject* uStr_decode_utf8(const wchar_t* _uStr, AlifSizeT _length) {

	if (_length == 0) {
		// return empty _object string
	}

	AlifSizeT maxChar = find_maxChar(_uStr);

	AlifObject* object_ = alifNew_uStr(_length, maxChar);

	copy_string((AlifObject*)object_, _uStr, _length, maxChar);

	return object_;

}

AlifObject* alifUStr_decodeUTF8Stateful(const wchar_t* _s, size_t _size, const wchar_t* _errors, size_t* _consumed) {
	return uStr_decode_utf8(_s, _size);
}

// من هنا يتم عمل كائن جديد ويتم فك ترميز النص وتخزينه في الكائن
AlifObject* alifUStr_decodeStringToUTF8(const wchar_t* _uStr) {

	return uStr_decode_utf8(_uStr, count_characters(_uStr));
}

AlifObject* alifUStr_fromString(const wchar_t* _u){

	size_t size_ = count_characters(_u);
	if (size_ > LLONG_MAX) {
		return nullptr;
	}
	return alifUStr_decodeUTF8Stateful(_u, size_, nullptr, nullptr);

}

bool isOnlyAscii(AlifUStrObject* _obj) {
	wchar_t* utf8_ = ((wchar_t*)(_obj->UTF));
	for (size_t i_ = 0; i_ < _obj->length_; ++i_) {
		if (((uint16_t)(utf8_[i_])) > 127) {
			return false;
		}
	}
	return true;
}

uint32_t utf8StringToCodePoint(const wchar_t* _utf8String) {

	if ((_utf8String[0] & 0xE0) == 0xC0) {
		// 2-byte sequence
		return (((uint32_t)(_utf8String[0] & 0x1F) << 6) |
			((uint32_t)(_utf8String[1] & 0x3F) << 0));
	}
	else if ((_utf8String[0] & 0xF8) == 0xF0) {
		// 4-byte sequence
		return (((uint32_t)(_utf8String[0] & 0x07) << 18) |
			((uint32_t)(_utf8String[1] & 0x3F) << 12) |
			((uint32_t)(_utf8String[2] & 0x3F) << 6) |
			((uint32_t)(_utf8String[3] & 0x3F) << 0));
	}
	//else {
		//return nullptr
	//}
}

bool isDecimalDigit(uint32_t _ch) {
	return (_ch >= 0x0030 && _ch <= 0x0039);
}

int toDecimalValue(uint32_t _ch) {
	switch (_ch) {
	case 0x0030: return 0;
	case 0x0031: return 1;
	case 0x0032: return 2;
	case 0x0033: return 3;
	case 0x0034: return 4;
	case 0x0035: return 5;
	case 0x0036: return 6;
	case 0x0037: return 7;
	case 0x0038: return 8;
	case 0x0039: return 9;
	}
}

AlifObject* alifSubUStr_transformDecimalAndSpaceToASCII(AlifObject* _uStr)
{

	if (isOnlyAscii(ALIFUSTR_CAST(_uStr))) {
		return ALIF_NEWREF(_uStr);
	}

	int64_t len_ = ALIFUSTR_CAST((_uStr))->length_;
	AlifObject* result_ = alifNew_uStr(len_, 2);
	if (result_ == nullptr) {
		return nullptr;
	}

	uint16_t* out_ = ((uint16_t*)ALIFUSTR_CAST(result_)->UTF);
	int kind_ = ALIFUSTR_CAST(_uStr)->kind_;
	const void* data_ = ALIFUSTR_CAST(_uStr)->UTF;
	int64_t i_;
	for (i_ = 0; i_ < len_; ++i_) {
		uint32_t ch_ = ALIFUSTR_READ(kind_, data_, i_);
		if (ch_ < 127) {
			out_[i_] = ch_;
		}
		//else if (ALIF_USTR_ISSPACE(_ch)) {
			//out_[i_] = ' ';
		//}
		else {
			int decimal_ = toDecimalValue(ch_);
			if (decimal_ < 0) {
				out_[i_] = L'?';
				out_[i_ + 1] = L'\0';
				ALIFUSTR_CAST(result_)->length_ = i_ + 1;
				break;
			}
			out_[i_] = L'0' + decimal_;
		}
	}

	return result_;
}

AlifSizeT length_uStr(AlifObject* _uStr) {
	return ALIFUSTR_CAST(_uStr)->length_;
}

static int64_t anyLib_find(int _kind, AlifObject* _str1, const void* _buf1, int64_t _len1,
	AlifObject* _str2, const void* _buf2, int64_t _len2, int64_t _offset)
{
	switch (_kind) {
	case USTR_2BYTE:
		return find((uint16_t*)_buf1, _len1, (uint16_t*)_buf2, _len2, _offset);
	case USTR_4BYTE:
		return find((uint32_t*)_buf1, _len1, (uint32_t*)_buf2, _len2, _offset);
	}
	//UNREACHABLE();
}

static int64_t anyLib_count(int _kind, AlifObject* _str1, const void* _buf1, int64_t _len1,
	AlifObject* _str2, const void* _buf2, int64_t _len2, int64_t _offset)
{
	switch (_kind) {
	case USTR_2BYTE:
		return count((uint16_t*)_buf1, _len1, (uint16_t*)_buf2, _len2, _offset);
	case USTR_4BYTE:
		return count((uint32_t*)_buf1, _len1, (uint32_t*)_buf2, _len2, _offset);
	}
	//UNREACHABLE();
}

static void* uStr_asKind(int _sKind, void const* _data, int64_t _len, int _kind)
{
	void* result_{};
	// هنا يوجد تحقق واحد لانه من الاعلى للادني ولا يوجد سوى نوعين 2byte or 4byte فاما ان يكون 2byte فيبقى كما هو  او 4byte فيتحول للادنى
	switch (_kind) {
	case USTR_4BYTE:
		result_ = (uint32_t*)alifMem_dataAlloc(_len);
		//if (!result_)
			//return Err_NoMemory();
		if (_sKind == USTR_2BYTE) {
			ALIFSUBUSTR_CONVERT_BYTES(
				uint16_t, uint32_t,
				(const uint16_t*)_data,
				((const uint16_t*)_data) + _len,
				result_);
		}

		return result_;
	default:
		//UNREACHABLE();
		return nullptr;
	}
}

#define MAX_INTMAX_CHARS (5 + (sizeof(intmax_t)*8-1) / 3)

// in file AlifCore_format.h
#define F_LJUST (1<<0)
#define F_SIGN  (1<<1)
#define F_BLANK (1<<2)
#define F_ALT   (1<<3)
#define F_ZERO  (1<<4)

static int uStr_fromFormat_write_str(AlifSubUStrWriter* _writer, AlifObject* _str, int64_t _width, int64_t _precision, int _flags)
{
	int64_t length_, fill_, argLen;
	uint32_t maxChar;

	length_ = ALIFUSTR_GET_LENGTH(_str);
	if ((_precision == -1 || _precision >= length_)
		&& _width <= length_)
		return alifSubUStrWriter_writeStr(_writer, _str);

	if (_precision != -1)
		length_ = ALIF_MIN(_precision, length_);

	argLen = ALIF_MAX(length_, _width);
	if (ALIFUSTR_MAX_CHAR_VALUE(_str) > _writer->maxChar)
		maxChar = alifUStr_maxCharValue(_str);
	else
		maxChar = _writer->maxChar;

	//if (alifSubUStrWriter_prepare(_writer, argLen, maxChar) == -1)
		//return -1;

	fill_ = ALIF_MAX(_width - length_, 0);
	if (fill_ && !(_flags & F_LJUST)) {
		if (alifUStr_fill(_writer->buffer_, _writer->pos_, fill_, ' ') == -1)
			return -1;
		_writer->pos_ += fill_;
	}

	alifSubUStr_fastCopyCharacters(_writer->buffer_, _writer->pos_,
		_str, 0, length_, 0);
	_writer->pos_ += length_;

	if (fill_ && (_flags & F_LJUST)) {
		if (alifUStr_fill(_writer->buffer_, _writer->pos_, fill_, ' ') == -1)
			return -1;
		_writer->pos_ += fill_;
	}

	return 0;
}

static int uStr_fromFormat_write_cStr(AlifSubUStrWriter* _writer, const wchar_t* _str,
	int64_t _width, int64_t _precision, int _flags)
{
	/* UTF-8 */
	int64_t length_;
	AlifObject* uStr;
	int res_;

	if (_precision == -1) {
		length_ = wcslen(_str);
	}
	else {
		length_ = 0;
		while (length_ < _precision && _str[length_]) {
			length_++;
		}
	}
	uStr = alifUStr_decodeUTF8Stateful(_str, length_, L"replace", nullptr);
	if (uStr == nullptr)
		return -1;

	res_ = uStr_fromFormat_write_str(_writer, uStr, _width, -1, _flags);
	ALIF_DECREF(uStr);
	return res_;
}


static int uStr_fromFormat_write_wCStr(AlifSubUStrWriter* _writer, const wchar_t* _str,
	int64_t _width, int64_t _precision, int _flags)
{
	/* UTF-8 */
	int64_t length_;
	AlifObject* uStr;
	int res_;

	if (_precision == -1) {
		length_ = wcslen(_str);
	}
	else {
		length_ = 0;
		while (length_ < _precision && _str[length_]) {
			length_++;
		}
	}
	uStr = alifUStr_fromWideChar(_str, length_);
	if (uStr == nullptr)
		return -1;

	res_ = uStr_fromFormat_write_str(_writer, uStr, _width, -1, _flags);
	ALIF_DECREF(uStr);
	return res_;
}

#define F_LONG 1
#define F_LONGLONG 2
#define F_SIZE 3
#define F_PTRDIFF 4
#define F_INTMAX 5
static const wchar_t* const _formats_[] = { L"%d", L"%ld", L"%lld", L"%zd", L"%td", L"%jd" };
static const wchar_t* const _formatsO_[] = { L"%o", L"%lo", L"%llo", L"%zo", L"%to", L"%jo" };
static const wchar_t* const _formatsU_[] = { L"%u", L"%lu", L"%llu", L"%zu", L"%tu", L"%ju" };
static const wchar_t* const _formatsX_[] = { L"%x", L"%lx", L"%llx", L"%zx", L"%tx", L"%jx" };
static const wchar_t* const _formatsXC_[] = { L"%X", L"%lX", L"%llX", L"%zX", L"%tX", L"%jX" };

static const wchar_t* uStr_fromFormat_arg(AlifSubUStrWriter* _writer,
	const wchar_t* _f, va_list* _vArgs)
{
	const wchar_t* p_;
	int64_t len_;
	int flags_ = 0;
	int64_t width_;
	int64_t precision_;

	p_ = _f;
	_f++;
	if (*_f == L'%') {
		if (alifSubUStrWriter_writeWcharInline(_writer, L'%') < 0)
			return nullptr;
		_f++;
		return _f;
	}

	while (1) {
		switch (*_f++) {
		case L'-': flags_ |= F_LJUST; continue;
		case L'0': flags_ |= F_ZERO; continue;
		}
		_f--;
		break;
	}

	width_ = -1;
	if (*_f == L'*') {
		width_ = va_arg(*_vArgs, int);
		if (width_ < 0) {
			flags_ |= F_LJUST;
			width_ = -width_;
		}
		_f++;
	}
	else if (ALIF_ISDIGIT((unsigned)*_f)) {
		width_ = *_f - L'0';
		_f++;
		while (ALIF_ISDIGIT((unsigned)*_f)) {
			if (width_ > (LLONG_MAX - ((int)*_f - L'0')) / 10) {
				return nullptr;
			}
			width_ = (width_ * 10) + (*_f - L'0');
			_f++;
		}
	}
	precision_ = -1;
	if (*_f == L'.') {
		_f++;
		if (*_f == L'*') {
			precision_ = va_arg(*_vArgs, int);
			if (precision_ < 0) {
				precision_ = -2;
			}
			_f++;
		}
		else if (ALIF_ISDIGIT((unsigned)*_f)) {
			precision_ = (*_f - L'0');
			_f++;
			while (ALIF_ISDIGIT((unsigned)*_f)) {
				if (precision_ > (LLONG_MAX - ((int)*_f - L'0')) / 10) {

					return nullptr;
				}
				precision_ = (precision_ * 10) + (*_f - L'0');
				_f++;
			}
		}
	}

	int sizeMod = 0;
	if (*_f == L'l') {
		if (_f[1] == L'l') {
			sizeMod = F_LONGLONG;
			_f += 2;
		}
		else {
			sizeMod = F_LONG;
			++_f;
		}
	}
	else if (*_f == L'z') {
		sizeMod = F_SIZE;
		++_f;
	}
	else if (*_f == L't') {
		sizeMod = F_PTRDIFF;
		++_f;
	}
	else if (*_f == L'j') {
		sizeMod = F_INTMAX;
		++_f;
	}
	if (_f[0] != L'\0' && _f[1] == L'\0')
		_writer->overAllocate = 0;

	switch (*_f) {
	case L'd': case L'i': case L'o': case L'u': case L'x': case L'X':
		break;
	case L'c': case L'p':
		if (sizeMod || width_ >= 0 || precision_ >= 0) goto invalidFormat;
		break;
	case L's':
	case L'V':
		if (sizeMod && sizeMod != F_LONG) goto invalidFormat;
		break;
	default:
		if (sizeMod) goto invalidFormat;
		break;
	}

	switch (*_f) {
	case L'c':
	{
		int ordinal_ = va_arg(*_vArgs, int);
		if (ordinal_ < 0 || ordinal_ > 0x10ffff) {
			return nullptr;
		}
		if (alifSubUStrWriter_writeWcharInline(_writer, ordinal_) < 0)
			return nullptr;
		break;
	}

	case L'd': case L'i':
	case L'o': case L'u': case L'x': case L'X':
	{
		wchar_t buffer_[MAX_INTMAX_CHARS];
		const wchar_t* fmt_ = nullptr;
		switch (*_f) {
		case L'o': fmt_ = _formatsO_[sizeMod]; break;
		case L'u': fmt_ = _formatsU_[sizeMod]; break;
		case L'x': fmt_ = _formatsX_[sizeMod]; break;
		case L'X': fmt_ = _formatsXC_[sizeMod]; break;
		default: fmt_ = _formats_[sizeMod]; break;
		}
		int isSigned = (*_f == L'd' or *_f == L'i');
		switch (sizeMod) {
		case F_LONG:
			len_ = isSigned ?
				swprintf(buffer_, sizeof(long), fmt_, va_arg(*_vArgs, long)) :
				swprintf(buffer_, sizeof(unsigned long), fmt_, va_arg(*_vArgs, unsigned long));
			break;
		case F_LONGLONG:
			len_ = isSigned ?
				swprintf(buffer_, sizeof(long long), fmt_, va_arg(*_vArgs, long long)) :
				swprintf(buffer_, sizeof(unsigned long long), fmt_, va_arg(*_vArgs, unsigned long long));
			break;
		case F_SIZE:
			len_ = isSigned ?
				swprintf(buffer_, sizeof(int64_t), fmt_, va_arg(*_vArgs, int64_t)) :
				swprintf(buffer_, sizeof(size_t), fmt_, va_arg(*_vArgs, size_t));
			break;
		case F_PTRDIFF:
			len_ = swprintf(buffer_, sizeof(ptrdiff_t), fmt_, va_arg(*_vArgs, ptrdiff_t));
			break;
		case F_INTMAX:
			len_ = isSigned ?
				swprintf(buffer_, sizeof(intmax_t), fmt_, va_arg(*_vArgs, intmax_t)) :
				swprintf(buffer_, sizeof(uintmax_t), fmt_, va_arg(*_vArgs, uintmax_t));
			break;
		default:
			len_ = isSigned ?
				swprintf(buffer_, sizeof(int), fmt_, va_arg(*_vArgs, int)) :
				swprintf(buffer_, sizeof(unsigned int), fmt_, va_arg(*_vArgs, unsigned int));
			break;
		}

		int sign_ = (buffer_[0] == '-');
		len_ -= sign_;

		precision_ = ALIF_MAX(precision_, len_);
		width_ = ALIF_MAX(width_, precision_ + sign_);
		if ((flags_ & F_ZERO) && !(flags_ & F_LJUST)) {
			precision_ = width_ - sign_;
		}

		int64_t spacePad = ALIF_MAX(width_ - precision_ - sign_, 0);
		int64_t zeroPad = ALIF_MAX(precision_ - len_, 0);

		if (ALIFSUBUSTRWRITER_PREPARE(_writer, width_, 127) == -1)
			return nullptr;

		if (spacePad && !(flags_ & F_LJUST)) {
			if (alifUStr_fill(_writer->buffer_, _writer->pos_, spacePad, ' ') == -1)
				return nullptr;
			_writer->pos_ += spacePad;
		}

		if (sign_) {
			if (alifSubUStrWriter_writeChar(_writer, '-') == -1)
				return nullptr;
		}

		if (zeroPad) {
			if (alifUStr_fill(_writer->buffer_, _writer->pos_, zeroPad, L'0') == -1)
				return nullptr;
			_writer->pos_ += zeroPad;
		}

		if (spacePad && (flags_ & F_LJUST)) {
			if (alifUStr_fill(_writer->buffer_, _writer->pos_, spacePad, L' ') == -1)
				return nullptr;
			_writer->pos_ += spacePad;
		}
		break;
	}

	case L'p':
	{
		wchar_t number_[MAX_INTMAX_CHARS];

		len_ = swprintf(number_, MAX_INTMAX_CHARS, L"%p", va_arg(*_vArgs, void*));

		if (number_[1] == L'X')
			number_[1] = L'x';
		else if (number_[1] != L'x') {
			memmove(number_ + 2, number_,
				wcslen(number_) + 1);
			number_[0] = L'0';
			number_[1] = L'x';
			len_ += 2;
		}

		break;
	}

	case L's':
	{
		if (sizeMod) {
			const wchar_t* s = va_arg(*_vArgs, const wchar_t*);
			if (uStr_fromFormat_write_wCStr(_writer, s, width_, precision_, flags_) < 0)
				return nullptr;
		}
		else {
			/* UTF-8 */
			const wchar_t* s = va_arg(*_vArgs, const wchar_t*);
			if (uStr_fromFormat_write_wCStr(_writer, s, width_, precision_, flags_) < 0)
				return nullptr;
		}
		break;
	}

	case L'U':
	{
		AlifObject* obj = va_arg(*_vArgs, AlifObject*);

		if (uStr_fromFormat_write_str(_writer, obj, width_, precision_, flags_) == -1)
			return nullptr;
		break;
	}

	case L'V':
	{
		AlifObject* obj_ = va_arg(*_vArgs, AlifObject*);
		const wchar_t* str_;
		const wchar_t* wstr_;
		if (sizeMod) {
			wstr_ = va_arg(*_vArgs, const wchar_t*);
		}
		else {
			str_ = va_arg(*_vArgs, const wchar_t*);
		}
		if (obj_) {
			if (uStr_fromFormat_write_str(_writer, obj_, width_, precision_, flags_) == -1)
				return nullptr;
		}
		else if (sizeMod) {
			if (uStr_fromFormat_write_wCStr(_writer, wstr_, width_, precision_, flags_) < 0)
				return nullptr;
		}
		else {
			if (uStr_fromFormat_write_cStr(_writer, str_, width_, precision_, flags_) < 0)
				return nullptr;
		}
		break;
	}

	case L'S':
	{
		AlifObject* obj_ = va_arg(*_vArgs, AlifObject*);
		AlifObject* str_;
		//str_ = Object_Str(obj_);
		if (!str_)
			return nullptr;
		if (uStr_fromFormat_write_str(_writer, str_, width_, precision_, flags_) == -1) {
			ALIF_DECREF(str_);
			return nullptr;
		}
		ALIF_DECREF(str_);
		break;
	}

	case L'R':
	{
		AlifObject* obj_ = va_arg(*_vArgs, AlifObject*);
		AlifObject* repr;
		//repr = alifObject_Repr(obj_);
		if (!repr)
			return nullptr;
		if (uStr_fromFormat_write_str(_writer, repr, width_, precision_, flags_) == -1) {
			ALIF_DECREF(repr);
			return nullptr;
		}
		ALIF_DECREF(repr);
		break;
	}

	case L'A':
	{
		AlifObject* obj_ = va_arg(*_vArgs, AlifObject*);
		AlifObject* ascii;
		//ascii = alifObject_ASCII(obj_);
		if (!ascii)
			return nullptr;
		if (uStr_fromFormat_write_str(_writer, ascii, width_, precision_, flags_) == -1) {
			ALIF_DECREF(ascii);
			return nullptr;
		}
		ALIF_DECREF(ascii);
		break;
	}

	case L'T':
	{
		AlifObject* obj_ = va_arg(*_vArgs, AlifObject*);
		AlifTypeObject* type = (AlifTypeObject*)ALIF_NEWREF(ALIF_TYPE(obj_));

		AlifObject* type_name;
		if (_f[1] == L'#') {
			//type_name = Type_GetFullyQualifiedName(type, ':');
			_f++;
		}
		else {
			//type_name = Type_GetFullyQualifiedName(type);
		}
		ALIF_DECREF(type);
		if (!type_name) {
			return nullptr;
		}

		if (uStr_fromFormat_write_str(_writer, type_name,
			width_, precision_, flags_) == -1) {
			ALIF_DECREF(type_name);
			return nullptr;
		}
		ALIF_DECREF(type_name);
		break;
	}

	case L'N':
	{
		AlifObject* typeRaw = va_arg(*_vArgs, AlifObject*);

		//if (!Type_Check(typeRaw)) {
			//return nullptr;
		//}
		AlifTypeObject* type_ = (AlifTypeObject*)typeRaw;
		
		AlifObject* typeName;
		if (_f[1] == L'#') {
			//typeName = Type_GetFullyQualifiedName(type_, ':');
			_f++;
		}
		else {
			//typeName = Type_GetFullyQualifiedName(type_);
		}
		if (!typeName) {
			return nullptr;
		}
		if (uStr_fromFormat_write_str(_writer, typeName,
			width_, precision_, flags_) == -1) {
			ALIF_DECREF(typeName);
			return nullptr;
		}
		ALIF_DECREF(typeName);
		break;
	}

	default:
	invalidFormat:
		return nullptr;
	}

	_f++;
	return _f;
}

const wchar_t* alifUStr_asUTF8AndSize(AlifObject* _uStr, int64_t* _pSize)
{
	if (!(_uStr->type_ == &_alifUStrType_)) {
		if (_pSize) {
			*_pSize = -1;
		}
		return nullptr;
	}

	//if (uStr_fill_utf8(_uStr) == -1) {
		//if (_pSize) {
			//*_pSize = -1;
		//}
		//return nullptr;
	//}
	

	if (_pSize) {
		*_pSize = ALIFUSTR_CAST(_uStr)->length_;
	}
	return (const wchar_t*)ALIFUSTR_CAST(_uStr)->UTF;
}

const wchar_t* alifUStr_asUTF8(AlifObject* _uStr)
{
	return alifUStr_asUTF8AndSize(_uStr, nullptr);
}

AlifObject* alifUStr_decodeUTF8(const wchar_t* _s, int64_t _size, const wchar_t* _errors)
{
	return alifUStr_decodeUTF8Stateful(_s, _size, _errors, nullptr);
}

AlifObject* combine_uStr(AlifObject* _left, AlifObject* _right) {

	AlifObject* result_;
	uint16_t maxChar;
	AlifSizeT leftLen = (ALIFUSTR_CAST(_left))->length_
		, rightLen = (ALIFUSTR_CAST(_right))->length_,
		newLen;

	if (leftLen == 0) {
		return _right;
	}
	if (rightLen == 0) {
		return _left;
	}

	newLen = leftLen + rightLen;

	maxChar = ALIF_MAX((ALIFUSTR_CAST(_left))->kind_, (ALIFUSTR_CAST(_right))->kind_);

	result_ = (AlifObject*)alifNew_uStr(newLen, maxChar);
	if (result_ == nullptr) {
		return nullptr;
	}

	combine_string(result_, 0, _left, 0, leftLen, maxChar);
	combine_string(result_, leftLen, _right, 0, rightLen, maxChar);

	return result_;

}

AlifObject* repeat_uStr(AlifObject* _uStr, AlifSizeT _repeat) {

	if (_repeat == 0) {
		// return empty string
	}

	if (_repeat == 1) {
		return _uStr;
	}

	AlifSizeT lenUStr = (ALIFUSTR_CAST(_uStr))->length_;
	AlifSizeT len_ = lenUStr * _repeat;
	uint8_t kind_ = (ALIFUSTR_CAST(_uStr))->kind_;

	AlifObject* object_ = (AlifObject*)alifNew_uStr(len_, kind_);

	if(object_ == nullptr){
		return nullptr;
	}

	if (kind_ == 2) {
		uint16_t* str = (uint16_t*)(ALIFUSTR_CAST(_uStr))->UTF;
		uint16_t* UTF = (uint16_t*)(ALIFUSTR_CAST(object_))->UTF;
		for (AlifSizeT n = 0; n < _repeat; ++n)
		{
			memcpy(UTF + (lenUStr * n), str, lenUStr * 2);
		}
	}
	else {
		uint32_t* str = (uint32_t*)(ALIFUSTR_CAST(_uStr))->UTF;
		uint32_t* UTF = (uint32_t*)(ALIFUSTR_CAST(object_))->UTF;
		for (AlifSizeT n = 0; n < _repeat; ++n)
		{
			memcpy(UTF + (lenUStr * n), str, lenUStr * 4);
		}
	}

	return object_;

}

bool contain_uStr(AlifObject* _uStr, AlifObject* _uStr2) {

    uint8_t kind1_, kind2_;
	const void* utf1_, * utf2_;
	AlifSizeT len1_, len2_;


	kind1_ = (ALIFUSTR_CAST(_uStr))->kind_;
	kind2_ = (ALIFUSTR_CAST(_uStr2))->kind_;

	if (kind1_ < kind2_) {
		return 0;
	}

	len1_ = (ALIFUSTR_CAST(_uStr))->length_;
	len2_ = (ALIFUSTR_CAST(_uStr2))->length_;

	if (len1_ < len2_) {
		return 0;
	}

	utf1_ = (ALIFUSTR_CAST(_uStr))->UTF;
	utf2_ = (ALIFUSTR_CAST(_uStr2))->UTF;
	if (kind1_ != kind2_) {
		void* result_ = (uint32_t*)alifMem_dataAlloc(len2_ * 4);
		ALIFSUBUSTR_CONVERT_BYTES(
			uint16_t, uint32_t,
			(const uint16_t*)utf2_,
			((const uint16_t*)utf2_) + len2_,
			result_);
	}

	if (kind1_ == 2) {
	
		const uint16_t* srcStr = (uint16_t*)utf1_;
		const uint16_t* subStr = (uint16_t*)utf2_;

		for (size_t i_ = 0; i_ <= len1_ - len2_; ++i_) {

			if (std::memcmp(&srcStr[i_], subStr, len2_ * sizeof(uint16_t)) == 0) {
				return true;
			}
		}
		return false;

	}
	else if (kind1_ == 4) {
		const uint32_t* srcStr = (uint32_t*)utf1_;
		const uint32_t* subStr = (uint32_t*)utf2_;

		for (size_t i_ = 0; i_ <= len1_ - len2_; ++i_) {
			
			if (std::memcmp(&srcStr[i_], subStr, len2_ * sizeof(uint32_t)) == 0) {
				return true;
			}
		}
		return false;

	}

}

/*
The constants fnvPrime and fnvOffsetBasis are part of the FNV - 1a(Fowler - Noll - Vo) hash algorithm.
The FNV hash algorithm is a non - cryptographic hash function that is often used for simple hash table implementations or other non - security - sensitive purposes.
The constants are chosen to have certain properties that help create a good spread of hash values.

In the case of FNV - 1a, the constants are as follows :
fnvPrime: 1099511628211
fnvOffsetBasis : 14695981039346656037*/
size_t hash_uStr(AlifObject* _uStr) {

	const uint64_t fnvPrime = 1099511628211ull;
	const uint64_t fnvOffsetBasis = 14695981039346656037ull;

	AlifUStrObject* self_ = ALIFUSTR_CAST(_uStr);
	
	uint8_t kind_ = self_->kind_;

	uint8_t* data_ = (uint8_t*)(self_->UTF);

	size_t hash_ = fnvOffsetBasis;
	
	if (kind_ == 2) {
		for (int i_ = 0; i_ < self_->length_; ++i_) {
			hash_ ^= *(uint16_t*)data_;
			int a_ = *(uint16_t*)data_;
			data_ += 2;
			hash_ *= fnvPrime;
		}
	}
	else {
		for (int i_ = 0; i_ < self_->length_ ; ++i_) {
			hash_ ^= *(uint32_t*)data_;
			data_ += 4;
			hash_ *= fnvPrime;
		}
	}
	return hash_;
}

int uStr_compare_eq(AlifObject* _str1, AlifObject* _str2)
{
	int kind_{};
	const void* data1_{}, * data2_{};
	size_t len_{};
	int cmp_{};

	len_ = ALIFUSTR_CAST(_str1)->length_;
	if (ALIFUSTR_CAST(_str2)->length_ != len_)
		return 0;
	kind_ = ALIFUSTR_CAST(_str1)->kind_;
	if (ALIFUSTR_CAST(_str2)->kind_ != kind_)
		return 0;
	data1_ = ALIFUSTR_CAST(_str1);
	data2_ = ALIFUSTR_CAST(_str2);

	cmp_ = memcmp(data1_, data2_, len_ * kind_);
	return (cmp_ == 0);
}

AlifObject* uStr_compare(AlifObject* _left, AlifObject* _right, int _op) {
	// this function need review

	if (_left->type_ != &_alifUStrType_ || 
		_right->type_ != &_alifUStrType_) {
		std::wcout << L"عمليه مقارنة النص غير صحيحة\n" << std::endl;
		exit(-1);
	}

	if (_left == _right && _op == ALIF_EQ) {
		return ALIF_TRUE;
	}
	else if (_op == ALIF_EQ || _op == ALIF_NE) {
		int result_ = uStr_compare_eq(_left, _right);
		return result_ ? ALIF_TRUE : ALIF_FALSE;
	}

	//return ALIF_NOTIMPLEMENTED; // need review
}

// in file eq.h
int uStr_eq(AlifObject* _a, AlifObject* _b)
{
	if (ALIFUSTR_CAST(_a)->length_ != ALIFUSTR_CAST(_b)->length_)
		return 0;
	if (ALIFUSTR_CAST(_a)->length_ == 0)
		return 1;
	if (ALIFUSTR_CAST(_a)->kind_ != ALIFUSTR_CAST(_b)->kind_)
		return 0;
	return memcmp((uint8_t*)(ALIFUSTR_CAST(_a)->UTF), (uint8_t*)(ALIFUSTR_CAST(_b)->UTF),
		ALIFUSTR_CAST(_a)->length_ * ALIFUSTR_CAST(_a)->kind_) == 0;
}

void uStr_dealloc(AlifObject* _uStr) {
	alifMem_objFree(_uStr);
}

/* helper macro to fixup start_/end slice values */
#define ADJUST_INDICES(_start, _end, _len)         \
    if (_end > _len)                              \
        _end = _len;                              \
    else if (_end < 0) {                         \
        _end += _len;                             \
        if (_end < 0)                            \
            _end = 0;                            \
    }                                           \
    if (_start < 0) {                            \
        _start += _len;                           \
        if (_start < 0)                          \
            _start = 0;                          \
    }

static int64_t any_find_slice(AlifObject* _s1, AlifObject* _s2, int64_t _start, int64_t _end, int _direction)
{
	int kind1_, kind2_;
	const void* buf1_, * buf2_;
	int64_t len1_, len2_, result_;

	kind1_ = ALIFUSTR_CAST(_s1)->kind_;
	kind2_ = ALIFUSTR_CAST(_s2)->kind_;
	if (kind1_ < kind2_)
		return -1;

	len1_ = ALIFUSTR_CAST(_s1)->length_;
	len2_ = ALIFUSTR_CAST(_s2)->length_;
	ADJUST_INDICES(_start, _end, len1_);
	if (_end - _start < len2_)
		return -1;

	buf1_ = ALIFUSTR_CAST(_s1)->UTF;
	buf2_ = ALIFUSTR_CAST(_s2)->UTF;
	if (len2_ == 1) {
		uint32_t ch_ = ALIFUSTR_READ(kind2_, buf2_, 0);
		result_ = findChar((const wchar_t*)buf1_ + kind1_ * _start,
			kind1_, _end - _start, ch_, _direction);
		if (result_ == -1)
			return -1;
		else
			return _start + result_;
	}

	if (kind2_ != kind1_) {
		buf2_ = uStr_asKind(kind2_, buf2_, len2_, kind1_);
		if (!buf2_)
			return -2;
	}

	if (_direction > 0) {
		switch (kind1_) {
		case USTR_2BYTE:
			result_ = find_slice((uint16_t*)buf1_, len1_, (uint16_t*)buf2_, len2_, _start, _end);
			break;
		case USTR_4BYTE:
			result_ = find_slice((uint32_t*)buf1_, len1_, (uint32_t*)buf2_, len2_, _start, _end);
			break;
		//default:
			//UNREACHABLE();
		}
	}
	else {
		switch (kind1_) {
		case USTR_2BYTE:
			result_ = rfind_slice((uint16_t*)buf1_, len1_, (uint16_t*)buf2_, len2_, _start, _end);
			break;
		case USTR_4BYTE:
			result_ = rfind_slice((uint32_t*)buf1_, len1_, (uint32_t*)buf2_, len2_, _start, _end);
			break;
		//default:
			//UNREACHABLE();
		}
	}

	if (kind2_ != kind1_)
		alifMem_dataFree((void*)buf2_);

	return result_;
}

// this function in file replace.h
template <typename STRINGLIB_CHAR>
void ucsLib_replace_1char_inplace(STRINGLIB_CHAR* _s, STRINGLIB_CHAR* _end,
	uint32_t _u1, uint32_t _u2, int64_t _maxCount)
{
	*_s = _u2;
	while (--_maxCount && ++_s != _end) {

		if (*_s != _u1) {
			int attempts_ = 10;
			while (1) {
				if (++_s == _end)
					return;
				if (*_s == _u1)
					break;
				if (!--attempts_) {

#ifdef STRINGLIB_FAST_MEMCHR
					_s++;
					_s = (STRINGLIB_CHAR*)STRINGLIB_FAST_MEMCHR(_s, _u1, _end - _s);
					if (_s == nullptr)
						return;
#else
					int64_t i_;
					STRINGLIB_CHAR ch1_ = (STRINGLIB_CHAR)_u1;
					_s++;
					i_ = FASTSEARCH(_s, _end - _s, &ch1_, 1, 0, FAST_SEARCH);
					if (i_ < 0)
						return;
					_s += i_;
#endif
					break;
				}
			}
		}
		*_s = _u2;
	}
}

static inline int parse_args_finds_uStr(const wchar_t* _functionName, AlifObject* _args,
	AlifObject** _substring,
	int64_t* _start, int64_t* _end)
{
	if (parse_args_finds(_functionName, _args, _substring, _start, _end)) {
		return 1;
	}
	return 0;
}

static void replace_1char_inplace(AlifObject* _u, int64_t _pos,
	uint32_t _u1, uint32_t _u2, int64_t _maxCount)
{
	int kind_ = ALIFUSTR_CAST(_u)->kind_;
	void* data_ = ALIFUSTR_CAST(_u)->UTF;
	int64_t len_ = ALIFUSTR_CAST(_u)->length_;
	if (kind_ == USTR_2BYTE) {
		ucsLib_replace_1char_inplace((uint16_t*)data_ + _pos,
			(uint16_t*)data_ + len_,
			_u1, _u2, _maxCount);
	}
	else {
		ucsLib_replace_1char_inplace((uint32_t*)data_ + _pos,
			(uint32_t*)data_ + len_,
			_u1, _u2, _maxCount);
	}
}

AlifObject* alifUStr_concat(AlifObject* _left, AlifObject* _right)
{
	AlifObject* result_;
	AlifUCS4 maxChar, maxChar2;
	int64_t leftLen, rightLen, newLen;

	if (!(_left->type_ == &_alifUStrType_) )
		return nullptr;

	if (!(_right->type_ == &_alifUStrType_)) {
		return nullptr;
	}

	//AlifObject* empty = ustr_get_empty();  
	//if (_left == empty) {
		//return alifUst_FromObject(_right);
	//}
	//if (_right == empty) {
		//return alifUst_FromObject(_left);
	//}

	leftLen = ALIFUSTR_GET_LENGTH(_left);
	rightLen = ALIFUSTR_GET_LENGTH(_right);
	if (leftLen > INTPTR_MAX - rightLen) {
		return nullptr;
	}
	newLen = leftLen + rightLen;

	maxChar = ALIFUSTR_MAX_CHAR_VALUE(_left);
	maxChar2 = ALIFUSTR_MAX_CHAR_VALUE(_right);
	maxChar = ALIF_MAX(maxChar, maxChar2);

	result_ = alifNew_uStr(newLen, maxChar);
	if (result_ == nullptr)
		return nullptr;
	alifSubUStr_fastCopyCharacters(result_, 0, _left, 0, leftLen, 0);
	alifSubUStr_fastCopyCharacters(result_, leftLen, _right, 0, rightLen, 0);
	return result_;
}

void alifUStr_append(AlifObject** _pLeft, AlifObject* _right)
{
	AlifObject* left_, * res_;
	AlifUCS4 maxChar, maxChar2;
	int64_t leftLen, rightLen, newLen;

	if (_pLeft == nullptr) {
		return;
	}
	left_ = *_pLeft;
	if (_right == nullptr || left_ == nullptr
		|| !(left_->type_ == &_alifUStrType_) || !(_right->type_ == &_alifUStrType_)) {
		goto error;
	}

	//AlifObject* empty = uStr_get_empty();  // Borrowed reference
	//if (left_ == empty) {
		//ALIF_DECREF(left_);
		//*_pLeft = ALIF_NEWREF(_right);
		//return;
	//}
	//if (_right == empty) {
		//return;
	//}

	leftLen = ALIFUSTR_GET_LENGTH(left_);
	rightLen = ALIFUSTR_GET_LENGTH(_right);
	if (leftLen > INTPTR_MAX - rightLen) {
		goto error;
	}
	newLen = leftLen + rightLen;

	if (uStr_modifiable(left_)
		&& (_right->type_ == &_alifUStrType_)
		&& ALIFUSTR_KIND(_right) <= ALIFUSTR_KIND(left_))
	{
		if (uStr_resize(_pLeft, newLen) != 0)
			goto error;

		alifSubUStr_fastCopyCharacters(*_pLeft, leftLen, _right, 0, rightLen, 0);
	}
	else {
		maxChar = ALIFUSTR_MAX_CHAR_VALUE(left_);
		maxChar2 = ALIFUSTR_MAX_CHAR_VALUE(_right);
		maxChar = ALIF_MAX(maxChar, maxChar2);

		res_ = alifNew_uStr(newLen, maxChar);
		if (res_ == nullptr)
			goto error;
		alifSubUStr_fastCopyCharacters(res_, 0, left_, 0, leftLen ,0 );
		alifSubUStr_fastCopyCharacters(res_, leftLen, _right, 0, rightLen,0);
		ALIF_DECREF(left_);
		*_pLeft = res_;
	}
	return;

error:
	ALIF_CLEAR(*_pLeft);
}


static int64_t uStr_countImpl(AlifObject* _uStr, AlifObject* _substr, int64_t _start, int64_t _end)
{

	int64_t result_;
	int kind1_, kind2_;
	const void* buf1_ = nullptr, * buf2_ = nullptr;
	int64_t len1_, len2_;

	kind1_ = ALIFUSTR_CAST(_uStr)->kind_;
	kind2_ = ALIFUSTR_CAST(_substr)->kind_;
	if (kind1_ < kind2_)
		return 0;

	len1_ = ALIFUSTR_CAST(_uStr)->length_;
	len2_ = ALIFUSTR_CAST(_substr)->length_;
	ADJUST_INDICES(_start, _end, len1_);
	if (_end - _start < len2_)
		return 0;

	buf1_ = ALIFUSTR_CAST(_uStr)->UTF;
	buf2_ = ALIFUSTR_CAST(_substr)->UTF;
	if (kind2_ != kind1_) {
		buf2_ = uStr_asKind(kind2_, buf2_, len2_, kind1_);
		if (!buf2_)
			goto onError;
	}

	switch (kind1_) {
	case USTR_2BYTE:
		result_ = count(
			((const uint16_t*)buf1_) + _start, _end - _start,
			(uint16_t*)buf2_, len2_, LLONG_MAX
		);
		break;
	case USTR_4BYTE:
		result_ = count(
			((const uint32_t*)buf1_) + _start, _end - _start,
			(uint32_t*)buf2_, len2_, LLONG_MAX
		);
		break;
	//default:
		//UNREACHABLE();
	}

	if (kind2_ != kind1_)
		alifMem_dataFree((void*)buf2_);

	return result_;
onError:
	if (kind2_ != kind1_)
		alifMem_dataFree((void*)buf2_);
	return -1;
}

static AlifObject* uStr_count(AlifObject* _self, AlifObject* _args)
{
	AlifObject* substring_ = nullptr;  
	int64_t start_ = 0;
	int64_t end_ = LLONG_MAX;
	int64_t result_;

	if (!parse_args_finds_uStr(L"count", _args, &substring_, &start_, &end_))
		return nullptr;

	result_ = uStr_countImpl(_self, substring_, start_, end_);
	if (result_ == -1)
		return nullptr;

	return alifInteger_fromLongLong(result_);
}

static AlifObject* replace(AlifObject*, AlifObject* ,
	AlifObject*, int64_t );

static AlifObject* uStr_replace(AlifObject* _self, AlifObject* const* _args, int64_t _nArgs, AlifObject* _kWNames)
{ // this should transfer to UStrObject.c.h or .h
	AlifObject* returnValue = nullptr;

#define NUM_KEYWORDS 1
	static class KwTuple{
	public:
		AlifVarObject base{};
		AlifObject* item_[NUM_KEYWORDS];
	}
	kwTuple = {
		//ALIFVAROBJECT_HEAD_INIT(&typeTuple, 1)
		//nullptr
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&kwTuple.base.object)

	static const wchar_t* const keywords_[] = { L"", L"", L"count", nullptr };
	static AlifArgParser parser_ = {
		0,
		L"",
		keywords_,
		L"replace",
		L"",
		0,
		0,
		0,
		//KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsBuf[3];
	int64_t nOptArgs = _nArgs + (_kWNames ? ((AlifVarObject*)_kWNames)->size_ : 0) - 2;
	AlifObject* old_;
	AlifObject* new_;
	int64_t count_ = -1;

	_args = ALIFARG_UNPACKKEYWORDS(_args, _nArgs, nullptr, _kWNames, &parser_, 2, 3, 0, argsBuf);
	if (!_args) {
		goto exit;
	}
	if (!(_args[0]->type_ == &_alifUStrType_)) {
		//_Arg_BadArgument("replace", "argument 1", "_uStr", args[0]);
		goto exit;
	}
	old_ = _args[0];
	if (!(_args[1]->type_ == &_alifUStrType_)) {
		//_Arg_BadArgument("replace", "argument 2", "_uStr", args[1]);
		goto exit;
	}
	new_ = _args[1];
	if (!nOptArgs) {
		goto skipOptionalPos;
	}
	{
		int64_t ival = -1;
		AlifObject* iobj = _args[2];
		if (iobj != nullptr) {
			ival = alifInteger_asLongLong(iobj);
			ALIF_DECREF(iobj);
		}

		count_ = ival;
	}
skipOptionalPos:
	returnValue = replace(_self, old_, new_, count_);

exit:
	return returnValue;
}

static AlifObject* replace(AlifObject* _self, AlifObject* _str1,
	AlifObject* _str2, int64_t _maxCount)
{
	AlifObject* u_{};
	const wchar_t* sBuf = (const wchar_t*)ALIFUSTR_CAST(_self)->UTF;
	const void* buf1_ = (const wchar_t*)ALIFUSTR_CAST(_str1)->UTF;
	const void* buf2_ = (const wchar_t*)ALIFUSTR_CAST(_str2)->UTF;
	int srelease = 0, release1 = 0, release2 = 0;
	int sKind = ALIFUSTR_CAST(_self)->kind_;
	int kind1_ = ALIFUSTR_CAST(_str1)->kind_;
	int kind2_ = ALIFUSTR_CAST(_str2)->kind_;
	int64_t sLen = ALIFUSTR_CAST(_self)->length_;
	int64_t len1_ = ALIFUSTR_CAST(_str1)->length_;
	int64_t len2_ = ALIFUSTR_CAST(_str2)->length_;
	int mayShrink;
	uint32_t maxChar, maxCharStr1, maxCharStr2;

	if (sLen < len1_)
		goto nothing;

	if (_maxCount < 0)
		_maxCount = LLONG_MAX;
	else if (_maxCount == 0)
		goto nothing;

	if (_str1 == _str2)
		goto nothing;

	maxChar = ALIFUSTR_MAX_CHAR_VALUE(_self);
	maxCharStr1 = ALIFUSTR_MAX_CHAR_VALUE(_str1);
	if (maxChar < maxCharStr1)
		goto nothing;
	maxCharStr2 = ALIFUSTR_MAX_CHAR_VALUE(_str2);

	mayShrink = (maxCharStr2 < maxCharStr1) && (maxChar == maxCharStr1);
	maxChar = ALIF_MAX(maxChar, maxCharStr2);

	if (len1_ == len2_) {
		if (len1_ == 0)
			goto nothing;
		if (len1_ == 1) {
			uint32_t u1, u2;
			int64_t pos;

			u1 = ALIFUSTR_READ(kind1_, buf1_, 0);
			pos = findChar(sBuf, sKind, sLen, u1, 1);
			if (pos < 0)
				goto nothing;
			u2 = ALIFUSTR_READ(kind2_, buf2_, 0);
			u_ = alifNew_uStr(sLen, maxChar);
			if (!u_)
				goto error;

			alifSubUStr_fastCopyCharacters(u_, 0, _self, 0, sLen,0);
			replace_1char_inplace(u_, pos, u1, u2, _maxCount);
		}
		else {
			int rkind = sKind;
			wchar_t* res_;
			int64_t i_;

			if (kind1_ < rkind) {
				buf1_ = uStr_asKind(kind1_, buf1_, len1_, rkind);
				if (!buf1_) goto error;
				release1 = 1;
			}
			i_ = anyLib_find(rkind, _self, sBuf, sLen, _str1, buf1_, len1_, 0);
			if (i_ < 0)
				goto nothing;
			if (rkind > kind2_) {
				buf2_ = uStr_asKind(kind2_, buf2_, len2_, rkind);
				if (!buf2_) goto error;
				release2 = 1;
			}
			else if (rkind < kind2_) {
				rkind = kind2_;
				if (release1) {
					alifMem_dataFree((void*)buf1_);
					buf1_ = ALIFUSTR_CAST(_str1)->UTF;
					release1 = 0;
				}
				sBuf = (const wchar_t*)uStr_asKind(sKind, sBuf, sLen, rkind);
				if (!sBuf) goto error;
				srelease = 1;
				buf1_ = uStr_asKind(kind1_, buf1_, len1_, rkind);
				if (!buf1_) goto error;
				release1 = 1;
			}
			u_ = alifNew_uStr(sLen, maxChar);
			if (!u_)
				goto error;
			res_ = (wchar_t*)ALIFUSTR_CAST(u_)->UTF;

			memcpy(res_, sBuf, rkind * sLen);
			memcpy(res_ + rkind * i_,
				buf2_,
				rkind * len2_);
			i_ += len1_;

			while (--_maxCount > 0) {
				i_ = anyLib_find(rkind, _self,
					sBuf + rkind * i_, sLen - i_,
					_str1, buf1_, len1_, i_);
				if (i_ == -1)
					break;
				memcpy(res_ + rkind * i_,
					buf2_,
					rkind * len2_);
				i_ += len1_;
			}
		}
	}
	else {
		int64_t n_, i_, j, ires;
		int64_t newSize;
		int rkind = sKind;
		wchar_t* res_{};

		if (kind1_ < rkind) {
			buf1_ = uStr_asKind(kind1_, buf1_, len1_, rkind);
			if (!buf1_) goto error;
			release1 = 1;
		}
		n_ = anyLib_count(rkind, _self, sBuf, sLen, _str1, buf1_, len1_, _maxCount);
		if (n_ == 0)
			goto nothing;
		if (kind2_ < rkind) {
			/* widen replacement */
			buf2_ = uStr_asKind(kind2_, buf2_, len2_, rkind);
			if (!buf2_) goto error;
			release2 = 1;
		}
		else if (kind2_ > rkind) {
			/* widen _self and buf1_ */
			rkind = kind2_;
			sBuf = (const wchar_t*)uStr_asKind(sKind, sBuf, sLen, rkind);
			if (!sBuf) goto error;
			srelease = 1;
			if (release1) {
				alifMem_dataFree((void*)buf1_);
				buf1_ = ALIFUSTR_CAST(_str1)->UTF;
				release1 = 0;
			}
			buf1_ = uStr_asKind(kind1_, buf1_, len1_, rkind);
			if (!buf1_) goto error;
			release1 = 1;
		}
		/* newSize = USTR_GET_LENGTH(_self) + n * (USTR_GET_LENGTH(_str2) -
		   USTR_GET_LENGTH(_str1)); */
		if (len1_ < len2_ && len2_ - len1_ >(LLONG_MAX - sLen) / n_) {
			//Err_SetString(Exc_OverflowError,
				//"replace string is too long");
			goto error;
		}
		newSize = sLen + n_ * (len2_ - len1_);
		if (newSize == 0) {
			//u_ = uStr_get_empty();
			goto done;
		}
		if (newSize > (LLONG_MAX / rkind)) {
			//Err_SetString(Exc_OverflowError,
				//"replace string is too long");
			goto error;
		}
		u_ = alifNew_uStr(newSize, maxChar);
		if (!u_)
			goto error;
		//res_ = USTR_DATA(u_);
		ires = i_ = 0;
		if (len1_ > 0) {
			while (n_-- > 0) {
				/* look for next match */
				j = anyLib_find(rkind, _self,
					sBuf + rkind * i_, sLen - i_,
					_str1, buf1_, len1_, i_);
				if (j == -1)
					break;
				else if (j > i_) {
					/* copy unchanged part [i_:j] */
					memcpy(res_ + rkind * ires,
						sBuf + rkind * i_,
						rkind * (j - i_));
					ires += j - i_;
				}
				/* copy substitution string */
				if (len2_ > 0) {
					memcpy(res_ + rkind * ires,
						buf2_,
						rkind * len2_);
					ires += len2_;
				}
				i_ = j + len1_;
			}
			if (i_ < sLen)
				/* copy tail [i_:] */
				memcpy(res_ + rkind * ires,
					sBuf + rkind * i_,
					rkind * (sLen - i_));
		}
		else {
			/* interleave */
			while (n_ > 0) {
				memcpy(res_ + rkind * ires,
					buf2_,
					rkind * len2_);
				ires += len2_;
				if (--n_ <= 0)
					break;
				memcpy(res_ + rkind * ires,
					sBuf + rkind * i_,
					rkind);
				ires++;
				i_++;
			}
			memcpy(res_ + rkind * ires,
				sBuf + rkind * i_,
				rkind * (sLen - i_));
		}
	}

	if (mayShrink) {
		//uStr_adjust_maxchar(&u_);
		if (u_ == nullptr)
			goto error;
	}

done:
	if (srelease)
		alifMem_dataFree((void*)sBuf);
	if (release1)
		alifMem_dataFree((void*)buf1_);
	if (release2)
		alifMem_dataFree((void*)buf2_);
	return u_;

nothing:
	/* nothing to replace; return original string (when possible) */
	
	if (srelease)
		alifMem_dataFree((void*)sBuf);
	if (release1)
		alifMem_dataFree((void*)buf1_);
	if (release2)
		alifMem_dataFree((void*)buf2_);
	return _self;
	return uStr_result_unchanged(_self);

error:

	if (srelease)
		alifMem_dataFree((void*)sBuf);
	if (release1)
		alifMem_dataFree((void*)buf1_);
	if (release2)
		alifMem_dataFree((void*)buf2_);
	return nullptr;
}

static AlifObject* uStr_replaceImp(AlifObject* _self, AlifObject* _old, AlifObject *_new, int64_t _count) {

	return replace(_self,_old,_new,_count);

}

static AlifObject* uStr_find(AlifObject* _self, AlifObject* _args)
{
	/* initialize variables to prevent gcc warning */
	AlifObject* substring_ = nullptr;
	int64_t start_ = 0;
	int64_t end_ = 0;
	int64_t result_;

	if (!parse_args_finds_uStr(L"find", _args, &substring_, &start_, &end_))
		return nullptr;

	result_ = any_find_slice(_self, substring_, start_, end_, 1);

	if (result_ == -2)
		return nullptr;

	return alifInteger_fromLongLong(result_);
}

static AlifObject* getItem_uStr(AlifObject* _self, int64_t _index)
{
	const void* data_;
	int kind_;
	uint32_t ch_;

	if (!(_self->type_ == &_alifUStrType_)) {
		//Err_BadArgument();
		return nullptr;
	}
	if (_index < 0 || _index >= ALIFUSTR_CAST(_self)->length_) {
		return nullptr;
	}
	kind_ = ALIFUSTR_CAST(_self)->kind_;
	data_ = ALIFUSTR_CAST(_self)->UTF;
	ch_ = ALIFUSTR_READ(kind_, data_, _index);
	return uStr_wChar(ch_);
}

AlifObject* alifUStr_join(AlifObject* _separator, AlifObject* _seq)
{
	AlifObject* res_;
	AlifObject* fSeq;
	int64_t seqLen;
	AlifObject** items_;

	fSeq = alifSequence_fast(_seq, L"can only join an iterable");
	if (fSeq == nullptr) {
		return nullptr;
	}

	items_ = ALIFSEQUENCE_FAST_ITEMS(fSeq);
	seqLen = ALIFSEQUENCE_FAST_GETSIZE(fSeq);
	res_ = alifUStr_joinArray(_separator, items_, seqLen);
	return res_;
}

AlifObject* alifUStr_joinArray(AlifObject* _separator, AlifObject* const* _items, int64_t _seqLen)
{
	AlifObject* res_ = nullptr; /* the result_ */
	AlifObject* sep = nullptr;
	int64_t sepLen;
	AlifObject* item_;
	int64_t sz, i_, resOffset;
	uint32_t maxChar;
	uint32_t itemMaxChar;
	int useMemcpy;
	wchar_t* resData = nullptr, * sepData = nullptr;
	AlifObject* lastObj;
	int kind_ = 0;

	if (_seqLen == 0) {
		//_RETURN_USTR_EMPTY();
	}

	lastObj = nullptr;
	if (_seqLen == 1) {
		if (_items[0]->type_ == &_alifUStrType_) {
			res_ = _items[0];
			return res_;
		}
		sepLen = 0;
		maxChar = 0;
	}
	else {
		if (_separator == nullptr) {
			//sep = uStr_FromOrdinal(' ');
			//if (!sep)
				//goto onError;
			sepLen = 1;
			maxChar = 32;
		}
		else {
			//if (!USTR_Check(separator)) {
				//Err_Format(Exc_TypeError,
					//"separator: expected _uStr instance,"
					//" %.80s found",
					//TYPE(separator)->tp_name);
				//goto onError;
			//}
			sep = _separator;
			sepLen = ALIFUSTR_CAST(_separator)->length_;
			maxChar = ALIFUSTR_MAX_CHAR_VALUE(_separator);

			sep;
		}
		lastObj = sep;
	}

	sz = 0;
#ifdef DEBUG
	useMemcpy = 0;
#else
	useMemcpy = 1;
#endif
	for (i_ = 0; i_ < _seqLen; i_++) {
		size_t add_sz;
		item_ = _items[i_];
		if (!(item_->type_ == &_alifUStrType_)) {
			//Err_Format(Exc_TypeError,
				//"sequence item_ %zd: expected _uStr instance,"
				//" %.80s found",
				//i_, TYPE(item_)->tp_name);
			//goto onError;
		}
		add_sz = ALIFUSTR_CAST(item_)->length_;
		itemMaxChar = ALIFUSTR_MAX_CHAR_VALUE(item_);
		maxChar = ALIF_MAX(maxChar, itemMaxChar);
		if (i_ != 0) {
			add_sz += sepLen;
		}
		if (add_sz > (size_t)(LLONG_MAX - sz)) {
			//Err_SetString(Exc_OverflowError,
				//"join() result_ is too long for a string");
			//goto onError;
		}
		sz += add_sz;
		if (useMemcpy && lastObj != nullptr) {
			if (ALIFUSTR_CAST(lastObj)->kind_ != ALIFUSTR_CAST(item_)->kind_)
				useMemcpy = 0;
		}
		lastObj = item_;
	}

	res_ = alifNew_uStr(sz, maxChar);
	if (res_ == nullptr)
		goto onError;
	if (useMemcpy) {
		for (i_ = 0; i_ < _seqLen; ++i_) {
			int64_t itemLen{};
			item_ = _items[i_];

			if (i_ && sepLen != 0) {
				memcpy(resData,
					sepData,
					kind_ * sepLen);
				resData += kind_ * sepLen;
			}

			itemLen = ALIFUSTR_CAST(item_)->kind_;
			if (itemLen != 0) {
				memcpy(resData,
					ALIFUSTR_CAST(item_)->UTF,
					kind_ * itemLen);
				resData += kind_ * itemLen;
			}
		}
	}
	else {
		for (i_ = 0, resOffset = 0; i_ < _seqLen; ++i_) {
			int64_t itemLen;
			item_ = _items[i_];

			if (i_ && sepLen != 0) {
				alifSubUStr_fastCopyCharacters(res_, resOffset, sep, 0, sepLen, 0);
				resOffset += sepLen;
			}

			itemLen = ALIFUSTR_CAST(item_)->length_;
			if (itemLen != 0) {
				alifSubUStr_fastCopyCharacters(res_, resOffset, item_, 0, itemLen, 0);
				resOffset += itemLen;
			}
		}
	}

	return res_;

onError:
	return nullptr;
}

static AlifObject* uStr_join(AlifObject* _self, AlifObject* _iterable) {
	
	return alifUStr_join(_self, _iterable);
}

void alifSubUStr_fastFill(AlifObject* _uStr,int64_t _start, int64_t _length,
	uint32_t _fillChar)
{
	const int kind_ = ALIFUSTR_CAST(_uStr)->kind_;
	void* data_ = ALIFUSTR_CAST(_uStr)->UTF;
	alifUStr_fill(kind_, data_, _fillChar, _start, _length);
}

int64_t alifUStr_fill(AlifObject* _uStr, int64_t _start, int64_t _length,
	uint32_t _fillChar)
{
	int64_t maxLen;

	if (!(_uStr->type_ == &_alifUStrType_)) {
		return -1;
	}
	if (uStr_check_modifiable(_uStr))
		return -1;

	if (_start < 0) {
		return -1;
	}
	if (_fillChar > ALIFUSTR_MAX_CHAR_VALUE(_uStr)) {

		return -1;
	}

	maxLen = ALIFUSTR_GET_LENGTH(_uStr) - _start;
	_length = ALIF_MIN(maxLen, _length);
	if (_length <= 0)
		return 0;

	alifSubUStr_fastFill(_uStr, _start, _length, _fillChar);
	return _length;
}

static AlifObject* uStr_splitImpl(AlifObject* , AlifObject* , int64_t );

static AlifObject* uStr_split(AlifObject* _self, AlifObject* const* _args, int64_t _nArgs, AlifObject* _kWNames)
{ // this should transfer to UStrObject.c.h or .h
	AlifObject* returnValue = nullptr;
#define NUM_KEYWORDS 2
	static class KwTuple {
	public:
		AlifVarObject base{};
		AlifObject* item_[NUM_KEYWORDS];
	} kwTuple = {
		//ALIFVAROBJECT_HEAD_INIT(&typeTuple, NUM_KEYWORDS)
		//nullptr
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&kwTuple.base.object)

//#  define KWTUPLE nullptr

	static const wchar_t* const keywords_[] = { L"sep", L"maxsplit", nullptr };
	static AlifArgParser parser_ = {
		0,
		L"",
		keywords_,
		L"split",
		L"",
		0,
		0,
		0,
		//KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsBuf[2];
	int64_t nOptArgs = _nArgs + (_kWNames ? ((AlifVarObject*)_kWNames)->size_ : 0) - 0;
	AlifObject* sep_ = ALIF_NONE;
	int64_t maxSplit = -1;

	_args = ALIFARG_UNPACKKEYWORDS(_args, _nArgs, nullptr, _kWNames, &parser_, 0, 2, 0, argsBuf);
	if (!_args) {
		goto exit;
	}
	if (!nOptArgs) {
		goto skip_optional_pos;
	}
	if (_args[0]) {
		sep_ = _args[0];
		if (!--nOptArgs) {
			goto skip_optional_pos;
		}
	}
	{
		int64_t iVal = -1;
		AlifObject* iObj = _args[1];
		if (iObj != nullptr) {
			iVal = alifInteger_asLongLong(iObj);
			ALIF_DECREF(iObj);
		}

		maxSplit = iVal;
	}
skip_optional_pos:
	returnValue = uStr_splitImpl(_self, sep_, maxSplit);

exit:
	return returnValue;
}

static AlifObject* split(AlifObject* _self, AlifObject* _substring, int64_t _maxCount)
{
	int kind1_, kind2_;
	const void* buf1_, * buf2_;
	int64_t len1_, len2_;
	AlifObject* out;
	len1_ = ALIFUSTR_CAST(_self)->length_;
	kind1_ = ALIFUSTR_CAST(_self)->kind_;

	//if (substring == nullptr) {
	//	if (_maxCount < 0) {
	//		_maxCount = (len1_ - 1) / 2 + 1;
	//	}
	//	switch (kind1_) {
	//	case USTR_2BYTE:
	//		return ucs2lib_split_whitespace(
	//			_self, USTR_2BYTE_DATA(_self),
	//			len1_, _maxCount
	//		);
	//	case USTR_4BYTE:
	//		return ucs4lib_split_whitespace(
	//			_self, USTR_4BYTE_DATA(_self),
	//			len1_, _maxCount
	//		);
	//	default:
	//		//UNREACHABLE();
	//	}
	//}

	kind2_ = ALIFUSTR_CAST(_substring)->kind_;
	len2_ = ALIFUSTR_CAST(_substring)->length_;
	if (_maxCount < 0) {
		_maxCount = len2_ == 0 ? 0 : (len1_ / len2_) + 1;
		_maxCount = _maxCount < 0 ? len1_ : _maxCount;
	}
	if (kind1_ < kind2_ || len1_ < len2_) {
		out = alifNew_list(1);
		if (out == nullptr)
			return nullptr;
		((AlifListObject*)out)->items_[0] = _self;
		return out;
	}
	buf1_ = ALIFUSTR_CAST(_self)->UTF;
	buf2_ = ALIFUSTR_CAST(_substring)->UTF;
	if (kind2_ != kind1_) {
		buf2_ = uStr_asKind(kind2_, buf2_, len2_, kind1_);
		if (!buf2_)
			return nullptr;
	}

	switch (kind1_) {
	case USTR_2BYTE:
		out = split(
			_self, (uint16_t*)buf1_, len1_, (uint16_t*)buf2_, len2_, _maxCount);
		break;
	case USTR_4BYTE:
		out = split(
			_self, (uint32_t*)buf1_, len1_, (uint32_t*)buf2_, len2_, _maxCount);
		break;
	default:
		out = nullptr;
	}
	if (kind2_ != kind1_)
		alifMem_dataFree((void*)buf2_);
	return out;
}

static AlifObject* uStr_splitImpl(AlifObject* _self, AlifObject* _sep, int64_t _maxSplit)
{
	if (_sep == ALIF_NONE)
		return split(_self, nullptr, _maxSplit);
	if (_sep->type_ == &_alifUStrType_)
		return split(_self, _sep, _maxSplit);
	return nullptr;
}

static inline void alifSubUStrWriter_update(AlifSubUStrWriter* _writer)
{
	_writer->maxChar = ALIFUSTR_MAX_CHAR_VALUE(_writer->buffer_);
	_writer->data_ = ALIFUSTR_CAST(_writer->buffer_)->UTF;

	if (!_writer->readonly_) {
		_writer->kind_ = ALIFUSTR_KIND(_writer->buffer_);
		_writer->size_ = ALIFUSTR_GET_LENGTH(_writer->buffer_);
	}
	else {
		_writer->kind_ = 0;
		_writer->size_ = 0;
	}
}

int alifSubUStrWriter_prepareInternal(AlifSubUStrWriter* _writer,
	int64_t _length, uint32_t _maxChar)
{
	int64_t newLen;
	AlifObject* newBuffer{};

	if (_length > LLONG_MAX - _writer->pos_) {
		return -1;
	}
	newLen = _writer->pos_ + _length;

	_maxChar = ALIF_MAX(_maxChar, _writer->minChar);

	if (_writer->buffer_ == nullptr) {
		if (_writer->overAllocate
			&& newLen <= (LLONG_MAX - newLen / 2)) {
			newLen += newLen / 2;
		}
		if (newLen < _writer->minLength)
			newLen = _writer->minLength;

		_writer->buffer_ = alifNew_uStr(newLen, _maxChar);
		if (_writer->buffer_ == nullptr)
			return -1;
	}
	else if (newLen > _writer->size_) {
		if (_writer->overAllocate
			&& newLen <= (LLONG_MAX - newLen / 2)) {
			newLen += newLen / 2;
		}
		if (newLen < _writer->minLength)
			newLen = _writer->minLength;

		if (_maxChar > _writer->maxChar || _writer->readonly_) {
			/* resize + widen */
			_maxChar = ALIF_MAX(_maxChar, _writer->maxChar);
			newBuffer = alifNew_uStr(newLen, _maxChar);
			if (newBuffer == nullptr)
				return -1;
			alifSubUStr_fastCopyCharacters(newBuffer, 0,
				_writer->buffer_, 0, _writer->pos_, 0);
			ALIF_DECREF(_writer->buffer_);
			_writer->readonly_ = 0;
		}
		else {
			//newBuffer = resize_compact(_writer->buffer_, newLen);
			if (newBuffer == nullptr)
				return -1;
		}
		_writer->buffer_ = newBuffer;
	}
	else if (_maxChar > _writer->maxChar) {
		newBuffer = alifNew_uStr(_writer->size_, _maxChar);
		if (newBuffer == nullptr)
			return -1;
		alifSubUStr_fastCopyCharacters(newBuffer, 0,
			_writer->buffer_, 0, _writer->pos_, 0);
		ALIF_SETREF(_writer->buffer_, newBuffer);
	}
	alifSubUStrWriter_update(_writer);
	return 0;


}

int alifSubUStrWriter_writeStr(AlifSubUStrWriter* _writer, AlifObject* _str)
{
	AlifUCS4 maxChar;
	int64_t len_;

	len_ = ALIFUSTR_GET_LENGTH(_str);
	if (len_ == 0)
		return 0;
	maxChar = ALIFUSTR_MAX_CHAR_VALUE(_str);
	if (maxChar > _writer->maxChar || len_ > _writer->size_ - _writer->pos_) {
		if (_writer->buffer_ == nullptr && !_writer->overAllocate) {
			_writer->readonly_ = 1;
			_writer->buffer_ = ALIF_NEWREF(_str);
			alifSubUStrWriter_update(_writer);
			_writer->pos_ += len_;
			return 0;
		}
		if (alifSubUStrWriter_prepareInternal(_writer, len_, maxChar) == -1)
			return -1;
	}
	alifSubUStr_fastCopyCharacters(_writer->buffer_, _writer->pos_,
		_str, 0, len_, 0);
	_writer->pos_ += len_;
	return 0;
}

static inline int alifSubUStrWriter_writeWcharInline(AlifSubUStrWriter* _writer, uint32_t _ch)
{
	if (ALIFSUBUSTRWRITER_PREPARE(_writer, 1, _ch) < 0)
		return -1;
	ALIFUSTR_WRITE(_writer->kind_, _writer->data_, _writer->pos_, _ch);
	_writer->pos_++;
	return 0;
}

int alifSubUStrWriter_writeChar(AlifSubUStrWriter* _writer, uint32_t _ch)
{
	return alifSubUStrWriter_writeWcharInline(_writer, _ch);
}

static AlifMethodDef _uStrMethods_[] = {
	{L"replace", ALIF_CPPFUNCTION_CAST(uStr_replace), METHOD_FASTCALL | METHOD_KEYWORDS}, // this in UStrObject.c.h or .h
	{L"split", ALIF_CPPFUNCTION_CAST(uStr_split), METHOD_FASTCALL | METHOD_KEYWORDS},
	{L"join", (AlifCFunction)uStr_join, METHOD_O,},
	{L"count", (AlifCFunction)uStr_count, METHOD_VARARGS},
	{L"find", (AlifCFunction)uStr_find, METHOD_VARARGS},
	{nullptr, nullptr}
};

static AlifSequenceMethods _uStrAsSequence_ = {
	(LenFunc)length_uStr,
	combine_uStr,
	(SSizeArgFunc)repeat_uStr,
	(SSizeArgFunc)getItem_uStr,
	0,
	0,
	0,
	(ObjObjProc)contain_uStr,
};

static AlifObject* uStr_subscript(AlifObject* _self, AlifObject* _item)
{
	if ((_item->type_->asNumber != nullptr && _item->type_->asNumber->index_)) {
		int64_t i_ = alifInteger_asLongLong(_item);
		if (i_ < 0)
			i_ += ALIFUSTR_CAST(_self)->length_;
		return getItem_uStr(_self, i_);
	}
	else if (ALIFSLICE_CHECK(_item)) {
		int64_t start_, stop_, step_, sliceLength, i_;
		size_t cur_;
		AlifObject* result_;
		const void* srcData;
		void* destData;
		int srcKind, destKind;
		uint32_t ch_, maxChar, kindLimit;

		if (slice_unpack((AlifSliceObject*)_item, &start_, &stop_, &step_) < 0) {
			return nullptr;
		}
		sliceLength = alifSlice_adjustIndices(ALIFUSTR_CAST(_self)->length_,
			&start_, &stop_, step_);

		if (sliceLength <= 0) {
			// return empty string;
		}
		else if (start_ == 0 && step_ == 1 &&
			sliceLength == ALIFUSTR_CAST(_self)->length_) {
			return uStr_result_unchanged(_self);
		}
		else if (step_ == 1) {
			//return alifUStr_substring(_self,
				//start_, start_ + sliceLength);
		}
		srcKind = ALIFUSTR_CAST(_self)->kind_;
		srcData = ALIFUSTR_CAST(_self)->UTF;
		kindLimit = kind_maxChar_limit(srcKind);
		maxChar = 0;
		for (cur_ = start_, i_ = 0; i_ < sliceLength; cur_ += step_, i_++) {
			ch_ = ALIFUSTR_READ(srcKind, srcData, cur_);
			if (ch_ > maxChar) {
				maxChar = ch_;
				if (maxChar >= kindLimit)
					break;
			}
		}
		
		result_ = alifNew_uStr(sliceLength, maxChar);
		if (result_ == nullptr)
			return nullptr;
		destKind = ALIFUSTR_CAST(result_)->kind_;
		destData = ALIFUSTR_CAST(result_)->UTF;

		for (cur_ = start_, i_ = 0; i_ < sliceLength; cur_ += step_, i_++) {
			uint32_t ch_ = ALIFUSTR_READ(srcKind, srcData, cur_);
			ALIFUSTR_WRITE(destKind, destData, i_, ch_);
		}
		return result_;
	}
	else {
		return nullptr;
	}
}

static AlifMappingMethods _uStrAsMapping_ = {
	(LenFunc)length_uStr,       
	(BinaryFunc)uStr_subscript,  
	(ObjObjArgProc)0, 
};

AlifTypeObject _alifUStrType_ = {
	ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0)
	L"string",                        
	sizeof(AlifUStrObject),      
	0,                            
	(Destructor)uStr_dealloc,
	0,                            
	0,                         
	0,                         
	0,           
	0,          
	&_uStrAsSequence_,
	&_uStrAsMapping_,
	(HashFunc)hash_uStr,
	0,                     
	0,
	0,      
	0,                            
	0,                             
	ALIFTPFLAGS_DEFAULT | ALIFTPFLAGS_BASETYPE |
	ALIFTPFLAGS_USTR_SUBCLASS |
	ALIFSUBTPFLAGS_MATCH_SELF,
	0,               
	0, 
	0,
	(RichCmpFunc)uStr_compare,
	0,                           
	0,        
	0,               
	_uStrMethods_,
	0,                            
	0,                            
	0,                            
	0,                          
	0,                          
	0,
	0,                             
	0,                            
	0,                          
	0,                           
	0,               
	0,               
};

void alifSubUStr_internInPlace(AlifInterpreter* interp, AlifObject** p) { // 14901
	AlifObject* s = *p;
	if (s == nullptr or !ALIFUSTR_CHECK(s)) {
		return;
	}

	if (!ALIFUSTR_CHECKEXACT(s)) {
		return;
	}

	//if (ALIFUSTR_CHECK_INTERNED(s)) {
	//	return;
	//}

	//AlifObject* r = (AlifObject*)alifHashTable_get(INTERNED_STRINGS, s);
	//if (r != nullptr and r != s) {
	//	ALIF_SETREF(*p, ALIF_NEWREF(r));
	//	return;
	//}

	//if (ALIFUSTR_STATE(s).staticallyAllocated) {
	//	if (alifHashTable_set(INTERNED_STRINGS, s, s) == 0) {
	//		ALIFUSTR_STATE(*p).interned = SSTATE_INTERNED_IMMORTAL_STATIC;
	//	}
	//	return;
	//}

	//AlifObject* interned = get_internedDict(interp);

	//AlifObject* t;
	//int res = alifDict_setDefaultRef(interned, s, s, &t);
	//if (res < 0) {
	//	//alifErr_clear();
	//	return;
	//}
	//else if (res == 1) {
	//	ALIF_SETREF(*p, t);
	//	return;
	//}
	//ALIF_DECREF(t);

	//if (alif_isImmortal(s)) {
	//	ALIFUSTR_STATE(*p).interned = SSTATE_INTERNED_IMMORTAL_STATIC;
	//	return;
	//}

	alifSub_setImmortal(s);
	//ALIFUSTR_STATE(*p).interned = SSTATE_INTERNED_IMMORTAL;
}

void alifUStr_internInPlace(AlifObject** p) { // 14976
	AlifInterpreter* interp = alifInterpreter_get();
	alifSubUStr_internInPlace(interp, p);
}


AlifObject* alifUStr_internFromString(const wchar_t* _cp)
{
	AlifObject* s_ = alifUStr_decodeStringToUTF8(_cp);
	if (s_ == nullptr)
		return nullptr;
	alifUStr_internInPlace(&s_);
	return s_;
}
