#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Codecs.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_Long.h"
#include "AlifCore_Object.h"
#include "AlifCore_Errors.h"
#include "AlifCore_State.h"

#include "_IOModule.h"









typedef class NLDecoderObject NLDecoderObject;
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


class NLDecoderObject { // 217
public:
	ALIFOBJECT_HEAD;
	AlifObject* decoder{};
	AlifObject* errors{};
	AlifUIntT pendingcr : 1;
	AlifUIntT translate : 1;
	AlifUIntT seennl : 3;
};



static AlifIntT _ioIncrementalNewlineDecoder___init__Impl(NLDecoderObject* _self,
	AlifObject* _decoder, AlifIntT _translate, AlifObject* _errors) { // 242

	if (_errors == nullptr) {
		_errors = &ALIF_ID(strict);
	}
	else {
		_errors = ALIF_NEWREF(_errors);
	}

	ALIF_XSETREF(_self->errors, _errors);
	ALIF_XSETREF(_self->decoder, ALIF_NEWREF(_decoder));
	_self->translate = _translate ? 1 : 0;
	_self->seennl = 0;
	_self->pendingcr = 0;

	return 0;
}


static AlifIntT check_decoded(AlifObject* _decoded) { // 293
	if (_decoded == nullptr)
		return -1;
	if (!ALIFUSTR_CHECK(_decoded)) {
		alifErr_format(_alifExcTypeError_,
			"decoder should return a string result, not '%.200s'",
			ALIF_TYPE(_decoded)->name);
		ALIF_DECREF(_decoded);
		return -1;
	}
	return 0;
}


#define CHECK_INITIALIZED_DECODER(self) \
    if (self->errors == nullptr) { \
        alifErr_setString(_alifExcValueError_, \
                        "IncrementalNewlineDecoder.__init__() not called"); \
        return nullptr; \
    }

#define SEEN_CR   1
#define SEEN_LF   2
#define SEEN_CRLF 4
#define SEEN_ALL (SEEN_CR | SEEN_LF | SEEN_CRLF)

AlifObject* _alifIncrementalNewlineDecoder_decode(AlifObject* _mySelf,
	AlifObject* _input, AlifIntT _final) { // 320
	AlifObject* output{};
	AlifSizeT outputLen{};
	NLDecoderObject* self = (NLDecoderObject*)_mySelf;

	//* alif //* todo
	AlifBuffer* data = (AlifBuffer*)alifMem_dataAlloc(sizeof(AlifBuffer));
	AlifSizeT consumed = data->len;
	if (alifObject_getBuffer(_input, data, ALIFBUF_SIMPLE) != 0) {
		return nullptr;
	}
	output = alifUStr_decodeUTF8Stateful((const char*)data->buf, data->len,
		nullptr, _final ? nullptr : &consumed);
	//* alif

	//CHECK_INITIALIZED_DECODER(self);

	/* decode input (with the eventual \r from a previous pass) */
	//if (self->decoder != ALIF_NONE) {
	//	output = alifObject_callMethodObjArgs(self->decoder,
	//		&ALIF_ID(Decode), _input, _final ? ALIF_TRUE : ALIF_FALSE, nullptr);
	//}
	//else {
		//output = ALIF_NEWREF(_input);
	//}

	if (check_decoded(output) < 0)
		return nullptr;

	outputLen = ALIFUSTR_GET_LENGTH(output);
	//if (self->pendingcr and (_final or outputLen > 0)) {
	//	AlifIntT kind{};
	//	AlifObject* modified{};
	//	char* out{};

	//	modified = alifUStr_new(outputLen + 1,
	//		ALIFUSTR_MAX_CHAR_VALUE(output));
	//	if (modified == nullptr)
	//		goto error;
	//	kind = ALIFUSTR_KIND(modified);
	//	out = (char*)ALIFUSTR_DATA(modified);
	//	ALIFUSTR_WRITE(kind, out, 0, '\r');
	//	memcpy(out + kind, ALIFUSTR_DATA(output), kind * outputLen);
	//	ALIF_SETREF(output, modified); /* output remains ready */
	//	self->pendingcr = 0;
	//	outputLen++;
	//}

	if (!_final) {
		if (outputLen > 0
			and ALIFUSTR_READ_CHAR(output, outputLen - 1) == '\r') {
			AlifObject* modified = alifUStr_subString(output, 0, outputLen - 1);
			if (modified == nullptr)
				goto error;
			ALIF_SETREF(output, modified);
			self->pendingcr = 1;
		}
	}

	{
		const void* inStr{};
		AlifSizeT len{};
		AlifIntT seennl = self->seennl;
		AlifIntT onlyIf = 0;
		AlifIntT kind{};

		inStr = ALIFUSTR_DATA(output);
		len = ALIFUSTR_GET_LENGTH(output);
		kind = ALIFUSTR_KIND(output);

		if (len == 0)
			return output;
		if (seennl == SEEN_LF or seennl == 0) {
			onlyIf = (memchr(inStr, '\r', kind * len) == nullptr);
		}

		if (onlyIf) {
			if (seennl == 0 and
				memchr(inStr, '\n', kind * len) != nullptr) {
				if (kind == AlifUStr_1Byte_Kind)
					seennl |= SEEN_LF;
				else {
					AlifSizeT i = 0;
					for (;;) {
						AlifUCS4 c;
						while (ALIFUSTR_READ(kind, inStr, i) > '\n')
							i++;
						c = ALIFUSTR_READ(kind, inStr, i++);
						if (c == '\n') {
							seennl |= SEEN_LF;
							break;
						}
						if (i >= len)
							break;
					}
				}
			}
			/* Finished: we have scanned for newlines, and none of them
			   need translating */
		}
		else if (!self->translate) {
			AlifSizeT i = 0;
			/* We have already seen all newline types, no need to scan again */
			if (seennl == SEEN_ALL)
				goto endScan;
			for (;;) {
				AlifUCS4 c;
				/* Fast loop for non-control characters */
				while (ALIFUSTR_READ(kind, inStr, i) > '\r')
					i++;
				c = ALIFUSTR_READ(kind, inStr, i++);
				if (c == '\n')
					seennl |= SEEN_LF;
				else if (c == '\r') {
					if (ALIFUSTR_READ(kind, inStr, i) == '\n') {
						seennl |= SEEN_CRLF;
						i++;
					}
					else
						seennl |= SEEN_CR;
				}
				if (i >= len)
					break;
				if (seennl == SEEN_ALL)
					break;
			}
endScan:
			;
		}
		else {
			void* translated{};
			AlifIntT kind = ALIFUSTR_KIND(output);
			const void* inStr = ALIFUSTR_DATA(output);
			AlifSizeT in{}, out{};

			translated = alifMem_dataAlloc(kind * len);
			if (translated == nullptr) {
				//alifErr_noMemory();
				goto error;
			}
			in = out = 0;
			for (;;) {
				AlifUCS4 c;
				/* Fast loop for non-control characters */
				while ((c = ALIFUSTR_READ(kind, inStr, in++)) > '\r')
					ALIFUSTR_WRITE(kind, translated, out++, c);
				if (c == '\n') {
					ALIFUSTR_WRITE(kind, translated, out++, c);
					seennl |= SEEN_LF;
					continue;
				}
				if (c == '\r') {
					if (ALIFUSTR_READ(kind, inStr, in) == '\n') {
						in++;
						seennl |= SEEN_CRLF;
					}
					else
						seennl |= SEEN_CR;
					ALIFUSTR_WRITE(kind, translated, out++, '\n');
					continue;
				}
				if (in > len)
					break;
				ALIFUSTR_WRITE(kind, translated, out++, c);
			}
			ALIF_DECREF(output);
			output = alifUStr_fromKindAndData(kind, translated, out);
			alifMem_dataFree(translated);
			if (!output)
				return nullptr;
		}
		self->seennl |= seennl;
	}

	return output;

error:
	ALIF_DECREF(output);
	return nullptr;
}

