#include "alif.h"

#include "AlifCore_State.h"
#include "AlifCore_Call.h"



const char* _alifHexDigits_ = "0123456789abcdef"; // 19 // need review



extern AlifIntT _alif_normalizeEncoding(const char*, char*, AlifUSizeT); // 85

static AlifObject* normalize_string(const char* string) { // 90
	AlifUSizeT len = strlen(string);
	char* encoding;
	AlifObject* v{};

	if (len > ALIF_SIZET_MAX) {
		//alifErr_setString(_alifExcOverflowError_, "string is too large");
		return nullptr;
	}

	encoding = (char*)alifMem_dataAlloc(len + 1);
	if (encoding == nullptr)
		//return alifErr_noMemory();

		if (!_alif_normalizeEncoding(string, encoding, len + 1)) {
			//alifErr_setString(_alifExcRuntimeError_, "_alif_normalizeEncoding() failed");
			alifMem_dataFree(encoding);
			return nullptr;
		}

	v = alifUStr_fromString(encoding);
	alifMem_dataFree(encoding);
	return v;
}




AlifObject* _alifCodec_lookup(const char* _encoding) { // 133

	AlifSizeT len{}; //* alif

	if (_encoding == nullptr) {
		//ALIFERR_BADARGUMENT();
		return nullptr;
	}

	AlifInterpreter* interp = _alifInterpreter_get();
	AlifObject* v = normalize_string(_encoding);
	if (v == nullptr) {
		return nullptr;
	}

	alifUStr_internMortal(interp, &v);

	/* First, try to lookup the name in the registry dictionary */
	AlifObject* result;
	if (alifDict_getItemRef(interp->codecs.searchCache, v, &result) < 0) {
		goto onError;
	}
	if (result != nullptr) {
		ALIF_DECREF(v);
		return result;
	}

	/* Next, scan the search functions in order of registration */
	len = alifList_size(interp->codecs.searchPath);
	if (len < 0)
		goto onError;
	if (len == 0) {
		//alifErr_setString(_alifExcLookupError_,
		//	"no codec search functions registered: "
		//	"can't find encoding");
		goto onError;
	}

	AlifSizeT i;
	for (i = 0; i < len; i++) {
		AlifObject* func;

		func = alifList_getItemRef(interp->codecs.searchPath, i);
		if (func == nullptr)
			goto onError;
		result = alifObject_callOneArg(func, v);
		ALIF_DECREF(func);
		if (result == nullptr)
			goto onError;
		if (result == ALIF_NONE) {
			ALIF_CLEAR(result);
			continue;
		}
		if (!ALIFTUPLE_CHECK(result) or ALIFTUPLE_GET_SIZE(result) != 4) {
			//alifErr_setString(_alifExcTypeError_,
			//	"codec search functions must return 4-tuples");
			ALIF_DECREF(result);
			goto onError;
		}
		break;
	}
	if (result == nullptr) {
		//alifErr_format(_alifExcLookupError_,
		//	"unknown encoding: %s", encoding);
		goto onError;
	}

	alifUStr_internImmortal(interp, &v);

	/* Cache and return the result */
	if (alifDict_setItem(interp->codecs.searchCache, v, result) < 0) {
		ALIF_DECREF(result);
		goto onError;
	}
	ALIF_DECREF(v);
	return result;

onError:
	ALIF_DECREF(v);
	return nullptr;
}

static AlifObject* args_tuple(AlifObject* object,
	const char* errors) { // 237
	AlifObject* args{};

	args = alifTuple_new(1 + (errors != nullptr));
	if (args == nullptr)
		return nullptr;
	ALIFTUPLE_SET_ITEM(args, 0, ALIF_NEWREF(object));
	if (errors) {
		AlifObject* v{};

		v = alifUStr_fromString(errors);
		if (v == nullptr) {
			ALIF_DECREF(args);
			return nullptr;
		}
		ALIFTUPLE_SET_ITEM(args, 1, v);
	}
	return args;
}


