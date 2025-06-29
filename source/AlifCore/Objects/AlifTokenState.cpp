#include "alif.h"

#include "AlifCore_State.h"
#include "AlifCore_Token.h"
#include "ErrorCode.h"


#include "AlifTokenState.h"
#include "AlifCore_Memory.h"


#define TABSIZE 8 // 9

TokenState* alifTokenizer_tokNew() { // 12
	TokenState* tokState = (TokenState*)alifMem_dataAlloc(sizeof(TokenState));
	if (tokState == nullptr) return nullptr;

	tokState->buf = tokState->cur = tokState->inp = nullptr;
	tokState->interactive = 0;
	tokState->interactiveSrcStart = nullptr;
	tokState->interactiveSrcEnd = nullptr;
	tokState->start = nullptr;
	tokState->end = nullptr;
	tokState->done = E_OK;
	tokState->fp = nullptr;
	tokState->input = nullptr;
	tokState->tabSize = TABSIZE;
	tokState->indent = 0;
	tokState->indStack[0] = 0;
	tokState->atBeginOfLine = 1;
	tokState->pendInd = 0;
	tokState->prompt = tokState->nextPrompt = nullptr;
	tokState->lineNo = 0;
	tokState->startingColOffset = -1;
	tokState->colOffset = -1;
	tokState->level = 0;
	tokState->alterIndStack[0] = 0;
	tokState->decodingState = DecodingState_::State_Init;
	tokState->decodingErred = 0;
	tokState->enc = nullptr;
	tokState->encoding = nullptr;
	tokState->contLine = 0;
	tokState->fn = nullptr;
	tokState->decodingReadline = nullptr;
	tokState->decodingBuffer = nullptr;
	tokState->readline = nullptr;
	tokState->comment = 0;
	tokState->interactiveUnderflow = InteractiveUnderflow_::IUnderflow_Normal;
	tokState->underflow = nullptr;
	tokState->string = nullptr;
	tokState->tokExtraTokens = 0;
	tokState->commentNewline = 0;
	tokState->implicitNewline = 0;
	tokState->tokModeStackIndex = 0;
	tokState->tokModeStack[0] = { .type = TokenizerModeType_::Token_RegularMode,
		.fStringQuote = '\0', .fStringQuoteSize = 0, .fStringDebug = 0};

	return tokState;
}

void alifTokenizer_free(TokenState* _tokState) { // 83
	if (_tokState->encoding != nullptr) {
		alifMem_dataFree(_tokState->encoding);
	}
	ALIF_XDECREF(_tokState->decodingReadline);
	ALIF_XDECREF(_tokState->decodingBuffer);
	ALIF_XDECREF(_tokState->readline);
	ALIF_XDECREF(_tokState->fn);
	if ((_tokState->readline != nullptr or _tokState->fp != nullptr)
		and _tokState->buf != nullptr) {
		alifMem_dataFree(_tokState->buf);
	}
	if (_tokState->input) {
		alifMem_dataFree(_tokState->input);
	}
	if (_tokState->interactiveSrcStart != nullptr) {
		alifMem_dataFree(_tokState->interactiveSrcStart);
	}
	//free_fstringExpressions(_tokState);
	alifMem_dataFree(_tokState);
}


void _alifToken_free(AlifToken* _token) { // 107
	ALIF_XDECREF(_token->data);
}


AlifIntT alifLexer_setupToken(TokenState* _tokState,
	AlifToken* _token, AlifIntT _type, const char* _pStart,
	const char* _pEnd) { // 129
	_token->level = _tokState->level;
	if (ISSTRINGLIT(_type)) {
		_token->lineNo = _tokState->firstLineNo;
	}
	else {
		_token->lineNo = _tokState->lineNo;
	}
	_token->endLineNo = _tokState->lineNo;
	_token->colOffset = _token->endColOffset = -1;
	_token->start = _pStart;
	_token->end = _pEnd;

	if (_pStart != nullptr and _pEnd != nullptr) {
		_token->colOffset = _tokState->startingColOffset;
		_token->endColOffset = _tokState->colOffset;
	}

	return _type;
}
