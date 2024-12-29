#include "alif.h"

#include "AlifCore_State.h"



const char* _alifHexDigits_ = "0123456789abcdef"; // 19 // need review




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





static AlifObject* _alifCodec_textEncoder(const char* encoding) { // 567
	//return codec_getItemChecked(encoding, "codecs.encode()", 0);
	printf("تعليق: _alifCodec_textEncoder() - Codecs.cpp");
	return nullptr;
}


AlifObject* _alifCodec_encodeText(AlifObject* object,
	const char* encoding, const char* errors) { // 577
	AlifObject* encoder{};

	encoder = _alifCodec_textEncoder(encoding);
	if (encoder == nullptr)
		return nullptr;

	return _alifCodec_encodeInternal(object, encoder, encoding, errors);
}