static AlifObject* codec_makeIncrementalCodec(AlifObject* _codecInfo,
	const char* _errors, const char* _attrName) { // 283
	AlifObject* ret{}, * inccodec{};

	inccodec = alifObject_getAttrString(_codecInfo, _attrName);
	if (inccodec == nullptr)
		return nullptr;
	if (_errors)
		ret = alifObject_callFunction(inccodec, "s", _errors);
	else
		ret = _alifObject_callNoArgs(inccodec);
	ALIF_DECREF(inccodec);
	return ret;
}

AlifObject* _alifCodecInfo_getIncrementalEncoder(AlifObject* _codecInfo,
	const char* _errors) { // 349
	return codec_makeIncrementalCodec(_codecInfo, _errors,
		"incrementalencoder");
}



static AlifObject* _alifCodec_encodeInternal(AlifObject* object,
	AlifObject* encoder, const char* encoding, const char* errors) { // 398
	AlifObject* args = nullptr, * result = nullptr;
	AlifObject* v = nullptr;

	args = args_tuple(object, errors);
	if (args == nullptr)
		goto onError;

	result = alifObject_call(encoder, args, nullptr);
	if (result == nullptr) {
		//_alifErr_formatNote("%s with '%s' codec failed", "encoding", encoding);
		goto onError;
	}

	if (!ALIFTUPLE_CHECK(result) or
		ALIFTUPLE_GET_SIZE(result) != 2) {
		//alifErr_setString(_alifExcTypeError_,
		//	"encoder must return a tuple (object, integer)");
		goto onError;
	}
	v = ALIF_NEWREF(ALIFTUPLE_GET_ITEM(result, 0));
	/* We don't check or use the second (integer) entry. */

	ALIF_DECREF(args);
	ALIF_DECREF(encoder);
	ALIF_DECREF(result);
	return v;

onError:
	ALIF_XDECREF(result);
	ALIF_XDECREF(args);
	ALIF_XDECREF(encoder);
	return nullptr;
}



static AlifObject* _alifCodec_decodeInternal(AlifObject* object, AlifObject* decoder,
	const char* encoding, const char* errors) { // 443
	AlifObject* args = nullptr, * result = nullptr;
	AlifObject* v{};

	args = args_tuple(object, errors);
	if (args == nullptr)
		goto onError;

	result = alifObject_call(decoder, args, nullptr);
	if (result == nullptr) {
		//_alifErr_formatNote("%s with '%s' codec failed", "decoding", encoding);
		goto onError;
	}
	if (!ALIFTUPLE_CHECK(result) ||
		ALIFTUPLE_GET_SIZE(result) != 2) {
		//alifErr_setString(_alifExcTypeError_,
		//	"decoder must return a tuple (object,integer)");
		goto onError;
	}
	v = ALIF_NEWREF(ALIFTUPLE_GET_ITEM(result, 0));
	/* We don't check or use the second (integer) entry. */

	ALIF_DECREF(args);
	ALIF_DECREF(decoder);
	ALIF_DECREF(result);
	return v;

onError:
	ALIF_XDECREF(args);
	ALIF_XDECREF(decoder);
	ALIF_XDECREF(result);
	return nullptr;
}




AlifObject* _alifCodec_lookupTextEncoding(const char* _encoding,
	const char* _alternateCommand) { // 510
	AlifObject* codec{};
	AlifObject* attr{};
	AlifIntT isTextCodec{};

	codec = _alifCodec_lookup(_encoding);
	if (codec == nullptr)
		return nullptr;

	if (!ALIFTUPLE_CHECKEXACT(codec)) {
		if (alifObject_getOptionalAttr(codec, &ALIF_ID(_isTextEncoding), &attr) < 0) {
			ALIF_DECREF(codec);
			return nullptr;
		}
		if (attr != nullptr) {
			isTextCodec = alifObject_isTrue(attr);
			ALIF_DECREF(attr);
			if (isTextCodec <= 0) {
				ALIF_DECREF(codec);
				if (!isTextCodec)
					//alifErr_format(_alifExcLookupError_,
						//"'%.400s' is not a text encoding; "
						//"use %s to handle arbitrary codecs",
						//_encoding, _alternateCommand);
					return nullptr;
			}
		}
	}

	return codec;
}


