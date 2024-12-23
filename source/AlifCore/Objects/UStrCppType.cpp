#include "alif.h"




#define PRINTABLE_MASK 0x400 // 21

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





AlifIntT _alifUStr_isPrintable(AlifUCS4 _ch) { // 158
	const AlifUStrTypeRecord* ctype = get_typeRecord(_ch);

	return (ctype->flags & PRINTABLE_MASK) != 0;
}
