#include "alif.h"

//#include "AlifCore_AlifState.h"
#include "AlifCore_AlifToken.h"
#include "ErrorCode.h"


#include "AlifTokenState.h"
#include "AlifCore_Memory.h"


#define TABSIZE 8

TokenInfo* alifTokenizer_newTokenInfo() {
	TokenInfo* tokInfo = (TokenInfo*)alifMem_dataAlloc(sizeof(TokenInfo));

	tokInfo->buf = tokInfo->cur = tokInfo->inp = nullptr;
	tokInfo->start = nullptr;
	tokInfo->end = nullptr;
	tokInfo->done = E_OK;
	tokInfo->fp = nullptr;
	tokInfo->input = nullptr;
	tokInfo->tabSize = TABSIZE;
	tokInfo->indent = 0;
	tokInfo->indStack[0] = 0;
	tokInfo->atBeginOfLine = 1;
	tokInfo->pendInd = 0;
	tokInfo->prompt = tokInfo->nextPrompt = nullptr;
	tokInfo->lineNo = 0;
	tokInfo->startingColOffset = -1;
	tokInfo->colOffset = -1;
	tokInfo->level = 0;
	tokInfo->alterIndStack[0] = 0;
	tokInfo->countLine = 0;
	tokInfo->fn = nullptr;
	tokInfo->comment = 0;
	tokInfo->underflow = nullptr;
	tokInfo->string = nullptr;
	tokInfo->tokExtraTokens = 0;
	tokInfo->commentNewline = 0;
	tokInfo->implicitNewline = 0;
	tokInfo->tokModeStackIndex = 0;
	tokInfo->tokModeStack[0] = { Token_RegularMode, 0, 0, '\0', 0, 0 };

	return tokInfo;
}

int alifLexer_setupToken(TokenInfo* _tokInfo, AlifToken* _token, int _type, const wchar_t* _pStart, const wchar_t* _pEnd) {
	_token->level = _tokInfo->level;
	if (ISSTRINGLITT(_type)) {
		_token->lineNo = _tokInfo->firstLineNo;
	}
	else {
		_token->lineNo = _tokInfo->lineNo;
	}
	_token->endLineNo = _tokInfo->lineNo;
	_token->colOffset = _token->endColOffset = -1;
	_token->start = _pStart;
	_token->end = _pEnd;

	if (_pStart != nullptr and _pEnd != nullptr) {
		_token->colOffset = _tokInfo->startingColOffset;
		_token->endColOffset = _tokInfo->colOffset;
	}

	return _type;
}
