#pragma once










// 94
using AlifUCS4 = uint32_t;
using AlifUCS2 = uint16_t;
using AlifUCS1 = uint8_t;






/* --------------------------------------------------------------------------------------------------- */






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
