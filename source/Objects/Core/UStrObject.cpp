#include "alif.h"

#include "alifCore_UString.h"
#include "AlifCore_Memory.h"

#ifdef _WINDOWS
#include "windows.h"
#endif // _WINDOWS

static inline int alifSubUnicodeWriter_writeWcharInline(AlifSubUnicodeWriter*, uint32_t);

static inline void unicode_fill(int _kind, void* _data, uint32_t _value,
	int64_t _start, int64_t _length)
{
	switch (_kind) {

	case UNICODE_2BYTE: {
		uint16_t ch_ = (uint16_t)_value;
		uint16_t* to_ = (uint16_t*)_data + _start;
		const uint16_t* end_ = to_ + _length;
		for (; to_ < end_; ++to_) *to_ = ch_;
		break;
	}
	case UNICODE_4BYTE: {
		uint32_t ch_ = _value;
		uint32_t* to_ = (uint32_t*)_data + _start;
		const uint32_t* end_ = to_ + _length;
		for (; to_ < end_; ++to_) *to_ = ch_;
		break;
	}
	//default: UNREACHABLE();
	}
}

static AlifObject* unicode_result(AlifObject* _unicode)
{

	int64_t length_ = ALIFUNICODE_GET_LENGTH(_unicode);
	if (length_ == 0) {
		//AlifObject* empty = unicode_get_empty();
		//if (unicode != empty) {
			//ALIF_DECREF(unicode);
		//}
		//return empty;
	}

	return _unicode;
}

static AlifObject* unicode_result_unchanged(AlifObject* _unicode)
{
	if ((_unicode->type_ == &_typeUnicode_)) {
		return ALIF_NEWREF(_unicode);
	}
	else
		return alifSubUnicode_copy(_unicode);
}

// in file bytearrayobject.c in 1111 and should call from there only 
#define STRINGLIB_FAST_MEMCHR memchr

#include "fastSearch.h"
#include "split.h"

// in file find.h in 8
template <typename STRINGLIB_CHAR>
int64_t find(const STRINGLIB_CHAR* str, int64_t strLen,
	const STRINGLIB_CHAR* sub, int64_t subLen,
	int64_t offset)
{
	int64_t pos;

	if (subLen == 0)
		return offset;

	pos = fastSearch(str, strLen, sub, subLen, -1, FAST_SEARCH);

	if (pos >= 0)
		pos += offset;

	return pos;
}

template <typename STRINGLIB_CHAR>
int64_t rfind(const STRINGLIB_CHAR* str, int64_t str_len,
	const STRINGLIB_CHAR* sub, int64_t sub_len,
	int64_t offset)
{
	int64_t pos;

	if (sub_len == 0)
		return str_len + offset;

	pos = fastSearch(str, str_len, sub, sub_len, -1, FAST_RSEARCH);

	if (pos >= 0)
		pos += offset;

	return pos;
}

template <typename STRINGLIB_CHAR>
int64_t find_slice(const STRINGLIB_CHAR* str, int64_t str_len,
	const STRINGLIB_CHAR* sub, int64_t sub_len,
	int64_t start_, int64_t end)
{
	return find(str + start_, end - start_, sub, sub_len, start_);
}

template <typename STRINGLIB_CHAR>
int64_t rfind_slice(const STRINGLIB_CHAR* str, int64_t str_len,
	const STRINGLIB_CHAR* sub, int64_t sub_len,
	int64_t start_, int64_t end)
{
	return rfind(str + start_, end - start_, sub, sub_len, start_);
}

#define FORMAT_BUFFER_SIZE 50

// in file ceval.c in 2369
int alifEval_sliceIndex(AlifObject * _v, int64_t * _pi)
{
	if (!ALIF_ISNONE(_v)) {
		int64_t x;
		if ((_v->type_->asNumber != nullptr)) {
			x = alifInteger_asLongLong(_v);
		}
		else {
			return 0;
		}
		*_pi = x;
	}
	return 1;
}

int parse_args_finds(const wchar_t* function_name, AlifObject* args,
	AlifObject** subobj,
	int64_t* start_, int64_t* end)
{
	AlifObject* tmp_subobj;
	int64_t tmp_start = 0;
	int64_t tmp_end = LLONG_MAX;
	AlifObject* obj_start = ALIF_NONE, * obj_end = ALIF_NONE;
	wchar_t format[FORMAT_BUFFER_SIZE] = L"O|OO:";
	size_t len = wcslen(format);

	wcsncpy_s(format + len, FORMAT_BUFFER_SIZE - len, function_name, _TRUNCATE);
	format[FORMAT_BUFFER_SIZE - 1] = '\0';

	if (!alifArg_parseTuple(args, format, &tmp_subobj, &obj_start, &obj_end))
		return 0;

	/* To support None in "start_" and "end" arguments, meaning
	   the same as if they were not passed.
	*/
	if (obj_start != ALIF_NONE)
		if (!alifEval_sliceIndex(obj_start, &tmp_start))
			return 0;
	if (obj_end != ALIF_NONE)
		if (!alifEval_sliceIndex(obj_end, &tmp_end))
			return 0;

	*start_ = tmp_start;
	*end = tmp_end;
	*subobj = tmp_subobj;
	return 1;
}

// in file count.h 
template <typename STRINGLIB_CHAR>
int64_t count(const STRINGLIB_CHAR* str, int64_t strLen,
	const STRINGLIB_CHAR* sub, int64_t subLen,
	int64_t maxCount)
{
	int64_t count;

	if (strLen < 0)
		return 0; /* start_ > len(str) */
	if (subLen == 0)
		return (strLen < maxCount) ? strLen + 1 : maxCount;

	count = fastSearch(str, strLen, sub, subLen, maxCount, FAST_COUNT);

	if (count < 0)
		return 0; /* no match */

	return count;
}

static inline int64_t findChar(const void* s, int kind,
	int64_t size_, uint32_t ch,
	int direction)
{
	switch (kind) {
	case UNICODE_2BYTE:
		if ((uint16_t)ch != ch)
			return -1;
		if (direction > 0)
			return find_char((const uint16_t*)s, size_, (uint16_t)ch);
		else
			return rfind_char((const uint16_t*)s, size_, (uint16_t)ch);
	case UNICODE_4BYTE:
		if (direction > 0)
			return find_char((const uint32_t*)s, size_, ch);
		else
			return rfind_char((const uint32_t*)s, size_, ch);
	//default:
		//UNREACHABLE();
	}
}

static int resize_inplace(AlifObject* _unicode, int64_t _length)
{

	int64_t newSize;
	int64_t charSize;
	int shareUTF8;
	void* data_;
#ifdef ALIF_DEBUG
	int64_t oldLength = ALIFUNICODE_GET_LENGTH(_unicode);
#endif

	data_ = ALIFUNICODE_CAST(_unicode)->UTF;
	charSize = ALIFUNICODE_KIND(_unicode);

	//(ALIFUNICODE_UTF8(op) == ALIFUNICODE_DATA(op)) : This is the main functionality of the macro.It checks if the UTF - 8 representation of the Unicode object op is the same as the raw data of op.
	// If they are the same, it means that op is a UTF - 8 encoded Unicode object and its data can be directly used as a UTF - 8 string.
	shareUTF8 = 1;

	if (_length > (LLONG_MAX / charSize - 1)) {
		return -1;
	}
	newSize = (_length + 1) * charSize;

	if (!shareUTF8
		// &&
		//ALIFSUBUNICODE_HAS_UTF8_MEMORY(_unicode)
		 )
	{
		alifMem_objFree(ALIFUNICODE_CAST(_unicode)->UTF);
		ALIFUNICODE_CAST(_unicode)->UTF = NULL;
		ALIFUNICODE_GET_LENGTH(_unicode) = 0;
	}

	data_ = (AlifObject*)alifMem_dataRealloc(data_, newSize);
	if (data_ == NULL) {
		return -1;
	}
	ALIFUNICODE_CAST(_unicode)->UTF = data_;
	if (shareUTF8) {
		ALIFUNICODE_CAST(_unicode)->UTF = data_;
		ALIFUNICODE_GET_LENGTH(_unicode) = _length;
	}
	ALIFUNICODE_GET_LENGTH(_unicode) = _length;
	ALIFUNICODE_WRITE(ALIFUNICODE_KIND(_unicode), data_, _length, 0);
#ifdef ALIF_DEBUG
	unicode_fill_invalid(_unicode, old_length);
#endif

	if (_length > LLONG_MAX / (int64_t)sizeof(wchar_t) - 1) {
		return -1;
	}
	return 0;
}

static AlifObject*
resize_copy(AlifObject* _unicode, int64_t _length)
{
	int64_t copyLength;
	AlifObject* copy_;

	copy_ = alifNew_unicode(_length, ALIFUNICODE_MAX_CHAR_VALUE(_unicode));
	if (copy_ == NULL)
		return NULL;

	copyLength = min(_length, ALIFUNICODE_GET_LENGTH(_unicode));
	alifSubUnicode_fastCopyCharacters(copy_, 0, _unicode, 0, copyLength, 0);
	return copy_;
}

// من هنا يتم انشاء كائن نصي
AlifObject* alifNew_uStr(size_t _size, uint8_t _maxChar) { /// M
	AlifObject* obj{};
	AlifUStrObject* uStr{};
	void* data{};
	uint8_t kind{};
	uint8_t charSize{};
	size_t structSize = sizeof(AlifUStrObject);


	if (_maxChar == 2) {
		kind = UNICODE_2BYTE;
		charSize = 2;
	}
	else if (_maxChar == 4) {
		kind = UNICODE_4BYTE;
		charSize = 4;
	}
	else {
		std::wcout << L"ترميز الحروف غير صحيح\n" << std::endl;
		exit(-1);
	}

	obj = (AlifObject*)alifMem_objAlloc(structSize + (_size + 1) * charSize);
	obj->type_ = &_typeUnicode_;

	uStr = (AlifUStrObject*)obj;

	data = uStr + 1;

	uStr->length = _size;
	uStr->kind = kind;
	uStr->hash = 0;
	uStr->UTF = data;

	if (kind == UNICODE_2BYTE) {
		((uint16_t*)data)[_size] = 0;
	}
	else if (kind == UNICODE_4BYTE) {
		((uint32_t*)data)[_size] = 0;
	}

	return obj;
}

AlifObject* alifNew_unicode(size_t size_, uint8_t maxChar) {

	uint8_t kind;
	size_t structSize;
	AlifUStrObject* object;

	if (maxChar == 2) {
		kind = UNICODE_2BYTE;
		structSize = sizeof(AlifUStrObject);
	}
	else if(maxChar == 4)
	{
		kind = UNICODE_4BYTE;
		structSize = sizeof(AlifUStrObject);
	}
	else {
		std::wcout << L"ترميز الحروف غير صحيح\n" << std::endl;
		exit(-1);
	}

	object = (AlifUStrObject*)alifMem_objAlloc(structSize + (size_ + 1) * kind);
	((AlifObject*)object)->type_ = &_typeUnicode_;

	object->hash = 0;
	object->length = size_;
	object->kind = kind;
	object->UTF = &object->UTF + 1;

	return (AlifObject*)object;
}

static int
unicode_modifiable(AlifObject* _unicode)
{
	if (ALIF_REFCNT(_unicode) != 1)
		return 0;
	if (ALIFUNICODE_CAST(_unicode)->hash != -1)
		return 0;
	//if (UNICODE_CHECK_INTERNED(_unicode)) // من الممكن ان يضاف
	//	return 0;
	if (!(_unicode->type_ == &_typeUnicode_))
		return 0;
	return 1;
}

static int unicode_check_modifiable(AlifObject* _unicode)
{
	if (!unicode_modifiable(_unicode)) {
		return -1;
	}
	return 0;
}

