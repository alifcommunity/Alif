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
	AlifObject* decoded_chars{};
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


static AlifIntT _io_TextIOWrapper___init___impl(TextIO* self, AlifObject* buffer,
	const char* encoding, AlifObject* errors,
	const char* newline, AlifIntT line_buffering,
	AlifIntT write_through) { // 1088
	AlifObject* raw, * codeAlifInfo = nullptr;
	AlifObject* res{};
	AlifIntT r{};

	self->ok = 0;
	self->detached = 0;

	if (encoding == nullptr) {
		AlifInterpreter* interp = alifInterpreter_get();
		if (alifInterpreter_getConfig(interp)->warnDefaultEncoding) {
			//if (alifErr_warnEx(_alifExcEncodingWarning_,
				//"'encoding' argument not specified", 1)) {
				//return -1;
			//}
		}
	}

	if (errors == ALIF_NONE) {
		errors = &ALIF_ID(strict);
	}
	else if (!ALIFUSTR_CHECK(errors)) {

		//alifErr_format(
			//_alifExcTypeError_,
			//"TextIOWrapper() argument 'errors' must be str or None, not %.50s",
			//ALIF_TYPE(errors)->name);
		return -1;
	}
	else if (io_checkErrors(errors)) {
		return -1;
	}
	const char* errorsStr = _alifUStr_asUTF8NoNUL(errors);
	if (errorsStr == nullptr) {
		return -1;
	}

	if (validate_newLine(newline) < 0) {
		return -1;
	}

	ALIF_CLEAR(self->buffer);
	ALIF_CLEAR(self->encoding);
	ALIF_CLEAR(self->encoder);
	ALIF_CLEAR(self->decoder);
	ALIF_CLEAR(self->readnl);
	ALIF_CLEAR(self->decoded_chars);
	ALIF_CLEAR(self->pendingBytes);
	ALIF_CLEAR(self->snapshot);
	ALIF_CLEAR(self->errors);
	ALIF_CLEAR(self->raw);
	self->decodedCharsUsed = 0;
	self->pendingBytesCount = 0;
	self->encodeFunc = nullptr;
	self->b2cratIO = 0.0;

	if (encoding == nullptr or (strcmp(encoding, "locale") == 0)) {
		self->encoding = alif_getLocaleEncodingObject();
		if (self->encoding == nullptr) {
			goto error;
		}
	}

	if (self->encoding != nullptr) {
		encoding = alifUStr_asUTF8(self->encoding);
		if (encoding == nullptr)
			goto error;
	}
	else if (encoding != nullptr) {
		self->encoding = alifUStr_fromString(encoding);
		if (self->encoding == nullptr)
			goto error;
	}
	else {
		//alifErr_setString(_alifExcOSError_,
			//"could not determine default encoding");
		goto error;
	}

	codeAlifInfo = _alifCodec_lookupTextEncoding(encoding, "codecs.open()");
	if (codeAlifInfo == nullptr) {
		ALIF_CLEAR(self->encoding);
		goto error;
	}

	self->errors = ALIF_NEWREF(errors);
	self->chunkSize = 8192;
	self->line_buffering = line_buffering;
	self->write_through = write_through;
	if (set_newLine(self, newline) < 0) {
		goto error;
	}

	self->buffer = ALIF_NEWREF(buffer);

	_PyIO_State* state = findIO_stateByDef(ALIF_TYPE(self));
	self->state = state;
	if (textIOWrapper_setDecoder(self, codeAlifInfo, errorsStr) != 0)
		goto error;

	if (textIOWrapper_setEncoder(self, codeAlifInfo, errorsStr) != 0)
		goto error;

	ALIF_CLEAR(codeAlifInfo);

	if (ALIF_IS_TYPE(buffer, state->alifBufferedReaderType) or
		ALIF_IS_TYPE(buffer, state->alifBufferedWriterType) or
		ALIF_IS_TYPE(buffer, state->alifBufferedRandomType))
	{
		if (alifObject_getOptionalAttr(buffer, &ALIF_ID(raw), &raw) < 0)
			goto error;
		if (raw != nullptr) {
			if (ALIF_IS_TYPE(raw, state->PyFileIO_Type))
				self->raw = raw;
			else
				ALIF_DECREF(raw);
		}
	}

	res = alifObject_callMethodNoArgs(buffer, &ALIF_ID(seekable));
	if (res == nullptr)
		goto error;
	r = alifObject_isTrue(res);
	ALIF_DECREF(res);
	if (r < 0)
		goto error;
	self->seekable = self->telling = r;

	r = alifObject_hasAttrWithError(buffer, &ALIF_ID(read1));
	if (r < 0) {
		goto error;
	}
	self->has_read1 = r;

	self->encodingStartOfStream = 0;
	if (textIOWrapper_fixEncoderState(self) < 0) {
		goto error;
	}

	self->ok = 1;
	return 0;

error:
	ALIF_XDECREF(codeAlifInfo);
	return -1;
}
