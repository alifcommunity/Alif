#pragma once

class AlifWBytesObject {
public:
    AlifVarObject object;

    wchar_t value[1];
};

extern AlifInitObject _typeBytes_;

int alifWBytes_asStringAndSize(AlifObject*, wchar_t**, int64_t*);

#define ALIFBYTES_CHECK(op) ALIF_IS_TYPE(op, &_typeBytes_)

#define ALIFBYTESOBJECT_SIZE (offsetof(AlifWBytesObject, value) + 1)

AlifObject* alifBytes_fromStringAndSize(const wchar_t*, int64_t);
AlifObject* alifBytes_fromString(const wchar_t*);
int alidSubBytes_resize(AlifObject**, int64_t);

int64_t alifBytes_size(AlifObject*);
wchar_t* alifWBytes_asString(AlifObject*);
void alifBytes_concat(AlifObject** , AlifObject*);

static inline wchar_t* alifWBytes_asString(AlifObject* _op)
{
    return ((AlifWBytesObject*)_op)->value;
}
#define ALIFWBYTES_AS_STRING(_op) alifWBytes_asString(_op)