static int alifSubUnicode_fastCopyCharacters(AlifObject* to, int64_t toStart,
	AlifObject* from, int64_t fromStart,
	int64_t howMany, int checkMaxChar)
{
	int fromKind, toKind;
	const void* fromData;
	void* toData;

	if (howMany == 0)
		return 0;

	fromKind = ALIFUNICODE_CAST(from)->kind;
	fromData = ALIFUNICODE_CAST(from)->UTF;
	toKind = ALIFUNICODE_CAST(to)->kind;
	toData = ALIFUNICODE_CAST(to)->UTF;

#ifdef DEBUG
	if (!checkMaxChar
		&& Unicode_MAX_CHAR_VALUE(from) > Unicode_MAX_CHAR_VALUE(to))
	{
		uint32_t to_maxchar = Unicode_MAX_CHAR_VALUE(to);
		uint32_t ch;
		int64_t i;
		for (i = 0; i < howMany; i++) {
			ch = Unicode_READ(fromKind, fromData, fromStart + i);
			assert(ch <= to_maxchar);
		}
	}
#endif

	if (fromKind == toKind) {
		memcpy((wchar_t*)toData + toKind * toStart,
			(const wchar_t*)fromData + fromKind * fromStart,
			toKind * howMany);
	}
	else if (fromKind == UNICODE_2BYTE
		&& toKind == UNICODE_4BYTE)
	{
		ALIFUNICODE_CONVERT_BYTES(
			uint16_t, uint32_t,
			((uint16_t*)(ALIFUNICODE_CAST(from))->UTF) + fromStart,
			((uint16_t*)(ALIFUNICODE_CAST(from))->UTF) + fromStart + howMany,
			((uint32_t*)(ALIFUNICODE_CAST(to))->UTF) + toStart
		);
	}
	else {

		if (!checkMaxChar) {
			 if (fromKind == UNICODE_4BYTE
				&& toKind == UNICODE_2BYTE)
			{
				 ALIFUNICODE_CONVERT_BYTES(
					 uint32_t, uint16_t,
					 ((uint32_t*)(ALIFUNICODE_CAST(from))->UTF) + fromStart,
					 ((uint32_t*)(ALIFUNICODE_CAST(from))->UTF) + fromStart + howMany,
					 ((uint16_t*)(ALIFUNICODE_CAST(to))->UTF) + toStart
				);
			}
			else {
				//UNREACHABLE();
			}
		}
		else {
			const uint32_t to_maxchar = ALIFUNICODE_MAX_CHAR_VALUE(to);
			uint32_t ch;
			int64_t i;

			for (i = 0; i < howMany; i++) {
				ch = ALIFUNICODE_READ(fromKind, fromData, fromStart + i);
				if (ch > to_maxchar)
					return -1;
				ALIFUNICODE_WRITE(toKind, toData, toStart + i, ch);
			}
		}
	}
	return 0;
}

AlifSizeT alifUnicode_copyCharacters(AlifObject* _to, AlifSizeT _toStart,
	AlifObject* _from, AlifSizeT _fromStart, AlifSizeT _howMany)
{
	int err{};

	_howMany = min(ALIFUNICODE_GET_LENGTH(_from) - _fromStart, _howMany);
	if (_toStart + _howMany > ALIFUNICODE_GET_LENGTH(_to)) {
		return -1;
	}

	if (_howMany == 0)
		return 0;

	if (unicode_check_modifiable(_to))
		return -1;

	err = alifSubUnicode_fastCopyCharacters(_to, _toStart, _from, _fromStart, _howMany, 1);
	if (err) {
		return -1;
	}
	return _howMany;
}

// هنا يتم ايجاد اعلى ترميز في النص لتحديد نوع ترميز النص
uint8_t find_maxChar(const wchar_t* str) {

	while (*str != L'\0') {
		if (*str >= 0xD800 && *str <= 0xDBFF) {
			str++;
			return 4;
		}
		str++;
	}
	return 2;
}

static int unicode_resize(AlifObject** _pUnicode, int64_t _length)
{
	AlifObject* unicode_;
	int64_t oldLength;

	unicode_ = *_pUnicode;
	oldLength = ALIFUNICODE_GET_LENGTH(unicode_);
	if (oldLength == _length)
		return 0;

	if (_length == 0) {
		//AlifObject* empty = uStr_get_empty();
		//ALIF_SETREF(*_pUnicode, empty);
		return 0;
	}

	if (!unicode_modifiable(unicode_)) {
		AlifObject* copy = resize_copy(unicode_, _length);
		if (copy == NULL)
			return -1;
		ALIF_SETREF(*_pUnicode, copy);
		return 0;
	}

	//if (PyUnicode_IS_COMPACT(unicode)) {
	//	AlifObject* new_unicode = resize_compact(unicode, _length);
	//	if (new_unicode == NULL)
	//		return -1;
	//	*_pUnicode = new_unicode;
	//	return 0;
	//}
	return resize_inplace(unicode_, _length);
}

AlifObject* alifUnicode_fromWideChar(const wchar_t* _u, int64_t _size)
{
	AlifObject* unicode_{};
	AlifUCS4 maxChar = 0;
	int64_t numSurrogates{};

	if (_u == NULL && _size != 0) {
		return NULL;
	}

	if (_size == -1) {
		_size = wcslen(_u);
	}

	//if (size == 0)
		//ALIFSUB_RETURN_UNICODE_EMPTY();


	/* If not empty and not single character, copy the Unicode data
	   into the new object */
	if ((maxChar = find_maxChar(_u)) == -1)
		return NULL;

	unicode_ = alifNew_uStr(_size - numSurrogates, maxChar);
	if (!unicode_)
		return NULL;

	switch (ALIFUNICODE_KIND(unicode_)) {
	case UNICODE_2BYTE:
#if ALIF_UNICODE_SIZE == 2
		memcpy(AlifUnicode_2BYTE_DATA(unicode), u, size * 2);
#else
		ALIFUNICODE_CONVERT_BYTES(wchar_t, AlifUCS2,
			_u, _u + _size, ALIFUNICODE_CAST(unicode_)->UTF);
#endif
		break;
	case UNICODE_4BYTE:
#if SIZEOF_WCHAR_T == 2
		/* This is the only case which has to process surrogates, thus
		   a simple copy loop is not enough and we need a function. */
		unicode_convert_wchar_to_ucs4(u, u + size, unicode);
#else
		memcpy(((uint32_t*)((AlifUStrObject*)unicode_)->UTF), _u, _size * 4);
#endif
		break;
	default:
		//ALIF_UNREACHABLE();
		return nullptr; // temp
	}

	return unicode_result(unicode_);
}

AlifObject* alifSubUnicode_copy(AlifObject* _unicode)
{
	int64_t length_;
	AlifObject* copy_;

	if (!(_unicode->type_ == &_typeUnicode_)) {
		return NULL;
	}

	length_ = ((AlifUStrObject*)_unicode)->length;
	copy_ = alifNew_uStr(length_, ALIFUNICODE_MAX_CHAR_VALUE(_unicode));
	if (!copy_)
		return NULL;

	memcpy(((AlifUStrObject*)copy_)->UTF, ((AlifUStrObject*)_unicode)->UTF,
		length_ * ((AlifUStrObject*)_unicode)->kind);
	return copy_;
}

// هنا يتم عد الحروف بترميز utf32 الموجودة في النص فقط
size_t count_4byteCharacters(const wchar_t* str) {
	size_t count4byte = 0;
	while (*str != L'\0') {
		if (*str >= 0xD800 && *str <= 0xDBFF) {
			str++;
			count4byte++;
		}
		str++;
	}

	return count4byte;
}

// هنا يتم عد الحروف بترميز utf16 الموجودة في النص فقط
size_t count_2byteCharacters(const wchar_t* str) {
	size_t count2byte = 0;
	while (*str != L'\0') {
		count2byte++;
		if (*str >= 0xD800 && *str <= 0xDBFF) {
			str++;
			count2byte--;
		}
		str++;
	}

	return count2byte;
}


// من هنا يتم عد الحروف الموجودة في النص
size_t count_characters(const wchar_t* str) {
	size_t count = 0;
	while (*str != L'\0') {
		count++;
		if (*str >= 0xD800 && *str <= 0xDBFF) {
			str++;
		}
		str++;
	}

	return count;
}


static void uStrConvert_wCharToUCS4(const wchar_t* _begin, const wchar_t* _end, AlifObject* _uStr) { /// M
	const wchar_t* iter{};
	uint32_t* uCS4{};

	uCS4 = (uint32_t*)((AlifUStrObject*)_uStr)->UTF;

	for (iter = _begin; iter < _end; ) {
		if ((0xD800 <= iter[0] && iter[0] <= 0xDBFF)
			&& (iter + 1) < _end
			&& (0xDC00 <= iter[1] && iter[1] <= 0xDFFF))
		{
			*uCS4++ = 0x10000 + (((iter[0] & 0x03FF) << 10) | (iter[1] & 0x03FF));
			iter += 2;
		}
		else {
			*uCS4++ = *iter;
			iter++;
		}
	}
}


// من هنا يتم فك الترميز النص ونقله الى الكائن
/// M
//#define ALIFUSTR_CONVERT_BYTES(_fromType, _toType, _begin, _end, _to) \
//    do {											    	    \
//        _toType *_to1 = (_toType *)(_to);						\
//        const _fromType *_iter1 = (const _fromType *)(_begin);	\
//        const _fromType *_end1 = (const _fromType *)(_end);		\
//        size_t _n = (_end1) - (_iter1);						\
//        const _fromType *_unrolledEnd1 =						\
//            _iter1 + ALIF_SIZE_ROUND_DOWN(_n, 4);				\
//        while (_iter1 < (_unrolledEnd1)) {						\
//            _to1[0] = (_toType) _iter1[0];						\
//            _to1[1] = (_toType) _iter1[1];						\
//            _to1[2] = (_toType) _iter1[2];						\
//            _to1[3] = (_toType) _iter1[3];						\
//            _iter1 += 4; _to1 += 4;								\
//        }														\
//        while (_iter1 < (_end1))								\
//            *_to1++ = (_toType) *_iter1++;						\
//    } while (0)

AlifObject* alifUStr_objFromWChar(wchar_t* _buffer) { /// M
	AlifObject* strObj{};
	size_t size_ = wcslen(_buffer);
	uint8_t maxChar = find_maxChar(_buffer);

	strObj = alifNew_uStr(size_, maxChar);

	
	if (((AlifUStrObject*)strObj)->kind == UNICODE_2BYTE) {
#if SIZEOF_WCHART == 2
		memcpy(((AlifUStrObject*)strObj)->UTF, _buffer, size_ * 2);
#else
		ALIFUNICODE_CONVERT_BYTES(wchar_t, uint16_t,
			_buffer, _buffer + size_, ((AlifUStrObject*)strObj)->UTF);
#endif
	}
	else if (((AlifUStrObject*)strObj)->kind == UNICODE_4BYTE) {
#if SIZEOF_WCHART == 2
		uStrConvert_wCharToUCS4(_buffer, _buffer + size_, strObj);
#else
		memcpy(((AlifUStrObject*)strObj)->UTF, _buffer, size_ * 4);
#endif
	}

	return strObj;
}

void copy_string(AlifObject* object, const wchar_t* str, SSIZE_T length, SSIZE_T maxChar) {

	if (maxChar == 2) {

		memcpy(ALIFUNICODE_CAST(object)->UTF, str, (length * maxChar));

	}
	else if (maxChar == 4) {

		uint32_t* utf = (uint32_t*)(ALIFUNICODE_CAST(object))->UTF;
		size_t len = 0;
		uint32_t point, surrogate;

		while (*str != L'\0') {
			point = (uint32_t)(*str);
			if (point >= 0xD800 && point <= 0xDBFF) {
				++str; 
				surrogate = (uint32_t)(*str);
				point = (point - 0xD800) * 0x400 + (surrogate - 0xDC00) + 0x10000;
			}

			utf[len++] = point;
			++str; 
		}

	}
	else {
		std::wcout << L"ترميز الحروف غير صحيح\n" << std::endl;
		exit(-1);
	}

}

