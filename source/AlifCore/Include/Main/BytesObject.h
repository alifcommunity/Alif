#pragma once




extern AlifTypeObject _alifBytesType_; // 24
extern AlifTypeObject _alifBytesIterType_;
// 27
#define ALIFBYTES_CHECK(op) \
                 ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(op), ALIF_TPFLAGS_BYTES_SUBCLASS)
#define ALIFBYTES_CHECKEXACT(op) ALIF_IS_TYPE((op), &_alifBytesType_)


AlifObject* alifBytes_fromStringAndSize(const char*, AlifSizeT); // 31
AlifObject* alifBytes_fromString(const char*);

AlifSizeT alifBytes_size(AlifObject*); // 38
char* alifBytes_asString(AlifObject*); // 39

void alifBytes_concat(AlifObject**, AlifObject*); // 41

AlifIntT alifBytes_asStringAndSize(AlifObject*, char**, AlifSizeT*); // 51

/* ------------------------------------------------------------------------------------------- */



class AlifBytesObject { // 5
public:
	ALIFOBJECT_VAR_HEAD;
	AlifHashT hash{}; // Deprecated
	char val[1]{};

	/* Invariants:
	 *     val contains space for 'ob_size+1' elements.
	 *     val[size] == 0.
	 *     hash is the hash of the byte string or -1 if not computed yet.
	 */
};

AlifIntT alifBytes_resize(AlifObject**, AlifSizeT); // 17

// 20
#define ALIFBYTES_CAST(op) \
    (ALIF_CAST(AlifBytesObject*, op))

static inline char* _alifBytes_asString(AlifObject* _op) { // 23
	return ALIFBYTES_CAST(_op)->val;
}
#define ALIFBYTES_AS_STRING(op) _alifBytes_asString(ALIFOBJECT_CAST(op))


static inline AlifSizeT alifBytes_getSize(AlifObject* op) { // 29
	AlifBytesObject* self = ALIFBYTES_CAST(op);
	return ALIF_SIZE(self);
}
#define ALIFBYTES_GET_SIZE(self) alifBytes_getSize(ALIFOBJECT_CAST(self))
