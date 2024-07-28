#pragma once


#define ALIFINTEGER_CHECK(op) \
        alifType_hasFeature(ALIF_TYPE(op), ALIFTPFLAGS_LONG_SUBCLASS)
#define ALIFINTEGER_CHECKEXACT(op) ALIFIS_TYPE((op), &_alifIntegerType_)


class AlifIntegerObject
{
public:

	ALIFOBJECT_HEAD

	size_t digits_{};

	bool sign_{}; // true == Negative Sign

};

extern AlifInitObject _alifIntegerType_;


AlifObject* alifInteger_fromSizeT(size_t, bool );
AlifObject* alifInteger_fromDouble(long double );
AlifObject* alifInteger_fromLongLong(int64_t );

long double alifInteger_asDouble(AlifObject*);
long alifInteger_asLong(AlifObject* );
size_t alifInteger_asSizeT(AlifObject* );
bool alifInteger_asSign(AlifObject* );
int64_t alifInteger_asLongLong(AlifObject* );
AlifObject* alifInteger_fromString(const wchar_t* );

int64_t alifOS_strToLong(const wchar_t* );

AlifObject* alifinteger_fromUnicodeObject(AlifObject*, int);

