#include "alif.h"

#include "AlifCore_State.h"



const char* _alifHexDigits_ = "0123456789abcdef"; // 19 // need review


AlifObject* _alifCodec_lookup(const char* encoding) { // 133
	if (encoding == NULL) {
		//ALIFERR_BADARGUMENT();
		return NULL;
	}

	AlifInterpreter* interp = alifInterpreter_get();
	AlifObject* v = normalizestring(encoding);
	if (v == NULL) {
		return NULL;
	}

	_alifUStr_internMortal(interp, &v);

	/* First, try to lookup the name in the registry dictionary */
	AlifObject* result;
	if (alifDict_getItemRef(interp->codecs.searchCache, v, &result) < 0) {
		goto onError;
	}
	if (result != NULL) {
		ALIF_DECREF(v);
		return result;
	}

	/* Next, scan the search functions in order of registration */
	const AlifSizeT len = alifList_size(interp->codecs.search_path);
	if (len < 0)
		goto onError;
	if (len == 0) {
		PyErr_SetString(PyExc_LookupError,
			"no codec search functions registered: "
			"can't find encoding");
		goto onError;
	}

	AlifSizeT i;
	for (i = 0; i < len; i++) {
		AlifObject* func;

		func = PyList_GetItemRef(interp->codecs.search_path, i);
		if (func == NULL)
			goto onError;
		result = PyObject_CallOneArg(func, v);
		ALIF_DECREF(func);
		if (result == NULL)
			goto onError;
		if (result == Py_None) {
			Py_CLEAR(result);
			continue;
		}
		if (!PyTuple_Check(result) || PyTuple_GET_SIZE(result) != 4) {
			PyErr_SetString(PyExc_TypeError,
				"codec search functions must return 4-tuples");
			ALIF_DECREF(result);
			goto onError;
		}
		break;
	}
	if (result == NULL) {
		/* XXX Perhaps we should cache misses too ? */
		PyErr_Format(PyExc_LookupError,
			"unknown encoding: %s", encoding);
		goto onError;
	}

	_PyUnicode_InternImmortal(interp, &v);

	/* Cache and return the result */
	if (PyDict_SetItem(interp->codecs.search_cache, v, result) < 0) {
		ALIF_DECREF(result);
		goto onError;
	}
	ALIF_DECREF(v);
	return result;

onError:
	ALIF_DECREF(v);
	return NULL;
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


AlifObject* _alifCodec_lookupTextEncoding(const char* _encoding,
	const char* _alternateCommand) { // 510
	AlifObject* codec{};
	AlifObject* attr{};
	AlifIntT isTextCodec{};

	codec = _alifCodec_lookup(_encoding);
	if (codec == NULL)
		return NULL;

	if (!ALIFTUPLE_CHECKEXACT(codec)) {
		if (alifObject_getOptionalAttr(codec, &ALIF_ID(_is_text_encoding), &attr) < 0) {
			ALIF_DECREF(codec);
			return NULL;
		}
		if (attr != NULL) {
			isTextCodec = alifObject_isTrue(attr);
			ALIF_DECREF(attr);
			if (isTextCodec <= 0) {
				ALIF_DECREF(codec);
				if (!isTextCodec)
					//alifErr_format(_alifExcLookupError_,
						//"'%.400s' is not a text encoding; "
						//"use %s to handle arbitrary codecs",
						//_encoding, _alternateCommand);
				return NULL;
			}
		}
	}

	return codec;
}


static AlifObject* codec_getitem_checked(const char* _encoding,
	const char* _alternateCommand,
	int _index)
{
	AlifObject* codec{};
	AlifObject* v_{};

	codec = _alifCodec_lookupTextEncoding(_encoding, _alternateCommand);
	if (codec == NULL)
		return NULL;

	v_ = ALIF_NEWREF(ALIFTUPLE_GET_ITEM(codec, _index));
	ALIF_DECREF(codec);
	return v_;
}


static AlifObject* _alifCodec_textEncoder(const char* encoding) { // 567
	//return codec_getItemChecked(encoding, "codecs.encode()", 0);
	printf("تعليق: _alifCodec_textEncoder() - Codecs.cpp");
	return nullptr;
}


static AlifObject* _alifCodec_textDecoder(const char* _encoding)
{
	return codec_getitem_checked(_encoding, "codecs.decode()", 1);
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
	const char* _encoding,
	const char* _errors) { // 590
	AlifObject* decoder{};

	decoder = _alifCodec_textDecoder(_encoding);
	if (decoder == nullptr)
		return nullptr;

	return _alifCodec_decodeInternal(_object, decoder, _encoding, _errors);
}
