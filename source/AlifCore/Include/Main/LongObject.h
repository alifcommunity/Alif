#pragma once





 // 5
#define ALIFLONG_CAST(_op) \
    (ALIF_CAST(AlifLongObject*, _op))


 // 12
#define ALIFLONG_CHECK(_op) \
        ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(_op), ALIF_TPFLAGS_LONG_SUBCLASS)
#define ALIFLONG_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifLongType_)


ALIFAPI_FUNC(AlifObject*)alifLong_fromLong(long); // 16
ALIFAPI_FUNC(AlifObject*) alifLong_fromUnsignedLong(unsigned long); // 17

ALIFAPI_FUNC(AlifObject*) alifLong_fromSizeT(AlifSizeT); // 19
ALIFAPI_FUNC(AlifObject*) alifLong_fromDouble(double); // 20

ALIFAPI_FUNC(long) alifLong_asLong(AlifObject* ); // 22
ALIFAPI_FUNC(long) alifLong_asLongAndOverflow(AlifObject*, AlifIntT*); // 23

ALIFAPI_FUNC(AlifSizeT) alifLong_asSizeT(AlifObject*); // 24

ALIFAPI_FUNC(unsigned long) alifLong_asUnsignedLong(AlifObject*); // 26
ALIFAPI_FUNC(unsigned long) alifLong_asUnsignedLongMask(AlifObject*); // 27

ALIFAPI_FUNC(AlifIntT) alifLong_asInt(AlifObject*); // 30


ALIFAPI_FUNC(AlifObject*) alifLong_fromInt64(int64_t); // 36

ALIFAPI_FUNC(AlifIntT) alifLong_asInt64(AlifObject*, int64_t*); // 41

ALIFAPI_FUNC(double) alifLong_asDouble(AlifObject*); // 86
ALIFAPI_FUNC(AlifObject*) alifLong_fromVoidPtr(void*); // 87
ALIFAPI_FUNC(AlifObject*) alifLong_fromUnsignedLongLong(unsigned long long); // 91
ALIFAPI_FUNC(long long) alifLong_asLongLong(AlifObject*); // 92
ALIFAPI_FUNC(unsigned long long) alifLong_asUnsignedLongLongMask(AlifObject*); // 94
ALIFAPI_FUNC(AlifObject*) alifLong_fromString(const char*, char**, AlifIntT); // 97

ALIFAPI_FUNC(AlifObject*) alifLong_fromLongLong(long long); // 102


ALIFAPI_FUNC(unsigned long long) alifLong_asUnsignedLongLong(AlifObject*); // 105


ALIFAPI_FUNC(unsigned long) alifOS_strToULong(const char*, char**, AlifIntT); // 102
ALIFAPI_FUNC(long) alifOS_strToLong(const char*, char**, AlifIntT); // 103





/* -------------------------------------------------------------------------------------------------------------------------------------- */




ALIFAPI_FUNC(AlifObject*) alifLong_fromUStrObject(AlifObject*, AlifIntT); // 5

 // 7
#define ALIF_ASNATIVEBYTES_DEFAULTS -1
#define ALIF_ASNATIVEBYTES_BIG_ENDIAN 0
#define ALIF_ASNATIVEBYTES_LITTLE_ENDIAN 1
#define ALIF_ASNATIVEBYTES_NATIVE_ENDIAN 3
#define ALIF_ASNATIVEBYTES_UNSIGNED_BUFFER 4
#define ALIF_ASNATIVEBYTES_REJECT_NEGATIVE 8
#define ALIF_ASNATIVEBYTES_ALLOW_INDEX 16



ALIFAPI_FUNC(AlifObject*) alifLong_fromNativeBytes(const void*, AlifUSizeT, AlifIntT); // 56


ALIFAPI_FUNC(AlifIntT) _alifLong_sign(AlifObject*); // 89


ALIFAPI_FUNC(int64_t) _alifLong_numBits(AlifObject*); // 97



ALIFAPI_FUNC(AlifObject*) _alifLong_fromByteArray(const unsigned char*, AlifUSizeT, AlifIntT, AlifIntT); // 94
ALIFAPI_FUNC(AlifIntT) _alifLong_asByteArray(AlifLongObject*, unsigned char*,
	AlifUSizeT, AlifIntT, AlifIntT, AlifIntT); // 117

ALIFAPI_FUNC(AlifObject*) alifLong_gcd(AlifObject*, AlifObject*); // 122