static AlifObject* codec_getItemChecked(const char* _encoding,
	const char* _alternateCommand, AlifIntT _index) { // 550
	AlifObject* codec{};
	AlifObject* v_{};

	codec = _alifCodec_lookupTextEncoding(_encoding, _alternateCommand);
	if (codec == nullptr)
		return nullptr;

	v_ = ALIF_NEWREF(ALIFTUPLE_GET_ITEM(codec, _index));
	ALIF_DECREF(codec);
	return v_;
}


static AlifObject* _alifCodec_textEncoder(const char* encoding) { // 567
	return codec_getItemChecked(encoding, "codecs.encode()", 0);
}


static AlifObject* _alifCodec_textDecoder(const char* _encoding) {
	return codec_getItemChecked(_encoding, "codecs.decode()", 1);
}

AlifObject* _alifCodec_encodeText(AlifObject* object,
	const char* encoding, const char* errors) { // 577
	AlifObject* encoder{};

	encoder = _alifCodec_textEncoder(encoding);
	if (encoder == nullptr)
		return nullptr;

	return _alifCodec_encodeInternal(object, encoder, encoding, errors);
}


AlifObject* _alifCodec_decodeText(AlifObject* _object,
	const char* _encoding, const char* _errors) { // 590
	AlifObject* decoder{};

	decoder = _alifCodec_textDecoder(_encoding);
	if (decoder == nullptr)
		return nullptr;

	return _alifCodec_decodeInternal(_object, decoder, _encoding, _errors);
}




AlifObject* alifCodec_strictErrors(AlifObject* exc) { // 669
	if (ALIFEXCEPTIONINSTANCE_CHECK(exc))
		alifErr_setObject(ALIFEXCEPTIONINSTANCE_CLASS(exc), exc);
	else
		alifErr_setString(_alifExcTypeError_, "codec must pass exception instance");
	return nullptr;
}

AlifObject* alifCodec_ignoreErrors(AlifObject* _exc) {
	AlifSizeT end{};

	//if (ALIFOBJECT_TYPECHECK(_exc, (AlifTypeObject*)_alifExcUnicodeEncodeError_)) {
	//	if (alifUnicodeEncodeError_getEnd(_exc, &end))
	//		return nullptr;
	//}
	//else if (ALIFOBJECT_TYPECHECK(_exc, (AlifTypeObject*)_alifExcUnicodeDecodeError_)) {
	//	if (alifUnicodeDecodeError_getEnd(_exc, &end))
	//		return nullptr;
	//}
	//else if (ALIFOBJECT_TYPECHECK(_exc, (AlifTypeObject*)alifExc_unicodeTranslateError)) {
	//	if (alifUnicodeTranslateError_getEnd(_exc, &end))
	//		return nullptr;
	//}
	//else {
	//	wrong_exceptionType(_exc);
	//	return nullptr;
	//}
	//return alif_buildValue("(Nn)", alif_getConstant(ALIF_CONSTANT_EMPTY_STR), end);
	return nullptr; //* alif
}



static AlifObject* strict_errors(AlifObject* _self, AlifObject* _exc) { // 1316
	return alifCodec_strictErrors(_exc);
}

static AlifObject* ignore_errors(AlifObject* _self, AlifObject* _exc) {
	return alifCodec_ignoreErrors(_exc);
}

static AlifObject* replace_errors(AlifObject* _self, AlifObject* _exc) {
	//return alifCodec_replaceErrors(_exc);
	return nullptr;
}

static AlifObject* nameReplace_errors(AlifObject* _self, AlifObject* _exc) {
	//return alifCodec_nameReplaceErrors(_exc);
	return nullptr; //* alif
}