static AlifObject* _io_incrementalNewlineDecoderDecodeImpl(NLDecoderObject* _self,
	AlifObject* _input, AlifIntT _final) { // 521
	return _alifIncrementalNewlineDecoder_decode((AlifObject*)_self, _input, _final);
}


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

static void textIOWrapper_setDecodedChars(TextIO*, AlifObject*); // 720


class EncodeFunCentry { // 812
public:
	const char* name{};
	EncodeFuncT encodeFunc{};
};

static const EncodeFunCentry encodefuncs[] = { // 817
	//{"ascii",       (EncodeFuncT)ascii_encode},
	//{"iso8859-1",   (EncodeFuncT)latin1_encode},
	//{"utf-8",       (EncodeFuncT)utf8_encode},
	//{"utf-16-be",   (EncodeFuncT)utf16be_encode},
	//{"utf-16-le",   (EncodeFuncT)utf16le_encode},
	//{"utf-16",      (EncodeFuncT)utf16_encode},
	//{"utf-32-be",   (EncodeFuncT)utf32be_encode},
	//{"utf-32-le",   (EncodeFuncT)utf32le_encode},
	//{"utf-32",      (EncodeFuncT)utf32_encode},
	{nullptr, nullptr}
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


static AlifIntT _textIOWrapper_setDecoder(TextIO* _self, AlifObject* _codecInfo,
	const char* _errors) { // 880
	AlifObject* res{};
	AlifIntT r{};

	res = alifObject_callMethodNoArgs(_self->buffer, &ALIF_ID(Readable));
	if (res == nullptr)
		return -1;

	r = alifObject_isTrue(res);
	ALIF_DECREF(res);
	if (r == -1)
		return -1;

	if (r != 1)
		return 0;

	ALIF_CLEAR(_self->decoder);
	//_self->decoder = _alifCodecInfo_getIncrementalDecoder(_codecInfo, _errors);
	//if (_self->decoder == nullptr)
	//	return -1;

	//if (_self->readuniversal) {
	//	AlifIOState* state = _self->state;
	//	AlifObject* incrementalDecoder = alifObject_callFunctionObjArgs(
	//		(AlifObject*)state->alifIncrementalNewlineDecoderType,
	//		_self->decoder, _self->readtranslate ? ALIF_TRUE : ALIF_FALSE, nullptr);
	//	if (incrementalDecoder == nullptr)
	//		return -1;
	//	ALIF_XSETREF(_self->decoder, incrementalDecoder);
	//}

	return 0;
}


static AlifObject* _textIOWrapper_decode(AlifIOState* state,
	AlifObject* decoder, AlifObject* bytes, AlifIntT eof) { // 917
	AlifObject* chars{};

	//if (ALIF_IS_TYPE(decoder, state->alifIncrementalNewlineDecoderType))
	decoder = (AlifObject*)state->alifIncrementalNewlineDecoderType; //* delete //* alif //* review //* todo
	chars = _alifIncrementalNewlineDecoder_decode(decoder, bytes, eof);
	//else
	//	chars = alifObject_callMethodObjArgs(decoder, &ALIF_ID(Decode), bytes,
	//		eof ? ALIF_TRUE : ALIF_FALSE, nullptr);

	if (check_decoded(chars) < 0)
		// check_decoded already decreases refcount
		return nullptr;

	return chars;
}

static AlifIntT _textIOWrapper_setEncoder(TextIO* self, AlifObject* codec_info,
	const char* errors) { // 936
	AlifObject* res{};
	AlifIntT r{};

	res = alifObject_callMethodNoArgs(self->buffer, &ALIF_ID(Writable));
	if (res == nullptr)
		return -1;

	r = alifObject_isTrue(res);
	ALIF_DECREF(res);
	if (r == -1)
		return -1;

	if (r != 1)
		return 0;

	ALIF_CLEAR(self->encoder);
	self->encodeFunc = nullptr;
	self->encoder = _alifCodecInfo_getIncrementalEncoder(codec_info, errors);
	if (self->encoder == nullptr)
		return -1;

	/* Get the normalized named of the codec */
	//if (alifObject_getOptionalAttr(codec_info, &ALIF_STR(Name), &res) < 0) {
	//	return -1;
	//}
	if (res != nullptr and ALIFUSTR_CHECK(res)) {
		const EncodeFunCentry* e = encodefuncs;
		while (e->name != nullptr) {
			if (alifUStr_equalToASCIIString(res, e->name)) {
				self->encodeFunc = e->encodeFunc;
				break;
			}
			e++;
		}
	}
	ALIF_XDECREF(res);

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
	const char* errorsStr = _alifUStr_asUTF8NoNUL(_errors);
	if (errorsStr == nullptr) {
		return -1;
	}
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

	if (_encoding == nullptr and _alifRuntime_.preConfig.utf8Mode) {
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

	codecInfo = _alifCodec_lookupTextEncoding(_encoding, "codecs.open()");
	if (codecInfo == nullptr) {
		ALIF_CLEAR(_self->encoding);
		goto error;
	}

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
	if (_textIOWrapper_setDecoder(_self, codecInfo, errorsStr) != 0)
		goto error;

	if (_textIOWrapper_setEncoder(_self, codecInfo, errorsStr) != 0)
		goto error;

	ALIF_CLEAR(codecInfo);

	if (ALIF_IS_TYPE(_buffer, state->alifBufferedReaderType) or
		ALIF_IS_TYPE(_buffer, state->alifBufferedWriterType) or
		ALIF_IS_TYPE(_buffer, state->alifBufferedRandomType)) {
		if (alifObject_getOptionalAttr(_buffer, &ALIF_STR(Raw), &raw) < 0)
			goto error;
		if (raw != nullptr) {
			if (ALIF_IS_TYPE(raw, state->alifFileIOType))
				_self->raw = raw;
			else
				ALIF_DECREF(raw);
		}
	}

	res = alifObject_callMethodNoArgs(_buffer, &ALIF_ID(Seekable));
	if (res == nullptr)
		goto error;
	r = alifObject_isTrue(res);
	ALIF_DECREF(res);
	if (r < 0)
		goto error;
	_self->seekable = _self->telling = r;

	r = alifObject_hasAttrWithError(_buffer, &ALIF_ID(Read1));
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

static AlifObject* _ioTextIOWrapper_closedGetImpl(TextIO*); // 1489

#define CHECK_CLOSED(self) \
    do { \
        AlifIntT r{}; \
        AlifObject *_res; \
        if (ALIF_IS_TYPE(self, self->state->alifTextIOWrapperType)) { \
            if (self->raw != nullptr) \
                r = _alifFileIO_closed(self->raw); \
            else { \
                _res = _ioTextIOWrapper_closedGetImpl(self); \
                if (_res == nullptr) \
                    return nullptr; \
                r = alifObject_isTrue(_res); \
                ALIF_DECREF(_res); \
                if (r < 0) \
                    return nullptr; \
            } \
            if (r > 0) { \
                alifErr_setString(_alifExcValueError_, \
                                "عملية تبادل على ملف مغلق"); \
                return nullptr; \
            } \
        } \
        else if (_alifIOBase_checkClosed((AlifObject *)self, ALIF_TRUE) == nullptr) \
            return nullptr; \
    } while (0)






static AlifObject* _ioTextIOWrapper_writeImpl(TextIO* self, AlifObject* text) { // 1645
	AlifObject* ret{};
	AlifObject* b{};
	AlifSizeT textlen{};
	AlifIntT haslf = 0;
	AlifIntT needflush = 0, text_needflush = 0;

	//CHECK_ATTACHED(self);
	CHECK_CLOSED(self);

	if (self->encoder == nullptr) {
		//return _unsupported(self->state, "not writable");
	}

	ALIF_INCREF(text);

	textlen = ALIFUSTR_GET_LENGTH(text);

	if ((self->writetranslate and self->writenl != nullptr) or self->lineBuffering)
		if (alifUStr_findChar(text, '\n', 0, ALIFUSTR_GET_LENGTH(text), 1) != -1)
			haslf = 1;

	if (haslf and self->writetranslate and self->writenl != nullptr) {
		AlifObject* newtext = _alifObject_callMethod(text, &ALIF_ID(Replace),
			"ss", "\n", self->writenl);
		ALIF_DECREF(text);
		if (newtext == nullptr)
			return nullptr;
		text = newtext;
	}

	if (self->writeThrough)
		text_needflush = 1;
	if (self->lineBuffering and
		(haslf or
			alifUStr_findChar(text, '\r', 0, ALIFUSTR_GET_LENGTH(text), 1) != -1))
		needflush = 1;

	/* XXX What if we were just reading? */
	if (self->encodeFunc != nullptr) {
		if (ALIFUSTR_IS_ASCII(text) and
			ALIFUSTR_GET_LENGTH(text) <= self->chunkSize /*and
			is_asciiCompatEncoding(self->encodeFunc)*/) {
			b = ALIF_NEWREF(text);
		}
		else {
			b = (*self->encodeFunc)((AlifObject*)self, text);
		}
		self->encodingStartOfStream = 0;
	}
	else {
		b = alifObject_callMethodOneArg(self->encoder, &ALIF_ID(Encode), text);
	}

	ALIF_DECREF(text);
	if (b == nullptr)
		return nullptr;
	if (b != text and !ALIFBYTES_CHECK(b)) {
		alifErr_format(_alifExcTypeError_,
			"encoder should return a bytes object, not '%.200s'",
			ALIF_TYPE(b)->name);
		ALIF_DECREF(b);
		return nullptr;
	}

	AlifSizeT bytes_len{};
	if (b == text) {
		bytes_len = ALIFUSTR_GET_LENGTH(b);
	}
	else {
		bytes_len = ALIFBYTES_GET_SIZE(b);
	}

	if (bytes_len >= self->chunkSize) {
		while (self->pendingBytes != nullptr) {
			//if (_textIOWrapper_writeFlush(self) < 0) {
			//	ALIF_DECREF(b);
			//	return nullptr;
			//}
		}
	}

	if (self->pendingBytes == nullptr) {
		self->pendingBytes = b;
	}
	else if (!ALIFLIST_CHECKEXACT(self->pendingBytes)) {
		AlifObject* list = alifList_new(2);
		if (list == nullptr) {
			ALIF_DECREF(b);
			return nullptr;
		}
		ALIFLIST_SET_ITEM(list, 0, self->pendingBytes);
		ALIFLIST_SET_ITEM(list, 1, b);
		self->pendingBytes = list;
	}
	else {
		if (alifList_append(self->pendingBytes, b) < 0) {
			ALIF_DECREF(b);
			return nullptr;
		}
		ALIF_DECREF(b);
	}

	self->pendingBytesCount += bytes_len;
	if (self->pendingBytesCount >= self->chunkSize or needflush or
		text_needflush) {
		//if (_textIOWrapper_writeFlush(self) < 0)
		//	return nullptr;
	}

	if (needflush) {
		//if (_alifFile_flush(self->buffer) < 0) {
		//	return nullptr;
		//}
	}

	if (self->snapshot != nullptr) {
		textIOWrapper_setDecodedChars(self, nullptr);
		ALIF_CLEAR(self->snapshot);
	}

	if (self->decoder) {
		ret = alifObject_callMethodNoArgs(self->decoder, &ALIF_ID(Reset));
		if (ret == nullptr)
			return nullptr;
		ALIF_DECREF(ret);
	}

	return alifLong_fromSizeT(textlen);
}




static void textIOWrapper_setDecodedChars(TextIO* _self, AlifObject* _chars) { // 1795
	ALIF_XSETREF(_self->decodedChars, _chars);
	_self->decodedCharsUsed = 0;
}

static AlifObject* textIOWrapper_getDecodedChars(TextIO* _self, AlifSizeT _n) { // 1802
	AlifObject* chars{};
	AlifSizeT avail{};

	if (_self->decodedChars == nullptr)
		return alifUStr_fromStringAndSize(nullptr, 0);

	/* decodedChars is guaranteed to be "ready". */
	avail = (ALIFUSTR_GET_LENGTH(_self->decodedChars)
		- _self->decodedCharsUsed);

	if (_n < 0 or _n > avail)
		_n = avail;

	if (_self->decodedCharsUsed > 0 or _n < avail) {
		chars = alifUStr_subString(_self->decodedChars,
			_self->decodedCharsUsed,
			_self->decodedCharsUsed + _n);
		if (chars == nullptr)
			return nullptr;
	}
	else {
		chars = ALIF_NEWREF(_self->decodedChars);
	}

	_self->decodedCharsUsed += _n;
	return chars;
}

static AlifIntT textIOWrapper_readChunk(TextIO* self, AlifSizeT size_hint) { // 1837
	AlifObject* decBuffer = nullptr;
	AlifObject* decFlags = nullptr;
	AlifObject* inputChunk = nullptr;
	AlifBuffer inputChunkBuf{};
	AlifObject* decodedChars{}, * chunkSize{};
	AlifSizeT nbytes{}, nchars{};
	AlifIntT eof{};

	//if (self->decoder == nullptr) {
	//	_unsupported(self->state, "not readable");
	//	return -1;
	//}

	if (self->telling) {
		//AlifObject* state = alifObject_callMethodNoArgs(self->decoder,
		//	&ALIF_ID(GetState));
		//if (state == nullptr)
		//	return -1;
		//if (!ALIFTUPLE_CHECK(state)) {
		//	alifErr_setString(_alifExcTypeError_,
		//		"illegal decoder state");
		//	ALIF_DECREF(state);
		//	return -1;
		//}
		//if (!alifArg_parseTuple(state,
		//	"OO;illegal decoder state", &decBuffer, &decFlags))
		//{
		//	ALIF_DECREF(state);
		//	return -1;
		//}

		//if (!ALIFBYTES_CHECK(decBuffer)) {
		//	alifErr_format(_alifExcTypeError_,
		//		"illegal decoder state: the first item should be a "
		//		"bytes object, not '%.200s'",
		//		ALIF_TYPE(decBuffer)->name);
		//	ALIF_DECREF(state);
		//	return -1;
		//}
		//ALIF_INCREF(decBuffer);
		//ALIF_INCREF(decFlags);
		//ALIF_DECREF(state);
	}

	/* Read a chunk, decode it, and put the result in self._decoded_chars. */
	if (size_hint > 0) {
		size_hint = (AlifSizeT)(ALIF_MAX(self->b2cratIO, 1.0) * size_hint);
	}
	chunkSize = alifLong_fromSizeT(ALIF_MAX(self->chunkSize, size_hint));
	if (chunkSize == nullptr)
		goto fail;

	inputChunk = alifObject_callMethodOneArg(self->buffer,
		(self->hasRead1 ? &ALIF_ID(Read1) : &ALIF_ID(Read)),
		chunkSize);
	ALIF_DECREF(chunkSize);
	if (inputChunk == nullptr)
		goto fail;

	if (alifObject_getBuffer(inputChunk, &inputChunkBuf, 0) != 0) {
		alifErr_format(_alifExcTypeError_,
			"underlying %s() should have returned a bytes-like object, "
			"not '%.200s'", (self->hasRead1 ? "read1" : "read"),
			ALIF_TYPE(inputChunk)->name);
		goto fail;
	}

	nbytes = inputChunkBuf.len;
	eof = (nbytes == 0);

	decodedChars = _textIOWrapper_decode(self->state, self->decoder,
		inputChunk, eof);
	alifBuffer_release(&inputChunkBuf);
	if (decodedChars == nullptr)
		goto fail;

	textIOWrapper_setDecodedChars(self, decodedChars);
	nchars = ALIFUSTR_GET_LENGTH(decodedChars);
	if (nchars > 0)
		self->b2cratIO = (double)nbytes / nchars;
	else
		self->b2cratIO = 0.0;
	if (nchars > 0)
		eof = 0;

	//if (self->telling) {
	//	AlifObject* next_input = decBuffer;
	//	alifBytes_concat(&next_input, inputChunk);
	//	decBuffer = nullptr;
	//	if (next_input == nullptr) {
	//		goto fail;
	//	}
	//	AlifObject* snapshot = alif_buildValue("NN", decFlags, next_input);
	//	if (snapshot == nullptr) {
	//		decFlags = nullptr;
	//		goto fail;
	//	}
	//	ALIF_XSETREF(self->snapshot, snapshot);
	//}
	ALIF_DECREF(inputChunk);

	return (eof == 0);

fail:
	ALIF_XDECREF(decBuffer);
	ALIF_XDECREF(decFlags);
	ALIF_XDECREF(inputChunk);
	return -1;
}

static AlifObject* _ioTextIOWrapper_readImpl(TextIO* self, AlifSizeT n) { // 1972
	AlifObject* result = nullptr, * chunks = nullptr;

	//CHECK_ATTACHED(self);
	CHECK_CLOSED(self);

	//if (self->decoder == nullptr) {
	//	return _unsupported(self->state, "not readable");
	//}

	//if (_textIOWrapper_writeFlush(self) < 0)
	//	return nullptr;

	if (n < 0) {
		/* Read everything */
		AlifObject* bytes = alifObject_callMethodNoArgs(self->buffer, &ALIF_STR(Read));
		AlifObject* decoded;
		if (bytes == nullptr)
			goto fail;

		AlifIOState* state = self->state;
		//if (ALIF_IS_TYPE(self->decoder, state->alifIncrementalNewlineDecoderType))
		self->decoder = (AlifObject*)state->alifIncrementalNewlineDecoderType; //* alif //* delete
		decoded = _alifIncrementalNewlineDecoder_decode(self->decoder,
			bytes, 1);
		//else
		//	decoded = alifObject_callMethodObjArgs(
		//		self->decoder, &ALIF_ID(Decode), bytes, ALIF_TRUE, nullptr);
		ALIF_DECREF(bytes);
		if (check_decoded(decoded) < 0)
			goto fail;

		result = textIOWrapper_getDecodedChars(self, -1);

		if (result == nullptr) {
			ALIF_DECREF(decoded);
			return nullptr;
		}

		alifUStr_appendAndDel(&result, decoded);
		if (result == nullptr)
			goto fail;

		if (self->snapshot != nullptr) {
			textIOWrapper_setDecodedChars(self, nullptr);
			ALIF_CLEAR(self->snapshot);
		}
		return result;
	}
	else {
		AlifIntT res = 1;
		AlifSizeT remaining = n;

		result = textIOWrapper_getDecodedChars(self, n);
		if (result == nullptr)
			goto fail;
		remaining -= ALIFUSTR_GET_LENGTH(result);

		while (remaining > 0) {
			res = textIOWrapper_readChunk(self, remaining);
			if (res < 0) {
				if (_alifIO_trapEintr()) {
					continue;
				}
				goto fail;
			}
			if (res == 0)  /* EOF */
				break;
			if (chunks == nullptr) {
				chunks = alifList_new(0);
				if (chunks == nullptr)
					goto fail;
			}
			if (ALIFUSTR_GET_LENGTH(result) > 0 and
				alifList_append(chunks, result) < 0)
				goto fail;
			ALIF_DECREF(result);
			result = textIOWrapper_getDecodedChars(self, remaining);
			if (result == nullptr)
				goto fail;
			remaining -= ALIFUSTR_GET_LENGTH(result);
		}
		if (chunks != nullptr) {
			if (result != nullptr and alifList_append(chunks, result) < 0)
				goto fail;
			ALIF_XSETREF(result, alifUStr_join(&ALIF_STR(Empty), chunks));
			if (result == nullptr)
				goto fail;
			ALIF_CLEAR(chunks);
		}
		return result;
	}
fail:
	ALIF_XDECREF(result);
	ALIF_XDECREF(chunks);
	return nullptr;
}


static const char* find_controlChar(AlifIntT kind,
	const char* s, const char* end, AlifUCS4 ch) { // 2080
	if (kind == AlifUStrKind_::AlifUStr_1Byte_Kind) {
		return (char*)memchr((const void*)s, (char)ch, end - s);
	}
	for (;;) {
		while (ALIFUSTR_READ(kind, s, 0) > ch)
			s += kind;
		if (ALIFUSTR_READ(kind, s, 0) == ch)
			return s;
		if (s == end)
			return nullptr;
		s += kind;
	}
}

AlifSizeT _alifIO_findLineEnding(AlifIntT translated,
	AlifIntT universal, AlifObject* readnl,
	AlifIntT kind, const char* start,
	const char* end, AlifSizeT* consumed) { // 2098
	AlifSizeT len = (end - start) / kind;

	if (translated) {
		const char* pos = find_controlChar(kind, start, end, '\n');
		if (pos != nullptr)
			return (pos - start) / kind + 1;
		else {
			*consumed = len;
			return -1;
		}
	}
	else if (universal) {
		const char* s = start;
		for (;;) {
			AlifUCS4 ch;
			while (ALIFUSTR_READ(kind, s, 0) > '\r')
				s += kind;
			if (s >= end) {
				*consumed = len;
				return -1;
			}
			ch = ALIFUSTR_READ(kind, s, 0);
			s += kind;
			if (ch == '\n')
				return (s - start) / kind;
			if (ch == '\r') {
				if (ALIFUSTR_READ(kind, s, 0) == '\n')
					return (s - start) / kind + 1;
				else
					return (s - start) / kind;
			}
		}
	}
	else {
		/* Non-universal mode. */
		AlifSizeT readNLLen = ALIFUSTR_GET_LENGTH(readnl);
		const AlifUCS1* nl = ALIFUSTR_1BYTE_DATA(readnl);
		if (readNLLen == 1) {
			const char* pos = find_controlChar(kind, start, end, nl[0]);
			if (pos != nullptr)
				return (pos - start) / kind + 1;
			*consumed = len;
			return -1;
		}
		else {
			const char* s = start;
			const char* e = end - (readNLLen - 1) * kind;
			const char* pos;
			if (e < s)
				e = s;
			while (s < e) {
				AlifSizeT i{};
				const char* pos = find_controlChar(kind, s, end, nl[0]);
				if (pos == nullptr or pos >= e)
					break;
				for (i = 1; i < readNLLen; i++) {
					if (ALIFUSTR_READ(kind, pos, i) != nl[i])
						break;
				}
				if (i == readNLLen)
					return (pos - start) / kind + readNLLen;
				s = pos + kind;
			}
			pos = find_controlChar(kind, e, end, nl[0]);
			if (pos == nullptr)
				*consumed = len;
			else
				*consumed = (pos - start) / kind;
			return -1;
		}
	}
}


static AlifObject* _textIOWrapper_readline(TextIO* self, AlifSizeT limit) { // 2184
	AlifObject* line = nullptr, * chunks = nullptr, * remaining = nullptr;
	AlifSizeT start{}, endpos{}, chunked{}, offset_to_buffer{};
	AlifIntT res{};

	CHECK_CLOSED(self);

	//if (_textIOWrapper_writeFlush(self) < 0)
	//	return nullptr;

	chunked = 0;

	while (1) {
		const char* ptr{};
		AlifSizeT line_len{};
		AlifIntT kind{};
		AlifSizeT consumed = 0;

		/* First, get some data if necessary */
		res = 1;
		while (!self->decodedChars or
			!ALIFUSTR_GET_LENGTH(self->decodedChars)) {
			res = textIOWrapper_readChunk(self, 0);
			if (res < 0) {
				if (_alifIO_trapEintr()) {
					continue;
				}
				goto error;
			}
			if (res == 0)
				break;
		}
		if (res == 0) {
			/* end of file */
			textIOWrapper_setDecodedChars(self, nullptr);
			ALIF_CLEAR(self->snapshot);
			start = endpos = offset_to_buffer = 0;
			break;
		}

		if (remaining == nullptr) {
			line = ALIF_NEWREF(self->decodedChars);
			start = self->decodedCharsUsed;
			offset_to_buffer = 0;
		}
		else {
			line = alifUStr_concat(remaining, self->decodedChars);
			start = 0;
			offset_to_buffer = ALIFUSTR_GET_LENGTH(remaining);
			ALIF_CLEAR(remaining);
			if (line == nullptr)
				goto error;
		}

		ptr = (const char*)ALIFUSTR_DATA(line);
		line_len = ALIFUSTR_GET_LENGTH(line);
		kind = ALIFUSTR_KIND(line);

		endpos = _alifIO_findLineEnding(
			self->readtranslate, self->readuniversal, self->readnl,
			kind,
			ptr + kind * start,
			ptr + kind * line_len,
			&consumed);
		if (endpos >= 0) {
			endpos += start;
			if (limit >= 0 and (endpos - start) + chunked >= limit)
				endpos = start + limit - chunked;
			break;
		}

		/* We can put aside up to `endpos` */
		endpos = consumed + start;
		if (limit >= 0 and (endpos - start) + chunked >= limit) {
			endpos = start + limit - chunked;
			break;
		}

		if (endpos > start) {
			AlifObject* s{};
			if (chunks == nullptr) {
				chunks = alifList_new(0);
				if (chunks == nullptr)
					goto error;
			}
			s = alifUStr_subString(line, start, endpos);
			if (s == nullptr)
				goto error;
			if (alifList_append(chunks, s) < 0) {
				ALIF_DECREF(s);
				goto error;
			}
			chunked += ALIFUSTR_GET_LENGTH(s);
			ALIF_DECREF(s);
		}
		if (endpos < line_len) {
			remaining = alifUStr_subString(line, endpos, line_len);
			if (remaining == nullptr)
				goto error;
		}
		ALIF_CLEAR(line);
		/* We have consumed the buffer */
		textIOWrapper_setDecodedChars(self, nullptr);
	}

	if (line != nullptr) {
		self->decodedCharsUsed = endpos - offset_to_buffer;
		if (start > 0 or endpos < ALIFUSTR_GET_LENGTH(line)) {
			AlifObject* s = alifUStr_subString(line, start, endpos);
			ALIF_CLEAR(line);
			if (s == nullptr)
				goto error;
			line = s;
		}
	}
	if (remaining != nullptr) {
		if (chunks == nullptr) {
			chunks = alifList_new(0);
			if (chunks == nullptr)
				goto error;
		}
		if (alifList_append(chunks, remaining) < 0)
			goto error;
		ALIF_CLEAR(remaining);
	}
	if (chunks != nullptr) {
		if (line != nullptr) {
			if (alifList_append(chunks, line) < 0)
				goto error;
			ALIF_DECREF(line);
		}
		line = alifUStr_join(&ALIF_STR(Empty), chunks);
		if (line == nullptr)
			goto error;
		ALIF_CLEAR(chunks);
	}
	if (line == nullptr) {
		line = &ALIF_STR(Empty);
	}

	return line;

error:
	ALIF_XDECREF(chunks);
	ALIF_XDECREF(remaining);
	ALIF_XDECREF(line);
	return nullptr;
}

static AlifObject* _ioTextIOWrapper_readlineImpl(TextIO* self, AlifSizeT size) { // 2350
	//CHECK_ATTACHED(self);
	return _textIOWrapper_readline(self, size);
}



static AlifObject* _ioTextIOWrapper_closeImpl(TextIO* self) { // 3115
	AlifObject* res{};
	AlifIntT r{};
	//CHECK_ATTACHED(self);

	res = _ioTextIOWrapper_closedGetImpl(self);
	if (res == nullptr)
		return nullptr;
	r = alifObject_isTrue(res);
	ALIF_DECREF(res);
	if (r < 0)
		return nullptr;

	if (r > 0) {
		return ALIF_NONE; /* stream already closed */
	}
	else {
		AlifObject* exc = nullptr;
		if (self->finalizing) {
			res = alifObject_callMethodOneArg(self->buffer, &ALIF_ID(_deallocWarn),
				(AlifObject*)self);
			if (res) {
				ALIF_DECREF(res);
			}
			else {
				alifErr_clear();
			}
		}
		//if (_alifFile_flush((AlifObject*)self) < 0) {
		//	exc = alifErr_getRaisedException();
		//}

		res = alifObject_callMethodNoArgs(self->buffer, &ALIF_STR(Close));
		if (exc != nullptr) {
			_alifErr_chainExceptions1(exc);
			ALIF_CLEAR(res);
		}
		return res;
	}
}



static AlifObject* _ioTextIOWrapper_closedGetImpl(TextIO* self) { // 3217
	//CHECK_ATTACHED(self);
	return alifObject_getAttr(self->buffer, &ALIF_ID(Closed));
}




static AlifMethodDef _incrementalNewlineDecoderMethods_[] = {
	//_IO_INCREMENTALNEWLINEDECODER_DECODE_METHODDEF
	//_IO_INCREMENTALNEWLINEDECODER_GETSTATE_METHODDEF
	//_IO_INCREMENTALNEWLINEDECODER_SETSTATE_METHODDEF
	//_IO_INCREMENTALNEWLINEDECODER_RESET_METHODDEF
	{nullptr}
};

static AlifTypeSlot _nlDecoderSlots_[] = { // 3314
	//{ALIF_TP_DEALLOC, incrementalnewlinedecoder_dealloc},
	{ALIF_TP_METHODS, _incrementalNewlineDecoderMethods_},
	//{ALIF_TP_GETSET, _incrementalNewlineDecoderGetset_},
	//{ALIF_TP_CLEAR, incrementalnewlinedecoder_clear},
	{ALIF_TP_INIT, (void*)_ioIncrementalNewlineDecoder___init__},
	{0, nullptr},
};


AlifTypeSpec _nlDecoderSpec_ = { // 3325
	.name = "تبادل.فك_ترميز_السطر_المتزايد",
	.basicsize = sizeof(NLDecoderObject),
	.flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC |
			  ALIF_TPFLAGS_IMMUTABLETYPE),
	.slots = _nlDecoderSlots_,
};

static AlifMethodDef _textIOWrapperMethods_[] = { // 3334
	_IO_TEXTIOWRAPPER_WRITE_METHODDEF
	_IO_TEXTIOWRAPPER_READ_METHODDEF
	_IO_TEXTIOWRAPPER_READLINE_METHODDEF
	_IO_TEXTIOWRAPPER_CLOSE_METHODDEF

	{nullptr, nullptr}
};


static AlifMemberDef _textIOWrapperMembers_[] = { // 3358
	{"encoding", ALIF_T_OBJECT, offsetof(TextIO, encoding), ALIF_READONLY},
	{"buffer", ALIF_T_OBJECT, offsetof(TextIO, buffer), ALIF_READONLY},
	{"line_buffering", ALIF_T_BOOL, offsetof(TextIO, lineBuffering), ALIF_READONLY},
	{"__weakListOffset__", ALIF_T_ALIFSIZET, offsetof(TextIO, weakRefList), ALIF_READONLY},
	{"__dictOffset__", ALIF_T_ALIFSIZET, offsetof(TextIO, dict), ALIF_READONLY},
	{nullptr}
};






AlifTypeSlot _textIOWrapperSlots_[] = { // 3380
	{ALIF_TP_TRAVERSE, (void*)textIOWrapper_traverse},
	{ALIF_TP_METHODS, _textIOWrapperMethods_},
	{ALIF_TP_MEMBERS, _textIOWrapperMembers_},
	{ALIF_TP_INIT, (void*)_ioTextIOWrapper___init__},
	{0, nullptr},
};


AlifTypeSpec _textIOWrapperSpec_ = { // 3394
	.name = "تبادل.غلاف_النص",
	.basicsize = sizeof(TextIO),
	.flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC |
			  ALIF_TPFLAGS_IMMUTABLETYPE),
	.slots = _textIOWrapperSlots_,
};
