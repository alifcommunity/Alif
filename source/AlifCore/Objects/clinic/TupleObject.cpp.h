






















// 73
static AlifObject* tuple_newImpl(AlifTypeObject*, AlifObject*);

static AlifObject* tuple_new(AlifTypeObject* _type,
	AlifObject* _args, AlifObject* _kwargs) {
	AlifObject* returnValue{};
	AlifTypeObject* base_tp = &_alifTupleType_;
	AlifObject* iterable{};

	if ((_type == base_tp or _type->init == base_tp->init) and
		!_alifArg_noKeywords("مترابطة", _kwargs)) {
		goto exit;
	}
	if (!_ALIFARG_CHECKPOSITIONAL("مترابطة", ALIFTUPLE_GET_SIZE(_args), 0, 1)) {
		goto exit;
	}
	if (ALIFTUPLE_GET_SIZE(_args) < 1) {
		goto skip_optional;
	}
	iterable = ALIFTUPLE_GET_ITEM(_args, 0);
skip_optional:
	returnValue = tuple_newImpl(_type, iterable);

exit:
	return returnValue;
}













// 106
#define TUPLE___GETNEWARGS___METHODDEF    \
    {"__اجلب_وسيطات_جديدة__", (AlifCPPFunction)tuple___getnewargs__, METHOD_NOARGS},

static AlifObject* tuple___getnewargs__Impl(AlifTupleObject*);

static AlifObject* tuple___getnewargs__(AlifTupleObject* self,
	AlifObject* ALIF_UNUSED(ignored)) {
	return tuple___getnewargs__Impl(self);
}
