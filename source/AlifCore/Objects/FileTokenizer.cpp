#include "alif.h"

//#include "AlifCore_Call.h"
#include "AlifCore_Import.h"
#include "AlifCore_FileUtils.h"
#include "AlifCore_Memory.h"
#include "ErrorCode.h"

#include "Helpers.h"
#include "AlifTokenState.h"
#include "AlifLexer.h"
#include "Buffer.h"


static AlifIntT tok_readlineRaw(TokenState* _tokState) { // 55
	do {
		if (!_alifLexerTok_reserveBuf(_tokState, BUFSIZ)) {
			return 0;
		}
		AlifIntT nChars = (AlifIntT)(_tokState->end - _tokState->inp);
		AlifUSizeT lineSize{};

		char* line = alifUniversal_newLineFGetsWithSize(_tokState->inp, nChars, _tokState->fp, nullptr, &lineSize);
		if (line == nullptr) {
			return 1;
		}
		//if (_tokState->interactive and
		//	tok_concatenateInteractiveNewLine(_tokState, line) == -1) {
		//	return 0;
		//}
		_tokState->inp += lineSize;
		if (_tokState->inp == _tokState->buf) {
			return 0;
		}

	} while (_tokState->inp[-1] != '\n');

	return 1;
}



static AlifIntT tok_underflowInteractive(TokenState* _tok) { // 190
	if (_tok->interactiveUnderflow == InteractiveUnderflow_::IUnderflow_Stop) {
		_tok->done = E_INTERACT_STOP;
		return 1;
	}

	/*
	.
	.
	.
	*/

	return 1;
}




static AlifIntT tok_underflowFile(TokenState* _tokState) { // 282
	if (_tokState->start == nullptr and !INSIDE_FSTRING(_tokState)) {
		_tokState->cur = _tokState->inp = _tokState->buf;
	}

	if (_tokState->decodingState == DecodingState_::State_Init) {
		/* We have not yet determined the encoding.
		   If an encoding is found, use the file-pointer
		   reader functions from now on. */
		//if (!alifTokenizer_checkBOM(fp_getc, fp_ungetc, fp_setreadl, _tokState)) {
		//	alifTokenizer_errorRet(_tokState);
		//	return 0;
		//}
	}
	if (_tokState->decodingReadline != nullptr) {
		//if (!tok_readlineRecode(_tokState)) {
		//	return 0;
		//}
	}
	else {
		if (!tok_readlineRaw(_tokState)) {
			return 0;
		}
	}

	if (_tokState->inp == _tokState->cur) {
		_tokState->done = E_EOF;
		return 0;
	}
	_tokState->implicitNewline = 0;
	if (_tokState->inp[-1] != '\n') {
		*_tokState->inp++ = '\n';
		*_tokState->inp = '\0';
		_tokState->implicitNewline = 1;
	}

	//if (_tokState->tokModeStackIndex and !alifLexer_updateFStringExpr(_tokState, 0)) {
	//	return 0;
	//}

	LINE_ADVANCE();
	if (_tokState->decodingState != DecodingState_::State_Normal) {
		if (_tokState->lineNo > 2) {
			_tokState->decodingState = DecodingState_::State_Normal;
		}
		//else if (!alifTokenizer_checkCodingSpec(_tokState->cur,
		//	strlen(_tokState->cur), _tokState, fp_setreadl))
		//{
		//	return 0;
		//}
	}
	/* The default encoding is UTF-8, so make sure we don't have any
	   non-UTF-8 sequences in it. */
	//if (!_tokState->encoding and !alifTokenizer_ensureUTF8(_tokState->cur, _tokState)) {
	//	alifTokenizer_errorRet(_tokState);
	//	return 0;
	//}
	return _tokState->done == E_OK;
}

TokenState* alifTokenizerInfo_fromFile(FILE* _fp, const char* _enc,
	const char* _ps1, const char* _ps2) { // 349
	TokenState* tokState = alifTokenizer_tokNew();
	if (tokState == nullptr) return nullptr;

	tokState->buf = (char*)alifMem_dataAlloc(BUFSIZ);
	if (tokState->buf == nullptr) {
		alifTokenizer_free(tokState);
		return nullptr;
	}
	tokState->cur = tokState->inp = tokState->buf;
	tokState->end = tokState->buf + BUFSIZ;
	tokState->fp = _fp;
	tokState->prompt = _ps1;
	tokState->nextPrompt = _ps2;
	if (_ps1 or _ps2) {
		tokState->underflow = &tok_underflowInteractive;
	}
	else {
		tokState->underflow = &tok_underflowFile;
	}
	if (_enc != nullptr) {
		/* Must copy encoding declaration since it
		   gets copied into the parse tree. */
		tokState->encoding = _alifTokenizer_newString(_enc, strlen(_enc), tokState);
		if (!tokState->encoding) {
			alifTokenizer_free(tokState);
			return nullptr;
		}
		tokState->decodingState = DecodingState_::State_Normal;
	}
	return tokState;
}







// 383
#if defined(__wasi__) or (defined(__EMSCRIPTEN__) and (__EMSCRIPTEN_major__ >= 3))
typedef union {
	void* cookie{};
	int fd{};
} borrowed{};

static ssize_t
borrow_read(void* cookie, char* buf, size_t size)
{
	borrowed b = { .cookie = cookie };
	return read(b.fd, (void*)buf, size);
}

static FILE*
fdopen_borrow(int fd) {
	// supports only reading. seek fails. close and write are no-ops.
	cookie_io_functions_t io_cb = { borrow_read, NULL, NULL, NULL };
	borrowed b = { .fd = fd };
	return fopencookie(b.cookie, "r", io_cb);
}
#else
static FILE*
fdopen_borrow(int fd) {
	fd = _alif_dup(fd);
	if (fd < 0) {
		return nullptr;
	}
	return fdopen(fd, "r");
}
#endif


char* _alifTokenizer_findEncodingFilename(AlifIntT _fd, AlifObject* _filename) { // 425
	TokenState* tok{};
	FILE* fp{};
	char* encoding = nullptr;

	fp = fdopen_borrow(_fd);
	if (fp == nullptr) {
		return nullptr;
	}
	tok = alifTokenizerInfo_fromFile(fp, nullptr, nullptr, nullptr);
	if (tok == nullptr) {
		fclose(fp);
		return nullptr;
	}
	if (_filename != nullptr) {
		tok->fn = ALIF_NEWREF(_filename);
	}
	else {
		tok->fn = alifUStr_fromString("<string>");
		if (tok->fn == nullptr) {
			fclose(fp);
			alifTokenizer_free(tok);
			return encoding;
		}
	}
	AlifToken token{};
	//tok->reportWarnings = 0;
	while (tok->lineNo < 2 and tok->done == E_OK) {
		//alifToken_init(&token); ?!
		alifTokenizer_get(tok, &token);
		_alifToken_free(&token);
	}
	fclose(fp);
	if (tok->encoding) {
		encoding = (char*)alifMem_dataAlloc(strlen(tok->encoding) + 1);
		if (encoding) {
			strcpy(encoding, tok->encoding);
		}
	}
	alifTokenizer_free(tok);
	return encoding;
}
