
#include "AlifCore_Abstract.h"
#include "AlifCore_ModSupport.h"



























// 785
#define BYTES_REPLACE_METHODDEF    \
    {"استبدل", ALIF_CPPFUNCTION_CAST(bytes_replace), METHOD_FASTCALL},

static AlifObject* bytes_replaceImpl(AlifBytesObject*, AlifBuffer*, AlifBuffer*,
	AlifSizeT);

static AlifObject* bytes_replace(AlifBytesObject* _self,
	AlifObject* const* _args, AlifSizeT _nargs) { // 792
	AlifObject* returnValue{};
	AlifBuffer old = { nullptr, nullptr };
	AlifBuffer new_ = { nullptr, nullptr };
	AlifSizeT count = -1;

	if (!_ALIFARG_CHECKPOSITIONAL("استبدل", _nargs, 2, 3)) {
		goto exit;
	}
	if (alifObject_getBuffer(_args[0], &old, ALIFBUF_SIMPLE) != 0) {
		goto exit;
	}
	if (alifObject_getBuffer(_args[1], &new_, ALIFBUF_SIMPLE) != 0) {
		goto exit;
	}
	if (_nargs < 3) {
		goto skip_optional;
	}
	{
		AlifSizeT ival = -1;
		AlifObject* iobj = _alifNumber_index(_args[2]);
		if (iobj != nullptr) {
			ival = alifLong_asSizeT(iobj);
			ALIF_DECREF(iobj);
		}
		if (ival == -1 and alifErr_occurred()) {
			goto exit;
		}
		count = ival;
	}
skip_optional:
	returnValue = bytes_replaceImpl(_self, &old, &new_, count);

exit:
	/* Cleanup for old */
	if (old.obj) {
		alifBuffer_release(&old);
	}
	/* Cleanup for new */
	if (new_.obj) {
		alifBuffer_release(&new_);
	}

	return returnValue;
}


































static AlifObject* bytes_newImpl(AlifTypeObject*,
	AlifObject*, const char*, const char*); // 1302


static AlifObject* bytes_new(AlifTypeObject* _type,
	AlifObject* _args, AlifObject* _kwargs) { // 1306
	AlifObject* returnValue{};
#  define KWTUPLE nullptr

	static const char* const _keywords[] = { "مصدر", "ترميز", "اخطاء", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "ثمانيات",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[3]{};
	AlifObject* const* fastargs{};
	AlifSizeT nargs = ALIFTUPLE_GET_SIZE(_args);
	AlifSizeT noptargs = nargs + (_kwargs ? ALIFDICT_GET_SIZE(_kwargs) : 0) - 0;
	AlifObject* x = nullptr;
	const char* encoding = nullptr;
	const char* errors = nullptr;

	fastargs = _ALIFARG_UNPACKKEYWORDS(ALIFTUPLE_CAST(_args)->item, nargs, _kwargs, nullptr, &_parser,
		/*minpos*/ 0, /*maxpos*/ 3, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
	if (!fastargs) {
		goto exit;
	}
	if (!noptargs) {
		goto skip_optional_pos;
	}
	if (fastargs[0]) {
		x = fastargs[0];
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (fastargs[1]) {
		if (!ALIFUSTR_CHECK(fastargs[1])) {
			//_alifArg_badArgument("bytes", "argument 'encoding'", "str", fastargs[1]);
			goto exit;
		}
		AlifSizeT encodingLength;
		encoding = alifUStr_asUTF8AndSize(fastargs[1], &encodingLength);
		if (encoding == nullptr) {
			goto exit;
		}
		if (strlen(encoding) != (size_t)encodingLength) {
			alifErr_setString(_alifExcValueError_, "يحتوي احرف فارغة");
			goto exit;
		}
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (!ALIFUSTR_CHECK(fastargs[2])) {
		//_alifArg_badArgument("bytes", "argument 'errors'", "str", fastargs[2]);
		goto exit;
	}
	AlifSizeT errorsLength;
	errors = alifUStr_asUTF8AndSize(fastargs[2], &errorsLength);
	if (errors == nullptr) {
		goto exit;
	}
	if (strlen(errors) != (size_t)errorsLength) {
		alifErr_setString(_alifExcValueError_, "يحتوي احرف فارغة");
		goto exit;
	}
skip_optional_pos:
	returnValue = bytes_newImpl(_type, x, encoding, errors);

exit:
	return returnValue;
}










