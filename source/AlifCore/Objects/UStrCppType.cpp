#include "alif.h"






#define ALPHA_MASK 0x01 // 13
#define DECIMAL_MASK 0x02 // 14
#define DIGIT_MASK 0x04
#define XID_START_MASK 0x100 // 19
#define XID_CONTINUE_MASK 0x200 // 20
#define PRINTABLE_MASK 0x400 // 21
#define NUMERIC_MASK 0x800
#define CASE_IGNORABLE_MASK 0x1000 // 23
#define CASED_MASK 0x2000 // 24
#define EXTENDED_CASE_MASK 0x4000 // 25

class AlifUStrTypeRecord { // 27
public:
	const AlifIntT upper{};
	const AlifIntT lower{};
	const AlifIntT title{};
	const unsigned char decimal{};
	const unsigned char digit{};
	const unsigned short flags{};
};

#include "UStrTypeDB.h" // 41


static const AlifUStrTypeRecord* get_typeRecord(AlifUCS4 _code) { // 43
	AlifIntT index{};

	if (_code >= 0x110000)
		index = 0;
	else
	{
		index = _index1_[(_code >> SHIFT)];
		index = _index2_[(index << SHIFT) + (_code & ((1 << SHIFT) - 1))];
	}

	return &_alifUStrTypeRecords_[index];
}



AlifIntT _alifUStr_isXIDStart(AlifUCS4 ch) { // 84
	const AlifUStrTypeRecord* ctype = get_typeRecord(ch);

	return (ctype->flags & XID_START_MASK) != 0;
}


AlifIntT _alifUStr_isXIDContinue(AlifUCS4 ch) { // 94
	const AlifUStrTypeRecord* ctype = get_typeRecord(ch);

	return (ctype->flags & XID_CONTINUE_MASK) != 0;
}



AlifIntT _alifUStr_toDecimalDigit(AlifUCS4 ch) { // 104
	const AlifUStrTypeRecord* ctype = get_typeRecord(ch);

	return (ctype->flags & DECIMAL_MASK) ? ctype->decimal : -1;
}


AlifIntT _alifUStr_isDecimalDigit(AlifUCS4 _ch) { // 111
	if (_alifUStr_toDecimalDigit(_ch) < 0)
		return 0;
	return 1;
}


AlifIntT _alifUStr_toDigit(AlifUCS4 _ch) { // 121
	const AlifUStrTypeRecord* ctype = get_typeRecord(_ch);

	return (ctype->flags & DIGIT_MASK) ? ctype->digit : -1;
}


AlifIntT _alifUStr_isDigit(AlifUCS4 _ch) { // 128
	if (_alifUStr_toDigit(_ch) < 0)
		return 0;
	return 1;
}


AlifIntT _alifUStr_isNumeric(AlifUCS4 _ch) { // 138
	const AlifUStrTypeRecord* ctype = get_typeRecord(_ch);

	return (ctype->flags & NUMERIC_MASK) != 0;
}


AlifIntT _alifUStr_isPrintable(AlifUCS4 _ch) { // 158
	const AlifUStrTypeRecord* ctype = get_typeRecord(_ch);

	return (ctype->flags & PRINTABLE_MASK) != 0;
}





AlifIntT _alifUStr_toLowerFull(AlifUCS4 ch, AlifUCS4* res) { // 209
	const AlifUStrTypeRecord* ctype = get_typeRecord(ch);

	if (ctype->flags & EXTENDED_CASE_MASK) {
		AlifIntT index = ctype->lower & 0xFFFF;
		AlifIntT n = ctype->lower >> 24;
		AlifIntT i{};
		for (i = 0; i < n; i++)
			res[i] = _alifUStrExtendedCase_[index + i];
		return n;
	}
	res[0] = ch + ctype->lower;
	return 1;
}



AlifIntT _alifUStr_toUpperFull(AlifUCS4 ch, AlifUCS4* res) { // 241
	const AlifUStrTypeRecord* ctype = get_typeRecord(ch);

	if (ctype->flags & EXTENDED_CASE_MASK) {
		int index = ctype->upper & 0xFFFF;
		int n = ctype->upper >> 24;
		int i;
		for (i = 0; i < n; i++)
			res[i] = _alifUStrExtendedCase_[index + i];
		return n;
	}
	res[0] = ch + ctype->upper;
	return 1;
}



AlifIntT _alifUStr_isCased(AlifUCS4 ch) { // 272
	const AlifUStrTypeRecord* ctype = get_typeRecord(ch);

	return (ctype->flags & CASED_MASK) != 0;
}

AlifIntT _alifUStr_isCaseIgnorable(AlifUCS4 ch) { // 279
	const AlifUStrTypeRecord* ctype = get_typeRecord(ch);

	return (ctype->flags & CASE_IGNORABLE_MASK) != 0;
}



AlifIntT _alifUStr_isAlpha(AlifUCS4 _ch) { // 289
	const AlifUStrTypeRecord* ctype = get_typeRecord(_ch);

	return (ctype->flags & ALPHA_MASK) != 0;
}
