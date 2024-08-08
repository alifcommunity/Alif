#pragma once

class AlifWBytesObject {
public:

	ALIFOBJECT_VAR_HEAD;

    wchar_t value_[1];
};

int alifSubBytes_resize(AlifObject**, int64_t );

extern AlifInitObject _typeBytes_;

int alifWBytes_asStringAndSize(AlifObject*, wchar_t**, AlifSizeT*);

#define ALIFBYTES_CHECK(op) ALIF_IS_TYPE(op, &_typeBytes_)

#define ALIFBYTESOBJECT_SIZE (offsetof(AlifWBytesObject, value_) + 1)

AlifObject* alifBytes_fromStringAndSize(const wchar_t*, int64_t);
AlifObject* alifBytes_fromString(const wchar_t*);

int64_t alifBytes_size(AlifObject*);
//wchar_t* alifWBytes_asString(AlifObject*);
void alifBytes_concat(AlifObject** , AlifObject*);

static inline wchar_t* _alifWBytes_asString(AlifObject* _op)
{
    return ((AlifWBytesObject*)_op)->value_;
}
#define ALIFWBYTES_AS_STRING(_op) _alifWBytes_asString(_op)

static inline int64_t alifBytes_get_size(AlifObject* _op) {
	AlifWBytesObject* self_ = (AlifWBytesObject*)(_op);
	return ALIF_SIZE(self_);
}
#define ALIFWBYTES_GET_SIZE(_self) alifBytes_get_size(ALIFOBJECT_CAST(_self))
