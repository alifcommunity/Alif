#pragma once





 // 5
#define ALIFLONG_CAST(_op) \
    (ALIF_CAST(AlifLongObject*, _op))


 // 12
#define ALIFLONG_CHECK(_op) \
        ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(_op), ALIF_TPFLAGS_LONG_SUBCLASS)
#define ALIFLONG_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifLongType_)


AlifObject* alifLong_fromLong(long); // 16
AlifObject* alifLong_fromUnsignedLong(unsigned long); // 17

AlifObject* alifLong_fromSizeT(AlifSizeT); // 19
AlifObject* alifLong_fromDouble(double); // 20

long alifLong_asLong(AlifObject* ); // 22
long alifLong_asLongAndOverflow(AlifObject*, AlifIntT*); // 23

AlifSizeT alifLong_asSizeT(AlifObject*); // 24

unsigned long alifLong_asUnsignedLongMask(AlifObject*); // 27

AlifIntT alifLong_asInt(AlifObject*); // 30


AlifIntT alifLong_asInt64(AlifObject*, int64_t*); // 41

double alifLong_asDouble(AlifObject*); // 86
AlifObject* alifLong_fromVoidPtr(void*); // 87
AlifObject* alifLong_fromUnsignedLongLong(unsigned long long); // 91
long long alifLong_asLongLong(AlifObject*); // 92
unsigned long long alifLong_asUnsignedLongLongMask(AlifObject*); // 94
AlifObject* alifLong_fromString(const char*, char**, AlifIntT); // 97

unsigned long alifOS_strToULong(const char*, char**, AlifIntT); // 102
long alifOS_strToLong(const char*, char**, AlifIntT); // 103





/* -------------------------------------------------------------------------------------------------------------------------------------- */




AlifObject* alifLong_fromUStrObject(AlifObject*, AlifIntT); // 5

 // 7
#define ALIF_ASNATIVEBYTES_DEFAULTS -1
#define ALIF_ASNATIVEBYTES_BIG_ENDIAN 0
#define ALIF_ASNATIVEBYTES_LITTLE_ENDIAN 1
#define ALIF_ASNATIVEBYTES_NATIVE_ENDIAN 3
#define ALIF_ASNATIVEBYTES_UNSIGNED_BUFFER 4
#define ALIF_ASNATIVEBYTES_REJECT_NEGATIVE 8
#define ALIF_ASNATIVEBYTES_ALLOW_INDEX 16





AlifIntT _alifLong_sign(AlifObject*); // 68


int64_t _alifLong_numBits(AlifObject*); // 77



AlifObject* _alifLong_fromByteArray(const unsigned char*, AlifUSizeT, AlifIntT, AlifIntT); // 94
