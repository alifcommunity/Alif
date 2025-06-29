#pragma once




extern AlifTypeObject _alifByteArrayType_; // 20
extern AlifTypeObject _alifByteArrayIterType_;


#define ALIFBYTEARRAY_CHECK(_self) ALIFOBJECT_TYPECHECK((_self), &_alifByteArrayType_)
#define ALIFBYTEARRAY_CHECKEXACT(_self) ALIF_IS_TYPE((_self), &_alifByteArrayType_)


AlifObject* alifByteArray_fromStringAndSize(const char*, AlifSizeT); // 30

AlifIntT alifByteArray_resize(AlifObject*, AlifSizeT); // 33



/* -------------------------------------------------------------------------------------------- */


class AlifByteArrayObject { // 6
public:
	ALIFOBJECT_VAR_HEAD;
	AlifSizeT alloc{};   /* How many bytes allocated in ob_bytes */
	char* bytes{};        /* Physical backing buffer */
	char* start{};        /* Logical start inside ob_bytes */
	AlifSizeT exports{}; /* How many buffer exports */
};

extern char _alifByteArrayEmptyString_[]; // 14

/* Macros and static inline functions, trading safety for speed */
#define ALIFBYTEARRAY_CAST(op) ALIF_CAST(AlifByteArrayObject*, op)

static inline char* alifByteArray_asString(AlifObject* op) { // 20
	AlifByteArrayObject* self = ALIFBYTEARRAY_CAST(op);
	if (ALIF_SIZE(self)) {
		return self->start;
	}
	return _alifByteArrayEmptyString_;
}
#define ALIFBYTEARRAY_AS_STRING(self) alifByteArray_asString(ALIFOBJECT_CAST(self))

static inline AlifSizeT alifByteArray_getSize(AlifObject* op) { // 30
	AlifByteArrayObject* self = ALIFBYTEARRAY_CAST(op);
	return ALIF_SIZE(self);
}
#define ALIFBYTEARRAY_GET_SIZE(self) alifByteArray_getSize(ALIFOBJECT_CAST(self))
