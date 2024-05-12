#include "Alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_FileUtils.h"
#include "ErrorCode.h"

#include "Helpers.h"
#include "AlifTokenState.h"
#include "AlifLexer.h"
#include "AlifBuffer.h"

#include "AlifCore_Memory.h" // temp


static int tokInfo_readlineRaw(TokenInfo* _tokInfo) {
	do {
		int nChars = (int)(_tokInfo->end - _tokInfo->inp);
		AlifSizeT lineSize{};

		wchar_t* line = alifUniversal_newLineFGetsWithSize(_tokInfo->inp, nChars, _tokInfo->fp, &lineSize);
		if (line == nullptr) {
			return 1;
		}
		_tokInfo->inp += lineSize;
		if (_tokInfo->inp == _tokInfo->buf) {
			return 0;
		}

	} while (_tokInfo->inp[-1] != L'\n');

	return 1;
}

static int tokInfo_underflowFile(TokenInfo* _tokInfo) {
	if (_tokInfo->start == nullptr and !INSIDE_FSTRING(_tokInfo)) {
		_tokInfo->cur = _tokInfo->inp = _tokInfo->buf;
	}

	//if (_tokInfo.decodingState != nullptr) {
	//	if (!tokInfo_readlineRecode(_tokInfo)) {
	//		return 0;
	//	}
	//}
	//else {
	if (!tokInfo_readlineRaw(_tokInfo)) {
		return 0;
	}
	//}

	if (_tokInfo->inp == _tokInfo->cur) {
		_tokInfo->done = E_WEOF;
		return 0;
	}
	_tokInfo->implicitNewline = 0;
	if (_tokInfo->inp[-1] != L'\n') {
		*_tokInfo->inp++ = L'\n';
		*_tokInfo->inp = L'\0';
		_tokInfo->implicitNewline = 1;
	}

	LINE_ADVANCE();

	return _tokInfo->done == E_OK;
}

TokenInfo* alifTokenizerInfo_fromFile(FILE* _fp) {
	TokenInfo* tokInfo = alifTokenizer_newTokenInfo();

	tokInfo->buf = (wchar_t*)alifMem_dataAlloc(BUFSIZ * sizeof(wchar_t));

	tokInfo->cur = tokInfo->inp = tokInfo->buf;
	tokInfo->end = tokInfo->buf + BUFSIZ;
	tokInfo->fp = _fp;
	tokInfo->prompt = nullptr;
	tokInfo->nextPrompt = nullptr;

	//if (pos1 or pos2) {
	//	tokInfo->underflow = &tokInfo_underflowInteractive;
	//}
	//else {
	tokInfo->underflow = &tokInfo_underflowFile;
	//}

	return tokInfo;
}

