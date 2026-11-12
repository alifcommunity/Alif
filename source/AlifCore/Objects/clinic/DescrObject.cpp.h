


























static AlifIntT property_initImpl(PropertyObject*, AlifObject*, AlifObject*,
	AlifObject*, AlifObject*); // 103

static AlifIntT property_init(AlifObject* self,
	AlifObject* args, AlifObject* kwargs) { // 107

	AlifIntT returnValue = -1;

#  define KWTUPLE nullptr

	static const char* const _keywords[] = { "fget", "fset", "fdel", "doc", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "خاصية",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[4]{};
	AlifObject* const* fastargs{};
	AlifSizeT nargs = ALIFTUPLE_GET_SIZE(args);
	AlifSizeT noptargs = nargs + (kwargs ? ALIFDICT_GET_SIZE(kwargs) : 0) - 0;
	AlifObject* fget = nullptr;
	AlifObject* fset = nullptr;
	AlifObject* fdel = nullptr;
	AlifObject* doc = nullptr;

	fastargs = _ALIFARG_UNPACKKEYWORDS(ALIFTUPLE_CAST(args)->item, nargs, kwargs, nullptr, &_parser,
		/*minpos*/ 0, /*maxpos*/ 4, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
	if (!fastargs) {
		goto exit;
	}
	if (!noptargs) {
		goto skip_optional_pos;
	}
	if (fastargs[0]) {
		fget = fastargs[0];
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (fastargs[1]) {
		fset = fastargs[1];
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (fastargs[2]) {
		fdel = fastargs[2];
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	doc = fastargs[3];
skip_optional_pos:
	returnValue = property_initImpl((PropertyObject*)self, fget, fset, fdel, doc);

exit:
	return returnValue;
}