static AlifObject* unicode_wchar_t(uint32_t ch)
{
	AlifObject* unicode;

	unicode = alifNew_uStr(1, ch);
	if (unicode == nullptr)
		return nullptr;

	if (((AlifUStrObject*)unicode)->kind == UNICODE_2BYTE) {
		((uint16_t*)((AlifUStrObject*)unicode)->UTF)[0] = (uint32_t)ch;
	}
	else {
		((uint32_t*)((AlifUStrObject*)unicode)->UTF)[0] = ch;
	}
	return unicode;
}

static uint32_t kind_maxChar_limit(int kind)
{
	switch (kind) {
	case UNICODE_2BYTE:
		return 0x100;
	case UNICODE_4BYTE:
		return 0x10000;
	//default:
		//UNREACHABLE();
	}
}

static AlifObject*
alifUnicode_fromUint16(const uint16_t* u, int64_t size_)
{
	AlifObject* res;
	uint16_t max_char;

	//if (size_ == 0)
	//	__RETURN_UNICODE_EMPTY();
	if (size_ == 1)
		return unicode_wchar_t(u[0]);

	max_char = find_maxChar((const wchar_t*)u);
	res = alifNew_uStr(size_, max_char);
	if (!res)
		return nullptr;
	if (max_char >= 256)
		memcpy(((uint16_t*)((AlifUStrObject*)res)->UTF), u, sizeof(uint16_t) * size_);
	return res;
}

static AlifObject*
alifUnicode_fromUint32(const uint32_t* u, int64_t size_)
{
	AlifObject* res;
	uint32_t max_char;

	//if (size_ == 0)
		//__RETURN_UNICODE_EMPTY();
	if (size_ == 1)
		return unicode_wchar_t(u[0]);

	max_char = find_maxChar((const wchar_t*)u);
	res = alifNew_uStr(size_, max_char);
	//if (!res)
		//return nullptr;
	 if (max_char < 0x10000)
	 {
		 ALIFUNICODE_CONVERT_BYTES(uint32_t, uint16_t, u, u + size_,
			 ((uint16_t*)((AlifUStrObject*)res)->UTF));
	 }
	else
	 {
		 memcpy(((uint32_t*)((AlifUStrObject*)res)->UTF), u, sizeof(uint32_t) * size_);
	 }
	return res;
}

void combine_string(AlifObject* result, SSIZE_T start_, AlifObject* from, SSIZE_T fromStart, SSIZE_T length, uint8_t maxChar) {

	SSIZE_T fromKind = (ALIFUNICODE_CAST(from))->kind;
	SSIZE_T toKind = (ALIFUNICODE_CAST(result))->kind;
	if (fromKind == toKind) {

		void* fromData = ((wchar_t*)(ALIFUNICODE_CAST(from))->UTF + (fromStart * fromKind));
		void* toData = ((wchar_t*)(ALIFUNICODE_CAST(result))->UTF + (start_ * toKind));

		memcpy(toData, fromData, length * maxChar);

	}
	else if (fromKind == 2 && toKind == 4) {

		uint16_t* fromData = ((uint16_t*)(ALIFUNICODE_CAST(from))->UTF);
		uint32_t* toData = ((uint32_t*)(ALIFUNICODE_CAST(result))->UTF);

		ALIFUNICODE_CONVERT_BYTES(uint16_t, uint32_t,
			fromData + fromStart,
			fromData + fromStart + length, 
			toData + start_);

	}
	else if (fromKind == 4 && toKind == 2) {
	
		uint32_t* fromData = ((uint32_t*)(ALIFUNICODE_CAST(from))->UTF);
		uint16_t* toData = ((uint16_t*)(ALIFUNICODE_CAST(result))->UTF);

		ALIFUNICODE_CONVERT_BYTES(uint32_t, uint16_t,
			fromData + fromStart,
			fromData + fromStart + length,
			toData + start_);
	}
	else {
		std::wcout << L"ترميز الحروف غير صحيح\n" << std::endl;
		exit(-1);
	}

}

static AlifObject* unicode_char(uint32_t ch)
{
	AlifObject* unicode = alifNew_uStr(1, ch);
	if (unicode == nullptr)
		return nullptr;

	if (((AlifUStrObject*)unicode)->kind == UNICODE_2BYTE) {
		((uint16_t*)(((AlifUStrObject*)unicode)->UTF))[0] = (uint16_t)ch;
	}
	else {
		((uint32_t*)(((AlifUStrObject*)unicode)->UTF))[0] = ch;
	}
	return unicode;
}


AlifObject* unicode_decode_utf8(const wchar_t* str, SSIZE_T length) {

	if (length == 0) {
		// return empty object string
	}

	SSIZE_T maxChar = find_maxChar(str);

	AlifObject* object = alifNew_unicode(length, maxChar);

	copy_string((AlifObject*)object, str, length, maxChar);

	return object;

}

AlifObject* alifUStr_decodeUTF8Stateful(const wchar_t* _s, size_t _size, const wchar_t* _errors, size_t* _consumed) {
	return unicode_decode_utf8(_s, _size);
}


