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


#include "clinic/TextIO.cpp.h" // 33













static AlifMethodDef _textIOBaseMethods_[] = { // 185
	{nullptr, nullptr}
};

static AlifGetSetDef _textIOBaseGetSet_[] = { // 193
	{nullptr}
};


static AlifTypeSlot _textIOBaseSlots_[] = { // 200
	{ALIF_TP_METHODS, _textIOBaseMethods_},
	{ALIF_TP_GETSET, _textIOBaseGetSet_},
	{0, nullptr},
};


AlifTypeSpec _textIOBaseSpec_ = { // 208
	.name = "تبادل.قاعدة_نص",
	.flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
			  ALIF_TPFLAGS_IMMUTABLETYPE),
	.slots = _textIOBaseSlots_,
};
















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
	char lineBuffering;
	char writeThrough;
	char readuniversal;
	char readtranslate;
	char writetranslate;
	char seekable;
	char hasRead1;
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




static AlifIntT validate_newline(const char* newline) { // 830
	if (newline and newline[0] != '\0'
		and !(newline[0] == '\n' && newline[1] == '\0')
		and !(newline[0] == '\r' && newline[1] == '\0')
		and !(newline[0] == '\r' && newline[1] == '\n' && newline[2] == '\0')) {
		//alifErr_format(_alifExcValueError_,
		//	"illegal newline value: %s", newline);
		return -1;
	}
	return 0;
}


static AlifIntT set_newline(TextIO* self, const char* newline) { // 844
	AlifObject* old = self->readnl;
	if (newline == nullptr) {
		self->readnl = nullptr;
	}
	else {
		self->readnl = alifUStr_fromString(newline);
		if (self->readnl == nullptr) {
			self->readnl = old;
			return -1;
		}
	}
	self->readuniversal = (newline == nullptr or newline[0] == '\0');
	self->readtranslate = (newline == nullptr);
	self->writetranslate = (newline == nullptr or newline[0] != '\0');
	if (!self->readuniversal && self->readnl != nullptr) {
		// validate_newline() accepts only ASCII newlines.
		self->writenl = (const char*)ALIFUSTR_1BYTE_DATA(self->readnl);
		if (strcmp(self->writenl, "\n") == 0) {
			self->writenl = nullptr;
		}
	}
	else {
#ifdef _WINDOWS
		self->writenl = "\r\n";
#else
		self->writenl = nullptr;
#endif
	}
	ALIF_XDECREF(old);
	return 0;
}


