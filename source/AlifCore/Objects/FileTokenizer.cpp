#include "alif.h"

//#include "AlifCore_Call.h"
#include "AlifCore_FileUtils.h"
#include "AlifCore_Memory.h"
//#include "ErrorCode.h"

//#include "Helpers.h"
#include "AlifTokenState.h"
//#include "AlifLexer.h"
//#include "AlifBuffer.h"



//static int tokInfo_readlineRaw(TokenInfo* _tokInfo) {
//	do {
//		int nChars = (int)(_tokInfo->end - _tokInfo->inp);
//		AlifSizeT lineSize{};
//
//		wchar_t* line = alifUniversal_newLineFGetsWithSize(_tokInfo->inp, nChars, _tokInfo->fp, &lineSize);
//		if (line == nullptr) {
//			return 1;
//		}
//		_tokInfo->inp += lineSize;
//		if (_tokInfo->inp == _tokInfo->buf) {
//			return 0;
//		}
//
//	} while (_tokInfo->inp[-1] != L'\n');
//
//	return 1;
//}
//
//static int tokState_underflowFile(TokenInfo* _tokInfo) {
//	if (_tokInfo->start == nullptr and !INSIDE_FSTRING(_tokInfo)) {
//		_tokInfo->cur = _tokInfo->inp = _tokInfo->buf;
//	}
//
//	//if (_tokInfo.decodingState != nullptr) {
//	//	if (!tokInfo_readlineRecode(_tokInfo)) {
//	//		return 0;
//	//	}
//	//}
//	//else {
//	if (!tokInfo_readlineRaw(_tokInfo)) {
//		return 0;
//	}
//	//}
//
//	if (_tokInfo->inp == _tokInfo->cur) {
//		_tokInfo->done = E_WEOF;
//		return 0;
//	}
//	_tokInfo->implicitNewline = 0;
//	if (_tokInfo->inp[-1] != L'\n') {
//		*_tokInfo->inp++ = L'\n';
//		*_tokInfo->inp = L'\0';
//		_tokInfo->implicitNewline = 1;
//	}
//
//	LINE_ADVANCE();
//
//	return _tokInfo->done == E_OK;
//}

TokenState* alifTokenizerInfo_fromFile(FILE* _fp, const char* _enc,
	const char* _ps1, const char* _ps2) { // 349
	TokenState* tokState = alifTokenizer_newTokenState();
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
		tokState->underflow = &tokState_underflowInteractive;
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
		tokState->decodingState = STATE_NORMAL;
	}
	return tokState;
}

