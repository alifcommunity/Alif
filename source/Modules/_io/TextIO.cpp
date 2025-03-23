#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Codecs.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_Long.h"
#include "AlifCore_Object.h"
#include "AlifCore_Errors.h"
#include "AlifCore_State.h"

#include "_IOModule.h"










typedef class TextIO TextIO;


#include "clinic/textio.cpp.h" // 33




































/* TextIOWrapper */

typedef AlifObject* (*EncodeFuncT)(AlifObject*, AlifObject*); // 655

class TextIO { // 658
public:
	ALIFOBJECT_HEAD{};
	AlifIntT ok{}; /* initialized? */
	AlifIntT detached{};
	AlifSizeT chunkSize{};
	AlifObject* buffer{};
	AlifObject* encoding{};
	AlifObject* encoder{};
	AlifObject* decoder{};
	AlifObject* readnl{};
	AlifObject* errors{};
	const char* writenl; /* ASCII-encoded; nullptr stands for \n */
	char line_buffering;
	char write_through;
	char readuniversal;
	char readtranslate;
	char writetranslate;
	char seekable;
	char has_read1;
	char telling;
	char finalizing;
	EncodeFuncT encodeFunc{};
	char encodingStartOfStream{};
	AlifObject* decodedChars{};
	AlifSizeT decodedCharsUsed{};
	AlifObject* pendingBytes{};
	AlifSizeT pendingBytesCount{};
	AlifObject* snapshot{};
	double b2cratIO{};

	AlifObject* raw{};

	AlifObject* weakRefList{};
	AlifObject* dict{};

	AlifIOState* state{};
};


static AlifIntT _ioTextIOWrapper_init_impl(TextIO* _self, AlifObject* _buffer,
	const char* _encoding, AlifObject* _errors,
	const char* _newLine, AlifIntT _lineBuffering,
	AlifIntT _writeThrough) { // 1088
	AlifObject* raw, * codeAlifInfo = nullptr;
	AlifObject* res{};
	AlifIntT r{};

	_self->ok = 0;
	_self->detached = 0;

	if (_encoding == nullptr) {
		AlifInterpreter* interp = _alifInterpreter_get();
		if (alifInterpreter_getConfig(interp)->warnDefaultEncoding) {
			//if (alifErr_warnEx(_alifExcEncodingWarning_,
				//"'encoding' argument not specified", 1)) {
				//return -1;
			//}
		}
	}
	if (_errors == ALIF_NONE) {
		_errors = &ALIF_ID(strict);
	}
	else if (!ALIFUSTR_CHECK(_errors)) {

		//alifErr_format(
			//_alifExcTypeError_,
			//"TextIOWrapper() argument 'errors' must be str or None, not %.50s",
			//ALIF_TYPE(errors)->name);
		return -1;
	}
	else if (io_checkErrors(_errors)) {
		return -1;
	}
	const char* errorsStr = _alifUStr_asUTF8NoNUL(_errors);
	if (errorsStr == nullptr) {
		return -1;
	}
	if (validate_newLine(_newLine) < 0) {
		return -1;
	}

	ALIF_CLEAR(_self->buffer);
	ALIF_CLEAR(_self->encoding);
	ALIF_CLEAR(_self->encoder);
	ALIF_CLEAR(_self->decoder);
	ALIF_CLEAR(_self->readnl);
	ALIF_CLEAR(_self->decodedChars);
	ALIF_CLEAR(_self->pendingBytes);
	ALIF_CLEAR(_self->snapshot);
	ALIF_CLEAR(_self->errors);
	ALIF_CLEAR(_self->raw);
	_self->decodedCharsUsed = 0;
	_self->pendingBytesCount = 0;
	_self->encodeFunc = nullptr;
	_self->b2cratIO = 0.0;

	if (_encoding == nullptr and _alifDureRun_.preConfig.utf8Mode) {
		_self->encoding = &ALIF_STR(utf8);
	}
	if (_encoding == nullptr or (strcmp(_encoding, "locale") == 0)) {
		_self->encoding = alif_getLocaleEncodingObject();
		if (_self->encoding == nullptr) {
			goto error;
		}
	}
	if (_self->encoding != nullptr) {
		_encoding = alifUStr_asUTF8(_self->encoding);
		if (_encoding == nullptr)
			goto error;
	}
	else if (_encoding != nullptr) {
		_self->encoding = alifUStr_fromString(_encoding);
		if (_self->encoding == nullptr)
			goto error;
	}
	else {
		//alifErr_setString(_alifExcOSError_,
			//"could not determine default encoding");
		goto error;
	}

	codeAlifInfo = _alifCodec_lookupTextEncoding(_encoding, "codecs.open()");
	if (codeAlifInfo == nullptr) {
		ALIF_CLEAR(_self->encoding);
		goto error;
	}

	_self->errors = ALIF_NEWREF(_errors);
	_self->chunkSize = 8192;
	_self->line_buffering = _lineBuffering;
	_self->write_through = _writeThrough;
	if (set_newLine(_self, _newLine) < 0) {
		goto error;
	}

	_self->buffer = ALIF_NEWREF(_buffer);

	_PyIO_State* state = findIO_stateByDef(ALIF_TYPE(_self));
	_self->state = state;
	if (textIOWrapper_setDecoder(_self, codeAlifInfo, errorsStr) != 0)
		goto error;

	if (textIOWrapper_setEncoder(_self, codeAlifInfo, errorsStr) != 0)
		goto error;

	ALIF_CLEAR(codeAlifInfo);

	if (ALIF_IS_TYPE(_buffer, state->alifBufferedReaderType) or
		ALIF_IS_TYPE(_buffer, state->alifBufferedWriterType) or
		ALIF_IS_TYPE(_buffer, state->alifBufferedRandomType))
	{
		if (alifObject_getOptionalAttr(_buffer, &ALIF_ID(raw), &raw) < 0)
			goto error;
		if (raw != nullptr) {
			if (ALIF_IS_TYPE(raw, state->PyFileIO_Type))
				_self->raw = raw;
			else
				ALIF_DECREF(raw);
		}
	}

	res = alifObject_callMethodNoArgs(_buffer, &ALIF_ID(seekable));
	if (res == nullptr)
		goto error;
	r = alifObject_isTrue(res);
	ALIF_DECREF(res);
	if (r < 0)
		goto error;
	_self->seekable = _self->telling = r;

	r = alifObject_hasAttrWithError(_buffer, &ALIF_ID(read1));
	if (r < 0) {
		goto error;
	}
	_self->has_read1 = r;

	_self->encodingStartOfStream = 0;
	if (textIOWrapper_fixEncoderState(_self) < 0) {
		goto error;
	}

	_self->ok = 1;
	return 0;

error:
	ALIF_XDECREF(codeAlifInfo);
	return -1;
}
