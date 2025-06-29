#include "alif.h"

#include "AlifCore_State.h"



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

	if (!_alif_normalizeEncoding(string, encoding, len + 1))
	{
		//alifErr_setString(_alifExcRuntimeError_, "_alif_normalizeEncoding() failed");
		alifMem_dataFree(encoding);
		return nullptr;
	}

	v = alifUStr_fromString(encoding);
	alifMem_dataFree(encoding);
	return v;
}




AlifObject* _alifCodec_lookup(const char* encoding) { // 133

	AlifSizeT len{}; //* alif

	if (encoding == nullptr) {
		//ALIFERR_BADARGUMENT();
		return nullptr;
	}

	AlifInterpreter* interp = _alifInterpreter_get();
	AlifObject* v = normalize_string(encoding);
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


static AlifObject* _alifCodec_textDecoder(const char* _encoding)
{
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