static AlifIntT _ioTextIOWrapper___init__Impl(TextIO* _self, AlifObject* _buffer,
	const char* _encoding, AlifObject* _errors,
	const char* _newLine, AlifIntT _lineBuffering,
	AlifIntT _writeThrough) { // 1088
	AlifObject* raw{}, * codecInfo = nullptr;
	AlifObject* res{};
	AlifIntT r{};

	_self->ok = 0;
	_self->detached = 0;

	if (_encoding == nullptr) {
		AlifInterpreter* interp = _alifInterpreter_get();
		//if (alifInterpreter_getConfig(interp)->warnDefaultEncoding) {
		//	//if (alifErr_warnEx(_alifExcEncodingWarning_,
		//		//"'encoding' argument not specified", 1)) {
		//		//return -1;
		//	//}
		//}
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
	//else if (io_checkErrors(_errors)) {
	//	return -1;
	//}
	//const char* errorsStr = _alifUStr_asUTF8NoNUL(_errors);
	//if (errorsStr == nullptr) {
	//	return -1;
	//}
	if (validate_newline(_newLine) < 0) {
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

	if (_encoding == nullptr /*and _alifDureRun_.preConfig.utf8Mode*/) {
		_self->encoding = &ALIF_STR(utf_8);
	}
	//else if (_encoding == nullptr or (strcmp(_encoding, "locale") == 0)) {
	//	_self->encoding = _alif_getLocaleEncodingObject();
	//	if (_self->encoding == nullptr) {
	//		goto error;
	//	}
	//}

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

	//codecInfo = _alifCodec_lookupTextEncoding(_encoding, "codecs.open()");
	//if (codecInfo == nullptr) {
	//	ALIF_CLEAR(_self->encoding);
	//	goto error;
	//}

	_self->errors = ALIF_NEWREF(_errors);
	_self->chunkSize = 8192;
	_self->lineBuffering = _lineBuffering;
	_self->writeThrough = _writeThrough;
	if (set_newline(_self, _newLine) < 0) {
		goto error;
	}

	_self->buffer = ALIF_NEWREF(_buffer);

	AlifIOState* state; state = findIOState_byDef(ALIF_TYPE(_self));
	_self->state = state;
	//if (_textIOWrapper_setDecoder(_self, codecInfo, errorsStr) != 0)
	//	goto error;

	//if (_textIOWrapper_setEncoder(_self, codecInfo, errorsStr) != 0)
	//	goto error;

	ALIF_CLEAR(codecInfo);

	if (ALIF_IS_TYPE(_buffer, state->alifBufferedReaderType) or
		ALIF_IS_TYPE(_buffer, state->alifBufferedWriterType) or
		ALIF_IS_TYPE(_buffer, state->alifBufferedRandomType))
	{
		if (alifObject_getOptionalAttr(_buffer, &ALIF_STR(Raw), &raw) < 0)
			goto error;
		if (raw != nullptr) {
			if (ALIF_IS_TYPE(raw, state->alifFileIOType))
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
	_self->hasRead1 = r;

	_self->encodingStartOfStream = 0;
	//if (_textIOWrapper_fixEncoderState(_self) < 0) {
	//	goto error;
	//}

	_self->ok = 1;
	return 0;

error:
	ALIF_XDECREF(codecInfo);
	return -1;
}







static AlifIntT textIOWrapper_traverse(TextIO* self, VisitProc visit, void* arg) { // 1470
	ALIF_VISIT(ALIF_TYPE(self));
	ALIF_VISIT(self->buffer);
	ALIF_VISIT(self->encoding);
	ALIF_VISIT(self->encoder);
	ALIF_VISIT(self->decoder);
	ALIF_VISIT(self->readnl);
	ALIF_VISIT(self->decodedChars);
	ALIF_VISIT(self->pendingBytes);
	ALIF_VISIT(self->snapshot);
	ALIF_VISIT(self->errors);
	ALIF_VISIT(self->raw);

	ALIF_VISIT(self->dict);
	return 0;
}










static AlifMemberDef _textIOWrapperMembers_[] = { // 3358
	{"encoding", ALIF_T_OBJECT, offsetof(TextIO, encoding), ALIF_READONLY},
	{"buffer", ALIF_T_OBJECT, offsetof(TextIO, buffer), ALIF_READONLY},
	{"line_buffering", ALIF_T_BOOL, offsetof(TextIO, lineBuffering), ALIF_READONLY},
	{"__weakListOffset__", ALIF_T_ALIFSIZET, offsetof(TextIO, weakRefList), ALIF_READONLY},
	{"__dictOffset__", ALIF_T_ALIFSIZET, offsetof(TextIO, dict), ALIF_READONLY},
	{nullptr}
};







AlifTypeSlot _textIOWrapperSlots_[] = { // 3380
	{ALIF_TP_TRAVERSE, textIOWrapper_traverse},
	{ALIF_TP_MEMBERS, _textIOWrapperMembers_},
	{ALIF_TP_INIT, _ioTextIOWrapper___init__},
	{0, nullptr},
};


AlifTypeSpec _textIOWrapperSpec_ = { // 3394
	.name = "تبادل.غلاف_النص",
	.basicsize = sizeof(TextIO),
	.flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC |
			  ALIF_TPFLAGS_IMMUTABLETYPE),
	.slots = _textIOWrapperSlots_,
};
