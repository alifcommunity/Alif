#pragma once










// 94
using AlifUCS4 = uint32_t;
using AlifUCS2 = uint16_t;
using AlifUCS1 = uint8_t;



AlifObject* alifUStr_fromString(const char*); // 129










AlifObject* alifUStr_decodeUTF8Stateful(const char*, AlifSizeT, const char*, AlifSizeT*); // 435







/* ---------------------------------------------------------------------------------------------------------- */






/* ----------------------------------- Internal Unicode Operations ----------------------------------- */

// Static inline functions to work with surrogates
static inline int alifUnicode_isSurrogate(AlifUCS4 _ch) { // 16
	return (0xD800 <= _ch && _ch <= 0xDFFF);
}

// High surrogate = top 10 bits added to 0xD800.
// The character must be in the range [U+10000; U+10ffff].
static inline AlifUCS4 alifUnicode_highSurrogate(AlifUCS4 ch) { // 35
	return (0xD800 - (0x10000 >> 10) + (ch >> 10));
}

// Low surrogate = bottom 10 bits added to 0xDC00.
// The character must be in the range [U+10000; U+10ffff].
static inline AlifUCS4 alifUnicode_lowSurrogate(AlifUCS4 ch) { // 42
	return (0xDC00 + (ch & 0x3FF));
}








/* ---------------------------------------------------- UStr Type ---------------------------------------------------- */

class AlifASCIIObject { // 54
public:
	ALIFOBJECT_HEAD{};
	AlifSizeT length{};
	AlifHashT hash{};
	class {
	public:
		AlifUIntT interned : 2;
		AlifUIntT kind : 3;
		AlifUIntT compact : 1;
		AlifUIntT ascii : 1;
		AlifUIntT statically_allocated : 1;
		AlifUIntT : 24;
	} state;
};

class AlifCompactUStrObject { // 156
public:
	AlifASCIIObject base;
	AlifSizeT utf8Length;
	char* utf8;
};

class AlifUnicodeObject { // 164
public:
	AlifCompactUStrObject base;
	union {
		void* any;
		AlifUCS1* latin1;
		AlifUCS2* ucs2;
		AlifUCS4* ucs4;
	} data;
};


#define ALIFASCIIOBJECT_CAST(op) ALIF_CAST(AlifASCIIObject*, (op)) // 175










enum AlifUStrKind_ { // 230
	AlifUStr_1Byte_Kind = 1,
	AlifUStr_2Byte_Kind = 2,
	AlifUStr_4Byte_Kind = 4
};
