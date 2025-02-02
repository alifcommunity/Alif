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

static AlifIntT tokState_underflowFile(TokenState* _tokState) { // 282
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
		//tokState->underflow = &tokState_underflowInteractive;
	}
	else {
		tokState->underflow = &tokState_underflowFile;
	}
	if (_enc != nullptr) {
		/* Must copy encoding declaration since it
		   gets copied into the parse tree. */
		tokState->encoding = alifTokenizer_newString(_enc, strlen(_enc), tokState);
		if (!tokState->encoding) {
			alifTokenizer_free(tokState);
			return nullptr;
		}
		tokState->decodingState = DecodingState_::State_Normal;
	}
	return tokState;
}