static AlifObject* surrogatePass_errors(AlifObject* _self, AlifObject* _exc) {
	//return alifCodec_surrogatePassErrors(_exc);
	return nullptr; //* alif
}

static AlifObject* surrogateEscape_errors(AlifObject* _self, AlifObject* _exc) { // 1400
	//return alifCodec_surrogateEscapeErrors(_exc);
	return nullptr; //* alif
}

AlifStatus _alifCodec_initRegistry(AlifInterpreter* _interp) { // 1405
	static struct {
		const char* name;
		AlifMethodDef def;
	} methods[] =
	{
		{
			"strict",
			{
				"strict_errors",
				strict_errors,
				METHOD_O,
				//ALIFDOC_STR("Implements the 'strict' error handling, which "
				//		  "raises a UnicodeError on coding errors.")
			}
		},
		{
			"ignore",
			{
				"ignore_errors",
				ignore_errors,
				METHOD_O,
				//ALIFDOC_STR("Implements the 'ignore' error handling, which "
				//		  "ignores malformed data and continues.")
			}
		},
		{
			"replace",
			{
				"replace_errors",
				replace_errors,
				METHOD_O,
				//ALIFDOC_STR("Implements the 'replace' error handling, which "
				//		  "replaces malformed data with a replacement marker.")
			}
		},
		//{
		//	"xmlcharrefreplace",
		//	{
		//		"xmlcharrefreplace_errors",
		//		xmlcharrefreplace_errors,
		//		METHOD_O,
		//		ALIFDOC_STR("Implements the 'xmlcharrefreplace' error handling, "
		//				  "which replaces an unencodable character with the "
		//				  "appropriate XML character reference.")
		//	}
		//},
		//{
		//	"backslashreplace",
		//	{
		//		"backslashreplace_errors",
		//		backslashreplace_errors,
		//		METHOD_O,
		//		ALIFDOC_STR("Implements the 'backslashreplace' error handling, "
		//				  "which replaces malformed data with a backslashed "
		//				  "escape sequence.")
		//	}
		//},
		{
			"namereplace",
			{
				"namereplace_errors",
				nameReplace_errors,
				METHOD_O,
				//ALIFDOC_STR("Implements the 'namereplace' error handling, "
				//		  "which replaces an unencodable character with a "
				//		  "\\N{...} escape sequence.")
			}
		},
		{
			"surrogatepass",
			{
				"surrogatepass",
				surrogatePass_errors,
				METHOD_O
			}
		},
		{
			"surrogateescape",
			{
				"surrogateescape",
				surrogateEscape_errors,
				METHOD_O
			}
		}
	};

	_interp->codecs.searchPath = alifList_new(0);
	if (_interp->codecs.searchPath == nullptr) {
		return alifStatus_noMemory();
	}
	_interp->codecs.searchCache = alifDict_new();
	if (_interp->codecs.searchCache == nullptr) {
		return alifStatus_noMemory();
	}
	_interp->codecs.errorRegistry = alifDict_new();
	if (_interp->codecs.errorRegistry == nullptr) {
		return alifStatus_noMemory();
	}
	for (AlifUSizeT i = 0; i < ALIF_ARRAY_LENGTH(methods); ++i) {
		AlifObject* func = ALIFCPPFUNCTION_NEWEX(&methods[i].def, nullptr, nullptr);
		if (func == nullptr) {
			return alifStatus_noMemory();
		}

		AlifIntT res = alifDict_setItemString(_interp->codecs.errorRegistry,
			methods[i].name, func);
		ALIF_DECREF(func);
		if (res < 0) {
			return alifStatus_error("Failed to insert into codec error registry");
		}
	}

	_interp->codecs.initialized = 1;

	//AlifObject* mod = alifImport_importModule("encodings");
	//if (mod == NULL) {
	//	return alifStatus_error("Failed to import encodings module");
	//}
	//ALIF_DECREF(mod);

	return alifStatus_ok();
}