// من هنا يتم عمل كائن جديد ويتم فك ترميز النص وتخزينه في الكائن
AlifObject* alifUnicode_decodeStringToUTF8(const wchar_t* str) {

	return unicode_decode_utf8(str, count_characters(str));
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
	for (size_t i = 0; i < _obj->length; ++i) {
		if (((uint16_t)(utf8_[i])) > 127) {
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

int toDecimalValue(uint32_t ch) {
	switch (ch) {
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

AlifObject* alifSubUnicode_transformDecimalAndSpaceToASCII(AlifObject* _unicode)
{

	if (isOnlyAscii(((AlifUStrObject*)_unicode))) {
		return ALIF_NEWREF(_unicode);
	}

	int64_t len_ = ((AlifUStrObject*)(_unicode))->length;
	AlifObject* result_ = alifNew_uStr(len_, 2);
	if (result_ == nullptr) {
		return nullptr;
	}

	uint16_t* out_ = ((uint16_t*)((AlifUStrObject*)result_)->UTF);
	int kind_ = ((AlifUStrObject*)_unicode)->kind;
	const void* data_ = ((AlifUStrObject*)_unicode)->UTF;
	int64_t i_;
	for (i_ = 0; i_ < len_; ++i_) {
		uint32_t ch_ = ALIFUNICODE_READ(kind_, data_, i_);
		if (ch_ < 127) {
			out_[i_] = ch_;
		}
		//else if (ALIF_UNICODE_ISSPACE(ch)) {
			//out_[i_] = ' ';
		//}
		else {
			int decimal_ = toDecimalValue(ch_);
			if (decimal_ < 0) {
				out_[i_] = L'?';
				out_[i_ + 1] = L'\0';
				((AlifUStrObject*)result_)->length = i_ + 1;
				break;
			}
			out_[i_] = L'0' + decimal_;
		}
	}

	return result_;
}

SSIZE_T length_unicode(AlifObject* unicode) {
	return ALIFUNICODE_CAST(unicode)->length;
}

static int64_t anyLib_find(int kind, AlifObject* str1, const void* buf1, int64_t len1,
	AlifObject* str2, const void* buf2, int64_t len2, int64_t offset)
{
	switch (kind) {
	case UNICODE_2BYTE:
		return find((uint16_t*)buf1, len1, (uint16_t*)buf2, len2, offset);
	case UNICODE_4BYTE:
		return find((uint32_t*)buf1, len1, (uint32_t*)buf2, len2, offset);
	}
	//UNREACHABLE();
}

static int64_t anyLib_count(int kind, AlifObject* sstr, const void* sbuf, int64_t slen,
	AlifObject* str1, const void* buf1, int64_t len1, int64_t maxcount)
{
	switch (kind) {
	case UNICODE_2BYTE:
		return count((uint16_t*)sbuf, slen, (uint16_t*)buf1, len1, maxcount);
	case UNICODE_4BYTE:
		return count((uint32_t*)sbuf, slen, (uint32_t*)buf1, len1, maxcount);
	}
	//UNREACHABLE();
}

static void* unicode_asKind(int skind, void const* data, int64_t len, int kind)
{
	void* result{};
	// هنا يوجد تحقق واحد لانه من الاعلى للادني ولا يوجد سوى نوعين 2byte or 4byte فاما ان يكون 2byte فيبقى كما هو  او 4byte فيتحول للادنى
	switch (kind) {
	case UNICODE_4BYTE:
		result = (uint32_t*)alifMem_dataAlloc(len);
		//if (!result)
			//return Err_NoMemory();
		if (skind == UNICODE_2BYTE) {
			ALIFUNICODE_CONVERT_BYTES(
				uint16_t, uint32_t,
				(const uint16_t*)data,
				((const uint16_t*)data) + len,
				result);
		}

		return result;
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

static int
unicode_fromFormat_write_str(AlifSubUnicodeWriter* _writer, AlifObject* _str, int64_t _width, int64_t _precision, int _flags)
{
	int64_t length_, fill_, argLen;
	uint32_t maxChar;

	length_ = ALIFUNICODE_GET_LENGTH(_str);
	if ((_precision == -1 || _precision >= length_)
		&& _width <= length_)
		return alifSubUnicodeWriter_writeStr(_writer, _str);

	if (_precision != -1)
		length_ = min(_precision, length_);

	argLen = max(length_, _width);
	//if (ALIFUNICODE_MAX_CHAR_VALUE(_str) > _writer->maxChar)
		//maxChar = fin_maxChar(_str);
	//else
		//maxChar = _writer->maxChar;

	//if (alifSubUnicodeWriter_prepare(_writer, argLen, maxChar) == -1)
		//return -1;

	fill_ = max(_width - length_, 0);
	if (fill_ && !(_flags & F_LJUST)) {
		if (alifUnicode_fill(_writer->buffer_, _writer->pos_, fill_, ' ') == -1)
			return -1;
		_writer->pos_ += fill_;
	}

	alifSubUnicode_fastCopyCharacters(_writer->buffer_, _writer->pos_,
		_str, 0, length_, 0);
	_writer->pos_ += length_;

	if (fill_ && (_flags & F_LJUST)) {
		if (alifUnicode_fill(_writer->buffer_, _writer->pos_, fill_, ' ') == -1)
			return -1;
		_writer->pos_ += fill_;
	}

	return 0;
}

static int
unicode_fromFormat_write_cstr(AlifSubUnicodeWriter* _writer, const wchar_t* _str,
	int64_t _width, int64_t _precision, int _flags)
{
	/* UTF-8 */
	int64_t length_;
	AlifObject* unicode_;
	int res;

	if (_precision == -1) {
		length_ = wcslen(_str);
	}
	else {
		length_ = 0;
		while (length_ < _precision && _str[length_]) {
			length_++;
		}
	}
	unicode_ = alifUStr_decodeUTF8Stateful(_str, length_, L"replace", nullptr);
	if (unicode_ == NULL)
		return -1;

	res = unicode_fromFormat_write_str(_writer, unicode_, _width, -1, _flags);
	ALIF_DECREF(unicode_);
	return res;
}


static int unicode_fromFormat_write_wcstr(AlifSubUnicodeWriter* _writer, const wchar_t* _str,
	int64_t _width, int64_t _precision, int _flags)
{
	/* UTF-8 */
	int64_t length_;
	AlifObject* unicode_;
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
	unicode_ = alifUnicode_fromWideChar(_str, length_);
	if (unicode_ == NULL)
		return -1;

	//res_ = unicode_fromFormat_write_str(_writer, unicode_, _width, -1, _flags);
	ALIF_DECREF(unicode_);
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

static const wchar_t* unicode_fromFormat_arg(AlifSubUnicodeWriter* _writer,
	const wchar_t* _f, va_list* _vArgs)
{
	const wchar_t* p_;
	int64_t len_;
	int flags_ = 0;
	int64_t width_;
	int64_t precision_;

	p_ = _f;
	_f++;
	if (*_f == '%') {
		if (alifSubUnicodeWriter_writeWcharInline(_writer, '%') < 0)
			return NULL;
		_f++;
		return _f;
	}

	/* Parse flags. Example: "%-i" => flags=F_LJUST. */
	/* Flags '+', ' ' and '#' are not particularly useful.
	 * They are not worth the implementation and maintenance costs.
	 * In addition, '#' should add "0" for "o" conversions for compatibility
	 * with printf, but it would confuse alif users. */
	while (1) {
		switch (*_f++) {
		case '-': flags_ |= F_LJUST; continue;
		case '0': flags_ |= F_ZERO; continue;
		}
		_f--;
		break;
	}

	/* parse the width_.precision_ part, e.g. "%2.5s" => width_=2, precision_=5 */
	width_ = -1;
	if (*_f == '*') {
		width_ = va_arg(*_vArgs, int);
		if (width_ < 0) {
			flags_ |= F_LJUST;
			width_ = -width_;
		}
		_f++;
	}
	else if (ALIF_ISDIGIT((unsigned)*_f)) {
		width_ = *_f - '0';
		_f++;
		while (ALIF_ISDIGIT((unsigned)*_f)) {
			if (width_ > (LLONG_MAX - ((int)*_f - '0')) / 10) {
				return NULL;
			}
			width_ = (width_ * 10) + (*_f - '0');
			_f++;
		}
	}
	precision_ = -1;
	if (*_f == '.') {
		_f++;
		if (*_f == '*') {
			precision_ = va_arg(*_vArgs, int);
			if (precision_ < 0) {
				precision_ = -2;
			}
			_f++;
		}
		else if (ALIF_ISDIGIT((unsigned)*_f)) {
			precision_ = (*_f - '0');
			_f++;
			while (ALIF_ISDIGIT((unsigned)*_f)) {
				if (precision_ > (LLONG_MAX - ((int)*_f - '0')) / 10) {

					return NULL;
				}
				precision_ = (precision_ * 10) + (*_f - '0');
				_f++;
			}
		}
	}

	int sizeMod = 0;
	if (*_f == 'l') {
		if (_f[1] == 'l') {
			sizeMod = F_LONGLONG;
			_f += 2;
		}
		else {
			sizeMod = F_LONG;
			++_f;
		}
	}
	else if (*_f == 'z') {
		sizeMod = F_SIZE;
		++_f;
	}
	else if (*_f == 't') {
		sizeMod = F_PTRDIFF;
		++_f;
	}
	else if (*_f == 'j') {
		sizeMod = F_INTMAX;
		++_f;
	}
	if (_f[0] != '\0' && _f[1] == '\0')
		_writer->overAllocate = 0;

	switch (*_f) {
	case 'd': case 'i': case 'o': case 'u': case 'x': case 'X':
		break;
	case 'c': case 'p':
		if (sizeMod || width_ >= 0 || precision_ >= 0) goto invalidFormat;
		break;
	case 's':
	case 'V':
		if (sizeMod && sizeMod != F_LONG) goto invalidFormat;
		break;
	default:
		if (sizeMod) goto invalidFormat;
		break;
	}

	switch (*_f) {
	case 'c':
	{
		int ordinal_ = va_arg(*_vArgs, int);
		if (ordinal_ < 0 || ordinal_ > 0x10ffff) {
			return NULL;
		}
		if (alifSubUnicodeWriter_writeWcharInline(_writer, ordinal_) < 0)
			return NULL;
		break;
	}

	case 'd': case 'i':
	case 'o': case 'u': case 'x': case 'X':
	{
		/* used by sprintf */
		wchar_t buffer_[MAX_INTMAX_CHARS];
		const wchar_t* fmt_ = NULL;
		switch (*_f) {
		case 'o': fmt_ = _formatsO_[sizeMod]; break;
		case 'u': fmt_ = _formatsU_[sizeMod]; break;
		case 'x': fmt_ = _formatsX_[sizeMod]; break;
		case 'X': fmt_ = _formatsXC_[sizeMod]; break;
		default: fmt_ = _formats_[sizeMod]; break;
		}
		int isSigned = (*_f == 'd' || *_f == 'i');
		switch (sizeMod) {
		case F_LONG:
			len_ = isSigned ?
				swprintf_s(buffer_, fmt_, va_arg(*_vArgs, long)) :
				swprintf_s(buffer_, fmt_, va_arg(*_vArgs, unsigned long));
			break;
		case F_LONGLONG:
			len_ = isSigned ?
				swprintf_s(buffer_, fmt_, va_arg(*_vArgs, long long)) :
				swprintf_s(buffer_, fmt_, va_arg(*_vArgs, unsigned long long));
			break;
		case F_SIZE:
			len_ = isSigned ?
				swprintf_s(buffer_, fmt_, va_arg(*_vArgs, int64_t)) :
				swprintf_s(buffer_, fmt_, va_arg(*_vArgs, size_t));
			break;
		case F_PTRDIFF:
			len_ = swprintf_s(buffer_, fmt_, va_arg(*_vArgs, ptrdiff_t));
			break;
		case F_INTMAX:
			len_ = isSigned ?
				swprintf_s(buffer_, fmt_, va_arg(*_vArgs, intmax_t)) :
				swprintf_s(buffer_, fmt_, va_arg(*_vArgs, uintmax_t));
			break;
		default:
			len_ = isSigned ?
				swprintf_s(buffer_, fmt_, va_arg(*_vArgs, int)) :
				swprintf_s(buffer_, fmt_, va_arg(*_vArgs, unsigned int));
			break;
		}

		int sign_ = (buffer_[0] == '-');
		len_ -= sign_;

		precision_ = max(precision_, len_);
		width_ = max(width_, precision_ + sign_);
		if ((flags_ & F_ZERO) && !(flags_ & F_LJUST)) {
			precision_ = width_ - sign_;
		}

		int64_t spacePad = max(width_ - precision_ - sign_, 0);
		int64_t zeroPad = max(precision_ - len_, 0);

		if (ALIFSUBUNICODEWRITER_PREPARE(_writer, width_, 127) == -1)
			return NULL;

		if (spacePad && !(flags_ & F_LJUST)) {
			if (alifUnicode_fill(_writer->buffer_, _writer->pos_, spacePad, ' ') == -1)
				return NULL;
			_writer->pos_ += spacePad;
		}

		if (sign_) {
			if (alifSubUnicodeWriter_writeChar(_writer, '-') == -1)
				return NULL;
		}

		if (zeroPad) {
			if (alifUnicode_fill(_writer->buffer_, _writer->pos_, zeroPad, '0') == -1)
				return NULL;
			_writer->pos_ += zeroPad;
		}

		if (spacePad && (flags_ & F_LJUST)) {
			if (alifUnicode_fill(_writer->buffer_, _writer->pos_, spacePad, ' ') == -1)
				return NULL;
			_writer->pos_ += spacePad;
		}
		break;
	}

	case 'p':
	{
		wchar_t number_[MAX_INTMAX_CHARS];

		len_ = swprintf_s(number_, L"%p", va_arg(*_vArgs, void*));

		if (number_[1] == 'X')
			number_[1] = 'x';
		else if (number_[1] != 'x') {
			memmove(number_ + 2, number_,
				wcslen(number_) + 1);
			number_[0] = '0';
			number_[1] = 'x';
			len_ += 2;
		}

		break;
	}

	case 's':
	{
		if (sizeMod) {
			const wchar_t* s = va_arg(*_vArgs, const wchar_t*);
			if (unicode_fromFormat_write_wcstr(_writer, s, width_, precision_, flags_) < 0)
				return NULL;
		}
		else {
			/* UTF-8 */
			const wchar_t* s = va_arg(*_vArgs, const wchar_t*);
			if (unicode_fromFormat_write_wcstr(_writer, s, width_, precision_, flags_) < 0)
				return NULL;
		}
		break;
	}

	case 'U':
	{
		AlifObject* obj = va_arg(*_vArgs, AlifObject*);

		if (unicode_fromFormat_write_str(_writer, obj, width_, precision_, flags_) == -1)
			return NULL;
		break;
	}

	case 'V':
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
			if (unicode_fromFormat_write_str(_writer, obj_, width_, precision_, flags_) == -1)
				return NULL;
		}
		else if (sizeMod) {
			if (unicode_fromFormat_write_wcstr(_writer, wstr_, width_, precision_, flags_) < 0)
				return NULL;
		}
		else {
			if (unicode_fromFormat_write_cstr(_writer, str_, width_, precision_, flags_) < 0)
				return NULL;
		}
		break;
	}

	case 'S':
	{
		AlifObject* obj_ = va_arg(*_vArgs, AlifObject*);
		AlifObject* str_;
		//str_ = Object_Str(obj_);
		if (!str_)
			return NULL;
		if (unicode_fromFormat_write_str(_writer, str_, width_, precision_, flags_) == -1) {
			ALIF_DECREF(str_);
			return NULL;
		}
		ALIF_DECREF(str_);
		break;
	}

	case 'R':
	{
		AlifObject* obj_ = va_arg(*_vArgs, AlifObject*);
		AlifObject* repr;
		//repr = alifObject_Repr(obj_);
		if (!repr)
			return NULL;
		if (unicode_fromFormat_write_str(_writer, repr, width_, precision_, flags_) == -1) {
			ALIF_DECREF(repr);
			return NULL;
		}
		ALIF_DECREF(repr);
		break;
	}

	case 'A':
	{
		AlifObject* obj_ = va_arg(*_vArgs, AlifObject*);
		AlifObject* ascii;
		//ascii = alifObject_ASCII(obj_);
		if (!ascii)
			return NULL;
		if (unicode_fromFormat_write_str(_writer, ascii, width_, precision_, flags_) == -1) {
			ALIF_DECREF(ascii);
			return NULL;
		}
		ALIF_DECREF(ascii);
		break;
	}

	case 'T':
	{
		AlifObject* obj_ = va_arg(*_vArgs, AlifObject*);
		AlifTypeObject* type = (AlifTypeObject*)ALIF_NEWREF(ALIF_TYPE(obj_));

		AlifObject* type_name;
		if (_f[1] == '#') {
			//type_name = Type_GetFullyQualifiedName(type, ':');
			_f++;
		}
		else {
			//type_name = Type_GetFullyQualifiedName(type);
		}
		ALIF_DECREF(type);
		if (!type_name) {
			return NULL;
		}

		if (unicode_fromFormat_write_str(_writer, type_name,
			width_, precision_, flags_) == -1) {
			ALIF_DECREF(type_name);
			return NULL;
		}
		ALIF_DECREF(type_name);
		break;
	}

	case 'N':
	{
		AlifObject* typeRaw = va_arg(*_vArgs, AlifObject*);

		//if (!Type_Check(typeRaw)) {
			//return NULL;
		//}
		AlifTypeObject* type_ = (AlifTypeObject*)typeRaw;
		
		AlifObject* typeName;
		if (_f[1] == '#') {
			//typeName = Type_GetFullyQualifiedName(type_, ':');
			_f++;
		}
		else {
			//typeName = Type_GetFullyQualifiedName(type_);
		}
		if (!typeName) {
			return NULL;
		}
		if (unicode_fromFormat_write_str(_writer, typeName,
			width_, precision_, flags_) == -1) {
			ALIF_DECREF(typeName);
			return NULL;
		}
		ALIF_DECREF(typeName);
		break;
	}

	default:
	invalidFormat:
		return NULL;
	}

	_f++;
	return _f;
}

const wchar_t* alifUnicode_asUTF8AndSize(AlifObject* _unicode, int64_t* _pSize)
{
	if (!(_unicode->type_ == &_typeUnicode_)) {
		if (_pSize) {
			*_pSize = -1;
		}
		return NULL;
	}

	//if (unicode_fill_utf8(unicode) == -1) {
		//if (_pSize) {
			//*_pSize = -1;
		//}
		//return NULL;
	//}
	

	if (_pSize) {
		*_pSize = ALIFUNICODE_CAST(_unicode)->length;
	}
	return (const wchar_t*)ALIFUNICODE_CAST(_unicode)->UTF;
}

const wchar_t* alifUnicode_asUTF8(AlifObject* _unicode)
{
	return alifUnicode_asUTF8AndSize(_unicode, NULL);
}

AlifObject* alifUnicode_decodeUTF8(const wchar_t* _s, int64_t _size, const wchar_t* _errors)
{
	return alifUStr_decodeUTF8Stateful(_s, _size, _errors, NULL);
}

AlifObject* combine_unicode(AlifObject* left, AlifObject* right) {

	AlifObject* result;
	uint16_t maxChar;
	SSIZE_T leftLen = (ALIFUNICODE_CAST(left))->length
		, rightLen = (ALIFUNICODE_CAST(right))->length,
		newLen;

	if (leftLen == 0) {
		return right;
	}
	if (rightLen == 0) {
		return left;
	}

	newLen = leftLen + rightLen;

	maxChar = max((ALIFUNICODE_CAST(left))->kind, (ALIFUNICODE_CAST(right))->kind);

	result = (AlifObject*)alifNew_unicode(newLen, maxChar);
	if (result == nullptr) {
		return nullptr;
	}

	combine_string(result, 0, left, 0, leftLen, maxChar);
	combine_string(result, leftLen, right, 0, rightLen, maxChar);

	return result;

}

AlifObject* repeat_unicode(AlifObject* unicode, SSIZE_T repeat) {

	if (repeat == 0) {
		// return empty string
	}

	if (repeat == 1) {
		return unicode;
	}

	SSIZE_T lenUnicode = (ALIFUNICODE_CAST(unicode))->length;
	SSIZE_T len = lenUnicode * repeat;
	uint8_t kind = (ALIFUNICODE_CAST(unicode))->kind;

	AlifObject* object = (AlifObject*)alifNew_unicode(len, kind);

	if(object == nullptr){
		return nullptr;
	}

	if (kind == 2) {
		uint16_t* str = (uint16_t*)(ALIFUNICODE_CAST(unicode))->UTF;
		uint16_t* utf = (uint16_t*)(ALIFUNICODE_CAST(object))->UTF;
		for (SSIZE_T n = 0; n < repeat; ++n)
		{
			memcpy(utf + (lenUnicode * n), str, lenUnicode * 2);
		}
	}
	else {
		uint32_t* str = (uint32_t*)(ALIFUNICODE_CAST(unicode))->UTF;
		uint32_t* utf = (uint32_t*)(ALIFUNICODE_CAST(object))->UTF;
		for (SSIZE_T n = 0; n < repeat; ++n)
		{
			memcpy(utf + (lenUnicode * n), str, lenUnicode * 4);
		}
	}

	return object;

}

bool contain_unicode(AlifObject* unicode, AlifObject* str) {

    uint8_t kind1, kind2;
	const void* utf1, * utf2;
	SSIZE_T len1, len2;


	kind1 = (ALIFUNICODE_CAST(unicode))->kind;
	kind2 = (ALIFUNICODE_CAST(str))->kind;

	if (kind1 < kind2) {
		return 0;
	}

	len1 = (ALIFUNICODE_CAST(unicode))->length;
	len2 = (ALIFUNICODE_CAST(str))->length;

	if (len1 < len2) {
		return 0;
	}

	utf1 = (ALIFUNICODE_CAST(unicode))->UTF;
	utf2 = (ALIFUNICODE_CAST(str))->UTF;
	if (kind1 != kind2) {
		void* result = (uint32_t*)alifMem_dataAlloc(len2 * 4);
		ALIFUNICODE_CONVERT_BYTES(
			uint16_t, uint32_t,
			(const uint16_t*)utf2,
			((const uint16_t*)utf2) + len2,
			result);
	}

	if (kind1 == 2) {
	
		const uint16_t* srcStr = (uint16_t*)utf1;
		const uint16_t* subStr = (uint16_t*)utf2;

		for (size_t i = 0; i <= len1 - len2; ++i) {

			if (std::memcmp(&srcStr[i], subStr, len2 * sizeof(uint16_t)) == 0) {
				return true;
			}
		}
		return false;

	}
	else if (kind1 == 4) {
		const uint32_t* srcStr = (uint32_t*)utf1;
		const uint32_t* subStr = (uint32_t*)utf2;

		for (size_t i = 0; i <= len1 - len2; ++i) {
			
			if (std::memcmp(&srcStr[i], subStr, len2 * sizeof(uint32_t)) == 0) {
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
size_t hash_unicode(AlifObject* unicode) {

	const uint64_t fnvPrime = 1099511628211ull;
	const uint64_t fnvOffsetBasis = 14695981039346656037ull;

	AlifUStrObject* self = ALIFUNICODE_CAST(unicode);
	
	uint8_t kind = self->kind;

	uint8_t* data = (uint8_t*)(self->UTF);

	size_t hash = fnvOffsetBasis;
	
	if (kind == 2) {
		for (int i = 0; i < self->length; ++i) {
			hash ^= *(uint16_t*)data;
			int a = *(uint16_t*)data;
			data += 2;
			hash *= fnvPrime;
		}
	}
	else {
		for (int i = 0; i < self->length ; ++i) {
			hash ^= *(uint32_t*)data;
			data += 4;
			hash *= fnvPrime;
		}
	}
	return hash;
}

int unicode_compare_eq(AlifObject* str1, AlifObject* str2)
{
	int kind{};
	const void* data1{}, * data2{};
	size_t len{};
	int cmp{};

	len = ALIFUNICODE_CAST(str1)->length;
	if (ALIFUNICODE_CAST(str2)->length != len)
		return 0;
	kind = ALIFUNICODE_CAST(str1)->kind;
	if (ALIFUNICODE_CAST(str2)->kind != kind)
		return 0;
	data1 = ALIFUNICODE_CAST(str1);
	data2 = ALIFUNICODE_CAST(str2);

	cmp = memcmp(data1, data2, len * kind);
	return (cmp == 0);
}

AlifObject* unicode_compare(AlifObject* left, AlifObject* right, int op) {

	if (left->type_ != &_typeUnicode_ || 
		right->type_ != &_typeUnicode_) {
		std::wcout << L"عمليه مقارنة النص غير صحيحة\n" << std::endl;
		exit(-1);
	}

	if (left == right && op == ALIF_EQ) {
		return ALIF_TRUE;
	}
	else if (op == ALIF_EQ || op == ALIF_NE) {
		int result = unicode_compare_eq(left, right);
		return result ? ALIF_TRUE : ALIF_FALSE;
	}

}

// in file eq.h
int unicode_eq(AlifObject* a, AlifObject* b)
{
	if (((AlifUStrObject*)a)->length != ((AlifUStrObject*)b)->length)
		return 0;
	if (((AlifUStrObject*)a)->length == 0)
		return 1;
	if (((AlifUStrObject*)a)->kind != ((AlifUStrObject*)b)->kind)
		return 0;
	return memcmp((uint8_t*)(((AlifUStrObject*)a)->UTF), (uint8_t*)(((AlifUStrObject*)b)->UTF),
		((AlifUStrObject*)a)->length * ((AlifUStrObject*)a)->kind) == 0;
}

void unicode_dealloc(AlifObject* unicode) {
	alifMem_objFree(unicode);
}

/* helper macro to fixup start_/end slice values */
#define ADJUST_INDICES(start_, end, len)         \
    if (end > len)                              \
        end = len;                              \
    else if (end < 0) {                         \
        end += len;                             \
        if (end < 0)                            \
            end = 0;                            \
    }                                           \
    if (start_ < 0) {                            \
        start_ += len;                           \
        if (start_ < 0)                          \
            start_ = 0;                          \
    }

static int64_t
any_find_slice(AlifObject* s1, AlifObject* s2,
	int64_t start_,
	int64_t end,
	int direction)
{
	int kind1, kind2;
	const void* buf1, * buf2;
	int64_t len1, len2, result;

	kind1 = ((AlifUStrObject*)s1)->kind;
	kind2 = ((AlifUStrObject*)s2)->kind;
	if (kind1 < kind2)
		return -1;

	len1 = ((AlifUStrObject*)s1)->length;
	len2 = ((AlifUStrObject*)s2)->length;
	ADJUST_INDICES(start_, end, len1);
	if (end - start_ < len2)
		return -1;

	buf1 = ((AlifUStrObject*)s1)->UTF;
	buf2 = ((AlifUStrObject*)s2)->UTF;
	if (len2 == 1) {
		uint32_t ch = ALIFUNICODE_READ(kind2, buf2, 0);
		result = findChar((const wchar_t*)buf1 + kind1 * start_,
			kind1, end - start_, ch, direction);
		if (result == -1)
			return -1;
		else
			return start_ + result;
	}

	if (kind2 != kind1) {
		buf2 = unicode_asKind(kind2, buf2, len2, kind1);
		if (!buf2)
			return -2;
	}

	if (direction > 0) {
		switch (kind1) {
		case UNICODE_2BYTE:
			result = find_slice((uint16_t*)buf1, len1, (uint16_t*)buf2, len2, start_, end);
			break;
		case UNICODE_4BYTE:
			result = find_slice((uint32_t*)buf1, len1, (uint32_t*)buf2, len2, start_, end);
			break;
		//default:
			//UNREACHABLE();
		}
	}
	else {
		switch (kind1) {
		case UNICODE_2BYTE:
			result = rfind_slice((uint16_t*)buf1, len1, (uint16_t*)buf2, len2, start_, end);
			break;
		case UNICODE_4BYTE:
			result = rfind_slice((uint32_t*)buf1, len1, (uint32_t*)buf2, len2, start_, end);
			break;
		//default:
			//UNREACHABLE();
		}
	}

	if (kind2 != kind1)
		alifMem_dataFree((void*)buf2);

	return result;
}

// this function in file replace.h
template <typename STRINGLIB_CHAR>
void ucsLib_replace_1char_inplace(STRINGLIB_CHAR* s, STRINGLIB_CHAR* end,
	uint32_t u1, uint32_t u2, int64_t maxCount)
{
	*s = u2;
	while (--maxCount && ++s != end) {
		/* Find the next character to be replaced.

		   If it occurs often, it is faster to scan for it using an inline
		   loop.  If it occurs seldom, it is faster to scan for it using a
		   function call; the overhead of the function call is amortized
		   across the many characters that call covers.  We start_ with an
		   inline loop and use a heuristic to determine whether to fall back
		   to a function call. */
		if (*s != u1) {
			int attempts = 10;
			/* search u1 in a dummy loop */
			while (1) {
				if (++s == end)
					return;
				if (*s == u1)
					break;
				if (!--attempts) {
					/* if u1 was not found for attempts iterations,
					   use FASTSEARCH() or memchr() */
#ifdef STRINGLIB_FAST_MEMCHR
					s++;
					s = (STRINGLIB_CHAR*)STRINGLIB_FAST_MEMCHR(s, u1, end - s);
					if (s == nullptr)
						return;
#else
					int64_t i;
					STRINGLIB_CHAR ch1 = (STRINGLIB_CHAR)u1;
					s++;
					i = FASTSEARCH(s, end - s, &ch1, 1, 0, FAST_SEARCH);
					if (i < 0)
						return;
					s += i;
#endif
					/* restart the dummy loop */
					break;
				}
			}
		}
		*s = u2;
	}
}

static inline int parse_args_finds_unicode(const wchar_t* function_name, AlifObject* args,
	AlifObject** substring,
	int64_t* start_, int64_t* end)
{
	if (parse_args_finds(function_name, args, substring, start_, end)) {
		return 1;
	}
	return 0;
}

static void replace_1char_inplace(AlifObject* u, int64_t pos,
	uint32_t u1, uint32_t u2, int64_t maxcount)
{
	int kind = ALIFUNICODE_CAST(u)->kind;
	void* data = ALIFUNICODE_CAST(u)->UTF;
	int64_t len = ALIFUNICODE_CAST(u)->length;
	if (kind == UNICODE_2BYTE) {
		ucsLib_replace_1char_inplace((uint16_t*)data + pos,
			(uint16_t*)data + len,
			u1, u2, maxcount);
	}
	else {
		ucsLib_replace_1char_inplace((uint32_t*)data + pos,
			(uint32_t*)data + len,
			u1, u2, maxcount);
	}
}

AlifObject* alifUStr_concat(AlifObject* _left, AlifObject* _right)
{
	AlifObject* result_;
	AlifUCS4 maxChar, maxChar2;
	int64_t leftLen, rightLen, newLen;

	if (!(_left->type_ == &_typeUnicode_) )
		return NULL;

	if (!(_right->type_ == &_typeUnicode_)) {
		return NULL;
	}

	//AlifObject* empty = ustr_get_empty();  
	//if (left == empty) {
		//return PyUnicode_FromObject(right);
	//}
	//if (right == empty) {
		//return PyUnicode_FromObject(left);
	//}

	leftLen = ALIFUNICODE_GET_LENGTH(_left);
	rightLen = ALIFUNICODE_GET_LENGTH(_right);
	if (leftLen > INTPTR_MAX - rightLen) {
		return NULL;
	}
	newLen = leftLen + rightLen;

	maxChar = ALIFUNICODE_MAX_CHAR_VALUE(_left);
	maxChar2 = ALIFUNICODE_MAX_CHAR_VALUE(_right);
	maxChar = max(maxChar, maxChar2);

	result_ = alifNew_unicode(newLen, maxChar);
	if (result_ == NULL)
		return NULL;
	alifSubUnicode_fastCopyCharacters(result_, 0, _left, 0, leftLen, 0);
	alifSubUnicode_fastCopyCharacters(result_, leftLen, _right, 0, rightLen, 0);
	return result_;
}

void alifUstr_append(AlifObject** _pLeft, AlifObject* _right)
{
	AlifObject* left_, * res;
	AlifUCS4 maxChar, maxChar2;
	int64_t leftLen, rightLen, newLen;

	if (_pLeft == NULL) {
		return;
	}
	left_ = *_pLeft;
	if (_right == NULL || left_ == NULL
		|| !(left_->type_ == &_typeUnicode_) || !(_right->type_ == &_typeUnicode_)) {
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

	leftLen = ALIFUNICODE_GET_LENGTH(left_);
	rightLen = ALIFUNICODE_GET_LENGTH(_right);
	if (leftLen > INTPTR_MAX - rightLen) {
		goto error;
	}
	newLen = leftLen + rightLen;

	if (unicode_modifiable(left_)
		&& (_right->type_ == &_typeUnicode_)
		&& ALIFUNICODE_KIND(_right) <= ALIFUNICODE_KIND(left_))
	{
		if (unicode_resize(_pLeft, newLen) != 0)
			goto error;

		alifSubUnicode_fastCopyCharacters(*_pLeft, leftLen, _right, 0, rightLen, 0);
	}
	else {
		maxChar = ALIFUNICODE_MAX_CHAR_VALUE(left_);
		maxChar2 = ALIFUNICODE_MAX_CHAR_VALUE(_right);
		maxChar = max(maxChar, maxChar2);

		res = alifNew_unicode(newLen, maxChar);
		if (res == NULL)
			goto error;
		alifSubUnicode_fastCopyCharacters(res, 0, left_, 0, leftLen ,0 );
		alifSubUnicode_fastCopyCharacters(res, leftLen, _right, 0, rightLen,0);
		ALIF_DECREF(left_);
		*_pLeft = res;
	}
	return;

error:
	ALIF_CLEAR(*_pLeft);
}


static int64_t unicode_countImpl(AlifObject* str,
	AlifObject* substr,
	int64_t start_,
	int64_t end)
{

	int64_t result;
	int kind1, kind2;
	const void* buf1 = nullptr, * buf2 = nullptr;
	int64_t len1, len2;

	kind1 = ((AlifUStrObject*)str)->kind;
	kind2 = ((AlifUStrObject*)substr)->kind;
	if (kind1 < kind2)
		return 0;

	len1 = ((AlifUStrObject*)str)->length;
	len2 = ((AlifUStrObject*)substr)->length;
	ADJUST_INDICES(start_, end, len1);
	if (end - start_ < len2)
		return 0;

	buf1 = ((AlifUStrObject*)str)->UTF;
	buf2 = ((AlifUStrObject*)substr)->UTF;
	if (kind2 != kind1) {
		buf2 = unicode_asKind(kind2, buf2, len2, kind1);
		if (!buf2)
			goto onError;
	}

	// We don't reuse `anylib_count` here because of the explicit casts.
	switch (kind1) {
	case UNICODE_2BYTE:
		result = count(
			((const uint16_t*)buf1) + start_, end - start_,
			(uint16_t*)buf2, len2, LLONG_MAX
		);
		break;
	case UNICODE_4BYTE:
		result = count(
			((const uint32_t*)buf1) + start_, end - start_,
			(uint32_t*)buf2, len2, LLONG_MAX
		);
		break;
	//default:
		//UNREACHABLE();
	}

	if (kind2 != kind1)
		alifMem_dataFree((void*)buf2);

	return result;
onError:
	if (kind2 != kind1)
		alifMem_dataFree((void*)buf2);
	return -1;
}

static AlifObject* unicode_count(AlifObject* self, AlifObject* args)
{
	AlifObject* substring = nullptr;   /* initialize to fix a compiler warning */
	int64_t start_ = 0;
	int64_t end = LLONG_MAX;
	int64_t result;

	if (!parse_args_finds_unicode(L"count", args, &substring, &start_, &end))
		return nullptr;

	result = unicode_countImpl(self, substring, start_, end);
	if (result == -1)
		return nullptr;

	return alifInteger_fromLongLong(result);
}

static AlifObject* replace(AlifObject*, AlifObject* ,
	AlifObject*, int64_t );
static AlifObject* unicode_replace(AlifObject* self, AlifObject* const* args, int64_t nargs, AlifObject* kwnames)
{
	AlifObject* return_value = nullptr;

#define NUM_KEYWORDS 1
	static class KwTuple{
	public:
		AlifVarObject base{};
		AlifObject* item[NUM_KEYWORDS];
	} kwTuple = {
		//ALIFVAROBJECT_HEAD_INIT(&typeTuple, 1)
		//nullptr
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&kwTuple.base.object)
//#  define KWTUPLE nullptr

	static const wchar_t* const _keywords[] = { L"", L"", L"count", nullptr };
	static AlifArgParser _parser = {
		0,
		L"",
		_keywords,
		L"replace",
		L"",
		0,
		0,
		0,
		//KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[3];
	int64_t noptargs = nargs + (kwnames ? ((AlifVarObject*)kwnames)->size_ : 0) - 2;
	AlifObject* old;
	AlifObject* _new;
	int64_t count = -1;

	args = ALIFARG_UNPACKKEYWORDS(args, nargs, nullptr, kwnames, &_parser, 2, 3, 0, argsbuf);
	if (!args) {
		goto exit;
	}
	if (!(args[0]->type_ == &_typeUnicode_)) {
		//_Arg_BadArgument("replace", "argument 1", "str", args[0]);
		goto exit;
	}
	old = args[0];
	if (!(args[1]->type_ == &_typeUnicode_)) {
		//_Arg_BadArgument("replace", "argument 2", "str", args[1]);
		goto exit;
	}
	_new = args[1];
	if (!noptargs) {
		goto skip_optional_pos;
	}
	{
		int64_t ival = -1;
		AlifObject* iobj = args[2];
		if (iobj != nullptr) {
			ival = alifInteger_asLongLong(iobj);
			ALIF_DECREF(iobj);
		}

		count = ival;
	}
skip_optional_pos:
	return_value = replace(self, old, _new, count);

exit:
	return return_value;
}

static AlifObject* replace(AlifObject* self, AlifObject* str1,
	AlifObject* str2, int64_t maxCount)
{
	AlifObject* u{};
	const wchar_t* sBuf = (const wchar_t*)ALIFUNICODE_CAST(self)->UTF;
	const void* buf1 = (const wchar_t*)ALIFUNICODE_CAST(str1)->UTF;
	const void* buf2 = (const wchar_t*)ALIFUNICODE_CAST(str2)->UTF;
	int srelease = 0, release1 = 0, release2 = 0;
	int sKind = ALIFUNICODE_CAST(self)->kind;
	int kind1 = ALIFUNICODE_CAST(str1)->kind;
	int kind2 = ALIFUNICODE_CAST(str2)->kind;
	int64_t sLen = ALIFUNICODE_CAST(self)->length;
	int64_t len1 = ALIFUNICODE_CAST(str1)->length;
	int64_t len2 = ALIFUNICODE_CAST(str2)->length;
	int mayShrink;
	uint32_t maxChar, maxCharStr1, maxCharStr2;

	if (sLen < len1)
		goto nothing;

	if (maxCount < 0)
		maxCount = LLONG_MAX;
	else if (maxCount == 0)
		goto nothing;

	if (str1 == str2)
		goto nothing;

	maxChar = ALIFUNICODE_MAX_CHAR_VALUE(self);
	maxCharStr1 = ALIFUNICODE_MAX_CHAR_VALUE(str1);
	if (maxChar < maxCharStr1)
		/* substring too wide to be present */
		goto nothing;
	maxCharStr2 = ALIFUNICODE_MAX_CHAR_VALUE(str2);
	/* Replacing str1 with str2 may cause a maxChar reduction in the
	   result string. */
	mayShrink = (maxCharStr2 < maxCharStr1) && (maxChar == maxCharStr1);
	maxChar = max(maxChar, maxCharStr2);

	if (len1 == len2) {
		/* same length */
		if (len1 == 0)
			goto nothing;
		if (len1 == 1) {
			/* replace characters */
			uint32_t u1, u2;
			int64_t pos;

			u1 = ALIFUNICODE_READ(kind1, buf1, 0);
			pos = findChar(sBuf, sKind, sLen, u1, 1);
			if (pos < 0)
				goto nothing;
			u2 = ALIFUNICODE_READ(kind2, buf2, 0);
			u = alifNew_uStr(sLen, maxChar);
			if (!u)
				goto error;

			alifSubUnicode_fastCopyCharacters(u, 0, self, 0, sLen,0);
			replace_1char_inplace(u, pos, u1, u2, maxCount);
		}
		else {
			int rkind = sKind;
			wchar_t* res;
			int64_t i;

			if (kind1 < rkind) {
				/* widen substring */
				buf1 = unicode_asKind(kind1, buf1, len1, rkind);
				if (!buf1) goto error;
				release1 = 1;
			}
			i = anyLib_find(rkind, self, sBuf, sLen, str1, buf1, len1, 0);
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
					buf1 = ALIFUNICODE_CAST(str1)->UTF;
					release1 = 0;
				}
				sBuf = (const wchar_t*)unicode_asKind(sKind, sBuf, sLen, rkind);
				if (!sBuf) goto error;
				srelease = 1;
				buf1 = unicode_asKind(kind1, buf1, len1, rkind);
				if (!buf1) goto error;
				release1 = 1;
			}
			u = alifNew_uStr(sLen, maxChar);
			if (!u)
				goto error;
			res = (wchar_t*)ALIFUNICODE_CAST(u)->UTF;

			memcpy(res, sBuf, rkind * sLen);
			/* change everything in-place, starting with this one */
			memcpy(res + rkind * i,
				buf2,
				rkind * len2);
			i += len1;

			while (--maxCount > 0) {
				i = anyLib_find(rkind, self,
					sBuf + rkind * i, sLen - i,
					str1, buf1, len1, i);
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
		int64_t n, i, j, ires;
		int64_t new_size;
		int rkind = sKind;
		wchar_t* res{};

		if (kind1 < rkind) {
			/* widen substring */
			buf1 = unicode_asKind(kind1, buf1, len1, rkind);
			if (!buf1) goto error;
			release1 = 1;
		}
		n = anyLib_count(rkind, self, sBuf, sLen, str1, buf1, len1, maxCount);
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
			sBuf = (const wchar_t*)unicode_asKind(sKind, sBuf, sLen, rkind);
			if (!sBuf) goto error;
			srelease = 1;
			if (release1) {
				alifMem_dataFree((void*)buf1);
				buf1 = ALIFUNICODE_CAST(str1)->UTF;
				release1 = 0;
			}
			buf1 = unicode_asKind(kind1, buf1, len1, rkind);
			if (!buf1) goto error;
			release1 = 1;
		}
		/* new_size = Unicode_GET_LENGTH(self) + n * (Unicode_GET_LENGTH(str2) -
		   Unicode_GET_LENGTH(str1)); */
		if (len1 < len2 && len2 - len1 >(LLONG_MAX - sLen) / n) {
			//Err_SetString(Exc_OverflowError,
				//"replace string is too long");
			goto error;
		}
		new_size = sLen + n * (len2 - len1);
		if (new_size == 0) {
			//u = unicode_get_empty();
			goto done;
		}
		if (new_size > (LLONG_MAX / rkind)) {
			//Err_SetString(Exc_OverflowError,
				//"replace string is too long");
			goto error;
		}
		u = alifNew_uStr(new_size, maxChar);
		if (!u)
			goto error;
		//res = Unicode_DATA(u);
		ires = i = 0;
		if (len1 > 0) {
			while (n-- > 0) {
				/* look for next match */
				j = anyLib_find(rkind, self,
					sBuf + rkind * i, sLen - i,
					str1, buf1, len1, i);
				if (j == -1)
					break;
				else if (j > i) {
					/* copy unchanged part [i:j] */
					memcpy(res + rkind * ires,
						sBuf + rkind * i,
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
			if (i < sLen)
				/* copy tail [i:] */
				memcpy(res + rkind * ires,
					sBuf + rkind * i,
					rkind * (sLen - i));
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
					sBuf + rkind * i,
					rkind);
				ires++;
				i++;
			}
			memcpy(res + rkind * ires,
				sBuf + rkind * i,
				rkind * (sLen - i));
		}
	}

	if (mayShrink) {
		//unicode_adjust_maxchar(&u);
		if (u == nullptr)
			goto error;
	}

done:
	if (srelease)
		alifMem_dataFree((void*)sBuf);
	if (release1)
		alifMem_dataFree((void*)buf1);
	if (release2)
		alifMem_dataFree((void*)buf2);
	return u;

nothing:
	/* nothing to replace; return original string (when possible) */
	
	if (srelease)
		alifMem_dataFree((void*)sBuf);
	if (release1)
		alifMem_dataFree((void*)buf1);
	if (release2)
		alifMem_dataFree((void*)buf2);
	return self;
	return unicode_result_unchanged(self);

error:

	if (srelease)
		alifMem_dataFree((void*)sBuf);
	if (release1)
		alifMem_dataFree((void*)buf1);
	if (release2)
		alifMem_dataFree((void*)buf2);
	return nullptr;
}

static AlifObject* unicode_replaceImp(AlifObject* self, AlifObject* old, AlifObject *_new, int64_t count) {

	return replace(self,old,_new,count);

}

static AlifObject* unicode_find(AlifObject* self, AlifObject* args)
{
	/* initialize variables to prevent gcc warning */
	AlifObject* substring = nullptr;
	int64_t start_ = 0;
	int64_t end = 0;
	int64_t result;

	if (!parse_args_finds_unicode(L"find", args, &substring, &start_, &end))
		return nullptr;

	result = any_find_slice(self, substring, start_, end, 1);

	if (result == -2)
		return nullptr;

	return alifInteger_fromLongLong(result);
}

static AlifObject* getItem_unicode(AlifObject* self, int64_t index)
{
	const void* data;
	int kind;
	uint32_t ch;

	if (!(self->type_ == &_typeUnicode_)) {
		//Err_BadArgument();
		return nullptr;
	}
	if (index < 0 || index >= ((AlifUStrObject*)self)->length) {
		return nullptr;
	}
	kind = ((AlifUStrObject*)self)->kind;
	data = ((AlifUStrObject*)self)->UTF;
	ch = ALIFUNICODE_READ(kind, data, index);
	return unicode_char(ch);
}

AlifObject* alifUnicode_join(AlifObject* separator, AlifObject* seq)
{
	AlifObject* res;
	AlifObject* fseq;
	int64_t seqlen;
	AlifObject** items;

	fseq = alifSequence_fast(seq, L"can only join an iterable");
	if (fseq == nullptr) {
		return nullptr;
	}

	items = ALIFSEQUENCE_FAST_ITEMS(fseq);
	seqlen = ALIFSEQUENCE_FAST_GETSIZE(fseq);
	res = alifUnicode_joinArray(separator, items, seqlen);
	return res;
}

AlifObject* alifUnicode_joinArray(AlifObject* separator, AlifObject* const* items, int64_t seqlen)
{
	AlifObject* res = nullptr; /* the result */
	AlifObject* sep = nullptr;
	int64_t seplen;
	AlifObject* item;
	int64_t sz, i, res_offset;
	uint32_t maxchar;
	uint32_t item_maxchar;
	int use_memcpy;
	wchar_t* res_data = nullptr, * sep_data = nullptr;
	AlifObject* last_obj;
	int kind = 0;

	/* If empty sequence, return u"". */
	if (seqlen == 0) {
		//_RETURN_UNICODE_EMPTY();
	}

	/* If singleton sequence with an exact Unicode, return that. */
	last_obj = nullptr;
	if (seqlen == 1) {
		if (items[0]->type_ == &_typeUnicode_) {
			res = items[0];
			return res;
		}
		seplen = 0;
		maxchar = 0;
	}
	else {
		/* Set up sep and seplen */
		if (separator == nullptr) {
			/* fall back to a blank space separator */
			//sep = Unicode_FromOrdinal(' ');
			//if (!sep)
				//goto onError;
			seplen = 1;
			maxchar = 32;
		}
		else {
			//if (!Unicode_Check(separator)) {
				//Err_Format(Exc_TypeError,
					//"separator: expected str instance,"
					//" %.80s found",
					//TYPE(separator)->tp_name);
				//goto onError;
			//}
			sep = separator;
			seplen = ((AlifUStrObject*)separator)->length;
			maxchar = ALIFUNICODE_MAX_CHAR_VALUE(separator);
			/* inc refcount to keep this code path symmetric with the
			   above case of a blank separator */
			sep;
		}
		last_obj = sep;
	}

	/* There are at least two things to join, or else we have a subclass
	 * of str in the sequence.
	 * Do a pre-pass to figure out the total amount of space we'll
	 * need (sz), and see whether all argument are strings.
	 */
	sz = 0;
#ifdef DEBUG
	use_memcpy = 0;
#else
	use_memcpy = 1;
#endif
	for (i = 0; i < seqlen; i++) {
		size_t add_sz;
		item = items[i];
		if (!(item->type_ == &_typeUnicode_)) {
			//Err_Format(Exc_TypeError,
				//"sequence item %zd: expected str instance,"
				//" %.80s found",
				//i, TYPE(item)->tp_name);
			//goto onError;
		}
		add_sz = ((AlifUStrObject*)item)->length;
		item_maxchar = ALIFUNICODE_MAX_CHAR_VALUE(item);
		maxchar = max(maxchar, item_maxchar);
		if (i != 0) {
			add_sz += seplen;
		}
		if (add_sz > (size_t)(LLONG_MAX - sz)) {
			//Err_SetString(Exc_OverflowError,
				//"join() result is too long for a string");
			//goto onError;
		}
		sz += add_sz;
		if (use_memcpy && last_obj != nullptr) {
			if (((AlifUStrObject*)last_obj)->kind != ((AlifUStrObject*)item)->kind)
				use_memcpy = 0;
		}
		last_obj = item;
	}

	res = alifNew_uStr(sz, maxchar);
	if (res == nullptr)
		goto onError;

	/* Catenate everything. */
#ifdef DEBUG
	use_memcpy = 0;
#else
	// سيتم النظر في هذا الكود لاحقا
	//if (use_memcpy) {
		//res_data = Unicode_1BYTE_DATA(res);
		//kind = Unicode_KIND(res);
		//if (seplen != 0)
			//sep_data = Unicode_1BYTE_DATA(sep);
	//}
#endif
	if (use_memcpy) {
		for (i = 0; i < seqlen; ++i) {
			int64_t itemlen{};
			item = items[i];

			/* Copy item, and maybe the separator. */
			if (i && seplen != 0) {
				memcpy(res_data,
					sep_data,
					kind * seplen);
				res_data += kind * seplen;
			}

			itemlen = ((AlifUStrObject*)item)->kind;
			if (itemlen != 0) {
				memcpy(res_data,
					((AlifUStrObject*)item)->UTF,
					kind * itemlen);
				res_data += kind * itemlen;
			}
		}
	}
	else {
		for (i = 0, res_offset = 0; i < seqlen; ++i) {
			int64_t itemlen;
			item = items[i];

			/* Copy item, and maybe the separator. */
			if (i && seplen != 0) {
				alifSubUnicode_fastCopyCharacters(res, res_offset, sep, 0, seplen, 0);
				res_offset += seplen;
			}

			itemlen = ((AlifUStrObject*)item)->length;
			if (itemlen != 0) {
				alifSubUnicode_fastCopyCharacters(res, res_offset, item, 0, itemlen, 0);
				res_offset += itemlen;
			}
		}
	}

	return res;

onError:
	return nullptr;
}

static AlifObject* unicode_join(AlifObject* self, AlifObject* iterable) {
	
	return alifUnicode_join(self, iterable);
}

void alifSubUnicode_fastFill(AlifObject* _unicode,int64_t _start, int64_t _length,
	uint32_t _fillChar)
{
	const int kind = ALIFUNICODE_CAST(_unicode)->kind;
	void* data = ALIFUNICODE_CAST(_unicode)->UTF;
	unicode_fill(kind, data, _fillChar, _start, _length);
}

int64_t alifUnicode_fill(AlifObject* _unicode, int64_t _start, int64_t _length,
	uint32_t _fillChar)
{
	int64_t maxLen;

	if (!(_unicode->type_ == &_typeUnicode_)) {
		return -1;
	}
	if (unicode_check_modifiable(_unicode))
		return -1;

	if (_start < 0) {
		return -1;
	}
	if (_fillChar > ALIFUNICODE_MAX_CHAR_VALUE(_unicode)) {

		return -1;
	}

	maxLen = ALIFUNICODE_GET_LENGTH(_unicode) - _start;
	_length = min(maxLen, _length);
	if (_length <= 0)
		return 0;

	alifSubUnicode_fastFill(_unicode, _start, _length, _fillChar);
	return _length;
}

static AlifObject* unicode_splitImpl(AlifObject* , AlifObject* , int64_t );

static AlifObject* unicode_split(AlifObject* self, AlifObject* const* args, int64_t nargs, AlifObject* kwnames)
{
	AlifObject* returnValue = nullptr;
#define NUM_KEYWORDS 2
	static class KwTuple {
	public:
		AlifVarObject base{};
		AlifObject* item[NUM_KEYWORDS];
	} kwTuple = {
		//ALIFVAROBJECT_HEAD_INIT(&typeTuple, NUM_KEYWORDS)
		//nullptr
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&kwTuple.base.object)

//#  define KWTUPLE nullptr

	static const wchar_t* const _keywords[] = { L"sep", L"maxsplit", nullptr };
	static AlifArgParser _parser = {
		0,
		L"",
		_keywords,
		L"split",
		L"",
		0,
		0,
		0,
		//KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[2];
	int64_t noptargs = nargs + (kwnames ? ((AlifVarObject*)kwnames)->size_ : 0) - 0;
	AlifObject* sep = ALIF_NONE;
	int64_t maxsplit = -1;

	args = ALIFARG_UNPACKKEYWORDS(args, nargs, nullptr, kwnames, &_parser, 0, 2, 0, argsbuf);
	if (!args) {
		goto exit;
	}
	if (!noptargs) {
		goto skip_optional_pos;
	}
	if (args[0]) {
		sep = args[0];
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	{
		int64_t ival = -1;
		AlifObject* iobj = args[1];
		if (iobj != nullptr) {
			ival = alifInteger_asLongLong(iobj);
			ALIF_DECREF(iobj);
		}

		maxsplit = ival;
	}
skip_optional_pos:
	returnValue = unicode_splitImpl(self, sep, maxsplit);

exit:
	return returnValue;
}

static AlifObject* split(AlifObject* self,
	AlifObject* substring,
	int64_t maxcount)
{
	int kind1, kind2;
	const void* buf1, * buf2;
	int64_t len1, len2;
	AlifObject* out;
	len1 = ((AlifUStrObject*)self)->length;
	kind1 = ((AlifUStrObject*)self)->kind;

	//if (substring == nullptr) {
	//	if (maxcount < 0) {
	//		maxcount = (len1 - 1) / 2 + 1;
	//	}
	//	switch (kind1) {
	//	case UNICODE_2BYTE:
	//		return ucs2lib_split_whitespace(
	//			self, Unicode_2BYTE_DATA(self),
	//			len1, maxcount
	//		);
	//	case UNICODE_4BYTE:
	//		return ucs4lib_split_whitespace(
	//			self, Unicode_4BYTE_DATA(self),
	//			len1, maxcount
	//		);
	//	default:
	//		//UNREACHABLE();
	//	}
	//}

	kind2 = ((AlifUStrObject*)substring)->kind;
	len2 = ((AlifUStrObject*)substring)->length;
	if (maxcount < 0) {
		// if len2 == 0, it will raise ValueError.
		maxcount = len2 == 0 ? 0 : (len1 / len2) + 1;
		// handle expected overflow case: (_SSIZE_T_MAX / 1) + 1
		maxcount = maxcount < 0 ? len1 : maxcount;
	}
	if (kind1 < kind2 || len1 < len2) {
		out = alifNew_list(1);
		if (out == nullptr)
			return nullptr;
		((AlifListObject*)out)->items[0] = self;
		return out;
	}
	buf1 = ((AlifUStrObject*)self)->UTF;
	buf2 = ((AlifUStrObject*)substring)->UTF;
	if (kind2 != kind1) {
		buf2 = unicode_asKind(kind2, buf2, len2, kind1);
		if (!buf2)
			return nullptr;
	}

	switch (kind1) {
	case UNICODE_2BYTE:
		out = split(
			self, (uint16_t*)buf1, len1, (uint16_t*)buf2, len2, maxcount);
		break;
	case UNICODE_4BYTE:
		out = split(
			self, (uint32_t*)buf1, len1, (uint32_t*)buf2, len2, maxcount);
		break;
	default:
		out = nullptr;
	}
	if (kind2 != kind1)
		alifMem_dataFree((void*)buf2);
	return out;
}

static AlifObject* unicode_splitImpl(AlifObject* self, AlifObject* sep, int64_t maxsplit)
/*[clinic end generated code: output=3a65b1db356948dc input=07b9040d98c5fe8d]*/
{
	if (sep == ALIF_NONE)
		return split(self, nullptr, maxsplit);
	if (sep->type_ == &_typeUnicode_)
		return split(self, sep, maxsplit);
	return nullptr;
}

static inline void alifSubUnicodeWriter_update(AlifSubUnicodeWriter* _writer)
{
	_writer->maxChar = ALIFUNICODE_MAX_CHAR_VALUE(_writer->buffer_);
	_writer->data_ = ((AlifUStrObject*)_writer->buffer_)->UTF;

	if (!_writer->readonly_) {
		_writer->kind_ = ALIFUNICODE_KIND(_writer->buffer_);
		_writer->size_ = ALIFUNICODE_GET_LENGTH(_writer->buffer_);
	}
	else {
		_writer->kind_ = 0;
		_writer->size_ = 0;
	}
}

int alifSubUnicodeWriter_prepareInternal(AlifSubUnicodeWriter* _writer,
	int64_t _length, uint32_t _maxChar)
{
	int64_t newLen;
	AlifObject* newBuffer{};

	if (_length > LLONG_MAX - _writer->pos_) {
		return -1;
	}
	newLen = _writer->pos_ + _length;

	_maxChar = max(_maxChar, _writer->minChar);

	if (_writer->buffer_ == NULL) {
		if (_writer->overAllocate
			&& newLen <= (LLONG_MAX - newLen / 2)) {
			/* overallocate to limit the number of realloc() */
			newLen += newLen / 2;
		}
		if (newLen < _writer->minLength)
			newLen = _writer->minLength;

		_writer->buffer_ = alifNew_uStr(newLen, _maxChar);
		if (_writer->buffer_ == NULL)
			return -1;
	}
	else if (newLen > _writer->size_) {
		if (_writer->overAllocate
			&& newLen <= (LLONG_MAX - newLen / 2)) {
			/* overallocate to limit the number of realloc() */
			newLen += newLen / 2;
		}
		if (newLen < _writer->minLength)
			newLen = _writer->minLength;

		if (_maxChar > _writer->maxChar || _writer->readonly_) {
			/* resize + widen */
			_maxChar = max(_maxChar, _writer->maxChar);
			newBuffer = alifNew_uStr(newLen, _maxChar);
			if (newBuffer == NULL)
				return -1;
			//alifSubUnicode_fastCopyCharacters(newBuffer, 0,
				//_writer->buffer_, 0, _writer->pos_);
			ALIF_DECREF(_writer->buffer_);
			_writer->readonly_ = 0;
		}
		else {
			//newBuffer = resize_compact(_writer->buffer_, newLen);
			if (newBuffer == NULL)
				return -1;
		}
		_writer->buffer_ = newBuffer;
	}
	else if (_maxChar > _writer->maxChar) {
		newBuffer = alifNew_uStr(_writer->size_, _maxChar);
		if (newBuffer == NULL)
			return -1;
		//alifSubUnicode_fastCopyCharacters(newBuffer, 0,
			//_writer->buffer_, 0, _writer->pos_);
		ALIF_SETREF(_writer->buffer_, newBuffer);
	}
	//alifSubUnicodeWriter_update(_writer);
	return 0;


}

int alifSubUnicodeWriter_writeStr(AlifSubUnicodeWriter* _writer, AlifObject* _str)
{
	AlifUCS4 maxChar;
	int64_t len_;

	len_ = ALIFUNICODE_GET_LENGTH(_str);
	if (len_ == 0)
		return 0;
	maxChar = ALIFUNICODE_MAX_CHAR_VALUE(_str);
	if (maxChar > _writer->maxChar || len_ > _writer->size_ - _writer->pos_) {
		if (_writer->buffer_ == NULL && !_writer->overAllocate) {
			_writer->readonly_ = 1;
			_writer->buffer_ = ALIF_NEWREF(_str);
			alifSubUnicodeWriter_update(_writer);
			_writer->pos_ += len_;
			return 0;
		}
		if (alifSubUnicodeWriter_prepareInternal(_writer, len_, maxChar) == -1)
			return -1;
	}
	alifSubUnicode_fastCopyCharacters(_writer->buffer_, _writer->pos_,
		_str, 0, len_, 0);
	_writer->pos_ += len_;
	return 0;
}

static inline int alifSubUnicodeWriter_writeWcharInline(AlifSubUnicodeWriter* _writer, uint32_t _ch)
{
	if (ALIFSUBUNICODEWRITER_PREPARE(_writer, 1, _ch) < 0)
		return -1;
	ALIFUNICODE_WRITE(_writer->kind_, _writer->data_, _writer->pos_, _ch);
	_writer->pos_++;
	return 0;
}

int alifSubUnicodeWriter_writeChar(AlifSubUnicodeWriter* _writer, uint32_t _ch)
{
	return alifSubUnicodeWriter_writeWcharInline(_writer, _ch);
}

static AlifMethodDef _unicodeMethods_[] = {
	{L"replace", ALIFCFunction_CAST(unicode_replace), METHOD_FASTCALL | METHOD_KEYWORDS},
	{L"split", ALIFCFunction_CAST(unicode_split), METHOD_FASTCALL | METHOD_KEYWORDS},
	{L"join", (AlifCFunction)unicode_join, METHOD_O,},
	{L"count", (AlifCFunction)unicode_count, METHOD_VARARGS},
	{L"find", (AlifCFunction)unicode_find, METHOD_VARARGS},
	{nullptr, nullptr}
};

static AlifSequenceMethods _unicodeAsSequence_ = {
	(LenFunc)length_unicode,
	combine_unicode,
	(SSizeArgFunc)repeat_unicode,
	(SSizeArgFunc)getItem_unicode,
	0,
	0,
	0,
	(ObjObjProc)contain_unicode,
};

static AlifObject* unicode_subscript(AlifObject* _self, AlifObject* _item)
{
	if ((_item->type_->asNumber != nullptr && _item->type_->asNumber->index_)) {
		int64_t i_ = alifInteger_asLongLong(_item);
		if (i_ < 0)
			i_ += ((AlifUStrObject*)_self)->length;
		return getItem_unicode(_self, i_);
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
		sliceLength = slice_adjustIndices(((AlifUStrObject*)_self)->length,
			&start_, &stop_, step_);

		if (sliceLength <= 0) {
			// return empty string;
		}
		else if (start_ == 0 && step_ == 1 &&
			sliceLength == ((AlifUStrObject*)_self)->length) {
			return unicode_result_unchanged(_self);
		}
		else if (step_ == 1) {
			//return alifUnicode_substring(self,
				//start_, start_ + sliceLength);
		}
		srcKind = ((AlifUStrObject*)_self)->kind;
		srcData = ((AlifUStrObject*)_self)->UTF;
		if (!(isOnlyAscii((AlifUStrObject*)_self))) {
			kindLimit = kind_maxChar_limit(srcKind);
			maxChar = 0;
			for (cur_ = start_, i_ = 0; i_ < sliceLength; cur_ += step_, i_++) {
				ch_ = ALIFUNICODE_READ(srcKind, srcData, cur_);
				if (ch_ > maxChar) {
					maxChar = ch_;
					if (maxChar >= kindLimit)
						break;
				}
			}
		}
		else
			maxChar = 127;
		result_ = alifNew_uStr(sliceLength, maxChar);
		if (result_ == nullptr)
			return nullptr;
		destKind = ((AlifUStrObject*)result_)->kind;
		destData = ((AlifUStrObject*)result_)->UTF;

		for (cur_ = start_, i_ = 0; i_ < sliceLength; cur_ += step_, i_++) {
			uint32_t ch_ = ALIFUNICODE_READ(srcKind, srcData, cur_);
			ALIFUNICODE_WRITE(destKind, destData, i_, ch_);
		}
		return result_;
	}
	else {
		return nullptr;
	}
}

static AlifMappingMethods _unicodeAsMapping_ = {
	(LenFunc)length_unicode,       
	(BinaryFunc)unicode_subscript,  
	(ObjObjArgProc)0, 
};

AlifTypeObject _typeUnicode_ = {
	0,
	0,
	0,
	L"string",                        
	sizeof(AlifUStrObject),      
	0,                            
	unicode_dealloc,
	0,                            
	0,                         
	0,                         
	0,           
	0,          
	& _unicodeAsSequence_,
	& _unicodeAsMapping_,
	(HashFunc)hash_unicode,
	0,                     
	0,
	0,      
	0,                            
	0,                             
	0,
	0,               
	0, 
	0,
	(RichCmpFunc)unicode_compare,
	0,                           
	0,        
	0,               
	_unicodeMethods_,
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

AlifObject* alifUnicode_internFromString(const wchar_t* _cp)
{
	AlifObject* s_ = alifUnicode_decodeStringToUTF8(_cp);
	if (s_ == nullptr)
		return nullptr;
	//alifUnicode_internInPlace(&s_);
	return s_;
}
