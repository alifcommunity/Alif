#include "alif.h"

#include "AlifCore_Memory.h"


#ifdef _WINDOWS
#include "windows.h"
#endif // _WINDOWS

#define ALIFSIZE_ROUND_DOWN(n, a) ((size_t)(n) & ~(size_t)((a) - 1))


// من هنا يتم انشاء كائن نصي
AlifObj* alifNew_uStr(AlifSizeT _size, uint8_t _maxChar) { /// M
	AlifObj* obj{};
	AlifUStrObject* uStr{};
	void* data{};
	uint8_t kind{};
	uint8_t charSize{};
	AlifSizeT structSize = sizeof(AlifUStrObject);


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

	obj = (AlifObj*)alifMem_objAlloc(structSize + (_size + 1) * charSize);
	obj->type = &typeUnicode;

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

AlifUStrObject* alifNew_unicode(AlifSizeT size, uint8_t maxChar) {

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

	object = (AlifUStrObject*)alifMem_objAlloc(structSize + (size + 1) * kind);
	((AlifObj*)object)->type = &typeUnicode;

	object->hash = 0;
	object->length = size;
	object->kind = kind;
	object->UTF = &object->UTF + 1;

	return object;
}

// هنا يتم ايجاد اعلى ترميز في النص لتحديد نوع ترميز النص
uint8_t find_MaxChar(const wchar_t* str) {

	while (*str != L'\0') {
		if (*str >= 0xD800 && *str <= 0xDBFF) {
			str++;
			return 4;
		}
		str++;
	}
	return 2;
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


static void uStrConvert_wCharToUCS4(const wchar_t* _begin, const wchar_t* _end, AlifObj* _uStr) { /// M
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
#define ALIFUSTR_CONVERT_BYTES(_fromType, _toType, _begin, _end, _to) \
    do {											    	    \
        _toType *_to1 = (_toType *)(_to);						\
        const _fromType *_iter1 = (const _fromType *)(_begin);	\
        const _fromType *_end1 = (const _fromType *)(_end);		\
        AlifSizeT _n = (_end1) - (_iter1);						\
        const _fromType *_unrolledEnd1 =						\
            _iter1 + ALIFSIZE_ROUND_DOWN(_n, 4);				\
        while (_iter1 < (_unrolledEnd1)) {						\
            _to1[0] = (_toType) _iter1[0];						\
            _to1[1] = (_toType) _iter1[1];						\
            _to1[2] = (_toType) _iter1[2];						\
            _to1[3] = (_toType) _iter1[3];						\
            _iter1 += 4; _to1 += 4;								\
        }														\
        while (_iter1 < (_end1))								\
            *_to1++ = (_toType) *_iter1++;						\
    } while (0)

AlifObj* alifUStr_objFromWChar(wchar_t* _buffer) { /// M
	AlifObj* strObj{};
	AlifSizeT size = wcslen(_buffer);
	uint8_t maxChar = find_MaxChar(_buffer);

	strObj = alifNew_uStr(size, maxChar);

	
	if (((AlifUStrObject*)strObj)->kind == UNICODE_2BYTE) {
#if SIZEOF_WCHART == 2
		memcpy(((AlifUStrObject*)strObj)->UTF, _buffer, size * 2);
#else
		ALIFUSTR_CONVERT_BYTES(wchar_t, uint16_t,
			_buffer, _buffer + size, ((AlifUStrObject*)strObj)->UTF);
#endif
	}
	else if (((AlifUStrObject*)strObj)->kind == UNICODE_4BYTE) {
#if SIZEOF_WCHART == 2
		uStrConvert_wCharToUCS4(_buffer, _buffer + size, strObj);
#else
		memcpy(((AlifUStrObject*)strObj)->UTF, _buffer, size * 4);
#endif
	}

	return strObj;
}



void copy_string(AlifObj* object, const wchar_t* str, SSIZE_T length, SSIZE_T maxChar) {

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

void combine_string(AlifObj* result, SSIZE_T start, AlifObj* from, SSIZE_T fromStart, SSIZE_T length, uint8_t maxChar) {

	SSIZE_T fromKind = (ALIFUNICODE_CAST(from))->kind;
	SSIZE_T toKind = (ALIFUNICODE_CAST(result))->kind;
	if (fromKind == toKind) {

		void* fromData = ((char*)(ALIFUNICODE_CAST(from))->UTF + (fromStart * fromKind));
		void* toData = ((char*)(ALIFUNICODE_CAST(result))->UTF + (start * toKind));

		memcpy(toData, fromData, length * maxChar);

	}
	else if (fromKind == 2 && toKind == 4) {

		uint16_t* fromData = ((uint16_t*)(ALIFUNICODE_CAST(from))->UTF);
		uint32_t* toData = ((uint32_t*)(ALIFUNICODE_CAST(result))->UTF);

		ALIFUNICODE_CONVERT_BYTES(uint16_t, uint32_t,
			fromData + fromStart,
			fromData + fromStart + length, 
			toData + start);

	}
	else if (fromKind == 4 && toKind == 2) {
	
		uint32_t* fromData = ((uint32_t*)(ALIFUNICODE_CAST(from))->UTF);
		uint16_t* toData = ((uint16_t*)(ALIFUNICODE_CAST(result))->UTF);

		ALIFUNICODE_CONVERT_BYTES(uint32_t, uint16_t,
			fromData + fromStart,
			fromData + fromStart + length,
			toData + start);
	}
	else {
		std::wcout << L"ترميز الحروف غير صحيح\n" << std::endl;
		exit(-1);
	}

}

AlifUStrObject* unicode_decode_utf(const wchar_t* str, SSIZE_T length) {

	if (length == 0) {
		// return empty object string
	}

	SSIZE_T maxChar = find_MaxChar(str);

	AlifUStrObject* object = alifNew_unicode(length, maxChar);

	copy_string((AlifObj*)object, str, length, maxChar);

	return object;

}
// من هنا يتم عمل كائن جديد ويتم فك ترميز النص وتخزينه في الكائن
AlifUStrObject* alifUnicode_decodeStringToUTF(const wchar_t* str) {

	return unicode_decode_utf(str, count_characters(str));
}

SSIZE_T length_unicode(AlifObj* unicode) {
	return ALIFUNICODE_CAST(unicode)->length;
}

AlifObj* combine_unicode(AlifObj* left, AlifObj* right) {

	AlifObj* result;
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

	result = (AlifObj*)alifNew_unicode(newLen, maxChar);
	if (result == nullptr) {
		return nullptr;
	}

	combine_string(result, 0, left, 0, leftLen, maxChar);
	combine_string(result, leftLen, right, 0, rightLen, maxChar);

	return result;

}

AlifObj* repeat_unicode(AlifObj* unicode, SSIZE_T repeat) {

	if (repeat == 0) {
		// return empty string
	}

	if (repeat == 1) {
		return unicode;
	}

	SSIZE_T lenUnicode = (ALIFUNICODE_CAST(unicode))->length;
	SSIZE_T len = lenUnicode * repeat;
	uint8_t kind = (ALIFUNICODE_CAST(unicode))->kind;

	AlifObj* object = (AlifObj*)alifNew_unicode(len, kind);

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

bool contain_unicode(AlifObj* unicode, AlifObj* str) {

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
size_t hash_unicode(AlifObj* unicode) {

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

int unicode_compare_eq(AlifObj* str1, AlifObj* str2)
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

AlifObj* unicode_compare(AlifObj* left, AlifObj* right, int op) {

	if (left->type != &typeUnicode || 
		right->type != &typeUnicode) {
		std::wcout << L"عمليه مقارنة النص غير صحيحة\n" << std::endl;
		exit(-1);
	}

	if (left == right && op == ALIF_EQ) {
		//return ALIF_TRUE;
	}
	else if (op == ALIF_EQ || op == ALIF_NE) {
		int result = unicode_compare_eq(left, right);
		//return result ? ALIF_TRUE : ALIF_FALSE;
	}

}

void unicode_dealloc(AlifObj* unicode) {
	alifMem_objFree(unicode);
}

AlifSequenceMethods sequenceUnicode = {
	(LenFunc)length_unicode,
	combine_unicode,
	(SSizeArgFunc)repeat_unicode,
	0,
	0,
	0,
	0,
	(ObjObjProc)contain_unicode,
};

AlifInitObject typeUnicode = {

	L"string",                        
	sizeof(AlifUStrObject),      
	0,                            
	unicode_dealloc,
	0,                            
	0,                         
	0,                         
	0,           
	0,          
	&sequenceUnicode,
	0,          
	(HashFunc)hash_unicode,
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
	0,               
};
