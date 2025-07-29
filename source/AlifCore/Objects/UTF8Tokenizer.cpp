#include "alif.h"
#include "ErrorCode.h"

#include "Helpers.h"
#include "AlifTokenState.h"



static AlifIntT tok_underflowString(class TokenState* _tokState) {
	char* end = strchr(_tokState->inp, '\n');
	if (end != nullptr) {
		end++;
	}
	else {
		end = strchr(_tokState->inp, '\0');
		if (end == _tokState->inp) {
			_tokState->done = E_EOF;
			return 0;
		}
	}
	if (_tokState->start == nullptr) {
		_tokState->buf = _tokState->cur;
	}
	_tokState->lineStart = _tokState->cur;
	LINE_ADVANCE();
	_tokState->inp = end;
	return 1;
}



TokenState* _alifTokenizer_fromUTF8(const char* _str,
	AlifIntT _execInput, AlifIntT _preserveCRLF) {
	TokenState* _tokState = alifTokenizer_tokNew();
	char* translated{};
	if (_tokState == nullptr)
		return nullptr;
	_tokState->input = translated = _alifTokenizer_translateNewlines(_str, _execInput, _preserveCRLF, _tokState);
	if (translated == nullptr) {
		alifTokenizer_free(_tokState);
		return nullptr;
	}
	_tokState->decodingState = DecodingState_::State_Normal;
	_tokState->enc = nullptr;
	_tokState->string = translated;
	_tokState->encoding = alifTokenizer_newString("utf-8", 5, _tokState);
	if (!_tokState->encoding) {
		alifTokenizer_free(_tokState);
		return nullptr;
	}

	_tokState->buf = _tokState->cur = _tokState->inp = translated;
	_tokState->end = translated;
	_tokState->underflow = &tok_underflowString;
	return _tokState;
}
