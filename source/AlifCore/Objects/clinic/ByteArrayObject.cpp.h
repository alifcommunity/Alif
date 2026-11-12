


























#define BYTEARRAY_EXTEND_METHODDEF    \
    {"اطل", (AlifCPPFunction)byteArray_extend, METHOD_O}, // 1070






// 1040
#define BYTEARRAY_APPEND_METHODDEF    \
    {"اضف", (AlifCPPFunction)byteArray_append, METHOD_O},

static AlifObject* byteArray_appendImpl(AlifByteArrayObject*, AlifIntT);

static AlifObject* byteArray_append(AlifByteArrayObject* _self,
	AlifObject* _arg) { // 1046
	AlifObject* returnValue{};
	AlifIntT item{};

	if (!_get_bytevalue(_arg, &item)) {
		goto exit;
	}
	returnValue = byteArray_appendImpl(_self, item);

exit:
	return returnValue;
}
